#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sock.h"
#include "rtspdef.h"
#include "vlog.h"
#include "rtsplib.h"
#include "rtpbuf.h"

#if defined(_WIN32) || defined(_WIN64) || defined(NOCROSS) || defined(IPCAM_SOLUTION)
enum
{
	AVENC_AUDIO = 0,
	AVENC_IDR,
	AVENC_PSLICE,
	AVENC_FRAME_TYPE_CNT,
};
#endif

#if defined(_WIN32) || defined(_WIN64)
#else
#ifndef NOCROSS
#if (PRODUCT_CLASS != ipcam)
#include "../../common/avenc_types.h"
#include "../decode.h"
#endif
#include "minirtsp.h"
#else
#endif
#endif



static Rtsp_t *g_rtsp=NULL;
static int m_toggle=true;

static void signal_handle(int sign_no) 
{
    m_toggle=false;
    VLOG(VLOG_DEBUG,"rtsp player kill!");
	//exit(0);
}

static void* file_new(const char *name)
{
	FILE *f=fopen(name,"wb+");
	if(f==NULL){
		printf("open file failed\n");
		return NULL;
	}
	VLOG(VLOG_CRIT,"create file:%s success",name);
	return (void *)f;
}

static int file_write(FILE *f,char *buf,int size)
{
	if(fwrite(buf,size,1,f)!=1){
		printf("write file failed\n");
		return -1;
	}
	return 0;
}

static inline int rtspc_decode(void *decoder,int type,void *payload,int size,int chn,uint64_t ts)
{
	static int flag=true;
	static MillisecondTimer_t t_base,t_tmp;
	static MilliSecond_t t_lasttime = 0,t_lasttime_tmp;
	if(flag == true){
		MilliTimerStart(t_base);
		flag =false;
	}
	
#if defined(_WIN32) || defined(_WIN64)
	CH264Decode *dec264= NULL;
	CAudioPlayer *decg711= NULL;
	if(type == AVENC_AUDIO){
		decg711 = (CAudioPlayer *)decoder;
		decg711->Play((char *)payload,size);
	}else{
		dec264 = (CH264Decode *)decoder;
		dec264->InputDate(payload,size);
	}
#else
#if !defined(NOCROSS) && !defined(IPCAM_SOLUTION)
	// add it to mediabuf
	stSDK_ENC_BUF_ATTR attr;
	attr.magic = kSDK_ENC_BUF_DATA_MAGIC;
	attr.timestamp_us = ts;
	attr.time_us = (unsigned long long)((ts/1000000) << 32) + (unsigned long long)(ts%1000000);
	attr.data_sz = size;
	if(type == AVENC_AUDIO){
		attr.type = kSDK_ENC_BUF_DATA_G711A;
		attr.g711a.sample_rate= 8000;
		attr.g711a.sample_width = 8;
		attr.g711a.packet = 1;
		attr.g711a.compression_ratio = 2.0;
	}else{
		attr.type = kSDK_ENC_BUF_DATA_H264;
		attr.h264.keyframe = (type == AVENC_IDR) ? TRUE : FALSE;
		attr.h264.ref_counter = 0;
		attr.h264.fps = 25;
		attr.h264.width = 0;
		attr.h264.height = 0;
	}
	if(0 == MEDIABUF_in_request(chn*2, attr.data_sz + sizeof(attr), attr.h264.keyframe)){
		MEDIABUF_in_append(chn*2, &attr, sizeof(attr));
		MEDIABUF_in_append(chn*2, payload, attr.data_sz);
		MEDIABUF_in_commit(chn*2);
		VLOG(VLOG_DEBUG,"commit a frame to mediabuf,type:%d size:%d",type,size);
	}
	else
	{
		VLOG(VLOG_ERROR,"commit a frame to mediabuf failed,size:%d",size);
	}
	// decode this frame
	//if(type != AVENC_AUDIO)
	
		DECODE_Request(chn,type,payload,size,ts,FALSE);
	t_lasttime_tmp = t_lasttime;
	MilliTimerStop(t_base,t_tmp,t_lasttime);
	VLOG(VLOG_DEBUG,"type:%d lasttime:%d\tdev:%d\n",type,t_lasttime,t_lasttime-t_lasttime_tmp);
#else
	FILE *f=(FILE *)decoder;
	Test264Frame_t header;
	header.flag = 0x7d22628c;
	header.isidr = (type == AVENC_IDR) ? true : false ;
	header.size = size;
	file_write(decoder,&header,sizeof(Test264Frame_t));
	file_write(decoder,payload,size);
	//printf("nodecoder\n");
#endif
#endif
	return 0;
}

void *RTSPC_NETWORK_proc(void *param)
{
	int ret;
	Rtsp_t *r=NULL;
	int max_sock=0;
	fd_set read_set;
	//fd_set write_set;
	struct timeval timeout;
	MillisecondTimer_t t_base,t_tmp;
	MilliSecond_t t_lasttime;
	int *trigger=NULL;
	int timeout_cnt=0;

#if defined(_WIN32) || defined(_WIN64)
	CRtspPlayerDlg *viewwnd=(CRtspPlayerDlg *)param;
	if(viewwnd==NULL /*|| !viewwnd->IsKindOf(RUNTIME_CLASS(CViewWnd)) */){
		return NULL;
	}
	r=(Rtsp_t *)viewwnd->m_rtsp;
	trigger = &viewwnd->m_toggle;
#else
	ThreadArgs_t *args=(ThreadArgs_t *)param;
	r=(Rtsp_t *)args->data;
	trigger = (int *)args->LParam;
#endif

	VLOG(VLOG_CRIT,"network proc start....");

	while((r->toggle == RTSPC_RUNNING) && (*trigger)){
		if(r->state != RTSP_STATE_PLAYING){
			//MSLEEP(5);
			continue;
		}
		timeout.tv_sec=0;
		timeout.tv_usec=5000;
		FD_ZERO(&read_set);
		max_sock=0;
		if(r->rtp_video){
			FD_SET(r->rtp_video->sock,&read_set);
			if(r->rtp_video->sock > max_sock) max_sock=r->rtp_video->sock;
		}
		if(r->rtp_audio){
			FD_SET(r->rtp_audio->sock,&read_set);
			if(r->rtp_audio->sock > max_sock) max_sock=r->rtp_audio->sock;
		}
		if(r->rtcp_video){
			FD_SET(r->rtcp_video->sock,&read_set);
			if(r->rtcp_video->sock > max_sock) max_sock=r->rtcp_video->sock;
		}
		if(r->rtcp_audio){
			FD_SET(r->rtcp_audio->sock,&read_set);
			if(r->rtcp_audio->sock > max_sock) max_sock=r->rtcp_audio->sock;
		}
		ret = select(max_sock +1,&read_set,NULL,NULL,&timeout);
		if(ret < 0){
			VLOG(VLOG_ERROR,"select failed");
			break;
		}else if(ret == 0){
			timeout_cnt ++;
			if(timeout_cnt > 500){
				VLOG(VLOG_ERROR,"rtspc: select timeout");
				break;
			}
			//VLOG(VLOG_DEBUG,"select timeout");
			// timeout
			//MSLEEP(5);
		}else{
			timeout_cnt = 0;
			//
			if(r->b_interleavedMode == true){
				ret=RTSP_read_message(r);
				if(ret == RTSP_RET_FAIL) break;
				ret=RTSP_parse_message(r,RTP_handle_packet,RTCP_HANDLE);
				if(ret == RTSP_RET_FAIL) break;
			}else{
				if(r->rtp_video){
					if(FD_ISSET(r->rtp_video->sock,&read_set)){
						if((ret=RTP_handle_packet(r->rtp_video,NULL,0))==RTSP_RET_FAIL) break;
					}
				}
				if(r->rtp_audio){
					if(FD_ISSET(r->rtp_audio->sock,&read_set)){
						if((ret=RTP_handle_packet(r->rtp_audio,NULL,0))==RTSP_RET_FAIL) break;
					}
				}
#if RTSP_ENABLE_RTCP == TRUE
				if(r->rtcp_audio){
					if(FD_ISSET(r->rtcp_audio->sock,&read_set)){
						RTCP_handle_packet(r->rtcp_audio,NULL,0);
					}
				}
				if(r->rtcp_video){
					if(FD_ISSET(r->rtcp_video->sock,&read_set)){
						RTCP_handle_packet(r->rtcp_video,NULL,0);
					}
				}
#endif
			}
		}
			
#if RTSP_ENABLE_RTCP == TRUE
		if(r->rtcp_audio){
			RTCP_process(r->rtcp_audio);
		}
		if(r->rtcp_video){
			RTCP_process(r->rtcp_video);
		}
#endif
	}
	
	VLOG(VLOG_CRIT,"network proc exit");
	
	r->toggle &= ~RTSP_NET_PROC;
	if(r->toggle == 0){
		RTSP_destroy(r);
		free(param);
	}

	*trigger = false;
	return NULL;
}

// decode with control of buffer play speed
void *RTSPC_DECODE_proc(void *param)
{
	int ret;
	int dec_chn=0;
	char buf[1024*256];
	MillisecondTimer_t t_tmp,t_base,t_base_v,t_base_a;
	MilliSecond_t t_now,t_dec_v,t_dec_a,t_dec;
	unsigned int t_sleep=5;// ms
	float t_fix_factor=1.0;
	uint32_t base_ts_v=0xffffffff,base_ts_a=0xffffffff,timestamp,last_ts_v=0xffffffff,last_ts_a=0xffffffff;
	int ts_dev_v[3],i_count_v=0,average_v=0;	
	int ts_dev_a[3],i_count_a=0,average_a=0;
	int frame_size;
	Rtsp_t *r=NULL;
	CircleBuffer_t *buffer=NULL,*buffer_a=NULL,*buffer_v=NULL;
	int b_audio = false,b_video=false,b_PlayAVideo = false;
	Attribute_t fmtp_attr;
	int sps_pps=false;
	int *trigger=NULL;
	void *decoder=NULL;

#if defined(_WIN32) || defined(_WIN64)
	CH264Decode *dec264=NULL;
	CAudioPlayer *decg711=NULL;
	CRtspPlayerDlg *viewwnd=(CRtspPlayerDlg *)param;
	if(viewwnd==NULL /*|| !viewwnd->IsKindOf(RUNTIME_CLASS(CViewWnd)) */){
		return NULL;
	}
	r=(Rtsp_t *)viewwnd->m_rtsp;
	dec264=(CH264Decode *)&viewwnd->m_hi264dec;
	decg711 = (CAudioPlayer *)&viewwnd->m_g711dec;
	trigger = &viewwnd->m_toggle;
	dec_chn = 0;
#else
#ifdef NOCROSS
	FILE *f1=(FILE *)file_new("test.264");
	FILE *f2=(FILE *)file_new("test.711a");
	void *dec264=(void *)f1;
	void *decg711=(void *)f2;
#else
	void *dec264=NULL;
	void *decg711=NULL;
#endif
	ThreadArgs_t *args=(ThreadArgs_t *)param;
	r=(Rtsp_t *)args->data;
	trigger = (int *)args->LParam;
	dec_chn = args->RParam;
#endif
	
	VLOG(VLOG_CRIT,"decode proc start, @ chn=%d",dec_chn);
	MilliTimerSet(t_base,0,0);
	MilliTimerSet(t_base_a,0,0);
	MilliTimerSet(t_base_v,0,0);

	while((r->toggle == RTSPC_RUNNING) && (*trigger)){
		if(r->state != RTSP_STATE_PLAYING){
			MSLEEP(1);
			continue;
		}
		// decode sps first , if sdp contain sps pps or ....
		if(sps_pps == false && r->sdp){
			if(SDP_get_media_attr(r->sdp,SDP_MEDIA_TYPE_VIDEO,SDP_ATTR_FMTP,(void *)&fmtp_attr)==RTSP_RET_OK){
				if(fmtp_attr.fmtp.sps_size){
					rtspc_decode(dec264,AVENC_IDR,fmtp_attr.fmtp.sps,fmtp_attr.fmtp.sps_size,dec_chn,0);
				}
				if(fmtp_attr.fmtp.pps_size){
					rtspc_decode(dec264,AVENC_IDR,fmtp_attr.fmtp.pps,fmtp_attr.fmtp.pps_size,dec_chn,0);
				}
			}
			sps_pps = true;
		}
		// audio and video sync
		if(r->rtp_audio && r->rtp_video){
			buffer_a=(CircleBuffer_t *)r->rtp_audio->packet.buffer;
			buffer_v=(CircleBuffer_t *)r->rtp_video->packet.buffer;
			if(buffer_a==NULL || buffer_v==NULL) continue;
			if((buffer_a->m_bRealloc == true) && (buffer_v->m_bRealloc == true)){
				b_audio = true;
				b_video = true;
				buffer = buffer_a;
			}else if((buffer_a->m_bRealloc == true) && (buffer_v->m_bRealloc == false)){
				b_audio = true;
				b_video = false;
				buffer = buffer_a;
			}else if((buffer_a->m_bRealloc == false) && (buffer_v->m_bRealloc == true)){
				b_audio = false;
				b_video = true;		
				buffer = buffer_v;
			}else{
				b_audio = false;
				b_video = false;
				buffer = NULL;
				MSLEEP(1);
				continue;
			}
		}
		else if(r->rtp_video){
			buffer=(CircleBuffer_t *)r->rtp_video->packet.buffer;
			b_audio = false;
			b_video = true;
			
		}else{
			b_video = false;
			b_audio = true;
			buffer=(CircleBuffer_t *)r->rtp_audio->packet.buffer;
		}	
		if(buffer == NULL) continue;
		if(buffer->IsAvailable(buffer) == TRUE){
			if(buffer->m_fUsedRate < CBUFFER_USED_RATE_LEVEL1){
				t_fix_factor= ((CBUFFER_USED_RATE_LEVEL1-buffer->m_fUsedRate)*20+1.8);// t_fix_factor must > 1
				if(buffer->m_fInSpeed < 1) t_fix_factor = t_fix_factor/buffer->m_fInSpeed;
				VLOG(VLOG_WARNING,"cbuffer used rate is too small:%f%% frames:%d fix:%f",buffer->m_fUsedRate,buffer->m_iFrameCnt,t_fix_factor);
			}else if(buffer->m_fUsedRate < CBUFFER_USED_RATE_LEVEL2){
				t_fix_factor= 1;
				//t_fix_factor= 1/buffer->m_fInSpeed;
				//VLOG(VLOG_WARNING,"cbuffer used rate: \t%f%%",buffer->m_fUsedRate);
			}else if(buffer->m_fUsedRate < CBUFFER_USED_RATE_LEVEL3){
				t_fix_factor= (1-buffer->m_fUsedRate)*2;// t_fix_factor must < 1
				if(buffer->m_fInSpeed > 1) t_fix_factor = t_fix_factor/buffer->m_fInSpeed;
				VLOG(VLOG_WARNING,"cbuffer used rate is too big:%f%% frames:%d fix:%f",buffer->m_fUsedRate,buffer->m_iFrameCnt,t_fix_factor);
			}else{				
				t_fix_factor= 0;
				VLOG(VLOG_WARNING,"cbuffer used rate is too too big:%f%% frames:%d fix:%f",buffer->m_fUsedRate,buffer->m_iFrameCnt,t_fix_factor);
				//ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				//ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				//ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				//ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
			}
		}
		else{
			MSLEEP(1);
			continue;
		}
		// audio decode first
		if(b_audio){
			decoder=(void *)decg711;
			buffer=(CircleBuffer_t *)r->rtp_audio->packet.buffer;
			if(buffer->IsAvailable(buffer) == TRUE){
				ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				if(ret==RTSP_RET_FAIL){
					*trigger=false;
				}else{
					if(base_ts_a == 0xffffffff) base_ts_a = timestamp;
					if(MilliTimerIsClear(t_base_a) == true) MilliTimerStart(t_base_a);
					if(last_ts_a != 0xffffffff && (timestamp != last_ts_a)){
						ts_dev_a[i_count_a++]=timestamp-last_ts_a;
						if(i_count_a == 3) {
							i_count_a=0;
							average_a=(ts_dev_a[0]+ts_dev_a[1]+ts_dev_a[2]-MAXABC(ts_dev_a[0],ts_dev_a[1],ts_dev_a[2]))/2;
							average_a = G711_TS_TO_MILLISECOND(average_a);
						}
					}
					t_dec_a = G711_TS_TO_MILLISECOND(timestamp-base_ts_a);
					t_dec = t_dec_a;
					MilliTimerStop(t_base_a,t_tmp,t_now);
					if(t_now < t_dec){// it's not time to decode, just sleep a a while
						if((t_dec-t_now)>(TIMESTAMP_FIX_FACTOR*average_a) && (average_a !=0)){// increrrect timestamp,fix it
							printf("A1 dev too big:%u-> ",t_dec-t_now);
							MilliTimerMinus(t_base_a,(t_dec-t_now-average_a));
							MilliTimerStop(t_base_a,t_tmp,t_now);
							printf("%u\n",t_dec-t_now);
							
						}
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_a,(unsigned int)((t_dec-t_now)*(t_fix_factor-1+0.1 )));
						}else if(t_fix_factor < 1.0){// play in fast speed
							MilliTimerMinus(t_base_a,(unsigned int)((t_dec-t_now)*(1-t_fix_factor+0.1)));
						}else{// play in normal speed
						}
						t_sleep=(t_dec-t_now)*t_fix_factor;
						VLOG(VLOG_WARNING,"A1 sleep:%u average:%d dev:%d",t_sleep,average_a,t_dec-t_now);
					}else{
						if(((t_now-t_dec) > (TIMESTAMP_FIX_FACTOR*average_a)) && (average_a != 0 )){
							printf("A2 dev too big:%u -> ",t_now-t_dec);
							MilliTimerAdd(t_base_a,(t_now-t_dec-average_a));
							MilliTimerStop(t_base_a,t_tmp,t_now);
							printf("%u\n",t_now-t_dec);
						}
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_a,(unsigned int)((t_now-t_dec)+average_a*t_fix_factor));
							t_sleep=average_a*t_fix_factor;
						}else if(t_fix_factor < 1.0){// play in fast speed
							//MilliTimerAdd(t_base,(unsigned int)(t_now-t_dec));
							//t_sleep=(t_now-t_dec)*t_fix_factor;
							t_sleep = 0;
						}else{// play in normal speed
							//MilliTimerAdd(t_base,average);
							t_sleep = 0;
						}
						VLOG(VLOG_WARNING,"A2 sleep:%d average:%d dev:%d",t_sleep,average_a,t_now-t_dec);
					}
					if(t_sleep > 0) MSLEEP(t_sleep);
					VLOG(VLOG_DEBUG,"||| get a audio frame success, ts:%u size:%d",timestamp,frame_size);
					rtspc_decode(decoder,AVENC_AUDIO,buf,frame_size,dec_chn,G711_TS_TO_MILLISECOND(timestamp-base_ts_a)*1000);
					last_ts_a = timestamp;
				}
			}
		}
		// video decode
		if(b_video){
			decoder = (void *)dec264;
			buffer=(CircleBuffer_t *)r->rtp_video->packet.buffer;
			if(buffer->IsAvailable(buffer) == TRUE){
VIDEO_FPLAY_NEXT_FRAME:
				ret=buffer->NextFrameTS(buffer,&timestamp);
				if(ret == RTSP_RET_FAIL){
					//*trigger = false;
					continue;
				}
				if(MilliTimerIsClear(t_base_v) == true) MilliTimerStart(t_base_v);
				if(base_ts_v == 0xffffffff) base_ts_v = timestamp;
				if(last_ts_v != 0xffffffff && (timestamp != last_ts_v)){
					ts_dev_v[i_count_v++]=timestamp-last_ts_v;
					if(i_count_v == 3) {
						i_count_v=0;
						average_v=(ts_dev_v[0]+ts_dev_v[1]+ts_dev_v[2]-MAXABC(ts_dev_v[0],ts_dev_v[1],ts_dev_v[2]))/2;
						average_v = H264_TS_TO_MILLISECOND(average_v);
					}
				}
				
				t_dec_v = H264_TS_TO_MILLISECOND(timestamp-base_ts_v);
				t_dec = t_dec_v;
				MilliTimerStop(t_base_v,t_tmp,t_now);
				if(b_audio){
					if(t_now < t_dec){// it's not time to decode, just sleep a a while
						if((t_dec-t_now)>(TIMESTAMP_FIX_FACTOR*average_v) && (average_v !=0)){// increrrect timestamp,fix it
							printf("V11 dev too big:%u-> ",t_dec-t_now);
							MilliTimerMinus(t_base_v,(t_dec-t_now-average_v));
							MilliTimerStop(t_base_v,t_tmp,t_now);
							printf("%u\n",t_dec-t_now);
							b_PlayAVideo = true;
							
						}else{
							continue;
						}
						/*
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_v,(unsigned int)((t_dec-t_now)*(t_fix_factor-1+0.1 )));
						}else if(t_fix_factor < 1.0){// play in fast speed
							MilliTimerMinus(t_base_v,(unsigned int)((t_dec-t_now)*(1-t_fix_factor+0.1)));
						}else{// play in normal speed
						}
						t_sleep=(t_dec-t_now)*t_fix_factor;
						VLOG(VLOG_WARNING,"V11 sleep:%u average:%d dev:%d",t_sleep,average_v,t_dec-t_now);
						*/
					}else{
						b_PlayAVideo = true;
						if(((t_now-t_dec) > (TIMESTAMP_FIX_FACTOR*average_v)) && (average_v != 0 )){
							printf("V21 dev too big:%u -> ",t_now-t_dec);
							MilliTimerAdd(t_base_v,(t_now-t_dec-average_v));
							MilliTimerStop(t_base_v,t_tmp,t_now);
							printf("%u\n",t_now-t_dec);
						}
						/*
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_v,(unsigned int)((t_now-t_dec)+average_v*t_fix_factor));
							t_sleep=average_v*t_fix_factor;
						}else if(t_fix_factor < 1.0){// play in fast speed
							//MilliTimerAdd(t_base,(unsigned int)(t_now-t_dec));
							//t_sleep=(t_now-t_dec)*t_fix_factor;
							t_sleep = 0;
						}else{// play in normal speed
							//MilliTimerAdd(t_base,average);
							t_sleep = 0;
						}
						VLOG(VLOG_WARNING,"V21 sleep:%d average:%d dev:%d",t_sleep,average_v,t_now-t_dec);
						*/
					}
				}else{
					b_PlayAVideo = true;
					if(t_now < t_dec){// it's not time to decode, just sleep a a while
						if((t_dec-t_now)>(TIMESTAMP_FIX_FACTOR*average_v) && (average_v !=0)){// increrrect timestamp,fix it
							printf("V1 dev too big:%u-> ",t_dec-t_now);
							MilliTimerMinus(t_base_v,(t_dec-t_now-average_v));
							MilliTimerStop(t_base_v,t_tmp,t_now);
							printf("%u\n",t_dec-t_now);
							
						}
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_v,(unsigned int)((t_dec-t_now)*(t_fix_factor-1+0.1 )));
						}else if(t_fix_factor < 1.0){// play in fast speed
							MilliTimerMinus(t_base_v,(unsigned int)((t_dec-t_now)*(1-t_fix_factor+0.1)));
						}else{// play in normal speed
						}
						t_sleep=(t_dec-t_now)*t_fix_factor;
						VLOG(VLOG_WARNING,"V1 sleep:%u average:%d dev:%d",t_sleep,average_v,t_dec-t_now);
					}else{
						if(((t_now-t_dec) > (TIMESTAMP_FIX_FACTOR*average_v)) && (average_v != 0 )){
							printf("V2 dev too big:%u -> ",t_now-t_dec);
							MilliTimerAdd(t_base_v,(t_now-t_dec-average_v));
							MilliTimerStop(t_base_v,t_tmp,t_now);
							printf("%u\n",t_now-t_dec);
						}
						if(t_fix_factor > 1.0){// play in slow speed
							MilliTimerAdd(t_base_v,(unsigned int)((t_now-t_dec)+average_v*t_fix_factor));
							t_sleep=average_v*t_fix_factor;
						}else if(t_fix_factor < 1.0){// play in fast speed
							//MilliTimerAdd(t_base,(unsigned int)(t_now-t_dec));
							//t_sleep=(t_now-t_dec)*t_fix_factor;
							t_sleep = 5;
						}else{// play in normal speed
							//MilliTimerAdd(t_base,average);
							t_sleep = 0;
						}
						VLOG(VLOG_WARNING,"V2 sleep:%d average:%d dev:%d",t_sleep,average_v,t_now-t_dec);
					}
					if(t_sleep > 0) MSLEEP(t_sleep);
				}
				if(b_PlayAVideo == true){
					ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
					if(ret==RTSP_RET_FAIL){
						*trigger=false;
					}else{
						VLOG(VLOG_INFO,"||| get a video frame success, ts:%u size:%d",timestamp,frame_size);
						if(((buf[4] & 0x1f) == H264_SPS || (buf[4] & 0x1f) == H264_PPS ) && sps_pps == false){
							printf("got sps pps \n");
							sps_pps = true;
						}
						//if(sps_pps == true)
						{
							if((buf[4] & 0x1f) == H264_IDR || (buf[4] & 0x1f) == H264_SPS || (buf[4] & 0x1f) == H264_PPS){
								rtspc_decode(decoder,AVENC_IDR,buf,frame_size,dec_chn,t_dec_v*1000);
							}else{
								rtspc_decode(decoder,AVENC_PSLICE,buf,frame_size,dec_chn,t_dec_v*1000);
							}
						}
						
						b_PlayAVideo = false;
						last_ts_v = timestamp;

						//if(buffer->m_fUsedRate >= CBUFFER_USED_RATE_LEVEL3 && b_audio == true){
						//	printf("video retry\n");
						//	goto VIDEO_FPLAY_NEXT_FRAME;
						//}
					}
				}
			}
		}

	}
	
	VLOG(VLOG_CRIT,"decode proc exit");

	r->toggle &= ~RTSP_DEC_PROC;
	if(r->toggle == 0){
		RTSP_destroy(r);
#if defined(_WIN32) || defined(_WIN64)
#else
		free(param);
#endif
	}
	
	*trigger = false;
	return 0;
}


// decode without control of buffer play speed
void *RTSPC_DECODE_proc2(void *param)
{
	int ret;
	int dec_chn=0;
	char buf[1024*256];
	MilliSecond_t t_dec;
	uint32_t base_ts=0xffffffff,timestamp;
	int frame_size;
	Rtsp_t *r=NULL;
	CircleBuffer_t *buffer=NULL;
	Attribute_t fmtp_attr;
	int sps_pps=false;
	int *trigger=NULL;
	void *decoder=NULL;

#if defined(_WIN32) || defined(_WIN64)
	CH264Decode *dec264=NULL;
	CAudioPlayer *decg711=NULL;
	CRtspPlayerDlg *viewwnd=(CRtspPlayerDlg *)param;
	if(viewwnd==NULL /*|| !viewwnd->IsKindOf(RUNTIME_CLASS(CViewWnd)) */){
		return NULL;
	}
	r=(Rtsp_t *)viewwnd->m_rtsp;
	dec264=(CH264Decode *)&viewwnd->m_hi264dec;
	decg711 = (CAudioPlayer *)&viewwnd->m_g711dec;
	trigger = &viewwnd->m_toggle;
	dec_chn = 0;
#else
#ifdef NOCROSS
	FILE *f1=(FILE *)file_new("test.264");
	FILE *f2=(FILE *)file_new("test.711a");
	void *dec264=(void *)f1;
	void *decg711=(void *)f2;
#else
	void *dec264=NULL;
	void *decg711=NULL;
#endif
	ThreadArgs_t *args=(ThreadArgs_t *)param;
	r=(Rtsp_t *)args->data;
	trigger = (int *)args->LParam;
	dec_chn = args->RParam;
#endif
	
	VLOG(VLOG_CRIT,"decode proc start, @ chn=%d",dec_chn);
	while((r->toggle == RTSPC_RUNNING) && (*trigger)){
		if(r->state != RTSP_STATE_PLAYING){
			MSLEEP(1);
			continue;
		}
		// decode sps first , if sdp contain sps pps or ....
		if(sps_pps == false && r->sdp){
			if(SDP_get_media_attr(r->sdp,SDP_MEDIA_TYPE_VIDEO,SDP_ATTR_FMTP,(void *)&fmtp_attr)==RTSP_RET_OK){
				printf("sps or pps\n");
				if(fmtp_attr.fmtp.sps_size){
					rtspc_decode(dec264,AVENC_IDR,fmtp_attr.fmtp.sps,fmtp_attr.fmtp.sps_size,dec_chn,0);
				}
				if(fmtp_attr.fmtp.pps_size){
					rtspc_decode(dec264,AVENC_IDR,fmtp_attr.fmtp.pps,fmtp_attr.fmtp.pps_size,dec_chn,0);
				}
			}
			sps_pps = true;
		}
		// video decode
		if(r->rtp_video){
			decoder = (void *)dec264;
			buffer=(CircleBuffer_t *)r->rtp_video->packet.buffer;
			if(buffer == NULL) continue;
			if(buffer->IsAvailable(buffer) == TRUE){
				ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				if(ret==RTSP_RET_FAIL){
					*trigger=false;
				}else{
					VLOG(VLOG_INFO,"||| get a frame success, ts:%u size:%d",timestamp,frame_size);
					if(((buf[4] & 0x1f) == H264_SPS || (buf[4] & 0x1f) == H264_PPS ) && sps_pps == false){
						printf("got sps pps \n");
						sps_pps = true;
					}
					if(base_ts == 0xffffffff){
						base_ts = timestamp;
					}
					t_dec = H264_TS_TO_MILLISECOND(timestamp-base_ts);
					//if(sps_pps == true)
					{
						if((buf[4] & 0x1f) == H264_IDR || (buf[4] & 0x1f) == H264_SPS || (buf[4] & 0x1f) == H264_PPS){
							rtspc_decode(decoder,AVENC_IDR,buf,frame_size,dec_chn,t_dec*1000);
						}else{
							rtspc_decode(decoder,AVENC_PSLICE,buf,frame_size,dec_chn,t_dec*1000);
						}
					}
				}
			}
		}
		// audio decode in here...
		if(r->rtp_audio){
			decoder=(void *)decg711;
			buffer=(CircleBuffer_t *)r->rtp_audio->packet.buffer;
			if(buffer == NULL) continue;
			if(buffer->IsAvailable(buffer) == TRUE){
				ret=buffer->NextFrame(buffer,buf,&frame_size,&timestamp);
				if(ret==RTSP_RET_FAIL){
					*trigger=false;
				}else{
					VLOG(VLOG_DEBUG,"||| get a audio frame success, ts:%u size:%d",timestamp,frame_size);
					rtspc_decode(decoder,AVENC_AUDIO,buf,frame_size,dec_chn,G711_TS_TO_MILLISECOND(timestamp-base_ts)*1000);
				}
			}
		}
	}
	
	VLOG(VLOG_CRIT,"decode proc exit");

	r->toggle &= ~RTSP_DEC_PROC;
	if(r->toggle == 0){
		RTSP_destroy(r);
#if defined(_WIN32) || defined(_WIN64)
#else
		free(param);
#endif
	}
	
	*trigger = false;
	return 0;
}



int RTSPC_daemon(void **rtsp,char *url,char *user,char *pwd,int bInterleaved,int buffer_time,int chn,int *trigger)
{
	int client_fd;
	ThreadId_t net_thread;
	ThreadId_t dec_thread;
	char sockname[20];
	ThreadArgs_t *args=(ThreadArgs_t *)malloc(sizeof(ThreadArgs_t));
	if(args == NULL){
		VLOG(VLOG_ERROR,"malloc for threadargs failed");
		return -1;
	}

	*rtsp=(void *)RTSP_connect_server(url,user,pwd,bInterleaved,buffer_time);
	if(*rtsp==NULL)
	{
		*trigger = false;
		VLOG(VLOG_ERROR,"rtsp connect failed");
		return -1;
	}else{
		VLOG(VLOG_CRIT,"rtsp connect success");
	}

	args->data = *rtsp;
	args->LParam = (void *)trigger;
	args->RParam = chn;
	THREAD_create(net_thread,RTSPC_NETWORK_proc,(void *)args);	
	SOCK_getsockname(((Rtsp_t *)(*rtsp))->sock,sockname);
	if(SOCK_isreservedip(sockname)!=true || SOCK_isreservedip(((Rtsp_t *)(*rtsp))->rtp_video->peername)!=true){
		VLOG(VLOG_CRIT,"rtsp enable player buffer control");
		THREAD_create(dec_thread,RTSPC_DECODE_proc,(void *)args);
	}else{
		VLOG(VLOG_CRIT,"rtsp disable player buffer control");
		THREAD_create(dec_thread,RTSPC_DECODE_proc2,(void *)args);
	}

	VLOG(VLOG_CRIT,"rtsp player running...");

	return 0;
}

int RTSPC_test(int argc,char *argv[])
{
	//const char *url="rtsp://192.168.2.10/user=admin&password=tlJwpbo6&channel=1&stream=0.sdp?real_stream";
	const char *url="rtsp://192.168.2.86:554/ch0_1.264";
    signal(SIGINT,signal_handle);

	RTSPC_daemon(&g_rtsp,url,NULL,NULL,RTSP_RTP_OVER_UDP,RTSP_PLAYER_BUFFER_TIME,0,&m_toggle);
	while(m_toggle);
	return 0;
}


