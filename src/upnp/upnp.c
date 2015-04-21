
/******************************************************************************

  Copyright (C), 2001-2012, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name     : upnp.c
  Version       : Initial Draft
  Author        : kaga
  Created       : 2012/03/01
  Last Modified : 2012/03/22
  Description   : upnp communication utils
 
  History       : 
  1.Date        : 2012/03/09
    Author      : kaga
 	Modification: Created file
  2.Date        : 2012/03/21
    Author      : kaga
 	Modification: 1>>to fix some router must send request by one packet
 					change send request method from multi packet to one packet
 				2>>add function to judge the router response in a packet or not
 				3>>add function to print long string (bigger than about 512)
 				4>>fix parse xml function,to consider of spaces in beginning of line
 				5>>fix parse xml function,to ignore the case of tag name
 				6>>and function to get Response header's length
 				7>>change M-Search sending from anywhere to specifed gateway when multi
 					upnp services in local network
  3.Date		:2012/03/22
 	Auther		:kaga
 	Modification:1>>remove udp connect to fix none response for M-Search which router
 					response port isn't 1900
 				2>>fix handle of state of UPNP_STATE_PARSE_RESPONSE_MAKESURE_DPORTMAP
	
******************************************************************************/

#include "upnp_util.h"
#include "upnp_debug.h"
#include "upnp.h"

#define IPPORT_UPNP		1900
#define IPPORT_UDP_SRC		53200

enum{
	CONTROL_TYPE_GET_STATUS,
	CONTROL_TYPE_GET_EXTERNAL_IP,
	CONTROL_TYPE_ADD_PORTMAP,
	CONTROL_TYPE_GET_SPECIFIED_PORTMAP,
	CONTROL_TYPE_END
};


/*********************************************************************************
Local Const Variable Declaration Section
*********************************************************************************/
const char *const ResponseOK[]={
	"HTTP/1.1 200 OK",
	"HTTP/1.0 200 OK"
	};
const char * const ResponseServerErr[]={
	"HTTP/1.1 500 Internal Server Error",
	"HTTP/1.0 500 Internal Server Error"
	};
const char sContentLen[]="CONTENT-LENGTH:";

const char MSearchMsgFmt[] = 
"M-SEARCH * HTTP/1.1\r\n"
"HOST: %d.%d.%d.%d:%d\r\n"
"ST: %s\r\n"
"MAN: \"ssdp:discover\"\r\n"
"MX: %u\r\n"
"\r\n";

const char * const deviceList[] = {
	"urn:schemas-upnp-org:device:InternetGatewayDevice:1",
	"urn:schemas-upnp-org:service:WANIPConnection:1",
	"urn:schemas-upnp-org:service:WANPPPConnection:1",
	"upnp:rootdevice",
	0
};

#define suffix	'<'
const char serverTypePrefix[]="<serviceType>";
const char specifiedServer[]="urn:schemas-upnp-org:service:WANIPConnection:1";
const char serverTypeSuffix='<';
const char serverIdPrefix[]="serviceId:";
const char serverIdSuffix='<';
const char specifiedServerId[]="WANIPConnection";
const char controlUrlPrefix[]="<controlURL>";
const char controlUrlSuffix='<';
const char sInternalPort[]="<NewInternalPort>";
const char sInternalIp[]="<NewInternalClient>";
const char sPortEnable[]="<NewEnabled>";
const char sInternalIP[]="<NewInternalClient>";
const char sEnabled[]="<NewEnabled>";
const char sWanIP[]="<NewExternalIPAddress>";
const char sErrorCode[]="<errorCode>";
const char sErrorDescription[]="<errorDescription>";
const char ErrorDescription[]={
	"Invalid ExternalPort",
};



const char *const controlCmd[]={
	"GetStatusInfo",
	"GetExternalIPAddress",
	"AddPortMapping",
	"GetSpecificPortMappingEntry",
	0,
};

const char sReqControl[]=
"POST %s HTTP/1.1\r\n"	//control url
"Host: %s:%d\r\n"//ip[4],port
"User-Agent: MSWindows/6.1.7601, UPnP/1.0, MiniUPnPc/1.6\r\n"
"Content-Length: %d\r\n"	//content length
"Content-Type: text/xml\r\n"
"SOAPAction: \"urn:schemas-upnp-org:service:WANIPConnection:1#%s\"\r\n"//controlCmd
"Connection: Close\r\n"
"Cache-Control: no-cache\r\n"

"Pragma: no-cache\r\n"
"\r\n";

const char sControlXml[512]=
	"<?xml version=\"1.0\"?>\r\n"
	"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
	"s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">";
	//"<s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"//command 
	//"%s</u:%s></s:Body></s:Envelope>";//parameter,command
const char sControlXmlSuffix[]="</s:Body></s:Envelope>\r\n";

const char *const sProtocal[]={
	[PROTOCAL_TCP]="TCP",
	[PROTOCAL_UDP]="UDP",
	0
};
const char *const sControlParam[]={
	"<s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"//command 
	"</u:%s>",//command 
		"<s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"//command 
		"</u:%s>",//command 
		//add port map
		"<s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"//command 
		"<NewRemoteHost></NewRemoteHost>"
		"<NewExternalPort>%d</NewExternalPort>"//external port
		"<NewProtocol>%s</NewProtocol>"			//protocal
		"<NewInternalPort>%d</NewInternalPort>",	//internal port

		//query specified port map
		"<s:Body><u:%s xmlns:u=\"urn:schemas-upnp-org:service:WANIPConnection:1\">"//command 
		"<NewRemoteHost></NewRemoteHost>"
		"<NewExternalPort>%d</NewExternalPort>"	//external port
		"<NewProtocol>%s</NewProtocol>"//protocal
		"</u:%s>",	//command 
};

const uint8_t sControlParam_AddPortMap[]=
		"<NewInternalClient>%s</NewInternalClient>"	//ip[4]-->%s
		"<NewEnabled>1</NewEnabled>"
		"<NewPortMappingDescription>IPCAM UPNP</NewPortMappingDescription>"
		"<NewLeaseDuration>0</NewLeaseDuration>"
		"</u:%s>";//command 

const uint8_t upnp_multicast_ip[4]={239,255,255,250};

////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////global variable/////////////////////////////////////////////////

typedef struct UPNP_CLIENT
{
	UPNP_PARA_t para;
	int sock_udp;
	int sock_tcp;
	uint16_t server_port;	
	uint8_t state;
	uint8_t curStep;	//用于标记多个端口映射的序列
	uint8_t sLocation[64];
	uint8_t sControlUrl[64];
	uint8_t sST[128];
	uint8_t isMapped;
	pthread_t tid;
	uint32_t trigger;
	uint32_t started;

	UPNP_CONTEXT_t context;
}UPNP_t;
static UPNP_t _upnp =
{
	.trigger = false,
};
static UPNP_t* _p_upnp = NULL;

/*********************************************************************************
Function Implementation Part
****************************************************************************/
/*********************************************************************************
* Description : Initial external ports
* Arguments   : none
* Returns	  : 
* Note	 : 
* rule:  1>   if inPort < 1024 , mapping to EXPORT_MIN~EXPORT_MAX
		2> if inPort >1024 , mapping itself
		3> if protocal is both UDP & TCP, the mapping export is identical
*********************************************************************************/
static void initExternalPorts()
{
	int i=0;
	for(i=0;i<_p_upnp->para.num;i++)
	{
		if(_p_upnp->para.port_maps[i].exPort==0)
		{
			if(_p_upnp->para.port_maps[i].inPort<1024)
			{
				_p_upnp->para.port_maps[i].exPort=EXPORT_MIN;
			}

			else{
				_p_upnp->para.port_maps[i].exPort=_p_upnp->para.port_maps[i].inPort;
			}
		}
		if(_p_upnp->para.port_maps[i].protocal==PROTOCAL_TCP_UDP)
		{
			if((_p_upnp->para.num+1)>MAX_PORT_MAP)
			{
				UPNP_TRACE("max port maps.");
			}
			else{
				_p_upnp->para.port_maps[i].protocal=PROTOCAL_TCP;
				memcpy(&_p_upnp->para.port_maps[_p_upnp->para.num],&_p_upnp->para.port_maps[i],sizeof(stPortMap));
				_p_upnp->para.port_maps[_p_upnp->para.num].protocal=PROTOCAL_UDP;
				_p_upnp->para.num++;
			}
		}
	}
}
/*********************************************************************************
* Description : Get max external port
* Arguments   : none
* Returns     : the max external port num
* Note        : 
*********************************************************************************/
static uint16_t GetMaxExternalPort(void)
{
	uint8_t i;
	uint32_t maxport=0;
	uint32_t tmp=0;
	for(i=0;i<_p_upnp->para.num;i++)
	{
		tmp=_p_upnp->para.port_maps[i].exPort;
		if((tmp>maxport) /*&& (_p_upnp->para.port_maps[i].isMapped==true)*/)
		{
			maxport=tmp;
		}
	}
	if(maxport<EXPORT_MIN) maxport=EXPORT_MIN;
	return maxport;
}

/*********************************************************************************
* Description : Get specific step's internal port
* Arguments   : step - which step
* Returns     : specific step's internal port
* Note        : 
*********************************************************************************/
static inline uint16_t GetInternalPort(uint8_t step)
{
	if(step>=_p_upnp->para.num)
	{
		return false;
	}
	return _p_upnp->para.port_maps[step].inPort;
}
/*********************************************************************************
* Description : Get specific step's external port
* Arguments   : step - which step
* Returns     : specific step's external port
* Note        : 
*********************************************************************************/
static inline uint16_t GetExternalPort(uint8_t step)
{
	if(step>=_p_upnp->para.num)
	{
		return false;
	}
	return _p_upnp->para.port_maps[step].exPort;
}
/*********************************************************************************
* Description : Get specific step's protocal index
* Arguments   : step - which step
* Returns     : specific step's protocal
* Note        : 
*********************************************************************************/
static inline uint16_t GetStepProtocal(uint8_t step)
{
	if(step>=_p_upnp->para.num)
	{
		return false;
	}
	return _p_upnp->para.port_maps[step].protocal;
}
/*********************************************************************************
* Description : Set specific step's external port
* Arguments   : step - which step 
* 				value - value setted to external port 
* Returns     : specific step's internal port
* Note        : 
*********************************************************************************/
static inline uint32_t SetExternalPort(uint16_t step,uint16_t value)
{
	if(step>=_p_upnp->para.num)
	{
		return false;
	}
	if(value>EXPORT_MAX)
	{
		value=EXPORT_MIN;
	}
	else if(value<EXPORT_MIN)
	{
		value=EXPORT_MIN;
	}
	_p_upnp->para.port_maps[step].exPort=value;
	//_p_upnp->para.port_maps[step].isMapped=true;

	return true;
}

/*********************************************************************************
* Description : udp init function
* Arguments   : 
* Returns     : 
* Note        : 
*********************************************************************************/
static int UPNPUdpInit()
{
	int ret;
	_p_upnp->sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	if(_p_upnp->sock_udp<0){
		_p_upnp->sock_udp=0;
		UPNP_TRACE("create udp socket failed.\n");
		return -1;
	}
	//set send timeout
	struct timeval timeo = {UPNP_READ_TIMEOUT, 0};
	ret=setsockopt(_p_upnp->sock_udp, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
	UPNP_ASSERT(ret>=0,"set send timeout failed.\n");
	//set receive timeout
	ret=setsockopt(_p_upnp->sock_udp,SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(timeo));
	UPNP_ASSERT(ret>=0,"set receive timeout failed.\n");
	struct sockaddr_in my_addr;
	bzero(&my_addr,sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(IPPORT_UDP_SRC);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	//bind 
	ret = bind(_p_upnp->sock_udp, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
	UPNP_ASSERT(ret>=0,"bind failed.\n");
	UPNP_TRACE("create udp sock ok.\n");

	return 0;
	
}
/*********************************************************************************
* Description : tcp connection function
* Arguments   : 
* Returns     : 
* Note        : 
*********************************************************************************/
static int UPNPTcpConnect(char *ip,uint16_t port)
{
	int ret;

	_p_upnp->sock_tcp= socket(AF_INET, SOCK_STREAM, 0);
	if(_p_upnp->sock_tcp<0){
		UPNP_TRACE("create tcp socket failed.\n");
		_p_upnp->sock_tcp=0;
		return -1;
	}
	//set send timeout
	struct timeval timeo = {UPNP_READ_TIMEOUT, 0};
	ret=setsockopt(_p_upnp->sock_tcp, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
	UPNP_ASSERT(ret>=0,"set send timeout failed.\n");
	//set receive timeout
	ret=setsockopt(_p_upnp->sock_tcp,SOL_SOCKET,SO_RCVTIMEO,&timeo,sizeof(timeo));
	UPNP_ASSERT(ret>=0,"set receive timeout failed.\n");

	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=inet_addr(ip);
	ret= connect(_p_upnp->sock_tcp, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if (ret<0)
	{
		UPNP_TRACE("connect failed @ errno=%d",errno);
	}
	else{
		UPNP_TRACE("connect ok");
	}
	return ret;
	
}

static int tcp_recv(int fd,char *buf,uint32_t size)
{
	int ret=0;
	int received=0;
	char *pbuf=buf;
	while(1)
	{	
		#ifdef DEBUG_UPNP
		printf(".");
		#endif
		ret=recv(fd,pbuf,size-received,0);
		if(ret==-1)
		{
			close(fd);
			if(errno==EAGAIN || errno==EINTR)
			{
				UPNP_TRACE("######## tcp recv error ############");
				continue;
			}
			else if(errno==ETIMEDOUT)
			{
				UPNP_TRACE("######## tcp recv time out ##########");
				return 0;
			}
			UPNP_TRACE("####### tcp recv error @%d ##############",errno);
			return -1;
		}
		else if(ret==0)
		{
			break;
		}
		else{
			pbuf+=ret;
			received+=ret;
		}
	}
	buf[received]=0;
	UPNP_TRACE("tcp recv %d:\n",received);
#ifdef DEBUG_UPNP
	printf("%s.\n",buf);
#endif
	close(fd);
	return received;
}


/*********************************************************************************
* Description : Parse the reply xml of command of "GetSepecificPortMappingEntry" 
* 				and extract internal ip,internal port enabled
* Arguments   : reply - [IN],the reply xml string 
*  				ip - [OUT],internal ip string
*  				port - [OUT],internal port
*  				enabled - [OUT],is enabled or not
* Returns     : number of found tags
* Note        : 
*********************************************************************************/
static uint8_t ParseGetSpecifiecPortMapping(uint8_t *reply,uint16_t len,uint8_t *ip,uint32_t *port,uint8_t *enabled)
{
	uint16_t i=0;
	uint16_t j=0;
	uint8_t tmp[20];
	uint8_t ret=0;
	//UPNP_TRACE("str:%s\r\n",reply);
	while(i<(len-1))
	{
		if((reply[i]=='<') && (reply[i+1]!='/'))
		{
			//get internal ip
			if(!strncasecmp(reply+i,sInternalIP,strlen(sInternalIP)))
			{
				i+=strlen(sInternalIP);
				j=0;
				while(reply[i]!='<') 
				{
					if(reply[i]!=' ')
					{
						tmp[j++]=reply[i];
					}
					i++;
				}
				tmp[j]='\0';
				ret++;
				UPNP_TRACE("Internal IP:%s.\r\n",tmp);
				strcpy(ip,tmp);
			}
			//get internal port
			else if(!strncasecmp(reply+i,sInternalPort,strlen(sInternalPort)))
			{
				i+=strlen(sInternalPort);
				j=0;
				while(reply[i]!='<') 
				{
					tmp[j++]=reply[i++];
				}
				tmp[j]='\0';

				ret++;
				*port=atoi(tmp);	
				UPNP_TRACE("Internal Port:%s,%d.\r\n",tmp,*port);
			}
			//get enabled
			else if(!strncasecmp(reply+i,sEnabled,strlen(sEnabled)))
			{
				i+=strlen(sEnabled);
				j=0;
				while(reply[i]!='<') 
				{
					tmp[j++]=reply[i++];
				}
				tmp[j]='\0';

				ret++;
				*enabled=atoi(tmp);			
				UPNP_TRACE("Enabled:%s,%d.\r\n",tmp,*enabled);
			}
		}
		i++;
	}
	return ret;
}
/*********************************************************************************
* Description : parse reply xml string and judge which the specific external are 
* 				mapped to mine ip and port 
* Arguments   : reply - [IN] ,the reply xml string by send command of "GetSepecificPortMappingEntry" 
*  				len - string size
* Returns     : true - yes , FAIL - NOT
* Note        : 
*********************************************************************************/

static uint8_t isSpecificPortMappingMe(uint8_t *reply,uint16_t len)
{
	uint8_t ip[20];
	uint32_t port;
	uint8_t enabled;
	ParseGetSpecifiecPortMapping(reply,len,ip,&port,&enabled);
	if(strcmp(ip,_p_upnp->para.ip_me))
	{
		UPNP_TRACE("is not my ip\r\n");
		_p_upnp->para.port_maps[_p_upnp->curStep].isMapped=false;
		return false;
	}
	else if(port!=GetInternalPort(_p_upnp->curStep))//sysenv.net.port)		//MODIFIED HERE
	{
		UPNP_TRACE("is not my port\r\n");
		_p_upnp->para.port_maps[_p_upnp->curStep].isMapped=false;
		return false;
	}
	else if(enabled!=1)
	{
		UPNP_TRACE("is not enabled\r\n");
		_p_upnp->para.port_maps[_p_upnp->curStep].isMapped=false;
		return false;
	}
	UPNP_TRACE("UPNP success:[%d]<<->>[%d]",GetInternalPort(_p_upnp->curStep),GetExternalPort(_p_upnp->curStep));
	_p_upnp->para.port_maps[_p_upnp->curStep].isMapped=true;
	return true;
}

/*********************************************************************************
* Description : parse response of M-Search Request,and extract the tag value of 
* 				"LOCATION" and "ST" 
* Arguments   : reply - [IN] response string of M-Search Request 
*  				size - [IN] string size
*  				location - [OUT], the tag value of "LOCATION"
*  				st - [OUT], the tag value of "ST"
*  				port - [OUT],the commution port
* Returns     : none
* Note        : 
*********************************************************************************/
static void parseMSEARCHReply(char * reply, int size,char *location,char *st,uint16_t *port)
{
	UPNP_TRACE("%s:\r\n",__FUNCTION__);
	char tmp[10];
	int a, b, i,j;
	j = 0;
	i = 0;
	a = i;	/* start of the line */
	b = 0;	/* end of the "header" (position of the colon) */
	while(i<size)
	{
		switch(reply[i])
		{
		case ':':
				if(b==0)
				{
					b = i; /* end of the "header" */
				}
				break;
		case '\r':
		case '\n':
				if(b!=0)
				{
					/* skip the colon and white spaces */
					do { b++; } while(reply[b]==' ');
					if(0==strncasecmp(reply+a, "LOCATION", 8))
					{
						//if start with http
						if(0==strncasecmp(reply+b,"http",4))
						{

							b+=4;

							if(reply[b]==' ')
							{
								b++;
							}
							b+=3;	//jump over "://"
							//do{b++;} while((reply[b]==' ') || (reply[b]=='/') || (reply[b]==':'));
						}
						while(reply[b]!=':')
						{
							b++;
						}
						if(reply[b]==':')
						{
							b++;
							j = 0;
						}
						do{
							tmp[j++]=reply[b];
							b++;
						} while(reply[b]!='/');
						tmp[j]=0;
						*port=atoi(tmp);
						UPNP_TRACE("PORT:%s##%d##\r\n",tmp,*port);
						//*location = reply+b;
						//*locationsize = i-b;
						memcpy(location,reply+b,i-b);
						location[i-b]=0;
						UPNP_TRACE("###LOC:%s##\r\n",location);
					}
					else if(0==strncasecmp(reply+a, "ST", 2))
					{
						//*st = reply+b;
						//*stsize = i-b;
						memcpy(st,reply+b,i-b);
						st[i-b]=0;
						UPNP_TRACE("###ST:%s##\r\n",st);
					}
					b = 0;
				}
				a = i+1;
				break;
		default:
				break;
		}
		i++;
	}
}


/*********************************************************************************
* Description : parse the request , then get wan ip
* Arguments   : reply - [IN] ,response string 
* 				len - [IN] ,string size 
* Returns     : 0 for this string is fragment of header,else for header length 
* Note        : 
*********************************************************************************/
static int GetWanIP(uint8_t *reply,uint16_t len)
{
	int i=0;
	uint8_t *p=NULL;
	while(i<len)
	{
		if(reply[i]=='<')
		{
			p=(reply+i);
			if(!strncasecmp(p,sWanIP,strlen(sWanIP)))
			{
				p+=strlen(sWanIP);
				sscanf(p,"%[^<]",_p_upnp->para.ip_wan);
				UPNP_TRACE("UPNP WAN IP: %s",_p_upnp->para.ip_wan);
				return 0;
			}
		}
		i++;
	}
	return -1;
}


/*********************************************************************************
* Description : parse http response to judge which the response is ok or not
* Arguments   : reply - [IN] ,http response
* Returns     : 
* Note        : surport http/1.1 or http/1.0
*********************************************************************************/
static inline uint8_t isHttpResponseOK(uint8_t* reply)
{
	if(0==strncasecmp(reply,ResponseOK[0],strlen(ResponseOK[0])))
	{
		return true;
	}
	else if(0==strncasecmp(reply,ResponseOK[1],strlen(ResponseOK[1])))
	{
		return true;
	}
	return false;
}

/*********************************************************************************
* Description : parse http response is "Internal Server Error" or not
* Arguments   : reply - [IN] ,http response
* Returns     : 
* Note        : surport http/1.1 or http/1.0
*********************************************************************************/
static inline uint8_t isHttpResponseInterServerErr(uint8_t* reply)
{
	if(0==strncasecmp(reply,ResponseServerErr[0],strlen(ResponseServerErr[0])))
	{
		return true;
	}
	if(0==strncasecmp(reply,ResponseServerErr[1],strlen(ResponseServerErr[1])))
	{
		return true;
	}
	return false;
}



/*********************************************************************************
* Description : parse upnp description xml string ,and extract the ControlURL,which 
* 			it's service type is "urn:schemas-upnp-org:service:WANIPConnection:1" 
* Arguments   : url - [OUT],the string of ControlURL
			msg- [IN],the xml string
			len- [IN] ,xml size
* Returns     : 0 for success,else for fail
* Note        : The description xml string are stored in DDR
*********************************************************************************/
static uint8_t GetControlUrl(char *url,char *msg,uint16_t len)
{
	uint16_t i=0;//total string index
	uint8_t prevChar=0xff;	//previous char
	uint16_t a=i;	//start of line
	uint16_t b=0;	//index of closing tag
	uint8_t flag=false;	//if the server type is specified server type
	
	while(i<len)
	{
		if(msg[i]=='\n')	//end of line
		{
			a=i+1;
		}
		else if((msg[i]==' ') && (a==i))//space in the start of the line
		{
			a++;
		}
		else if((prevChar=='<') && (msg[i]=='/'))// closing tag
		{
			b=i-1;
			
			if(0==strncasecmp(msg+a,serverTypePrefix,strlen(serverTypePrefix)))
			{
				if(0==strncasecmp(msg+a+strlen(serverTypePrefix),
					specifiedServer,strlen(specifiedServer)))
				{
					UPNP_TRACE("\r\n======T==========\r\n");
					flag=true;
				}
				else{
					UPNP_TRACE("\r\n==========F===========\r\n");
					flag=false;
				}
			}
			else if(0==strncasecmp(msg+a,controlUrlPrefix,strlen(controlUrlPrefix)))
			{
				if(flag)
				{
					memcpy(url,msg+a+strlen(controlUrlPrefix),b-a-strlen(controlUrlPrefix));
					url[b-a-strlen(controlUrlPrefix)]=0;
					UPNP_TRACE("\r\n\r\n________________URL:%s_________________\r\n\r\n",url);
					return 0;
				}
			}
		}
		
		prevChar=msg[i];
		i++;
	}
	return 1;
}

/*********************************************************************************
* Description : make and send a broadcast to find any UPNP device by command of M-Search
* Arguments   : none
* Returns     : 0 -- true
			-1 -- false
* Note        : protocal - udp 
*  				ip - 239.255.255.250
*  				port - 1900
*********************************************************************************/
static int UPNP_discovery(void)
{
	int len;
	uint8_t buf[1024];
	uint8_t buff[5*1024];
	memset(buf,0,sizeof(buf));
	len=sprintf(buf,MSearchMsgFmt,239,255,255,250,IPPORT_UPNP,deviceList[0],2);
	UPNP_TRACE("make broadcast@len=%d\r\n",len);
	#ifdef DEBUG_UPNP
		printf("%s",buf);
	#endif
	
	struct sockaddr_in addr;
	bzero(&addr,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(IPPORT_UPNP);
	addr.sin_addr.s_addr=inet_addr(_p_upnp->para.ip_gw);
	int ret=sendto(_p_upnp->sock_udp,buf,len,0,(struct sockaddr *)&addr, sizeof(struct sockaddr));
	if(ret==len){
		UPNP_TRACE("udp@%d send discovery to %s ok.\n", _p_upnp->sock_udp,_p_upnp->para.ip_gw);
	}
	else{
		UPNP_TRACE("udp@%d send discovery to %s failed.\n",_p_upnp->sock_udp, _p_upnp->para.ip_gw);
		return -1;
	}
	struct sockaddr_in dst;
	bzero(&dst,sizeof(dst));
	int dst_len=0;
	ret=recvfrom(_p_upnp->sock_udp,buff,sizeof(buff),0,(struct sockaddr *)&dst,&dst_len);
	if(ret>0)
	{
		UPNP_TRACE("udp receive:%d",ret);
		buff[ret]=0;
		#ifdef DEBUG_UPNP
			printf("%s",buff);
		#endif
		parseMSEARCHReply(buff,ret,_p_upnp->sLocation,_p_upnp->sST,&_p_upnp->server_port);
		if(!strcmp(_p_upnp->sST,deviceList[0]))
		{
			UPNP_TRACE("get location success:%s,server:%s:%d\r\n",_p_upnp->sLocation,_p_upnp->para.ip_gw,_p_upnp->server_port);
		}
		else{
			UPNP_TRACE("get location false.\r\n");
			return -1;
		}
	}
	else{
		UPNP_TRACE("udp recvfrom @ret=%d",ret);
		return -1;
	}
	return 0;
}

/*********************************************************************************
* Description : after get upnp device loction,send a request to get the device 
* 				description formatted by xml 
* Arguments   : 
* Returns     : 
* Note        : 
*********************************************************************************/
static int RequestDeviceXML(void)
{
	char buf[1024];
	int len=0;
	memset(buf,0,sizeof(buf));
	
	len=sprintf(buf,
		"GET %s HTTP/1.1\r\n"
		"Host: %s:%d\r\n"//4 %d ->%s
		"Connection: Close\r\n"
		"User-Agent: MSWindows/6.1.7601, UPnP/1.0, MiniUPnPc/1.6\r\n"
		"\r\n",
		_p_upnp->sLocation,_p_upnp->para.ip_gw,_p_upnp->server_port);

	UPNP_TRACE("MAKE REQUEST @size=%d \r\n%s",len, buf);
	#ifdef DEBUG_UPNP
		printf("%s",buf);
	#endif

	int ret=send(_p_upnp->sock_tcp,buf,len,0);
	if(ret==len){
		UPNP_TRACE("tcp@%d send cmd to request xml ok.\n",_p_upnp->sock_tcp);
		return 0;
	}
	else{
		UPNP_TRACE("tcp@%d send cmd to request xml failed.\n",_p_upnp->sock_tcp);
		return -1;
	}
	
	return 0;
}

/*********************************************************************************
* Description : make control request (include command of GetSepecificPortMappingEntry, 
* 				AddPortMapping,……) 
* Arguments   : type - [in] command type 
*  				exPort - [IN] ,external port
*  				inPort - [IN] ,internal port
*  				protocal - [IN] ,protocal
* Returns     : >0 success
		      <0 failed 
* Note        :
*********************************************************************************/
static int RequestControl(uint8_t type,uint16_t exPort,uint16_t inPort,uint8_t proctocal)
{
	uint8_t tmp[UPNP_MAX_BUF_SIZE];
	uint8_t buf[UPNP_MAX_BUF_SIZE];
	uint16_t len;
	uint16_t offset=0;//the data size written to ddr
			
	//make xml step 1
	//
	strcpy(tmp,sControlXml);
	offset+=strlen(sControlXml);	
	//make xml step 2,and write it to ddr
	//
	switch(type)
	{
	case CONTROL_TYPE_GET_STATUS:
	case CONTROL_TYPE_GET_EXTERNAL_IP:
		sprintf(tmp+offset,sControlParam[type],controlCmd[type],controlCmd[type]);
		offset+=strlen(tmp+offset);
		break;
	case CONTROL_TYPE_ADD_PORTMAP:
		sprintf(tmp+offset,sControlParam[type],controlCmd[type],exPort,sProtocal[proctocal],inPort);
		offset+=strlen(tmp+offset);

		sprintf(tmp+offset,sControlParam_AddPortMap,_p_upnp->para.ip_me,controlCmd[type]);
		offset+=strlen(tmp+offset);		
		break;
	case CONTROL_TYPE_GET_SPECIFIED_PORTMAP:
		sprintf(tmp+offset,sControlParam[type],
				controlCmd[type],exPort,sProtocal[proctocal],controlCmd[type]);
		offset+=strlen(tmp+offset);
		break;
	}
	//make xml step 3 
	strcpy(tmp+offset,sControlXmlSuffix);
	offset+=strlen(tmp+offset);
	tmp[offset]=0;
	//until now,xml has maked done
	//
	//make request header
	len=sprintf(buf,sReqControl,_p_upnp->sControlUrl,_p_upnp->para.ip_gw,_p_upnp->server_port,offset,controlCmd[type]);
	len+=sprintf(buf+len,"%s",tmp);
	
	UPNP_TRACE("request control @size=%d ",len);
#ifdef DEBUG_UPNP
	printf("%s",buf);
#endif

	return send(_p_upnp->sock_tcp,buf,len,0);
}


/*********************************************************************************
* Description : the process for upnp
* Arguments   : none
* Returns     : none
* Note        : some server not support keep-alive,so connect by every interation
*	Send M-Search(Discovery)(UDP)->(Location(xml path),ST,server port)
	-->connect to server(gateway:server port)
	-->parse xml->get Control URL
	-->send cmd of GetExternalIPAddress(TCP)->wan ip
	-->send cmd of GetSpecificPortMappingEntry(TCP)->{inport,export,local ip,protocal,remote ip}
	-->send cmd of AddPortMapping(TCP)
	-->make sure the AddPortMapping(TCP) (optional)
	-->END
*********************************************************************************/
static int ProcessForUPNP()
{
	char buf[UPNP_MAX_BUF_SIZE]={0,};
	int ret,len;
	
	while(_p_upnp->trigger){
		switch (_p_upnp->state){
			case UPNP_STATE_INIT:
			{
				_p_upnp->state=UPNP_STATE_UPNP_DISCOVERY;
				break;
			}
				
			case UPNP_STATE_UPNP_DISCOVERY:
			{
				
				if(0 == upnp_discovery_server("239.255.255.250", IPPORT_UPNP, &_p_upnp->context,_p_upnp->para.ip_gw)){
					// FIXME:
					strcpy(_p_upnp->sLocation, _p_upnp->context.location);
					strcpy(_p_upnp->sST, _p_upnp->context.device_type);
					_p_upnp->server_port= _p_upnp->context.server_port;
					
					_p_upnp->state = UPNP_STATE_GET_LOCATION;
				}else{
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}

				break;
			}

			case UPNP_STATE_GET_LOCATION:
			{
				if(0 == upnp_http_get_location(&_p_upnp->context)){
					_p_upnp->state = UPNP_STATE_PARSE_LOCATION;
				}else{
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}
				break;
			}

			case UPNP_STATE_PARSE_LOCATION:
			{
				FILE* fid = NULL;
				fid = fopen("/tmp/upnp/location.pcap", "r+b");
				len = fread(buf, 1, sizeof(buf), fid);
				fclose(fid);
				fid = NULL;
				
				if(isHttpResponseOK(buf))
				{
					UPNP_TRACE("GET DEVICE XML OK .");
					if(1==(ret=GetControlUrl(_p_upnp->sControlUrl,buf,len)))
					{
						_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
						break;
					}	
					_p_upnp->state=UPNP_STATE_REQUEST_WAN_IP;
				}
				else{
					UPNP_TRACE("GET DEVICE XML FAILE .");
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}
				break;
			}
			
			case UPNP_STATE_REQUEST_WAN_IP:
				UPNP_TRACE("Get Wan Lan IP.",_p_upnp->curStep);
				if(UPNPTcpConnect(_p_upnp->para.ip_gw,_p_upnp->server_port)<0){
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				ret=RequestControl(CONTROL_TYPE_GET_EXTERNAL_IP,0,0,0);
				if(ret<=0){
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					UPNP_TRACE("send request failed.");
				}
				else{
					UPNP_TRACE("SEND QUERY CMD DONE.");
					_p_upnp->state=UPNP_STATE_PARSE_RESPONSE_WAN_IP;
				}
				break;
				
			case UPNP_STATE_PARSE_RESPONSE_WAN_IP:
				if((ret=tcp_recv(_p_upnp->sock_tcp,buf,sizeof(buf)))<=0)
				{
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				len=ret;
				upnp_log("/tmp/upnp/wan_ip.pcap", buf, len);
				if(isHttpResponseOK(buf))
				{
					GetWanIP(buf,len);
					_p_upnp->state=UPNP_STATE_REQUEST_QUERY_PORT;
				}
				else if(isHttpResponseInterServerErr(buf))
				{
					UPNP_TRACE("WAN IP:SERVER NOT SUPPORT.");
					_p_upnp->state=UPNP_STATE_REQUEST_QUERY_PORT;
				}
				else{
					UPNP_TRACE("PARSE WAN IP:UNKNOW ERROR.");
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}
				break;
			//make GetSepecificPortMappingEntry request header and content formatted by xml
			
			case UPNP_STATE_REQUEST_QUERY_PORT:
				UPNP_TRACE("\r\n>>>>>>>>>>>>>>\r\nCUR STEP:%d.",_p_upnp->curStep+1);
				if(UPNPTcpConnect(_p_upnp->para.ip_gw,_p_upnp->server_port)<0){
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				ret=RequestControl(CONTROL_TYPE_GET_SPECIFIED_PORTMAP,
							   GetExternalPort(_p_upnp->curStep),
							   0,GetStepProtocal(_p_upnp->curStep));
				if(ret<=0){
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					UPNP_TRACE("send request failed.");
				}
				else{
					UPNP_TRACE("SEND QUERY CMD DONE.");
					_p_upnp->state=UPNP_STATE_PARSE_RESPONSE_QUERY_PORT;
				}
				break;
				
			case UPNP_STATE_PARSE_RESPONSE_QUERY_PORT:
				if((ret=tcp_recv(_p_upnp->sock_tcp,buf,sizeof(buf)))<=0)
				{
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				len=ret;
				upnp_log("/tmp/upnp/query_port.pcap", buf, len);
				if(isHttpResponseOK(buf))	//specific external port has mapped
				{
					UPNP_TRACE("CURRENT DPORT@%d HAS MAPPED.\r\n",GetExternalPort(_p_upnp->curStep));
					if(true==isSpecificPortMappingMe(buf,len))
					{
						//current port map finish,goto map next port
						//
						if(_p_upnp->curStep<(_p_upnp->para.num-1))	
						{
							_p_upnp->curStep++;
							_p_upnp->state=UPNP_STATE_REQUEST_QUERY_PORT;
						}
						else{//total ports map finish,wait for next time
							_p_upnp->isMapped=true;
							_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
						}
					}
					else{//cur port is mapped for another ip:port ,use another port(current port +1 )
						UPNP_TRACE("current port upnp fail,goto next port:%d\r\n",GetExternalPort(_p_upnp->curStep)+1);
						SetExternalPort(_p_upnp->curStep,GetMaxExternalPort()+1);
						_p_upnp->state=UPNP_STATE_REQUEST_QUERY_PORT;
					}
				}
				else if(isHttpResponseInterServerErr(buf))//specific external port has not mapped
				{
					UPNP_TRACE("CURRENT PORT:%d NOT MAPPED,GOTO PORT MAP\r\n",GetExternalPort(_p_upnp->curStep));
					_p_upnp->state=UPNP_STATE_REQUEST_ADD_PORT_MAP;
				}
				else{
					UPNP_TRACE("PARSE PORT MAP:UNKNOW ERROR.\r\n");
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}
				break;
			//make AddPortMapping request header and content formatted by xml
			case UPNP_STATE_REQUEST_ADD_PORT_MAP:
				if(UPNPTcpConnect(_p_upnp->para.ip_gw,_p_upnp->server_port)<0)
				{
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				ret=RequestControl(CONTROL_TYPE_ADD_PORTMAP,
							   GetExternalPort(_p_upnp->curStep),
							   GetInternalPort(_p_upnp->curStep),
							   GetStepProtocal(_p_upnp->curStep));
				if(ret<=0){
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					UPNP_TRACE("send request failed.");
				}
				else{
					UPNP_TRACE("SEND QUERY CMD DONE");
					_p_upnp->state=UPNP_STATE_PARSE_RESPONSE_ADD_PORT_MAP;
				}
				break;
			case UPNP_STATE_PARSE_RESPONSE_ADD_PORT_MAP:
				if((ret=tcp_recv(_p_upnp->sock_tcp,buf,sizeof(buf)))<=0)
				{
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					break;
				}
				len=ret;
				upnp_log("/tmp/upnp/add_port.pcap", buf, len);
				if(isHttpResponseOK(buf))
				{
					UPNP_TRACE("## ADD DPORT@(%d<->%d) MAP SUCCESS ##\r\n",
						GetInternalPort(_p_upnp->curStep),GetExternalPort(_p_upnp->curStep));
					_p_upnp->state=UPNP_STATE_WAIT_NEXT_TIME;
					
				}
				else{
					UPNP_TRACE("ADD DPORT MAP FAIL\r\n");
					_p_upnp->state = UPNP_STATE_WAIT_NEXT_TIME;
				}
				break;
			case UPNP_STATE_WAIT_NEXT_TIME:
				UPNP_TRACE("UPNP::WAIT NEXT TIME");
				sleep(UPNP_UPDATE_PERIOD);

				_p_upnp->started = true;
				_p_upnp->curStep=0;
				_p_upnp->state = UPNP_STATE_INIT;
				break;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//	external function interface
//////////////////////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
* Description : init upnp model
* Arguments   
* Returns     : none
* Note        : 
*********************************************************************************/
void UPNP_start(void *para)
{
	mkdir("/tmp/upnp/", 0);
	if(_p_upnp==NULL)
	{
		UPNP_PARA_t *upnp_para=(UPNP_PARA_t *)para;
		_p_upnp = calloc(sizeof(UPNP_t),1);
		UPNP_ASSERT(_p_upnp,"create upnp failed");
		UPNP_ASSERT(para,"para is null");
		memcpy(&_p_upnp->para,upnp_para,sizeof(UPNP_PARA_t));
		_p_upnp->isMapped=false;
		_p_upnp->curStep=0;
		_p_upnp->sock_tcp=0;
		_p_upnp->sock_udp=0;
		_p_upnp->trigger=true;
		_p_upnp->state=UPNP_STATE_INIT;

		initExternalPorts();

		UPNP_TRACE("ip gateway:%s ip-me:%s",upnp_para->ip_gw,upnp_para->ip_me);
	}
	
	if(!_p_upnp->tid)
		pthread_create(&_p_upnp->tid,NULL,ProcessForUPNP,NULL);
	
}

void UPNP_stop()
{

	if(_p_upnp->tid)
	{
		_p_upnp->trigger = false;
		pthread_join(_p_upnp->tid, NULL);
		_p_upnp->tid = 0;
		_p_upnp->curStep=0;
		_p_upnp->state=UPNP_STATE_INIT;
		_p_upnp->isMapped=false;
		_p_upnp->started = false;
		close(_p_upnp->sock_tcp);
		close(_p_upnp->sock_udp);
	}
}

void UPNP_restart()
{
	UPNP_stop();
	UPNP_start(NULL);
}

int UPNP_external_port(uint16_t in_port,uint16_t protocal /* 0 for tcp , 1 for udp */)
{
	int i;
	for(i=0;i<_p_upnp->para.num;i++)
	{
		if((_p_upnp->para.port_maps[i].inPort==in_port) &&
			(_p_upnp->para.port_maps[i].isMapped==true) &&
			(_p_upnp->para.port_maps[i].protocal==protocal) )
			return _p_upnp->para.port_maps[i].exPort;
	}
	return 0;
}

char *UPNP_wan_ip(char *ip_str)
{
	if(_p_upnp->para.ip_wan[0]==0)
	{
		return NULL;
	}
	else{
		if(ip_str)
			strcpy(ip_str,_p_upnp->para.ip_wan);
		return _p_upnp->para.ip_wan;
	}
}

int UPNP_done()
{
	return _p_upnp->isMapped;
}

int UPNP_started()
{
	return _p_upnp->started;
}

/*
int main()
{
	UPNP_PARA_t upnp_para;
	memset(&upnp_para,0,sizeof(UPNP_PARA_t));
	strcpy(upnp_para.ip_gw,"192.168.1.1");
	strcpy(upnp_para.ip_me,"192.168.1.40");
	printf("gw %s\r\n", upnp_para.ip_gw);
	printf("me %s\r\n", upnp_para.ip_me);
	upnp_para.num=1;
	upnp_para.port_maps[0].inPort = 80;
	upnp_para.port_maps[0].protocal = PROTOCAL_TCP;

	
	UPNP_start(&upnp_para);
	getchar();
	UPNP_stop();
	return 0;
}
*/

