/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	siplib.c
 * Describle:
 * History: 
 * Last modified: 2013-06-28 20:21
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
#include "siprtpsession.h"
#include "http_auth/authentication.h"
#include "sdplib.h"
#include "gb28181debug.h"
#include "ezxml.h"
//

#define RAND() ({static char rand_buf[128];sprintf(rand_buf,"%d",rand()); rand_buf;})

#define CRLF "\r\n"
#define CMD "%s sip:%s@%s SIP/2.0\r\n"
#define VIA "Via: SIP/2.0/UDP %s:%s;rport;branch=%s\r\n"
#define VIA_ECHO "Via: SIP/2.0/UDP %s:%s;rport=%s;branch=%s;received=%s\r\n"
#define VIA_MAGIC "Via: SIP/2.0/UDP %s:%s;rport;branch=z9hG4bk%s\r\n"
#define FROM "From: <sip:%s@%s>;tag=%s\r\n"
#define TO "To: <sip:%s@%s>\r\n"
#define TO_WITH_TAG "To: <sip:%s@%s>;tag=%s\r\n"
#define CALLID "Call-ID: %s\r\n"
#define CSEQ "CSeq: %d %s\r\n"
#define CONTACT "Contact: <sip:%s@%s:%s>\r\n"
#define MAX_FWD "Max-Forwards: 70\r\n"
#define USER_AGENT "User-Agent: %s\r\n"
#define EXPIRES "Expires: %d\r\n"
#define CONTENT_TYPE "Content-Type: %s\r\n"
#define CONTENT_LEN "Content-Length: %d\r\n"
#define AUTHORIZATION "Authorization: %s,algorithm=MD5\r\n"
#define ECHO_200 "SIP/2.0 200 OK\r\n"
#define AUTHORIZATION0 "Authorization: Capability, algorithm=\"MD5\"\r\n"

char *auth_fmt = CMD""VIA_MAGIC""FROM""TO""CALLID""CSEQ""CONTACT""MAX_FWD""\
				 USER_AGENT""EXPIRES""CONTENT_LEN""CRLF;

char *re_auth_fmt = CMD""VIA_MAGIC""FROM""TO""CALLID""CSEQ""CONTACT""AUTHORIZATION""\
					MAX_FWD""USER_AGENT""EXPIRES""CONTENT_LEN""CRLF;

char *hb_xml= 
"<?xml version=\"1.0\"?>\r\n" 
"<Notify>\r\n"
"<CmdType>Keepalive</CmdType>\r\n"
"<SN>1</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<Status>OK</Status>\r\n"
"</Notify>";

char *hb_fmt = CMD""VIA_MAGIC""MAX_FWD""FROM""TO""CALLID""CSEQ""CONTENT_TYPE""\
				CONTENT_LEN""CRLF"%s";

char *response_200_fmt =ECHO_200""VIA_ECHO""FROM""TO_WITH_TAG""CALLID""CSEQ""\
						USER_AGENT""CONTENT_LEN""CRLF;
char *invite_200_fmt = ECHO_200""VIA_ECHO""FROM""TO_WITH_TAG""CALLID""CSEQ""\
						CONTACT""CONTENT_TYPE""USER_AGENT""CONTENT_LEN""CRLF"%s";
char *dev_fmt = 
"<Item>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<Name>%s</Name>\r\n"
"<Manufacturer>%s</Manufacturer>\r\n"
"<Model>%s</Model>\r\n"
"<Owner>JUAN</Owner>\r\n"
"<CivilCode>CivilCode</CivilCode>\r\n"
"<Address>Address</Address>\r\n"
"<Parental>0</Parental>\r\n"
"<SafetyWay>4</SafetyWay>\r\n"
"<RegisterWay>1</RegisterWay>\r\n"
"<Secrecy>0</Secrecy>\r\n"
"<Status>ON</Status>\r\n"
"</Item>";

char *catalog_response_fmt =CMD""VIA_MAGIC""FROM""TO""CALLID""CSEQ""CONTENT_TYPE""\
							MAX_FWD""USER_AGENT""CONTENT_LEN""CRLF"%s";
							
char *catalog_xml_fmt =
"<?xml version=\"1.0\"?>\r\n"
"<Response>\r\n"
"<CmdType>Catalog</CmdType>\r\n"
"<SN>0</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<SumNum>10</SumNum>\r\n"
"<DeviceList Num=\"1\">\r\n"
"%s\r\n"
"</DeviceList>\r\n"
"</Response>";

char *invite_200_sdp_fmt =
"v=0\r\n"
"o=%s 0 0 IN IP4 %s\r\n"
"s=%s\r\n"
"c=IN IP4 %s\r\n"
"t=0 0\r\n"
"m=video %d RTP/AVP 96 98\r\n"
"a=sendonly\r\n"
"a=rtpmap:96 H264/90000\r\n"
"a=rtpmap:98 H264/90000\r\n"
"a=username:%s\r\n"
"a=password:%s\r\n"
"y=%010d\r\n"
"f=";

char *dev_ctrl_xml_fmt =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<Response>\r\n"
"<CmdType>DeviceControl</CmdType>\r\n"
"<SN>%s</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<Result>OK</Result>\r\n"
"</Response>";

char *record_fmt = CMD""VIA_MAGIC""FROM""TO""CALLID""CSEQ""CONTENT_TYPE""MAX_FWD""USER_AGENT""CONTENT_LEN""CRLF"%s";


char *dev_info_fmt = 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<Response>\r\n"
"<CmdType>DeviceInfo</CmdType>\r\n"
"<SN>%s</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<Result>OK</Result>\r\n"
"<DeviceType>IPC</DeviceType>\r\n"
"<Manufacturer>%s</Manufacturer>\r\n"
"<Model>%s</Model>\r\n"
"<Firmware>%s</Firmware>\r\n"
"<MaxCamera>%s</MaxCamera>\r\n"
"<MaxAlarm>%s</MaxAlarm>\r\n"
"</Response>";

char *dev_status_fmt = 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<Response>\r\n"
"<CmdType>DeviceStatus</CmdType>\r\n"
"<SN>%s</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<Result>OK</Result>\r\n"
"<Online>%s</Online>\r\n"
"<Status>OK</Status>\r\n"
"<DeviceTime>%s</DeviceTime>\r\n"
"<Alarmstatus Num=\"1\">\r\n"
"<Item>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<DutyStatus>%s</DutyStatus>\r\n"
"</Item>\r\n"
"</Alarmstatus>\r\n"
"<Encode>ON</Encode>\r\n"
"<Record>OFF</Record>\r\n"
"</Response>";

char *alarm_fmt = CMD""VIA_MAGIC""FROM""TO""CALLID""CSEQ""CONTENT_TYPE""\
							MAX_FWD""USER_AGENT""CONTENT_LEN""CRLF"%s";

char *alarm_info_fmt = 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"<Notify>\r\n"
"<CmdType>Alarm</CmdType>\r\n"
"<SN>45</SN>\r\n"
"<DeviceID>%s</DeviceID>\r\n"
"<AlarmPriority>0</AlarmPriority>\r\n"
"<AlarmTime>%s</AlarmTime>\r\n"
"<AlarmMethod>0</AlarmMethod>\r\n"
"</Notify>";



/*create a sock for sip comunication*/
static int create_sock(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	int sock_len = sizeof(struct sockaddr);
	GB_ASSERT((sock > 0),"create sock failed\n");
	struct sockaddr_in local = 	{
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = inet_addr(ip?ip:"0.0.0.0"),
	};
	int flag = 1;
	GB_ASSERT((0 == setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag))),\
				"setsockopt failed\n");
	GB_ASSERT((0 == bind(sock,(struct sockaddr*)&local,sock_len)),\
			"Bind sock failed");
	//FIXME,setsockopt,send,recv's timeout
	return sock;
}

int sip_pack_msg(pSipSession s,int MsgType,void *data)
{
	if(MSG_HEARTBEAT == MsgType)
	{
		char hb_buf[256];
		sprintf(hb_buf,hb_xml,s->get_env(s->env,"UserId"));
		sprintf(s->heartbeat_callid,"%s",RAND());
		sprintf(s->sendbuf,hb_fmt,"MESSAGE",
				s->get_env(s->env,"SipServerId"),
				s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),
				s->get_env(s->env,"DevPort"),
				RAND(),
				s->get_env(s->env,"UserId"),
				s->get_env(s->env,"Realm"),
				RAND(),
				s->get_env(s->env,"SipServerId"),
				s->get_env(s->env,"Realm"),
				s->heartbeat_callid,
				10,"MESSAGE","Application/MANSCDP+xml",
				strlen(hb_buf),
				hb_buf);
	}else if(MSG_200 == MsgType)
	{		
		sprintf(s->sendbuf,response_200_fmt,
				s->recv_siphead.sh_Via.src_ip,s->recv_siphead.sh_Via.src_port,
				s->get_env(s->env,"DevPort"),s->recv_siphead.sh_Via.branch,s->get_env(s->env,"DevIp"),
				s->recv_siphead.sh_From.id,s->recv_siphead.sh_From.realm,
				s->recv_siphead.sh_From.tag,
				s->recv_siphead.sh_To.id,s->recv_siphead.sh_To.realm,RAND(),
				s->recv_siphead.sh_CallID.call_id,
				atoi(s->recv_siphead.sh_CSeq.CSeq),"MESSAGE",
				s->get_env(s->env,"UserAgent"),0);
	}
	else if(MSG_AUTH == MsgType)
	{
		sprintf(s->sendbuf,auth_fmt,"REGISTER",
				s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),
				RAND(),
				1,"REGISTER",
				s->get_env(s->env,"UserId"),s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),
				s->get_env(s->env,"UserAgent"),
				0,0);

	}else if(MSG_RE_AUTH == MsgType)
	{			
		sip_parse_siphead(&s->send_siphead,s->sendbuf);
		char www_buf[128];
		sprintf(www_buf,"Digest realm=\"%s\",nonce=\"%s\"",\
				s->recv_siphead.sh_WWW_Auth.realm,s->recv_siphead.sh_WWW_Auth.nonce);

		Authentication_t *authinfo = NULL;
		GB_ASSERT((0 == HTTP_AUTH_client_init(&authinfo,www_buf)),"Http_auth_client init failed\n");	
		char uri[128];
		sprintf(uri,"sip:%s@%s",s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"));
		HTTP_AUTH_setup(authinfo,s->get_env(s->env,"UserId"),s->get_env(s->env,"UserPwd"),
				uri,"REGISTER",s->auth_key,sizeof(s->auth_key));	

		sprintf(s->sendbuf,re_auth_fmt,"REGISTER",
				s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),s->send_siphead.sh_From.tag,
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),			
				s->send_siphead.sh_CallID.call_id,
				atoi(s->send_siphead.sh_CSeq.CSeq) + 1,"REGISTER",
				s->send_siphead.sh_Contact.id,s->send_siphead.sh_Contact.ip,s->send_siphead.sh_Contact.port,
				s->auth_key,
				s->get_env(s->env,"UserAgent"),
				s->login_out?s->login_out:0,
				0);

	}
	else if(MSG_CATALOG == MsgType)
	{
		char dev_info[512];
		char catalog[1024];

		sprintf(dev_info,dev_fmt,s->get_env(s->env,"UserId"),
								 s->get_env(s->env,"UserAgent"),
								 s->get_env(s->env,"Manufacturer"),
								 s->get_env(s->env,"Model"));
		sprintf(catalog,catalog_xml_fmt,s->get_env(s->env,"UserId"),dev_info);	
		sprintf(s->sendbuf,catalog_response_fmt,"MESSAGE",
				s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->recv_siphead.sh_From.id,s->recv_siphead.sh_From.realm,
				RAND(),20,"MESSAGE","Application/MANSCDP+xml",
				s->get_env(s->env,"UserAgent"),
				strlen(catalog),catalog);					
	}
	else if (MSG_INVITE_200 == MsgType)
	{
		char sdp_buf[512];
		SipRtpSession_t *srtp = (SipRtpSession_t *)data;
		SessionDesc_t *sdp=srtp->data.context;
		sprintf(sdp_buf,invite_200_sdp_fmt,
				s->get_env(s->env,"UserId"),s->get_env(s->env,"DevIp"),
				s->get_env(s->env,"UserAgent"),
				s->get_env(s->env,"DevIp"),
				srtp->data.port,
				s->get_env(s->env,"UserId"),
				s->get_env(s->env,"UserPwd"),
				SDP_get_ssrc(sdp));
			
		sprintf(s->sendbuf,invite_200_fmt,
				s->recv_siphead.sh_Via.src_ip,s->recv_siphead.sh_Via.src_port,
				s->get_env(s->env,"DevPort"),s->recv_siphead.sh_Via.branch,s->get_env(s->env,"DevIp"),
				s->recv_siphead.sh_From.id,s->recv_siphead.sh_From.realm,
				s->recv_siphead.sh_From.tag,
				s->recv_siphead.sh_To.id,s->recv_siphead.sh_To.realm,RAND(),
				s->recv_siphead.sh_CallID.call_id,
				atoi(s->recv_siphead.sh_CSeq.CSeq),"INVITE",
				s->get_env(s->env,"UserId"),s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),
				"application/SDP",				
				s->get_env(s->env,"UserAgent"),strlen(sdp_buf),sdp_buf);
	}
	else if(MSG_DEV_CTRL == MsgType)
	{
		char xml_buf[256];
		char *xml_str = strstr(s->recvbuf,"\r\n\r\n");
		sprintf(xml_buf,dev_ctrl_xml_fmt,s->get_tagvalue(xml_str,"SN"),s->get_env(s->env,"UserId"));
		sprintf(s->sendbuf,record_fmt,
				"MESSAGE",s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				RAND(),20,"MESSAGE","Application/MANSCDP+xml",
				s->get_env(s->env,"UserAgent"),
				strlen(xml_buf),xml_buf);					
	}
	else if(MSG_DEV_INFO == MsgType)
	{
		char xml_buf[256];
		char *xml_str = strstr(s->recvbuf,"\r\n\r\n");
		sprintf(xml_buf,dev_info_fmt,
				s->get_tagvalue(xml_str,"SN"),s->get_env(s->env,"UserId"),
				s->get_env(s->env,"Manufacturer"),s->get_env(s->env,"Model"),
				s->get_env(s->env,"Firmware"),s->get_env(s->env,"MaxCamera"),
				s->get_env(s->env,"MaxAlarm"));
		sprintf(s->sendbuf,record_fmt,
				"MESSAGE",s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->recv_siphead.sh_From.id,s->recv_siphead.sh_From.realm,
				RAND(),20,"MESSAGE","Application/MANSCDP+xml",
				s->get_env(s->env,"UserAgent"),
				strlen(xml_buf),xml_buf);	

	}
	else if(MSG_DEV_STATUS == MsgType)
	{
		char xml_buf[256];
		char *xml_str = strstr(s->recvbuf,"\r\n\r\n");
		sprintf(xml_buf,dev_status_fmt,
				s->get_tagvalue(xml_str,"SN"),s->get_env(s->env,"UserId"),
				s->online?"ONLINE":"OFFLINE",s->get_systime(),
				s->get_env(s->env,"AlarmID"),
				(s->alarm_status>1)?"ALARM":(s->alarm_status?"ONDUTY":"OFFDUTY"));
		sprintf(s->sendbuf,record_fmt,
				"MESSAGE",s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->recv_siphead.sh_From.id,s->recv_siphead.sh_From.realm,
				RAND(),20,"MESSAGE","Application/MANSCDP+xml",
				s->get_env(s->env,"UserAgent"),
				strlen(xml_buf),xml_buf);		
	}
	else if(MSG_ALARM == MsgType)
	{
		char xml_buf[256];
		sprintf(xml_buf,alarm_info_fmt,
				s->get_env(s->env,"AlarmID"),s->get_systime());
		sprintf(s->sendbuf,alarm_fmt,
				"MESSAGE",s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				s->get_env(s->env,"DevIp"),s->get_env(s->env,"DevPort"),RAND(),
				s->get_env(s->env,"UserId"),s->get_env(s->env,"Realm"),RAND(),
				s->get_env(s->env,"SipServerId"),s->get_env(s->env,"Realm"),
				RAND(),40,"MESSAGE","Application/MANSCDP+xml",
				s->get_env(s->env,"UserAgent"),
				strlen(xml_buf),xml_buf);				
	}
	else
	{
		GB_ERROR("unsupport msg type\n");
	}
	return 0;
}

//0 login out , NZ login
void* sip_auth_loop(void* session)
{
	pSipSession s = (pSipSession)session;
	time_t cur_time;	
	int auth_re_cnt;
	
AUTH_AGAIN:
	auth_re_cnt = 0;
	s->auth_status = AUTH_S_REG;
	sip_pack_msg(s,MSG_AUTH,NULL);
	cur_time = time(NULL) - 4;
	for(;;)
	{
		usleep(1);
		if(s->auth_status == AUTH_S_401)
		{
			auth_re_cnt = 0;
			cur_time = time(NULL) - 4;
			s->auth_status = AUTH_S_REG_AGAIN;
		}
		if(time(NULL) - cur_time >= 3)
		{
			if(3 == auth_re_cnt++){
				sleep(5);	
				GB_NOTIFY("TRY TO LOGIN SIP SEVER AGAIN\n");
				goto AUTH_AGAIN; 
				}
			cur_time = time(NULL);
			switch(s->auth_status)
			{
				case AUTH_S_REG:
					{
						TRACE("START auth:%d\n",(int)time(NULL));
						sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
								0,(struct sockaddr*)&s->server,sizeof(s->server));
					}
					break;
				case AUTH_S_REG_AGAIN:
					{
						TRACE("RE auth:%d\n",(int)time(NULL));
						sendto(s->sock,s->sendbuf,strlen(s->sendbuf),\
								0,(struct sockaddr*)&s->server,sizeof(s->server));
					}
					break;
				default:
					break;
			}
		}
		if(s->auth_status == AUTH_S_SUCCESS)
			break;
	}

	//until login_alivetime fly
	if(s->login_out > 10){		
		sleep(s->login_out - 10);
		GB_NOTIFY("Re login the SIP server\n");
		s->online = 0;
		goto AUTH_AGAIN;
	}else if(s->login_out == 0){
		//do nothing
	}else{
		sleep(s->login_out - 1);
		GB_NOTIFY("Re login the SIP server\n");
		s->online = 0;
		goto AUTH_AGAIN;
		}
	return 0;
}


int sip_auth(pSipSession s,int login)
{
	if(login){
		s->login_out = atoi(s->get_env(s->env,"AliveTime"));
		GB_NOTIFY("START login SIPServer(%s:%s),alivetime:%d\n",
						s->get_env(s->env,"SipServerIp"),
						s->get_env(s->env,"SipServerPort"),
						s->login_out);
		
	}else{
		s->login_out = 0;
		GB_NOTIFY("START logout SIP server\n");		
	}
	s->auth_status = AUTH_S_REG;
	pthread_t auth;
	pthread_create(&auth,NULL,sip_auth_loop,s);
	pthread_detach(auth);
	time_t cur_time = time(NULL);
	while(AUTH_S_SUCCESS != s->auth_status && (time(NULL)-cur_time) < 20);	
	return 0;
}

//parse the xml_str ,from the xml string
int sip_get_subcmd(pSipSession s,char *xml_str,char *sub_cmd,char *cmd)
{
	GB_ASSERT((xml_str)||(sub_cmd)||(cmd),"NULL xml_get_ctrl_cmd");
	char *subcmd = s->get_tagvalue(xml_str,sub_cmd);
	if(NULL == subcmd)
	{
		TRACE("Cant find sub_cmd:%s \n",cmd);
		return 1;
	}
	if(0 == strcmp(subcmd,cmd)){
		return 0;
	}else{
		return 1;
	}
}

int sip_ptz_cmd_parse(pSipSession s,char *cmd)
{
    if(cmd && 16 != strlen(cmd))
    {
        printf("Invalid PTZcmd\n");
        return -1;
    }
    unsigned char cmd_buf[8];
    int i;
    long long int cmd_hex;
    sscanf(cmd,"%llx",&cmd_hex);
    printf("%llx\n",cmd_hex);

    for(i=0; i<8; i++)
    {
        cmd_buf[7-i] = (cmd_hex>>(8*i))&0x0ff;       
    }
    unsigned char cmd_head_check = (((cmd_buf[0]>>4)&0x0f) + ((cmd_buf[0])&0x0f)
    								+ ((cmd_buf[1]>>4)&0x0f))&0x0f;
    unsigned char cmd_check = (cmd_buf[0]+cmd_buf[1]+cmd_buf[2]+cmd_buf[3]
    							+cmd_buf[4]+cmd_buf[5]+cmd_buf[6])
    							&0xff;

    if((cmd_head_check != (cmd_buf[1]&0x0f)) || (cmd_check != cmd_buf[7]))
    {
        printf("ptzcmd check error\n");
        return -2;
    }
    if((cmd_buf[3]&0xc0) != 0 )
    {
		GB_ERROR("Unsurport cmd\n");
    }
    else//ptz
    {
    	if((cmd_buf[3] & 0x03) ==	SIP_DEV_CTRL_RIGHT) {
			s->ptz_ctrl(SIP_DEV_CTRL_RIGHT,cmd_buf[4]);
    	}else if((cmd_buf[3] & 0x03) ==	SIP_DEV_CTRL_LEFT) {
			s->ptz_ctrl(SIP_DEV_CTRL_LEFT,cmd_buf[4]);
    	}

    	if((cmd_buf[3] & 0x0c) ==	SIP_DEV_CTRL_DOWN) {
    		s->ptz_ctrl(SIP_DEV_CTRL_DOWN,cmd_buf[5]);
    	}else if((cmd_buf[3] & 0x0c) ==	SIP_DEV_CTRL_UP) {
			s->ptz_ctrl(SIP_DEV_CTRL_UP,cmd_buf[5]);
    	}

    	if((cmd_buf[3] & 0x30) ==	SIP_DEV_CTRL_ZOOM_IN) {
			s->ptz_ctrl(SIP_DEV_CTRL_ZOOM_IN,cmd_buf[6]&0x0f); 
    	}else if((cmd_buf[3] & 0x30) ==	SIP_DEV_CTRL_ZOOM_OUT) {
				s->ptz_ctrl(SIP_DEV_CTRL_ZOOM_OUT,cmd_buf[6]&0x0f);
    	}
    	if((cmd_buf[3]&0x3f) == SIP_DEV_CTRL_PTZ_STOP){
			s->ptz_ctrl(SIP_DEV_CTRL_PTZ_STOP,0);
    	}
    }
    return 0;
}     

/*get env by envname [name,value]*/
char *SIP_ENV_get(pSipEnv env,char *name)
{
	pSipEnv p = env->next;
	for(;p;p = p->next){		
		if( 0 == strncmp(p->name,name,strlen(name))){
			return p->value;
		}		
	}
	return NULL;
}

int SIP_ENV_insert(pSipEnv env,char *name,char *value)
{
	pSipEnv p=env,s=NULL;
	s=(pSipEnv)calloc(1,sizeof(sSipEnv));
	if(s == NULL){
		return -1;
	}
	strcpy(s->name,name);
	strcpy(s->value,value);
	s->next = NULL;
	while(p->next) p=p->next;
	p->next =s;
	
	return 0;
}

int SIP_ENV_edit(pSipEnv env,char *name,char *value)
{
	pSipEnv p = env;
	for(;p->next;p = p->next){
		if( 0 == strncmp(p->next->name,name,strlen(name))){
			strcpy(p->next->value,value);
			return 0;
		}		
	}
	return 0;
}

int SIP_ENV_destroy(pSipEnv env)
{
	pSipEnv p=env,q;
	while(p){
		q = p->next;
		free(p);
		p = q;
	}
	return 0;
}

int SIP_ENV_print(pSipEnv env)
{
	pSipEnv p = env->next;
	printf("line:%d,env:%p\n",__LINE__,env);
	while(p)
	{
		printf("ENV###%s:%s\n",p->name,p->value);
		p = p->next;
	}
	return 0;
}

#define XML_STATIC_SIZE 256 //use for store the xml

char *xml_get_tagvalue(char *xml_str,char *name)
{
	GB_ASSERT((xml_str && name),"Parse NULL xml str");
	static char xml_buf[XML_STATIC_SIZE];
	sprintf(xml_buf,"%s",xml_str);
	ezxml_t pXML = ezxml_parse_str(xml_buf, strlen(xml_buf));
	ezxml_t pCur = pXML, pChild, pSubChild;	 
	for(; NULL != pCur; pCur = pCur->sibling)
	{
		if(0 == strcmp(name,pCur->name))return pCur->txt;
		for(pChild = pCur->child; NULL != pChild; pChild = pChild->sibling)
		{
			if(0 == strcmp(name,pChild->name))return pChild->txt;
			for(pSubChild = pChild->child; NULL != pSubChild; pSubChild = pSubChild->sibling)
			{
				if(0 == strcmp(name,pSubChild->name))return pSubChild->txt;
			}
		}
	}
	return NULL;
}


char* sip_get_systime(void)
{
	static char buf[64];
	time_t now;
	struct tm *tm_now;
	time(&now);
	tm_now = localtime(&now);	
	strftime(buf,64,"%FT%T",tm_now);
	return buf;
}

int sip_destory(pSipSession s)
{
	s->trigger = FALSE;
	//logout 
	if(s->online)
		s->auth(s,0);
	//clear env	
	SIP_ENV_destroy(s->env);
	//clsose sock
	close(s->sock);
	return 0;
}

int sip_init_session(pSipSession s)
{	
	GB_ASSERT((s),"init_session failed\n");
	s->auth = sip_auth;
	s->pack_msg = sip_pack_msg;
	s->get_subcmd = sip_get_subcmd;
	s->ptz_cmd_parse = sip_ptz_cmd_parse;
	s->get_env = SIP_ENV_get;
	s->get_tagvalue = xml_get_tagvalue;
	s->get_systime = sip_get_systime;	
	s->destroy = sip_destory;
	
	s->sock = create_sock(NULL,atoi(s->get_env(s->env,"DevPort")));
	s->server.sin_family = AF_INET;
	s->server.sin_port = htons(atoi(s->get_env(s->env,"SipServerPort")));
	s->server.sin_addr.s_addr = inet_addr(s->get_env(s->env,"SipServerIp"));

	s->online = 0;
	s->alarm_status = 0;
	s->run_msg_loop = 0;
	s->heartbeat_time = atoi(s->get_env(s->env,"HeartBeatTime"));

	s->trigger = TRUE;
	return 0;
}




