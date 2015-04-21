
// dyndns, noip, 3322

#include "ddns.h"
#include "ddns_util.h"
#include "sysconf.h"
#include "generic.h"

typedef struct DDNS_ENUMTBL
{
	SYS_U64_t val;
	const SYS_CHR_t* text;
}DDNS_ENUMTBL_t;

#define DDNS_CHECKIP_DOMAIN "www.dvr163.com"

static DDNS_ENUMTBL_t ddns_lists[] =
{
	{SYS_DDNS_PROVIDER_DYNDNS, "members.dyndns.org"},
	{SYS_DDNS_PROVIDER_3322, "members.3322.org"},
	{SYS_DDNS_PROVIDER_CHANGEIP, "nic.changeip.com"},
	{SYS_DDNS_PROVIDER_NOIP, "dynupdate.no-ip.com"}, 

};

static DDNS_ENUMTBL_t ddns_provider_enum[] =
{
	{ SYS_DDNS_PROVIDER_DYNDNS, "dyndns" },
	{ SYS_DDNS_PROVIDER_NOIP, "noip" },
	{ SYS_DDNS_PROVIDER_3322, "3322" },
	{ SYS_DDNS_PROVIDER_CHANGEIP, "changeip" },
	{ SYS_DDNS_PROVIDER_POPDVR, "popdvr" },
	{ SYS_DDNS_PROVIDER_SKYBEST, "skybest" },
	{ SYS_DDNS_PROVIDER_DVRTOP, "dvrtop" },
};


static DDNS_ENUMTBL_t update_url[] =
{
	{SYS_DDNS_PROVIDER_DYNDNS, "/nic/update?hostname=%s"},
	{SYS_DDNS_PROVIDER_3322, "/dyndns/update?hostname=%s"},
	{SYS_DDNS_PROVIDER_NOIP, "/nic/update?hostname=%s"},
	{SYS_DDNS_PROVIDER_CHANGEIP, "/nic/update?hostname=%s"}, 
};

#define HTTP_REQUEST_TEMPLATE "GET %s HTTP/1.1\n" \
"Host:%s\n" \
"User-Agent:juan\n" \
"Authorization:Basic %s\n\n\n"

#define DDNS_CHECK_IP_DOMAIN "checkip.dyndns.org"


static const char* ddns_enum(SYS_U32_t val, DDNS_ENUMTBL_t* enu, ssize_t enu_size)
{
	int i = 0;
	for(i = 0; i < enu_size; ++i){
		if(val == enu[i].val){

			return enu[i].text;
		}
	}
	return SYS_NULL;
}

static SYS_U32_t ddns_uenum(const char* text, DDNS_ENUMTBL_t* enu, ssize_t enu_size)
{
	int i = 0;
	for(i = 0; i < enu_size; ++i){
		if(0 == strcmp(text, enu[i].text)){
			return enu[i].val;
		}
	}
	return -1;
}

static int ddns_get_wan_ip(char *wan_ip)
{
	char httpget[] = {
		"GET /addr.php HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Connection: Keep-Alive\r\n"
		"\r\n"
	};
	int sock = 0;
	int ret = 0;
	char sendbuf[512] = {0};
	char recvbuf[1024] = {0};
	char *ip = NULL;

	uint32_t u32_ip_addr = 0;
	GET_HOST_BYNAME(DDNS_CHECKIP_DOMAIN, u32_ip_addr);
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in my_addr;
	bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(80);
	my_addr.sin_addr.s_addr = u32_ip_addr;
	struct timeval timesend = {3, 0};
	struct timeval timerecv = {5, 0};
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timesend, sizeof(timesend));
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timerecv, sizeof(timerecv));
	ret = connect(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
	if(ret == 0)
	{
		//DDNS_TRACE("connect to checkip server:%s ok.", DDNS_CHECKIP_DOMAIN);
		sprintf(sendbuf, httpget, DDNS_CHECKIP_DOMAIN);
		//DDNS_TRACE("%s", sendbuf);
		ret = send(sock, sendbuf, strlen(sendbuf), 0);
		if(ret > 0){
			ret=recv(sock,recvbuf,sizeof(recvbuf),0);
			recvbuf[ret]=0;
			//DDNS_TRACE("update response: %s",recvbuf);
			ip = strstr(recvbuf, "\r\n\r\n");
			if(ip){
				ip += strlen("\r\n\r\n");
				strncpy(wan_ip, ip, 16);
			}
			else{
				ret = -1;
			}
		}
	}
	else{
		DDNS_TRACE("connect to checkip server:%s failed.",DDNS_CHECKIP_DOMAIN);
		close(sock);
		return ret;
	}
	close(sock);
	return 0;
}

static int  make_update_request( const char* register_url,
	const char* username, const char* password,const char provider,char *_out)
{
	char _user[256]={0,};
	char _url[256]={0,};
	char *map_ptr = NULL;
	char _user_base64[256]={0,};
	strcpy(_user, username);
	strcat(_user, ":");
	strcat(_user, password);
	base64_encode(_user, _user_base64, strlen(_user));
	map_ptr = ddns_enum(provider, update_url, ARRAY_ITEM(update_url));
	//sprintf(_url, update_url[provider], register_url);
	sprintf(_url, map_ptr, register_url);
	//int len=sprintf(_out, HTTP_REQUEST_TEMPLATE, _url, ddns_lists[provider], _user_base64);
	map_ptr = ddns_enum(provider, ddns_lists, ARRAY_ITEM(ddns_lists));
	int len=sprintf(_out, HTTP_REQUEST_TEMPLATE, _url, map_ptr, _user_base64);
	DDNS_TRACE("update request: %s", _out);
	return len;
}

static void _process(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url,
	const char* username, const char* password,const char provider)
{
	int ret;
	int sock=0;
	char local_host[20];
	char ddns_host[20];
	char wan_ip[20];
	char *map_ptr = NULL;
	char buf[1024*2]={0,};
	int len=0;
	*stat=DDNS_GET_LOCAL_HOST;
	DDNS_TRACE("start ...");
	//DDNS_TRACE("ddns:%s, host:%s, usr:%s, pwd:%s",ddns_lists[provider],register_url,username,password);
	while(*trigger)
	{
		switch(*stat)
		{
			case DDNS_GET_LOCAL_HOST:
				memset(local_host,0,sizeof(local_host));
				ret=DDNS_get_host(register_url,local_host);
				DDNS_TRACE("get local host:%s - %s",local_host, register_url);
				if(ret==0)
				{
					*stat=DDNS_GET_WAN_IP;
				}
				else{
					*stat=DDNS_SUSPEND;
				}
				break;
			case DDNS_GET_WAN_IP:
				memset(wan_ip,0,sizeof(wan_ip));
				ret=ddns_get_wan_ip(wan_ip);
				DDNS_TRACE("get wan ip:%s",wan_ip);
				if(ret == -1)
				{
					*stat=DDNS_SUSPEND;
				}
				else{
					if(!strcmp(wan_ip,local_host))
					{
						DDNS_TRACE("current wan ip equal to local host,no need update.");
						*stat=DDNS_SUSPEND;
					}
					else{
						DDNS_TRACE("current wan ip differ to local host,goto update.");
						*stat=DDNS_GET_DDNS_HOST;
					}
				}
				break;
			case DDNS_GET_DDNS_HOST:
				memset(ddns_host,0,sizeof(ddns_host));
				map_ptr = ddns_enum(provider, ddns_lists, ARRAY_ITEM(ddns_lists));
				ret=DDNS_get_host(map_ptr,ddns_host);
				DDNS_TRACE("get ddns host:%s",ddns_host);
				if(ret==0)
				{
					*stat=DDNS_TRYING_CONNECT;
				}
				else{
					*stat=DDNS_SUSPEND;
				}
				break;
			case DDNS_TRYING_CONNECT:
				sock = socket(AF_INET, SOCK_STREAM, 0);
				struct sockaddr_in my_addr;
				bzero(&my_addr,sizeof(my_addr));
				my_addr.sin_family = AF_INET;
				my_addr.sin_port = htons(80);
				my_addr.sin_addr.s_addr = inet_addr(ddns_host);
				struct timeval timeo = {3, 0};
				setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
				setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));
				ret = connect(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
				if(ret == 0)
				{
					DDNS_TRACE("connect to ddns:%s ok.",ddns_host);
					*stat=DDNS_UPDATE;
				}
				else{
					DDNS_TRACE("connect to ddns:%s failed.",ddns_host);
					close(sock);
					*stat=DDNS_SUSPEND;
				}
				break;
			case DDNS_UPDATE:
				len=make_update_request(register_url,username,password,provider,buf);
				ret=send(sock,buf,len,0);
				if(ret==len){
					ret=recv(sock,buf,sizeof(buf),0);
					buf[ret]=0;
					DDNS_TRACE("update response: %s",buf);
				}
				close(sock);
				*stat=DDNS_SUSPEND;
				break;
			case DDNS_SUSPEND:
				DDNS_TRACE("wait next time");
				sleep(DDNS_UPDATE_PERIOD);
				*stat=DDNS_GET_LOCAL_HOST;
				break;
			case 	DDNS_SERVICE_INVAILD:
			case DDNS_REGISTER_OK:
			case DDNS_UPDATE_OK:
			case DDNS_UNKNOW_ERROR:
				break;
		}
	}
}


void ddns_dyndns(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url,
	const char* username, const char* password)
{
	_process(trigger,stat,register_url,username,password,SYS_DDNS_PROVIDER_DYNDNS);
}

void ddns_3322(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url,
	const char* username, const char* password)
{
	_process(trigger,stat,register_url,username,password,SYS_DDNS_PROVIDER_3322);
}

void ddns_changeip(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url,
	const char* username, const char* password)
{
	_process(trigger,stat,register_url,username,password,SYS_DDNS_PROVIDER_CHANGEIP);
}

void ddns_noip(uint32_t* trigger, DDNS_STAT_t* stat, const char* register_url,
	const char* username, const char* password)
{
	_process(trigger,stat,register_url,username,password,SYS_DDNS_PROVIDER_NOIP);
}


