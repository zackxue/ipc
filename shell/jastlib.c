#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sock.h"
#include "jastlib.h"
#include "jastsession.h"
#include "vlog.h"

static const char * const g_JastMethod[JAST_METHOD_CNT]=
{
	"HANDSHAKE",
	"HEARTBREAK",
	"STDIN",
	"STDOUT",
	"INVOKE",
	"BYE"
};

inline int HTTP_get_number(char *src,char *key,int *ret)
{
	char *tmp=strstr(src,key);
	if(tmp == NULL){
		*ret=-1;
		return -1;
	}else{
		tmp+=strlen(key);
		if((*tmp) == ' ') tmp++;
		sscanf(tmp,"%d",ret);
		return JAST_RET_OK;
	}
}

int HTTP_get_content(char *src,int in_size,char **out,int *out_size)
{
	int header_size;
	int content_size=0;
	char *tmp=strstr(src,"Content-Length:");
	if(tmp == NULL){
 		return -1;
	}else{
		if((tmp=strstr(src,"\r\n\r\n")) == NULL){
			return -1;
		}
		header_size = tmp-src + strlen("\r\n\r\n");
		*out = tmp + strlen("\r\n\r\n");
		*out_size = in_size - header_size;
		return JAST_RET_OK;
	}
}

static inline int HTTP_get_string(char *src,char *key,char *ret)
{
	char *tmp=strstr(src,key);
	if(tmp == NULL){
		*ret=0;
		return -1;
	}else{
		tmp+=strlen(key);
		if(*tmp == ' ') tmp++;
		sscanf(tmp,"%[^\r\n]",ret);
		return JAST_RET_OK;
	}
}

static int HTTP_get_location(char *src,char *ip,int *port)
{
	char tmp[128];
	if(HTTP_get_string(src,"Location:",tmp) < 0){
		return -1;
	}
	if(sscanf(tmp,"%[^:]:%d",ip,port) == 2)
		return JAST_RET_OK;
	else
		return -1;
}

static int HTTP_get_url(char *src,char *ip,int *port)
{
	char tmp[128];
	if(sscanf(src,"%*s %s %*s",tmp) != 1){
		printf("invalid jast request!\n");
		return -1;
	}
	if(strncmp(tmp,"jast://",strlen("jast://"))!=0){
		printf("invalid jast url!\n");
		return -1;
	}
	sscanf(tmp,"jast://%[^:]:%d",ip,port);
	VLOG(VLOG_DEBUG,"ip:%s port:%d",ip,*port);
	return JAST_RET_OK;
}

static int jast_send(Jast_t *j)
{
	int ret=SOCK_sendto(j->sock,j->ip_dst,j->port_dst,j->payload,j->payload_size);
	if(ret != JAST_RET_OK){
		VLOG(VLOG_ERROR,"jast send packet(size:%d) failed.",j->payload_size);
		return JAST_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"jast send packet(size:%d) ok",j->payload_size);
	VLOG(VLOG_DEBUG2,"send string:%s\n",j->payload);
	return JAST_RET_OK;
}

int JAST_read_message(Jast_t *j,char *ip,int *port)
{
	int ret;
	ret=SOCK_recvfrom(j->sock,ip,port,j->payload,JAST_DEFAULT_PAYLOAD_SIZE);
	if(ret < 0){
		return JAST_RET_FAIL;
	}
	j->payload_size = ret;
	j->payload[ret] = 0;
	
	VLOG(VLOG_DEBUG2,"read string:%s\n",j->payload);
	return JAST_RET_OK;
}

int JAST_parse_response(Jast_t *j,int *code)
{
	int cseq;
	char tmp[256],*ptr=NULL;

	if(memcmp(j->payload,JAST_VERSION,strlen(JAST_VERSION)) == 0){
		sscanf(j->payload,"%*s %d %*s",code);
		VLOG(VLOG_DEBUG,"response code:%d",*code);
		if((*code) == JAST_RSC_UNAUTHORIZED){
			//AUTH_init(&j->auth,HTTP_AUTH_BASIC);
		}else if((*code) != JAST_RSC_OK){
			VLOG(VLOG_ERROR,"response code not ok");
			return JAST_RET_FAIL;
		}
	}else{
		VLOG(VLOG_ERROR,"invalid jast response,not start version:\n%s.\n",j->payload);
		return JAST_RET_FAIL;
	}
	// check cseq
	if(HTTP_get_number(j->payload,"CSeq:",&cseq)==JAST_RET_FAIL){
		VLOG(VLOG_ERROR,"invaild request format,not found cseq");
		return JAST_RET_FAIL;
	}else{
		//if(cseq != j->seq){
		//	VLOG(VLOG_ERROR,"cseq number is wrong");
		//	return JAST_RET_FAIL;
		//}
	}
	// auth
	if(HTTP_get_string(j->payload,"WWW-Authenticate:",tmp)==JAST_RET_OK){
		if(HTTP_AUTH_client_init(&j->auth,tmp)==AUTH_RET_FAIL){
			return JAST_RET_FAIL;
		}
	}
		
	VLOG(VLOG_DEBUG,"parse response success");
	return JAST_RET_OK;
}


static int jast_response_unauthorized(Jast_t *j)
{
	int ret;
	char www_auth[256];
	const char *format=
		"%s %d %s\r\n"\
		"CSeq: %d\r\n"\
		"WWW-Authenticate: %s\r\n"\
		"\r\n";

	if(HTTP_AUTH_chanllenge(j->auth,www_auth,sizeof(www_auth))==AUTH_RET_FAIL){
		return JAST_RET_FAIL;
	}
	sprintf(j->payload,format,JAST_VERSION,
		JAST_RSC_UNAUTHORIZED,
		JAST_RSS_UNAUTHORIZED,
		j->seq,
		www_auth);
	j->payload_size = strlen(j->payload);
	
	ret=jast_send(j);
	return ret;
}


// discovery available devices, client->server
int JAST_request_discovery(int sock,JastDevice_t *devlist)
{
	int ret;
	char buffer[2024];
	int devnum = 0;
	char ip[20];
	int port;
	int comm_port;
	int statuscode= 0;
	const char *format=\
		"DISCOVERY * %s\r\n"\
		"\r\n";
	sprintf(buffer,format,JAST_VERSION);
	// send discovery 
#ifdef JAST_USE_BROADCAST
	ret=SOCK_sendto(sock,"192.168.2.255",JAST_DISCOVERY_SPORT,buffer,strlen(buffer));
#else
	ret=SOCK_sendto(sock,JAST_MULTICAST_ADDR,JAST_DISCOVERY_SPORT,buffer,strlen(buffer));
#endif
	if(ret == JAST_RET_FAIL){
		return JAST_RET_FAIL;
	}
	//
	while(true){
		ret = SOCK_recvfrom(sock,ip,&port,buffer,sizeof(buffer));
		if( ret < 0 ) break;
		buffer[ret]=0;
		VLOG(VLOG_DEBUG,"recv %d ok,recv data:\n%s\n",ret,buffer);
		//
		if(memcmp(buffer,JAST_VERSION,strlen(JAST_VERSION)) == 0){
			sscanf(buffer,"%*s %d %*s",&statuscode);
			if(statuscode != JAST_RSC_OK){
				VLOG(VLOG_ERROR,"something wrong ,please check it");
			}
			if(HTTP_get_location(buffer,ip,&comm_port)!=JAST_RET_OK)
				return JAST_RET_FAIL;
			VLOG(VLOG_INFO,"search dev from %s:%d dataport:%d",ip,port,comm_port);
			strcpy(devlist[devnum].ip,ip);
			devlist[devnum].port = comm_port;
			devnum ++;
		}
	}
	VLOG(VLOG_INFO,"discovery total %d devices",devnum);
	
	return devnum;
}


int JAST_handle_discovery(int sock,char *dst_ip,int dst_port,char *mine_ip,int mine_port)
{
	int ret;
	char buffer[2024];
	const char *format=\
		"%s %d %s\r\n"\
		"Location: %s:%d"\
		"\r\n";
	sprintf(buffer,format,
		JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		mine_ip,mine_port);
	ret=SOCK_sendto(sock,dst_ip,dst_port,buffer,strlen(buffer));
	if(ret == strlen(buffer)) return JAST_RET_FAIL;
	return JAST_RET_OK;
}

int JAST_handle_handshake(Jast_t *j)
{
	int ret;
	const char *format=\
		"%s %d %s\r\n"\
		"CSeq: %d"\
		"\r\n";

	char szAuth[512];
	char *ptr;
	
	if(HTTP_get_string(j->payload,"Authorization:",szAuth)==JAST_RET_FAIL){
		//MSLEEP(500);
		if(jast_response_unauthorized(j)==JAST_RET_FAIL)
			return JAST_RET_FAIL;
		return JAST_RET_OK;
	}else{
		if(HTTP_get_string(j->payload,"Authorization:",szAuth)==JAST_RET_OK){
			j->bLogin = HTTP_AUTH_validate(j->auth,szAuth,"HANDSHAKE");
		}
		if(j->bLogin != TRUE){
			VLOG(VLOG_ERROR,"authorized failed");
			jast_response_unauthorized(j);
			return JAST_RET_FAIL;
		}
	}

	sprintf(j->payload,format,JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		j->seq);
	j->payload_size = strlen(j->payload);

	ret=jast_send(j);

	return ret;
}

/*
int JAST_handle_heartbreak(Jast_t *j)
{
	return JAST_RET_OK;
}
*/
int JAST_handle_bye(Jast_t *j)
{
	int ret;
	const char *format=\
		"%s %d %s\r\n"\
		"CSeq: %d"\
		"\r\n";

	sprintf(j->payload,format,JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		j->seq);
	j->payload_size = strlen(j->payload);

	ret=jast_send(j);
	j->trigger = false;

	//destroy myself
	JAST_session_del(j->ip_dst);
	JAST_destroy(j);

	printf("INFO: recv bye!!!!\n");
	//
	return JAST_RET_FAIL;
}

int JAST_handle_stdout(Jast_t *j)
{
	int ret;
	const char *format=\
		"%s %d %s\r\n"\
		"CSeq: %d"\
		"\r\n";

	sprintf(j->payload,format,JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		j->seq);
	j->payload_size = strlen(j->payload);

	ret=jast_send(j);

	return ret;
}

int JAST_handle_stdin(Jast_t *j)
{
	int ret;
	const char *format=\
		"%s %d %s\r\n"\
		"CSeq: %d"\
		"\r\n";

	sprintf(j->payload,format,JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		j->seq);
	j->payload_size = strlen(j->payload);

	ret=jast_send(j);

	return ret;
}

int JAST_handle_invoke(Jast_t *j)
{
	int ret;
	const char *format=\
		"%s %d %s\r\n"\
		"CSeq: %d"\
		"\r\n";

	sprintf(j->payload,format,JAST_VERSION,JAST_RSC_OK,JAST_RSS_OK,
		j->seq);
	j->payload_size = strlen(j->payload);

	ret=jast_send(j);

	return ret;
}

/***************************************************************
* function: only use for client  to setup session with server(device)
***************************************************************/
int JAST_request_handshake_ex(Jast_t *j)
{
	char ip[20];
	char szAuth[512];
	int port;
	int code;
	const char *format=\
		"HANDSHAKE jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"Authorization: %s\r\n"\
		"\r\n";

	char url[128];
	sprintf(url,"jast://%s:%d",j->ip_dst,j->port_dst);
	if(HTTP_AUTH_setup(j->auth,j->user,j->pwd,url,"HANDSHAKE",szAuth,sizeof(szAuth))==AUTH_RET_FAIL){
		return JAST_RET_FAIL;
	}
	sprintf(j->payload,format,j->ip_dst,j->port_dst,JAST_VERSION,
		++j->seq,
		szAuth);

	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;

	j->bLogin = true;

	return JAST_RET_OK;
}


int JAST_request_handshake(Jast_t *j)
{
	int ret = false;
	char ip[20];
	int port;
	int code;
	const char *format=\
		"HANDSHAKE jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"\r\n";
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	if(code == JAST_RSC_UNAUTHORIZED){
		ret=JAST_request_handshake_ex(j);	
		if(ret == JAST_RET_OK){
			MilliTimerStart(j->hb_timer);
		}
	}
	
	VLOG(VLOG_DEBUG,"handshake success!\n");
	return JAST_RET_OK;
}




/***************************************************************
* function: only use for server(device) ,send heartbreak to client
* note: it's shouldn't give a response
***************************************************************/
int JAST_send_heartbreak(Jast_t *j)
{
	char szTime[64];
	const char *format=\
		"HEARTBREAK jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"Time: %s\r\n"\
		"\r\n";
	
	time_t t;
	time(&t);
	strftime(szTime, sizeof(szTime), "%a, %b %d %Y %H:%M:%S GMT", localtime(&t));
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq,szTime);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	//if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
	//	return JAST_RET_FAIL;
	//
	//if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
	//	return JAST_RET_FAIL;

	return JAST_RET_OK;
}

/***************************************************************
* function: only use for server(device) ,send STDOOUT to client
* note: it's shouldn't give a response
***************************************************************/
int JAST_send_stdout(Jast_t *j,char *msg,int len)
{
	const char *format=\
		"STDOUT jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"Content-Length: %d\r\n"\
		"\r\n"\
		"%s";
	
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq,len,msg);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	
	return JAST_RET_OK;
}


/***************************************************************
* function: only use for client,send bye to teardown the existed session
***************************************************************/
int JAST_request_bye(Jast_t *j)
{
	const char *format=\
		"BYE jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"\r\n";
	
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	// no response
	/*
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	*/
	return JAST_RET_OK;
}

int JAST_request_stdout(Jast_t *j)
{
	char ip[20];
	int port,code;
	const char *format=\
		"STDOUT jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"\r\n";
	
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;

	return JAST_RET_OK;
}

int JAST_request_stdin(Jast_t *j,char *input)
{
	char ip[20];
	int port,code;
	const char *format=\
		"STDIN jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"Content-Length: %d\r\n"\
		"\r\n"\
		"%s";
	
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq,strlen(input),input);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;

	return JAST_RET_OK;

}

int JAST_request_invoke(Jast_t *j,char *command)
{
	char ip[20];
	int port,code;
	const char *format=\
		"INVOKE jast://%s:%d %s\r\n"\
		"CSeq: %d\r\n"\
		"Content-Length: %d\r\n"\
		"\r\n"\
		"%s";
	
	sprintf(j->payload,format,
		j->ip_dst,j->port_dst,JAST_VERSION,++j->seq,strlen(command),command);
	j->payload_size=strlen(j->payload);
	VLOG(VLOG_DEBUG,"request (size:%d):\r\n%s\r\n",j->payload_size,j->payload);
	// send request
	if(jast_send(j) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	// recv ack
	if(JAST_read_message(j,ip,&port) == JAST_RET_FAIL)
		return JAST_RET_FAIL;
	//
	if(JAST_parse_response(j,&code) == JAST_RET_FAIL)
		return JAST_RET_FAIL;

	return JAST_RET_OK;

}



int JAST_parse_request(Jast_t *j,char *ip,int port)
{
	int ret;
	int i;
	//char ip[20];
	//int port;
	for(i=0;i<JAST_METHOD_CNT;i++){
		if(memcmp(j->payload,g_JastMethod[i],strlen(g_JastMethod[i])) == 0){
			break;
		}
	}
	if(i == JAST_METHOD_CNT){
		VLOG(VLOG_ERROR,"unknow method:");
		return JAST_RET_FAIL;
	}
	if(HTTP_get_url(j->payload,j->ip_me,&j->port_me)==JAST_RET_FAIL){
		return JAST_RET_FAIL;
	}
	if(HTTP_get_number(j->payload,"CSeq:",&j->seq)==-1){
		VLOG(VLOG_ERROR,"invaild request format,not found cseq");
		return JAST_RET_FAIL;
	}
	VLOG(VLOG_DEBUG,"jast request %d <<%s>>",i,g_JastMethod[i]);
	switch(i){
		case JAST_METHOD_HANDSHAKE:
			j->port_dst = port;
			ret=JAST_handle_handshake(j);
			break;
		//case JAST_METHOD_HEARTBREAK:
		//	break;
		case JAST_METHOD_STDIN:
			ret=JAST_handle_stdin(j);
			break;
		case JAST_METHOD_STDOUT:
			ret=JAST_handle_stdout(j);
			break;
		case JAST_METHOD_INVOKE:
			ret=JAST_handle_invoke(j);
			break;
		case JAST_METHOD_BYE:
			ret=JAST_handle_bye(j);
			break;
		default:
			VLOG(VLOG_ERROR,"impossible method %d",i);
			return JAST_RET_FAIL;
	}
	
	return ret;
}

Jast_t *JAST_server_init(char *dst_ip,int port_mine)
{
	int sock;
	Jast_t *jast=(Jast_t *)malloc(sizeof(Jast_t));
	if(jast == NULL){
		VLOG(VLOG_ERROR,"malloc for jast failed!");
		return NULL;
	}
	memset(jast,0,sizeof(Jast_t));
	jast->role = JAST_ROLE_SERVER;
	jast->trigger = true;
	jast->seq = 0;
	// transport
	strcpy(jast->ip_dst,dst_ip);
	jast->port_me = port_mine;
	sock=SOCK_udp_init(port_mine,JAST_SOCK_TIMEOUT);
	if(sock < 0)
		return NULL;
	jast->sock = sock;
	// authentication
	strcpy(jast->user,JAST_DEFAULT_USER);
	strcpy(jast->pwd,JAST_DEFAULT_PWD);
	HTTP_AUTH_server_init(&jast->auth,HTTP_AUTH_DEFAULT_TYPE);
	jast->bLogin = false;
	// init timer
	MilliTimerSet(jast->hb_timer,0,0);
	//
	jast->remotelog = false;
	
	return jast;
}

Jast_t *JAST_client_init(char *dst_ip,int dst_port,int port_mine)
{
	int sock;
	Jast_t *jast=(Jast_t *)malloc(sizeof(Jast_t));
	if(jast == NULL){
		VLOG(VLOG_ERROR,"malloc for jast failed!");
		return NULL;
	}
	memset(jast,0,sizeof(Jast_t));
	jast->role = JAST_ROLE_SERVER;
	jast->trigger = true;
	jast->seq = 0;
	// transport
	strcpy(jast->ip_dst,dst_ip);
	jast->port_me = port_mine;
	jast->port_dst = dst_port;
	sock=SOCK_udp_init(port_mine,JAST_SOCK_TIMEOUT);
	if(sock < 0)
		return NULL;
	jast->sock = sock;
	// authentication
	strcpy(jast->user,JAST_DEFAULT_USER);
	strcpy(jast->pwd,JAST_DEFAULT_PWD);
	jast->auth = NULL;
	jast->bLogin = false;
	// init timer
	MilliTimerSet(jast->hb_timer,0,0);
	//
	jast->remotelog = false;
	
	return jast;
}


int JAST_destroy(Jast_t *j)
{
	if(j){
		if(j->auth){
			HTTP_AUTH_destroy(j->auth);
		}
		if(j->sock) SOCK_close(j->sock);
		free(j);
	}
	return JAST_RET_OK;
}

