/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		gb28181.c
 * Describle: how to do gb28181
 * History: 
 * Last modified:	2013-07-10 14:10
 =============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include "siplib.h"
#include "sipparse.h"
#include "http_auth/authentication.h"
#include "sdplib.h"
#include "gb28181debug.h"
//
#include "gb28181.h"
#include "gb28181/mpeg.h"
#include "rtp/rtplib.h"
#include "wav.h"
#include "../minirtsp/netstream.h"
#include "siprtpsession.h"
#include "sysconf.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"

#define DEFAULT_STREAM "720p.264"


static int gb28181_sigquit = 0; // ctrl + c ,logout sipserver,then quit prog,
static int gb28181_sigtstp = 0;//ctrl + z,send alarm to sipserver

sSipSession session;

Gb28181Conf_t def_gb_conf = 
{
	.SipServerId = "34020000002000000001",
	.SipServerIp = "192.168.2.82",
	.SipServerPort = 5060,
	.LoginDomain = "3402000000",
	.UserId = "34020000001320000001",
	.UserPwd = "12345678",
	.AliveTime = 500,
	.HeartBeatTime = 30,
	.AlarmId[0] = "34020000001340000010",
	.VideoId[0] = "34020000001320000001",
};

#define ITOA(x) ({static char buf[32];sprintf(buf,"%d",(x));buf;})

static unsigned short g_SipRtpPort=6000;


void* gb28181_heartbeat_loop(void *session)
{
	pSipSession s = (pSipSession)session;
	GB_ASSERT(s,"heartbeat ,null argument\n");
	time_t cur_time = time(NULL);	
	for(;;)
	{
		usleep(1);
		if(time(NULL)- cur_time > s->heartbeat_time){//send a heartbeat			
			cur_time = time(NULL);
			s->pack_msg(s,MSG_HEARTBEAT,NULL);
			sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
					0,(struct sockaddr*)&s->server,sizeof(s->server));
		}
		if(0 == s->online)break;
	}
	return NULL;
}

void* gb28181_msg_loop(void *session)
{
	pSipSession s = (pSipSession)session;
	GB_ASSERT(s,"sip_msg_loop null argument\n");
	int re_cnt = 0;
	char msg_buf[2048];
	char *ptzcmd = NULL;
	s->run_msg_loop  = 1;//that's to say msg_loop is run now;
	char *cmdtype = s->get_tagvalue(s->xml_str,"CmdType");

	if( 0 ==  strcmp("DeviceInfo",cmdtype))
	{
		TRACE("Get a dev info request\n");
		s->pack_msg(s,MSG_DEV_INFO,NULL);		
	}
	else if(0 == strcmp("DeviceStatus",cmdtype))
	{
		TRACE("Get a devicestatus request\n");
		s->pack_msg(s,MSG_DEV_STATUS,NULL);
	}
	else if(0 == strcmp("Alarm",cmdtype))
	{	
		s->pack_msg(s,MSG_200,NULL);			
		sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
				0,(struct sockaddr*)&s->server,sizeof(s->server));
		goto exit_msg_loop;
	}	
	else if (0 ==  strcmp("DeviceControl",cmdtype))
	{
		if(0 == s->get_subcmd(s,s->xml_str,"RecordCmd","Record"))
		{
			s->pack_msg(s,MSG_DEV_CTRL,NULL);
			s->dev_ctrl(SIP_DEV_CTRL_RECORD_START,NULL);			
		}
		else if(0 == s->get_subcmd(s,s->xml_str,"RecordCmd","StopRecord"))
		{
			s->pack_msg(s,MSG_DEV_CTRL,NULL);
			s->dev_ctrl(SIP_DEV_CTRL_RECORD_STOP,NULL);	
		}
		else if(0 == s->get_subcmd(s,s->xml_str,"GuardCmd","SetGuard"))
		{
			s->pack_msg(s,MSG_DEV_CTRL,NULL);	
			s->dev_ctrl(SIP_DEV_CTRL_GUARD_SET,NULL);
		}
		else if(0 == s->get_subcmd(s,s->xml_str,"GuardCmd","ResetGuard"))
		{
			s->pack_msg(s,MSG_DEV_CTRL,NULL);	
			s->dev_ctrl(SIP_DEV_CTRL_GUARD_RESET,NULL);
		}
		else if(0 == s->get_subcmd(s,s->xml_str,"AlarmCmd","ResetAlarm"))
		{
			s->pack_msg(s,MSG_DEV_CTRL,NULL);
			char *method = s->get_tagvalue(s->xml_str,"AlarmMethod");
			s->dev_ctrl(SIP_DEV_CTRL_ALARM_RESET,method);
		}
		else if(NULL != (ptzcmd = s->get_tagvalue(s->xml_str,"PTZCmd")))
		{
			s->ptz_cmd_parse(s,ptzcmd);	
			s->pack_msg(s,MSG_200,NULL);
			sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
					0,(struct sockaddr*)&s->server,sizeof(s->server));
			goto exit_msg_loop;
		}
		else if(0 == s->get_subcmd(s,s->xml_str,"TeleBoot","Boot"))
		{
			//login out the sipserver first
			s->pack_msg(s,MSG_200,NULL);
			sprintf(msg_buf,"%s",s->sendbuf);
			s->dev_ctrl(SIP_DEV_CTRL_TELEBOOT,NULL);
			GB_NOTIFY("Loginout ing...\n ");
			s->auth(s,0);	//FIXME this func need 2 do in dev_ctrl later			
			sendto(s->sock,msg_buf,strlen(msg_buf),\
					0,(struct sockaddr*)&s->server,sizeof(s->server));
			sleep(1);
			GB_NOTIFY("Login ing...\n ");
			s->auth(s,1);
			goto exit_msg_loop;
		}

	}
	else if( 0 ==  strcmp("Catalog",cmdtype))
	{
		TRACE("Recv a Catalog msg\n");
		s->pack_msg(s,MSG_CATALOG,NULL);		
	}
	sprintf(msg_buf,"%s",s->sendbuf);
	TRACE("\033[31mSIP PACK MSG:%s\n\033[0m",s->sendbuf);
	time_t cur_time = time(NULL) - 4;	
	for(;;)
	{	
		if(time(NULL)- cur_time > 3){//response a msg 
			sendto(s->sock,msg_buf,strlen(msg_buf),\
					0,(struct sockaddr*)&s->server,sizeof(s->server));					
			cur_time = time(NULL);
			if(++re_cnt >= 3)break;		
		}
		usleep(1);
		if(0 == s->run_msg_loop)break;
	}
exit_msg_loop:
	GB_ERROR("Exit the msg loop\n");
	if(NULL != s->xml_str)
	{
		free(s->xml_str);
		s->xml_str = NULL;
	}
	return NULL;
}


void *gb28181_invite_loop(void *session)
{
	SipRtpSession_t *srtp=(SipRtpSession_t *)session;
	pSipSession s = (pSipSession)srtp->data.data;

	char buf_200[1024];
	s->pack_msg(s,MSG_INVITE_200,(void *)srtp);
	sprintf(buf_200,"%s",s->sendbuf);
	int re_cnt = 0;	
	for(;;)
	{		
		re_cnt++;
		sendto(s->sock,buf_200,strlen(buf_200),\
				0,(struct sockaddr*)&s->server,sizeof(s->server));
		sleep(1);		
		if(srtp->data.flag == TRUE) break;
		else if(re_cnt == 10){
			SIPRTP_session_del(srtp->data.id);
			break;
		}
	}
	return NULL;	
}

typedef struct _test_264_2
{
	uint32_t flag;
	uint32_t size;
	uint32_t isidr;
}Test264Frame2_t;

static FILE* file_new(const char *name)
{
	FILE *f=fopen(name,"wb+");
	if(f==NULL){
		printf("open file failed\n");
		return NULL;
	}
	printf("create file:%s success",name);
	return f;
}

static int file_write(FILE *f,char *buf,int size)
{
	if(fwrite(buf,size,1,f)!=1){
		printf("write file failed\n");
		return -1;
	}
	return 0;
}

void *gb28181_rtp_loop(void *param)
{
#define AUDIO_FRAME_SIZE	320
#define BUF_SIZE	(1024*1024)
	SipRtpSession_t *srtp=NULL;
	Rtp_t *rtp = NULL;
	int *trigger = (int *)param;
	int i;
	int entries=0;
	uint8_t *buf=NULL;
	uint8_t *ptr;
	int out_success=false;
	unsigned int base_ts=0xffffffff,ts=0;
	unsigned int iRetSize=0;
	lpMEDIABUF_USER media_user=NULL;
	int media_id;
	int is_first_i_frame = 1, i_frame_cnt = 0;

	//
	FILE *f=file_new("test.720p.264");
	buf = (char *)malloc(BUF_SIZE);
	if(buf == NULL){
		printf("malloc for buffer size failed!\n");
		return NULL;
	}

	while(*trigger){
		entries = SIPRTP_session_entries();
		if(entries > 0){
			// init mediabuf
			if(media_user == NULL){
				media_id = MEDIABUF_lookup_byname("360p.264");
				if(media_id >= 0){
					media_user = MEDIABUF_attach(media_id);
					if(media_user == NULL){
						printf("media attach falied!\n");
						usleep(500*1000);
						continue;
					}
					//media_speed = MEDIABUF_in_speed(media_id);
					MEDIABUF_sync(media_user);
					is_first_i_frame = 1;
					i_frame_cnt = 0;
				}else{
					printf("lookup name failed falied!\n");
					usleep(500*1000);
					continue;
				}
			}
			/////////////////////////////
			ptr = buf;
			out_success = false;
			if(0 == MEDIABUF_out_lock(media_user)){
				const lpSDK_ENC_BUF_ATTR attr = NULL;
				size_t out_size = 0;
				
				if(0 == MEDIABUF_out(media_user, (void **)&attr, NULL, &out_size)){
					const void* const raw_ptr = (void*)(attr + 1);
					ssize_t const raw_size = attr->data_sz;
					
					MEDIABUF_out_unlock(media_user);
					if(is_first_i_frame && attr->h264.keyframe){
						if(++i_frame_cnt >= 2){
							is_first_i_frame = 0;
							base_ts = (uint32_t)(attr->timestamp_us/1000);
						}
					}
					if(i_frame_cnt < 2){
						//MEDIABUF_out_unlock(media_user);
						continue;
					}
					//if(base_ts == 0xffffffff) base_ts = (uint32_t)(attr->timestamp_us/1000);
					if(((uint32_t)(attr->timestamp_us/1000) - base_ts -ts) > 50){
						printf("ts dev:%d ts:%u->%u %u\n",(uint32_t)(attr->timestamp_us/1000) - base_ts -ts,ts,
							(uint32_t)(attr->timestamp_us/1000) - base_ts,base_ts);
					}
					ts = (uint32_t)(attr->timestamp_us/1000) - base_ts;
					if(kSDK_ENC_BUF_DATA_H264 == attr->type){
						if(MPEG_muxer_video(raw_ptr,raw_size,ts,
							MPEG_DEFAULT_VIDEO_STREAM,attr->h264.keyframe,ptr,BUF_SIZE,&iRetSize)==0)
						{
							entries = SIPRTP_session_entries();
							for(i=0;i<entries;i++){
								srtp = SIPRTP_session_get(i+1);
								if(srtp->data.flag == TRUE){
									//printf(">>>>>>index %d \n",i+1);
									rtp = (Rtp_t *)srtp->data.context;
									RTP_send_packet(rtp,ptr,iRetSize,ts,RTP_TYPE_PS);
									out_success = true;
								}
							}
							//file_write(f,ptr,iRetSize);
							/*
							Test264Frame2_t header;
							header.flag = 0x7d22628c;
							header.isidr = attr->h264.keyframe;
							header.size = raw_size;
							file_write(f,&header,sizeof(Test264Frame2_t));
							file_write(f,raw_ptr,raw_size);
							*/
							ptr+=iRetSize;
						}else{
							printf("muxer video failed!\n");
							break;
						}
					}/*
					else if(kSDK_ENC_BUF_DATA_G711A == attr->type){
						if(MPEG_muxer_audio(raw_ptr,raw_size,attr->timestamp_us/1000,
							MPEG_DEFAULT_AUDIO_STREAM,ptr,&iRetSize)==0)
						{
							entries = SIPRTP_session_entries();
							for(i=0;i<entries;i++){
								srtp = SIPRTP_session_get(i+1);
								if(srtp->data.flag == TRUE){
									rtp = (Rtp_t *)srtp->data.context;
									RTP_send_packet(rtp,ptr,iRetSize,attr->timestamp_us/1000,RTP_TYPE_PS);
								}
							}
							ptr += iRetSize;
						}else{
							printf("muxer audio failed!\n");
							break;
						}						
					}*/
					else {
						printf("GB28181 got type=%d impossible\n", attr->type);
					}
					//MEDIABUF_out_unlock(media_user);
				}else{
					MEDIABUF_out_unlock(media_user);
					//printf("out failed!\n");
					usleep(1000);
				}
			}
		}else{
			if(media_user){
				MEDIABUF_detach(media_user);
				media_user = NULL;
				base_ts = 0xffffffff;
			}
			usleep(10*1000);
		}
	}
	
	printf("rtp loop exit!!!!\n");
	if(media_user){
		MEDIABUF_detach(media_user);
		media_user = NULL;
	}
	free(buf);
	
	return NULL;	
}

void *gb28181_alarm_loop(void* session)
{
	GB_ASSERT(session,"NULL sip session handle\n");
	pSipSession s = (pSipSession)session;
	TRACE("now :%s\n",s->get_systime());
	s->pack_msg(s,MSG_ALARM,NULL);
	TRACE("send:\n%s\n",s->sendbuf);
	sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
			0,(struct sockaddr*)&s->server,sizeof(s->server));	
}




int gb28181_data_proc(pSipSession s)
{	
	char ip[32];
	int port,payloadtype;

	sip_parse_siphead(&s->recv_siphead,s->recvbuf);
	if(0 == strncmp(s->recv_siphead.sh_ReqLine.cmd,"MESSAGE",strlen("MESSAGE")))
	{
		TRACE("\033[32mGet a %s 2 parse\033[0m\n",s->recv_siphead.sh_ContentType.content_type);
		if(0 == strncmp(s->recv_siphead.sh_ContentType.content_type,"Application/MANSCDP+xml",strlen("Application/MANSCDP+xml")))
		{
			if( 0 ==  s->run_msg_loop)
			{
				char *data_ptr = strstr(s->recvbuf,"\r\n\r\n") + 4;
				s->xml_str = strdup(data_ptr);	
				char *cmdtype = s->get_tagvalue(s->xml_str,"CmdType");		
				if(NULL ==  s->get_tagvalue(s->xml_str,"PTZCmd")
						&& NULL == s->get_tagvalue(s->xml_str,"TeleBoot")
						&& 0 != strcmp("Alarm",cmdtype))
				{
					s->pack_msg(s,MSG_200,NULL);
					TRACE("echo 200;Message need 4 steps way\n");		
					sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
							0,(struct sockaddr*)&s->server,sizeof(s->server));
				}				
				pthread_t msg;
				pthread_create(&msg,NULL,&gb28181_msg_loop,s);
				pthread_detach(msg);
			}
		}
	}
	else if(0 == strncmp(s->recv_siphead.sh_EchoLine.msg_code,"401",3))
	{
		if(s->auth_status == AUTH_S_REG)
		{
			s->pack_msg(s,MSG_RE_AUTH,NULL);
			TRACE("401:::Re_auth\n");
			s->auth_status = AUTH_S_401;
		}
	}
	else if(0 == strncmp(s->recv_siphead.sh_EchoLine.msg_code,"200",3))
	{
		if((0 == s->online) && s->auth_status == AUTH_S_REG_AGAIN)
		{//this is auth success
			GB_NOTIFY("200:::AUTH success time:%d\n",(int)time(NULL));
			//FIXME the time on board is UTC time
			TRACE("ServerTime:%04d-%02d-%02d %02d:%02d:%02d.%03d\n",
					s->recv_siphead.sh_Date.YYYY,s->recv_siphead.sh_Date.MM,
					s->recv_siphead.sh_Date.DD,
					s->recv_siphead.sh_Date.hh,
					s->recv_siphead.sh_Date.mm,
					s->recv_siphead.sh_Date.ss,
					s->recv_siphead.sh_Date.mmm);
			char time_str[32];
			sprintf(time_str,"date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",
					s->recv_siphead.sh_Date.YYYY,
					s->recv_siphead.sh_Date.MM,
					s->recv_siphead.sh_Date.DD,
					s->recv_siphead.sh_Date.hh,
					s->recv_siphead.sh_Date.mm,
					s->recv_siphead.sh_Date.ss);
			system(time_str);

			s->auth_status = AUTH_S_SUCCESS;
			s->online = 1;
			pthread_t hb;
			pthread_create(&hb,NULL,&gb28181_heartbeat_loop,s);
			pthread_detach(hb);
		}
		else if((0 != s->online) && s->auth_status == AUTH_S_REG_AGAIN)
		{
			GB_NOTIFY("200:::Login out success:%d\n",(int)time(NULL));
			s->auth_status = AUTH_S_SUCCESS;
			s->online = 0 ;
		}
		else if((0 != s->online) && s->auth_status == AUTH_S_SUCCESS)
		{// heartbeat or other msg echo 200
			if((strlen(s->heartbeat_callid) > 0)&&(0 == strncmp(s->recv_siphead.sh_CallID.call_id,s->heartbeat_callid,
							strlen(s->heartbeat_callid)))){
				TRACE("Get A HeartBeat callID:%s\n",s->heartbeat_callid);
			}else{
				s->run_msg_loop = 0;
			}
		}		
	}
	else if(0 == strncmp(s->recv_siphead.sh_ReqLine.cmd,"INVITE",6))
	{
		SessionDesc_t *sdp=NULL;
		SipRtpSession_t *srtp=NULL;
		TRACE("sdp ###%s\n",strstr(s->recvbuf,"\r\n\r\n") + 4);
		TRACE("Get a media request\n");
		if(SIPRTP_session_find(s->recv_siphead.sh_CallID.call_id)==NULL){
			sdp = (void *)SDP_decode(strstr(s->recvbuf,"\r\n\r\n") + 4);
			srtp=SIPRTP_session_add(s->recv_siphead.sh_CallID.call_id,g_SipRtpPort++,sdp,s);
			pthread_t invite;
			pthread_create(&invite,NULL,gb28181_invite_loop,srtp);
			pthread_detach(invite);
		}
	}
	else if(0 == strncmp(s->recv_siphead.sh_ReqLine.cmd,"ACK",3))
	{
		TRACE("Get a media ACK\n");
		SessionDesc_t *sdp=NULL;
		SipRtpSession_t *srtp=NULL;
		Rtp_t *rtp=NULL;
		if((srtp=SIPRTP_session_find(s->recv_siphead.sh_CallID.call_id))!=NULL){
			sdp = (SessionDesc_t *)srtp->data.context;
			TRACE("ssrc:%u",sdp->media[0].ssrc);
			SDP_get_h264_info(sdp,&payloadtype,ip,&port);
			TRACE("payloadtype:%d,ip:%s:%d\n",payloadtype,ip,port);
			int sock=SOCK_udp_init(srtp->data.port,3000);
			rtp = (void *)RTP_server_new(sdp->media[0].ssrc,payloadtype,RTP_TRANSPORT_UDP,FALSE,sock,ip,port);
			SIPRTP_session_set_context(srtp,rtp);
			SDP_cleanup(sdp);
#if 0
			pthread_t thread_rtp;
			pthread_create(&thread_rtp,NULL,gb28181_rtp_loop,srtp);
#endif
		}
	}
	else if(0 == strncmp(s->recv_siphead.sh_ReqLine.cmd,"BYE",3))
	{		
		SipRtpSession_t *srtp=NULL;
		s->pack_msg(s, MSG_200,NULL);
		TRACE("BYE:\n%s\n",s->sendbuf);
		sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
				0,(struct sockaddr*)&s->server,sizeof(s->server));		
		if((srtp=SIPRTP_session_find(s->recv_siphead.sh_CallID.call_id))!=NULL){
#if 1
			RTP_destroy((Rtp_t *)srtp->data.context);
			SIPRTP_session_del(srtp->data.id);
#else
			SIPRTP_session_toggle(srtp,FALSE);
#endif
		}
	}
	return 0;
}

void* gb28181_recv(void *session)
{
	pSipSession s = (pSipSession)session;
	fd_set rfds;
	int sock_len = sizeof(s->server);
	for(;;) 
	{
		struct timeval tv = { .tv_sec = 3, .tv_usec = 0 };
		FD_ZERO(&rfds);
		FD_SET(s->sock,&rfds);
		int ret = select(s->sock+1,&rfds,NULL,NULL,&tv);
		if(ret < 0)
		{
			TRACE("gb28181_recv select error!!\n");
		}
		else if(ret == 0){
			//TRACE("gb28181_recv select timeout\n");
		}
		else
		{
			if(FD_ISSET(s->sock,&rfds))
			{
				memset(s->recvbuf,0,sizeof(s->recvbuf));
				ret = recvfrom(s->sock,s->recvbuf,sizeof(s->recvbuf),\
						0,(struct sockaddr*)&s->server,&sock_len);
				TRACE("recv:%d\n%s\n",(int)time(NULL),s->recvbuf);
				gb28181_data_proc(s);
			}
		}
	}
}

int ptz_ctrl(int cmd, char speed)
{
	if(0 == cmd){
		TRACE("Stop ptz ctrl\n");
	}else{
		TRACE("PTZ:%d speed:%d\n",cmd,speed);
	}
	return 0;
}

int dev_ctrl(int cmd, char *arg)
{	
	TRACE("Device Ctrl:%d arg:%s\n",cmd,arg);
	return 0;
}

static int gb28181_write_env(unsigned char *addr,int len)
{
	SYSCONF_t* sys_conf = SYSCONF_dup();
	GB_ASSERT(len < sizeof(sys_conf->ipcam.gb28181_param), "No space in sysconf for GB28181");
	memcpy(sys_conf->ipcam.gb28181_param.buf, addr, len);
	SYSCONF_save(sys_conf); // save to flash

	TRACE("Write Gb28181 param to flash size = %d\n", len);
	return 0;
}

static int gb28181_read_env(unsigned char *addr,int len)
{
	SYSCONF_t* sys_conf = SYSCONF_dup();
	GB_ASSERT(len < sizeof(sys_conf->ipcam.gb28181_param), "No space in sysconf for Gb28181");
	memcpy(addr, sys_conf->ipcam.gb28181_param.buf, len);

	TRACE("Read Gb28181 param from flash size = %d\n", len);
	return 0;
}


int gb28181_init_env(pSipEnv *env)
{
	TRACE("Init GB28181 env\n");
	//constly
	SYSCONF_t *sysconf = SYSCONF_dup();
	pSipEnv p = (pSipEnv)calloc(1,sizeof(sSipEnv));
	*env = p;
	char devip[20];
	SOCK_gethostname(devip);
	GB_ASSERT(*env,"Calloc env  Node failed\n");
	printf("calloc env hdr node %p\n",*env);
	SIP_ENV_insert(p,"Model",sysconf->ipcam.info.device_model);
	SIP_ENV_insert(p,"UserAgent",sysconf->ipcam.info.device_name);
	SIP_ENV_insert(p,"Firmware",sysconf->ipcam.info.software_version);
	SIP_ENV_insert(p,"MaxAlarm","1");
	SIP_ENV_insert(p,"MaxCamera","1");
	SIP_ENV_insert(p,"Manufacturer","JUAN");	

	// variable
	Gb28181Conf_t gb_conf;
	memset(&gb_conf, 0, sizeof(Gb28181Conf_t));
	gb28181_read_env(&gb_conf,sizeof(Gb28181Conf_t));
	if(strlen(gb_conf.SipServerId) > 0)
		//if(0)
	{
		TRACE("use the env in the flash,SipServerId:%s:%d\n",gb_conf.SipServerId,
				strlen(gb_conf.SipServerId));	
		SIP_ENV_insert(p,"Realm",gb_conf.LoginDomain);
		SIP_ENV_insert(p,"SipServerId",gb_conf.SipServerId);
		SIP_ENV_insert(p,"SipServerIp",gb_conf.SipServerIp);
		SIP_ENV_insert(p,"SipServerPort",ITOA(gb_conf.SipServerPort));
		SIP_ENV_insert(p,"UserId",gb_conf.UserId);
		SIP_ENV_insert(p,"UserPwd",gb_conf.UserPwd);
		SIP_ENV_insert(p,"DevPort",ITOA(GB28181_DEFAULT_DEV_PORT));
		SIP_ENV_insert(p,"DevIp",devip);
		SIP_ENV_insert(p,"AliveTime",ITOA(gb_conf.AliveTime));
		SIP_ENV_insert(p,"HeartBeatTime",ITOA(gb_conf.HeartBeatTime));
		SIP_ENV_insert(p,"Alarm1",gb_conf.AlarmId[0]);
		SIP_ENV_insert(p,"Video1",gb_conf.VideoId[0]);
	}
	else{ // use the default gb28181 env
		TRACE("NULL gb28181 env ,use default env\n");
		gb28181_write_env(&def_gb_conf,sizeof(Gb28181Conf_t));
		SIP_ENV_insert(p,"Realm",def_gb_conf.LoginDomain);
		SIP_ENV_insert(p,"SipServerId",def_gb_conf.SipServerId);
		SIP_ENV_insert(p,"SipServerIp",def_gb_conf.SipServerIp);
		SIP_ENV_insert(p,"SipServerPort",ITOA(def_gb_conf.SipServerPort));	
		SIP_ENV_insert(p,"UserId",def_gb_conf.UserId);
		SIP_ENV_insert(p,"UserPwd",def_gb_conf.UserPwd);
		SIP_ENV_insert(p,"DevIp",devip);
		SIP_ENV_insert(p,"DevPort",ITOA(GB28181_DEFAULT_DEV_PORT));
		SIP_ENV_insert(p,"AliveTime",ITOA(def_gb_conf.AliveTime));
		SIP_ENV_insert(p,"HeartBeatTime",ITOA(def_gb_conf.HeartBeatTime));
		SIP_ENV_insert(p,"Alarm1",def_gb_conf.AlarmId[0]);
		SIP_ENV_insert(p,"Video1",def_gb_conf.VideoId[0]);

	}
	return 0;
}


int GB28181_stop()
{
	GB_TRACE("Gb28181 stop\n");
	session.destroy(&session);
	SIPRTP_session_destroy();
	return 0;
}

int GB28181_resart()
{
	GB28181_stop();
	GB28181_start();
	return 0;
}


char env_node[][32] = {
	"SipServerIp","SipServerIp",
	"SipServerPort","SipServerPort",
	"SipServerId","SipServerId",
	"Realm","LoginDomain",
	"AliveTime","AliveTime",
	"HeartBeatTime","HeartBreakTime",
	"UserId","userid",
	"UserPwd","userpwd",
	"MaxAlarm","AlarmNum",
	"MaxCamera","VideoNum",
	"","",
};

int GB28181_configure(const void *conf_xml)
{
	if(!conf_xml)	return -1;
	TRACE("new CONF:\n%s\n",conf_xml);
	char *env_tag = NULL;

	int i = 0;
	for(; strlen(env_node[i]) > 0; i += 2)
	{		
		if(env_tag = xml_get_tagvalue(conf_xml,env_node[i+1])){
			SIP_ENV_edit(session.env,env_node[i],env_tag);
			TRACE("i = %d,node:%s==%s\n",i,env_node[i],env_tag);
		}	
	}
	char *max_camera = xml_get_tagvalue(conf_xml,"VideoNum");
	for(i=0;i<atoi(max_camera);i++)
	{
		char VideoId[32];
		sprintf(VideoId,"Video%d",i+1);
		if(env_tag = xml_get_tagvalue(conf_xml,VideoId)){
			SIP_ENV_edit(session.env,VideoId,env_tag);
			TRACE("VideoId:%s==%s\n",VideoId,env_tag);
		}
	}
	char *max_alarm = xml_get_tagvalue(conf_xml,"AlarmNum");
	for(i=0;i<atoi(max_alarm);i++)
	{
		char AlarmId[32];
		sprintf(AlarmId,"Alarm%d",i+1);
		if(env_tag = xml_get_tagvalue(conf_xml,AlarmId)){
			SIP_ENV_edit(session.env,AlarmId,env_tag);
			TRACE("AlarmId:%s==%s\n",AlarmId,env_tag);
		}
	}
	
	//SIP_ENV_print(session.env);	
	Gb28181Conf_t conf ;
	strcpy(conf.SipServerId, SIP_ENV_get(session.env,"SipServerId"));
	strcpy(conf.SipServerIp, SIP_ENV_get(session.env,"SipServerIp"));	
	conf.SipServerPort = atoi(SIP_ENV_get(session.env,"SipServerPort"));
	strcpy(conf.LoginDomain, SIP_ENV_get(session.env,"Realm"));
	conf.AliveTime = atoi(SIP_ENV_get(session.env,"AliveTime"));
	conf.HeartBeatTime = atoi(SIP_ENV_get(session.env,"HeartBeatTime"));
	strcpy(conf.UserId, SIP_ENV_get(session.env,"UserId"));
	strcpy(conf.UserPwd, SIP_ENV_get(session.env,"UserPwd"));
	conf.VideoNum = atoi(SIP_ENV_get(session.env,"MaxCamera"));
	for(i=0;i<conf.VideoNum;i++)
	{
		char VideoId[32];
		sprintf(VideoId,"Video%d",i+1);
		if(env_tag = xml_get_tagvalue(conf_xml,VideoId)){
			strcpy(conf.VideoId[i], SIP_ENV_get(session.env,VideoId));		
		}
	}	
	conf.AlarmNum = atoi(SIP_ENV_get(session.env,"MaxAlarm"));
	for(i=0; i<conf.AlarmNum; i++)
	{
		char AlarmId[32];
		sprintf(AlarmId,"Alarm%d",i+1);
		if(env_tag = xml_get_tagvalue(conf_xml,AlarmId)){
			strcpy(conf.AlarmId[0], SIP_ENV_get(session.env,AlarmId));
		}
	}	
	
	TRACE("#####################configure#################\n");
	TRACE("\nsSIP##%s:%s:%d\nDomain##%s\nAlive:%d,HB:%d\nUser:%s:%s\nVideo:%d:%s\nAlarm:%d:%s\n",
			conf.SipServerId,conf.SipServerIp,conf.SipServerPort,
			conf.LoginDomain,conf.AliveTime,conf.HeartBeatTime,
			conf.UserId,conf.UserPwd,
			conf.VideoNum,conf.VideoId[0],
			conf.AlarmNum,conf.AlarmId[0]);
			
	gb28181_write_env(&conf,sizeof(Gb28181Conf_t));	

	GB28181_resart();	
	return 0;
}

char* GB28181_conf2str(void)
{
	return NULL;
}


int GB28181_start(void)
{

	TRACE("GB28181 Start\n");

	session.ptz_ctrl = ptz_ctrl;
	session.dev_ctrl = dev_ctrl;
	gb28181_init_env(&session.env);	
	//SIP_ENV_print(session.env);
	sip_init_session(&session);
	SIPRTP_session_init();	

	pthread_t recv_threadId;
	pthread_create(&recv_threadId, NULL, gb28181_recv, &session);
	pthread_detach(recv_threadId);
	sleep(1);
	session.auth(&session, 1);

	pthread_t rtp_threadId;
	pthread_create(&rtp_threadId, NULL, gb28181_rtp_loop, (void *)&session.trigger);
	pthread_detach(rtp_threadId);


	//	for(;;)
	//	{
	//		usleep(1);
	//		if(gb28181_sigquit == 1){
	//			session.auth(&session,0);
	//			GB_NOTIFY("exit sip main\n");
	//			break;
	//			}
	//		if(gb28181_sigtstp == 1){
	//			pthread_t thread_alarm;
	//			pthread_create(&thread_alarm,NULL,gb28181_alarm_loop,&session);
	//			pthread_detach(thread_alarm);
	//			gb28181_sigtstp = 0;
	//		}
	//	}

	return 0;
}


