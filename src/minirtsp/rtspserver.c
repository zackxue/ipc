#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "sock.h"
#include "rtspdef.h"
#include "vlog.h"
#include "rtsplib.h"
#include "rtspserver.h"


static SOCK_t server_fd=-1;
static int m_toggle=true;
static int rtsp_user_num=0;

static void signal_handle(int sign_no) 
{
    m_toggle=false;
    VLOG(VLOG_DEBUG,"rtsp server kill!");
    SOCK_close(server_fd);
	RTSPS_stop();
	MSLEEP(3000);
}

/*
static int caculate_average(int a[],int n)
{
	int j, k;  
	int flag,tmp;  
	int sum=0;
	k = n;  
	flag = true;  
	while (flag)  
	{	
		flag = false;  
		for (j = 1; j < k; j++){
			if (a[j - 1] > a[j])  
			{ 
				tmp=a[j-1];
				a[j-1]=a[j];
				a[j]=tmp;
				flag = true;	
			}
		}
		k--;  
	}
	for(j=0;j<n;j++) printf("%d,",a[j]);
	printf("\r\n");
	for(j=0;j< n/2;j++)
		sum+=a[j];
	return sum/(n/2);
}
*/

void* RTSPS_proc(void *para) 
{
	int ret;
	Rtsp_t *r=NULL;
	RtspStream_t *stream=NULL;
	char media_name[16];
	uint32_t base_ts=0xffffffff,last_ts=0xffffffff;
	int out_success= false;
	int bStreamInit=false,avc_flag=false;
	MillisecondTimer_t t_tmp,base_t_v,base_t_a;
	MilliSecond_t m_lasttime;
	int i_count_v=0,real_fps_v=25,send_fps_v=0;
	int i_count_a=0,real_fps_a=25,send_fps_a=0;
	int i_fix_to = 0;
	int max_sock=0;
	//int i_count=0,static_fps[10];
	ThreadArgs_t *args=(ThreadArgs_t *)para;
	int sock=args->RParam;
	int *trigger=(int *)args->LParam;
	fd_set read_set;
	//fd_set write_set;
	struct timeval timeout;
	
#if defined(_WIN32) || defined(_WIN64)
#else	
	pthread_detach(pthread_self());
#endif
	r=RTSP_SERVER_init(sock,RTSP_ENABLE_AUDIO);
	if(r == NULL) return NULL;
	stream = (RtspStream_t *)&r->s;

	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;
	rtsp_user_num++;
	VLOG(VLOG_CRIT,"rtsp server proc enter,usernum:%d",rtsp_user_num);
	while(r->toggle && (*trigger)){
		//timeout.tv_sec = 0;
		//timeout.tv_usec = 10;
		FD_ZERO(&read_set);
		FD_SET(r->sock,&read_set);
		max_sock = r->sock;
		if(r->rtcp_video){
			FD_SET(r->rtcp_video->sock,&read_set);
			max_sock = MAXAB(max_sock,r->rtcp_video->sock);
		}		
		if(r->rtcp_audio){
			FD_SET(r->rtcp_audio->sock,&read_set);
			max_sock = MAXAB(max_sock,r->rtcp_audio->sock);
		}
		ret=select(max_sock+1,&read_set,NULL,NULL,&timeout);
		if(ret < 0){
			VLOG(VLOG_ERROR,"select failed");
			break;
		}else if(ret == 0){
			//timeout
		}else{
			if(FD_ISSET(r->sock,&read_set)){
				if(RTSP_read_message(r)==RTSP_RET_FAIL) break;
				if(RTSP_parse_message(r,NULL,RTCP_HANDLE) == RTSP_RET_FAIL) break;
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

		}// end select
		if(r->state == RTSP_STATE_PLAYING){
			out_success = false;
			if(bStreamInit == false){
				RTSP_find_stream(r->stream,media_name);
				if(RTSP_STREAM_init(stream,media_name)<0)
					break;
				bStreamInit = true;
				MilliTimerStart(base_t_v);
				MilliTimerStart(base_t_a);
			}
			if(RTSP_STREAM_next(stream)==RTSP_RET_OK){
				//printf("1  !!!!!!!!!!!!!!!!!!!!!!! streamtype:%x %d\n",r->stream_type,stream->type);
				VLOG(VLOG_DEBUG,"get stream type:%d success,size:%d",stream->type,stream->size);
				if(send_fps_v == 0) send_fps_v = stream->fps;
				if(avc_flag == false){
					if(stream->type == RTSP_STREAM_TYPE_VIDEO && stream->isKeyFrame == true){
						avc_flag=true;
						base_ts=stream->timestamp;
					}else
						continue;
				}
				if((stream->type == RTSP_STREAM_TYPE_VIDEO) && (r->stream_type & RTSP_STREAM_VIDEO)){
					if(RTP_send_packet(r->rtp_video,stream->data,stream->size,
						SDP_MEDIA_H264_FREQ/1000*(stream->timestamp-base_ts),
						RTP_DEFAULT_VIDEO_TYPE)==RTSP_RET_FAIL){
						break;
					}else{
						i_count_v++;
						MilliTimerStop(base_t_v,t_tmp,m_lasttime);
						if(m_lasttime > 1000){
							MilliTimerStart(base_t_v);
							real_fps_v = i_count_v;
							i_count_v = 0;
							if(real_fps_v < send_fps_v){
								i_fix_to += (send_fps_v - real_fps_v)*1500;
							}else{
								i_fix_to -= (real_fps_v - send_fps_v)*1500;
							}
							if(i_fix_to < 0) i_fix_to = 0;
							if(i_fix_to > stream->inspeed) i_fix_to = stream->inspeed;
							// caculate the send fps
							/*
							if(i_count < 10){
								static_fps[i_count]=real_fps_v;
								i_fix_to = stream->inspeed;
							}else if(i_count==10){
								i_count;
								send_fps_v=caculate_average(static_fps,10);
								i_fix_to = 5000;
							}
							i_count++;*/
							VLOG(VLOG_DEBUG,"**** real video fps:%d(%d,%d) fix_to:%d ****",
								real_fps_v,stream->fps,send_fps_v,i_fix_to);
						}
						out_success = true;
						if((stream->timestamp - last_ts) > 80){
							//printf("timestamp dev:%d\n",stream->timestamp -  last_ts);
						}
						last_ts = stream->timestamp;
					}
				}
				else if((stream->type == RTSP_STREAM_TYPE_AUDIO) && (r->stream_type & RTSP_STREAM_AUDIO)){
					if(RTP_send_packet(r->rtp_audio,stream->data,stream->size,
						SDP_MEDIA_G711_FREQ/1000*(stream->timestamp-base_ts),
						RTP_TYPE_PCMA)==RTSP_RET_FAIL){
						break;
					}else{
						i_count_a++;
						MilliTimerStop(base_t_a,t_tmp,m_lasttime);
						if(m_lasttime > 1000){
							MilliTimerStart(base_t_a);
							real_fps_a = i_count_a;
							i_count_a = 0;
							VLOG(VLOG_INFO,"**** real audio fps:%d ****",real_fps_a);
						}
					}
				}else{
					//VLOG(VLOG_DEBUG,"unsupport stream type:%d",stream->type);
					//break;
				}
				avc_flag =true;
			}
			if(out_success){
				if((stream->inspeed > i_fix_to) && (stream->isKeyFrame == false)){
					MSLEEP((stream->inspeed - i_fix_to)/1000);
				}
				out_success = false;
			}
		}
		// 	process rtcp
#if RTSP_ENABLE_RTCP == TRUE
		if(r->rtcp_audio){
			RTCP_process(r->rtcp_audio);
		}
		if(r->rtcp_video){
			RTCP_process(r->rtcp_video);
		}
#endif
	}// end of while

	*trigger = false;
	RTSP_destroy(r);
	
	rtsp_user_num--;
	VLOG(VLOG_CRIT,"rtsp server proc exit,usernum:%d",rtsp_user_num);
	return NULL;
}



int RTSPS_start() 
{
    SOCK_t client_fd;
	SOCKADDR_t client;
	SOCKADDR_IN_t sin;
	SOCKLEN_t size;	
	ThreadId_t tid;
	ThreadArgs_t args;
	args.data = NULL;
	args.LParam = (void *)&m_toggle;

    signal(SIGINT,signal_handle);            

    server_fd=SOCK_tcp_listen(RTSP_DEFAULT_PORT);
    do {
        size=sizeof(struct sockaddr);
        if ((client_fd = accept(server_fd,(SOCKADDR_t *)&client,&size)) == -1) {
            VLOG(VLOG_ERROR,"accept failed, errno=%d!!!",errno);
            return -1;
        } else {
            sin=*((SOCKADDR_IN_t *)&client);
            VLOG(VLOG_INFO,"accept connect from:%s sockfd:%d",inet_ntoa(sin.sin_addr),client_fd);
			m_toggle = true;
			args.RParam = client_fd;
            THREAD_create(tid,RTSPS_proc,((void *)&args));
        }
    } while (m_toggle);

    return 0;
}

void RTSPS_stop() 
{
	m_toggle = false;
}

int RTSPS_test(int argc,char *argv[])
{
	RTSP_add_stream("test.h264","test.h264");
	RTSP_add_stream("720p.h264","test.h264.720p");
	RTSP_add_stream("360p.h264","test.h264.360p");
	RTSPS_start();
	return 0;
}


