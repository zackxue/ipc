
#include "adobe/rtmplog.h"
#include "adobe/rtmplib.h"
#include "adobe/handshake.h"
#include "rtmpd.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"


SPOOK_SESSION_PROBE_t RTMPD_probe(const void *msg, ssize_t msg_sz){
    unsigned char *ptr = (unsigned char *)msg;
    if(msg_sz >= 1){
		if(0x03 == ptr[0]){
			return SPOOK_PROBE_MATCH;
		}
		else{
			return SPOOK_PROBE_MISMATCH;
		}
	}
	else{
		return SPOOK_PROBE_UNDETERMIN;
	}
}

SPOOK_SESSION_LOOP_t RTMPD_loop(bool *trigger, int sock, time_t *read_pts){
	int media_speed = 0;
    RtmpPacket_t packet;
	//RtmpStream_t *s=NULL;
    Rtmp_t r;
    int ret,out_success,avc_flag=RTMP_FALSE;
	uint32_t base_ts=-1;
	fd_set read_fds,write_fds;
	struct timeval poll_wait;

    pthread_detach(pthread_self());
    RTMP_init(&r, sock);
	
    if(SHandShake(&r)== RTMP_TRUE) {
		lpMEDIABUF_USER media_user = NULL;
		
        while (r.m_bLinkOk && (*trigger)) {
			bool media_send_loop = true;
			out_success = RTMP_FALSE;
			
			FD_ZERO(&read_fds);
			FD_ZERO(&write_fds);
			FD_SET(r.sock, &read_fds);
			FD_SET(r.sock,&write_fds);
			poll_wait.tv_sec = 0;
			poll_wait.tv_usec = 50000;
			ret = select(r.sock+ 1, &read_fds, NULL, NULL, &poll_wait);
			if(ret < 0) break;
			if( ret > 0){
				if(FD_ISSET(r.sock, &read_fds)){
		            if (RTMP_read_packet(&r,&packet)==RTMP_RET_OK) {
		                if(RTMP_parse_packet(&r,&packet)==RTMP_RET_FAIL){
							*trigger =false;
							break;
		                }
		            }else{
		            	break;
		            }
				}
			}else{
				//APP_TRACE("select timeout & not data ready");
			}//end of  select

			while(r.m_bPlayStart == RTMP_TRUE && *trigger && media_send_loop){
				if(NULL == media_user){
					int const media_id = MEDIABUF_lookup_byname(strndupa(r.m_playPath.av_val, r.m_playPath.av_len));
					if(media_id >= 0){
						media_user = MEDIABUF_attach(media_id);
						if(media_user == NULL){
							printf("media attach falied!\n");
							*trigger = false;
							break;
						}
						media_speed = MEDIABUF_in_speed(media_id);
						MEDIABUF_sync(media_user);
					}else{
						// do something when the stream is not found!
						*trigger = false;
						break;
					}
				}
				
				if(0 == MEDIABUF_out_lock(media_user)){
					const lpSDK_ENC_BUF_ATTR attr = NULL;
					size_t out_size = 0;
					
					if(0 == MEDIABUF_out(media_user, &attr, NULL, &out_size)){
						const void* const raw_ptr = (void*)(attr + 1);
						ssize_t const raw_size = attr->data_sz;

						if(RTMP_FALSE == avc_flag){
							if(attr->h264.keyframe){
								avc_flag = RTMP_TRUE;
								base_ts = attr->timestamp_us / 1000;
								if(RTMP_RET_OK != RTMP_send_avc(&r, raw_ptr, raw_size)){
									// break this loop
									*trigger = false;
								}
								if(RTMP_RET_OK != RTMP_send_frame(&r, raw_ptr, raw_size, attr->h264.keyframe, attr->timestamp_us / 1000 - base_ts)){
									// break this loop
									*trigger = false;
								}else{
									out_success = true;
								}
							}
						}else{
							if(kSDK_ENC_BUF_DATA_H264 == attr->type){
								if(RTMP_RET_OK != RTMP_send_frame(&r, raw_ptr, raw_size, attr->h264.keyframe, attr->timestamp_us / 1000 - base_ts)){
									// break this loop
									*trigger = false;
								}else{
									out_success = true;
								}
							}else if(kSDK_ENC_BUF_DATA_G711A == attr->type){
								if(RTMP_RET_OK != RTMP_send_g711(&r, raw_ptr, raw_size, attr->timestamp_us / 1000 - base_ts, 0x72)){
								//	// break this loop
									*trigger = false;
								}
							}else {
								printf("RTMP got type=%d impossible\n", attr->type);
								assert(0);
							}
						}
					}else{
						//printf("An error out of mediabuf\n");
						// break this loop;
						media_send_loop = false;
					}
					MEDIABUF_out_unlock(media_user);
				}else{
					printf("An error lock of mediabuf\n");
					// break this loop;
					*trigger = false;
				}

				if(out_success){					
					usleep(media_speed-15*1000);
					out_success = false;
				}
			}
			
        }// end of while

		if(media_user){
			MEDIABUF_detach(media_user);
			media_user = NULL;
		}
    }// end of handshake
	RTMP_destroy(&r);
    //pthread_exit(NULL);
	*trigger = 0;
	return SPOOK_LOOP_SUCCESS;
}


