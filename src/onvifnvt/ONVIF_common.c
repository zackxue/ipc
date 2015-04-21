#include <time.h>
#include <pthread.h>
#include <assert.h>
#include "onvif.nsmap"
#include "soapH.h"
#include "ONVIF_common.h"
#include "onvif_debug.h"
#include "sysconf.h"
#include "uuid/uuid.h"
#include "soapStub.h"
#include "sys/socket.h"

#include "sensor.h"

/*
#define FONT_COLOR_BLACK	"30"
#define FONT_COLOR_RED		"31"
#define FONT_COLOR_GREEN	"32"
#define FONT_COLOR_YELLOW	"33"
#define FONT_COLOR_BLUE		"34"
#define FONT_COLOR_WHITE	"37"

#define FONT_BKG_BLACK		"40"
#define FONT_BKG_RED		"41"
#define FONT_BKG_GREEN		"42"
#define FONT_BKG_YELLOW		"43"
#define FONT_BKG_BLUE		"44"
#define FONT_BKG_WHITE		"47"

#define CLOSE_ALL_ATTR		"0m"
#define SET_HEIGHT			"1m"
#define SET_UNDERLINE		"4m"
#define SET_FLICKER			"5m"
#define CLEAR_SCREEN		"2J"
#define ONVIFDEBUG(fmt...) \
	do{\
		printf("\033["FONT_BKG_BLACK";"FONT_COLOR_WHITE"m");\
		printf(fmt);\
		printf("\033["CLOSE_ALL_ATTR"\r\n");\
	}while(0);


*/
//#define INFO_LENGTH  50
#define SMALL_INFO_LENGTH  30
#define MID_INFO_LENGTH 50
#define LARGE_INFO_LENGTH 100

#define VIDEO_HD_WIDTH 1280
#define VIDEO_HD_HEIGHT 720
#define ONVIF_ARRAY_MAX_LEN		3
#define IPC_SERVICE_ADDR "192.168.1.18:10010"
#define IPC_SERVER_GATEWAY "192.168.1.1"
#define IPADDRESS_NTOA(strAddr, netAddr) do{ sprintf(strAddr, "%d.%d.%d.%d", netAddr.s1, netAddr.s2, netAddr.s3, netAddr.s4); }while(0)
#define ONVIF_MALLOC(Type) (Type *)memset(soap_malloc(soap, sizeof(Type)), 0, sizeof(Type)); 
#define ONVIF_MALLOC_SIZE(Type, size) (Type *)memset(soap_malloc(soap, size * sizeof(Type)), 0, size * sizeof(Type)); 

static ONVIF_Conf_t g_OnvifConf;
enum xsd__boolean_  nfalse = 0;
enum xsd__boolean_  ntrue = 1;
#define NOT_EXIST  0
#define EXIST  1

/* @brief Check if IP is valid */
int isValidIp4 (char *str) 
{
	int segs = 0;   /* Segment count. */     
	int chcnt = 0;  /* Character count within segment. */     
	int accum = 0;  /* Accumulator for segment. */      
	/* Catch NULL pointer. */      
	if (str == NULL) return 0;      
	/* Process every character in string. */      
	while (*str != '\0') 
	{         
		/* Segment changeover. */          
		if (*str == '.') 
		{             
			/* Must have some digits in segment. */              
			if (chcnt == 0) return 0;              
			/* Limit number of segments. */              
			if (++segs == 4) return 0;              
			/* Reset segment values and restart loop. */              
			chcnt = accum = 0;             
			str++;             
			continue;         
		}  

		/* Check numeric. */          
		if ((*str < '0') || (*str > '9')) return 0;
		/* Accumulate and check segment. */          
		if ((accum = accum * 10 + *str - '0') > 255) return 0;
		/* Advance other segment specific stuff and continue loop. */          
		chcnt++;         
		str++;     
	}      
	/* Check enough segments and enough characters in last segment. */      
	if (segs != 3) return 0;      
	if (chcnt == 0) return 0;      
	/* Address okay. */      
	return 1; 
} 


static void* onvif_server_thread(void* arg)
{
	struct soap soap;
	struct ip_mreq mcast;

	int nRet = -1;

	soap_init2(&soap, SOAP_IO_UDP|SOAP_IO_FLUSH, SOAP_IO_UDP|SOAP_IO_FLUSH);
	soap_set_namespaces(&soap, namespaces); 
	//soap_set_mode(&soap, SOAP_C_UTFSTRING);
	if(!soap_valid_socket(soap_bind(&soap, NULL, 3702, 100)))
	{ 
		soap_print_fault(&soap, stderr);
		exit(1);
	}
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
	mcast.imr_interface.s_addr = htonl(INADDR_ANY);
	if(setsockopt(soap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
		return (void*)0;
	}
	for( ; ; )
	{ 

		if(soap_serve(&soap)){
			soap_print_fault(&soap, stderr);
		}
		//soap_destroy(&soap);
		soap_end(&soap);
	}
	soap_done(&soap);
	return (void*)0;
}

static void *http_onvif_server(void *arg)
{
	int nRet = 0;
	int m, s = 0; /* master and slave sockets */
	struct timeval timeout = { 1, 0 };
	struct soap add_soap, *temp_soap;
	soap_init(&add_soap);
	soap_set_namespaces(&add_soap, namespaces);
	m = soap_bind(&add_soap, NULL, 10010, 100);
	if (m < 0)
	{
		soap_print_fault(&add_soap, stderr);
		exit(-1);
	}

	fprintf(stderr, "Socket connection successful: master socket = %d\n", m);
	add_soap.bind_flags = add_soap.bind_flags | SO_REUSEADDR;
	add_soap.recv_timeout = 3;
	add_soap.send_timeout = 3;

	for ( ; ; )
	{ 
		s = soap_accept(&add_soap); 
		if (s < 0)
		{ 
			soap_print_fault(&add_soap, stderr);
			exit(-1);
		}
		fprintf(stderr, "Socket connection successful: slave socket = %d\n", add_soap.socket);
		soap_serve(&add_soap);
		soap_end(&add_soap);
	}
	soap_done(&add_soap);	 
}




int ONVIF_listen(){
	pthread_t pt_listen = 0;
	pthread_create(&pt_listen, NULL, onvif_server_thread, NULL);
	return pt_listen;
}
int ONVIF_server_run(){
	pthread_t pt_server = 0;
	pthread_create(&pt_server, NULL, http_onvif_server, NULL);
	return pt_server;
}

extern void ONVIF_init(){
	memset(&g_OnvifConf, 0, sizeof(g_OnvifConf));
	g_OnvifConf.pConf = SYSCONF_dup();
	g_OnvifConf.nProfileCount = 0;
	g_OnvifConf.Scopes[0].item = "onvif://www.onvif.org/type/video_encoder";
	g_OnvifConf.Scopes[0].definition = 1;
	g_OnvifConf.Scopes[1].item = "onvif://www.onvif.org/type/audio_encoder";
	g_OnvifConf.Scopes[1].definition = 0;
	g_OnvifConf.Scopes[2].item = "onvif://www.onvif.org/hardware/IPC-model";
	g_OnvifConf.Scopes[2].definition = 0;
	g_OnvifConf.Scopes[3].item = "onvif://www.onvif.org/name/IPC";
	g_OnvifConf.Scopes[3].definition = 1;
	g_OnvifConf.Scopes[4].item = "onvif://www.onvif.org/location/country/china";
	g_OnvifConf.Scopes[4].definition = 1;
	//profile_1  config
	strcpy(g_OnvifConf.Profile[0].ProfileName, "MainStreamProfile");
	strcpy(g_OnvifConf.Profile[0].ProfileToken, "MainStreamToken");
	g_OnvifConf.Profile[0].Profilefixed = 1;
	//Video source config
	strcpy(g_OnvifConf.Profile[0].VSCName, "VideoSourceConfDefault");
	strcpy(g_OnvifConf.Profile[0].VSCToken, "VideoSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[0].VSCSourceToken, "VideoSourceTokenDefault");
	g_OnvifConf.Profile[0].VSCUseCount = 1;
	g_OnvifConf.Profile[0].Bounds.x = 0;
	g_OnvifConf.Profile[0].Bounds.y = 0;
	g_OnvifConf.Profile[0].Bounds.width = 1280;
	g_OnvifConf.Profile[0].Bounds.height = 720;
	//audio source config
	strcpy(g_OnvifConf.Profile[0].ASCName, "AudioSourceConfDefault");
	strcpy(g_OnvifConf.Profile[0].ASCToken, "AudioSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[0].ASCSourceToken, "AudioSourceTokenDefault");
	g_OnvifConf.Profile[0].ASCUseCount = 1;
	//video encoder config
	strcpy(g_OnvifConf.Profile[0].VECName, "MainStreamVencConf");
	strcpy(g_OnvifConf.Profile[0].VECToken, "MainStreamVencConfToken");
	g_OnvifConf.Profile[0].VECUseCount = 1;
	strcpy(g_OnvifConf.Profile[0].VECTimeOut, "PT5S");
	g_OnvifConf.Profile[0].VECbps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].bps;
	g_OnvifConf.Profile[0].VECfps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].fps;
	g_OnvifConf.Profile[0].VECQuality = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].quality.val;
	g_OnvifConf.Profile[0].VECWidth = 1280;
	g_OnvifConf.Profile[0].VECHeight = 720;
	g_OnvifConf.Profile[0].VECEncoding = 2;
	g_OnvifConf.Profile[0].VECH264Gop = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].gop;
	g_OnvifConf.Profile[0].VECH264Profile = 0;
	//audio encoder config
	g_OnvifConf.Profile[0].AECSampleRate = g_OnvifConf.pConf->ipcam.ain[0].sample_rate.val;
	strcpy(g_OnvifConf.Profile[0].AECName, "MainStreamAencConf");
	strcpy(g_OnvifConf.Profile[0].AECToken, "MainStreamAencConfToken");
	g_OnvifConf.Profile[0].AECUseCount = 1;
	g_OnvifConf.Profile[0].AECEncoding = 0;
	strcpy(g_OnvifConf.Profile[0].AECTimeOut, "PT5S");
	//profile_2  config
	strcpy(g_OnvifConf.Profile[1].ProfileName, "SubStreamProfile");
	strcpy(g_OnvifConf.Profile[1].ProfileToken, "SubStreamToken");
	g_OnvifConf.Profile[1].Profilefixed = 1;

	strcpy(g_OnvifConf.Profile[1].VSCName, "VideoSourceConfDefault");
	strcpy(g_OnvifConf.Profile[1].VSCToken, "VideoSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[1].VSCSourceToken, "VideoSourceTokenDefault");
	g_OnvifConf.Profile[1].VSCUseCount = 1;
	g_OnvifConf.Profile[1].Bounds.x = 0;
	g_OnvifConf.Profile[1].Bounds.y = 0;
	g_OnvifConf.Profile[1].Bounds.width = 1280;
	g_OnvifConf.Profile[1].Bounds.height = 720;
	
	strcpy(g_OnvifConf.Profile[1].ASCName, "AudioSourceConfDefault");
	strcpy(g_OnvifConf.Profile[1].ASCToken, "AudioSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[1].ASCSourceToken, "AudioSourceTokenDefault");
	g_OnvifConf.Profile[1].ASCUseCount = 1;

	strcpy(g_OnvifConf.Profile[1].VECName, "SubStreamVencConf");
	strcpy(g_OnvifConf.Profile[1].VECToken, "SubStreamVencConfToken");
	g_OnvifConf.Profile[1].VECUseCount = 1;
	strcpy(g_OnvifConf.Profile[1].VECTimeOut, "PT5S");
	g_OnvifConf.Profile[1].VECbps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].bps;
	g_OnvifConf.Profile[1].VECfps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].fps;
	g_OnvifConf.Profile[1].VECQuality = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].quality.val;
	g_OnvifConf.Profile[1].VECWidth = 640;
	g_OnvifConf.Profile[1].VECHeight = 360;
	g_OnvifConf.Profile[1].VECEncoding = 2;
	g_OnvifConf.Profile[1].VECH264Gop = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].gop;
	g_OnvifConf.Profile[1].VECH264Profile = 0;

	g_OnvifConf.Profile[1].AECSampleRate = g_OnvifConf.pConf->ipcam.ain[1].sample_rate.val;
	strcpy(g_OnvifConf.Profile[1].AECName, "SubStreamAencConf");
	strcpy(g_OnvifConf.Profile[1].AECToken, "SubStreamAencConfToken");
	g_OnvifConf.Profile[1].AECUseCount = 1;
	g_OnvifConf.Profile[1].AECEncoding = 0;
	strcpy(g_OnvifConf.Profile[1].AECTimeOut, "PT5S");

	g_OnvifConf.nProfileCount = 2;

/**/
	g_OnvifConf.Profile[2].Profilefixed = 0;

	strcpy(g_OnvifConf.Profile[2].VSCName, "VideoSourceConfDefault");
	strcpy(g_OnvifConf.Profile[2].VSCToken, "VideoSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[2].VSCSourceToken, "VideoSourceTokenDefault");
	g_OnvifConf.Profile[2].VSCUseCount = 1;
	
	strcpy(g_OnvifConf.Profile[2].ASCName, "AudioSourceConfDefault");
	strcpy(g_OnvifConf.Profile[2].ASCToken, "AudioSourceConfDefaultToken");
	strcpy(g_OnvifConf.Profile[2].ASCSourceToken, "AudioSourceTokenDefault");
	g_OnvifConf.Profile[2].ASCUseCount = 1;

	strcpy(g_OnvifConf.Profile[2].VECName, "MainStreamVencConf");
	strcpy(g_OnvifConf.Profile[2].VECToken, "MainStreamVencConfToken");
	g_OnvifConf.Profile[2].VECUseCount = 1;
	strcpy(g_OnvifConf.Profile[2].VECTimeOut, "PT5S");
	g_OnvifConf.Profile[2].VECbps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].bps;
	g_OnvifConf.Profile[2].VECfps = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].fps;
	g_OnvifConf.Profile[2].VECQuality = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].quality.val;
	g_OnvifConf.Profile[2].VECWidth = 1280;
	g_OnvifConf.Profile[2].VECHeight = 720;
	g_OnvifConf.Profile[2].VECEncoding = 2;
	g_OnvifConf.Profile[2].VECH264Gop = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[1].stream[0].gop;
	g_OnvifConf.Profile[2].VECH264Profile = 0;

	g_OnvifConf.Profile[2].AECSampleRate = g_OnvifConf.pConf->ipcam.ain[1].sample_rate.val;
	strcpy(g_OnvifConf.Profile[2].AECName, "MainStreamAencConf");
	strcpy(g_OnvifConf.Profile[2].AECToken, "MainStreamAencConfToken");
	g_OnvifConf.Profile[2].AECUseCount = 1;
	g_OnvifConf.Profile[2].AECEncoding = 0;
	strcpy(g_OnvifConf.Profile[2].AECTimeOut, "PT5S");

	ONVIF_listen();
}

extern void ONVIF_Ctrldata(ONVIF_CTRLDATA_TYPE_t type, void *pData){
	switch(type){
		case ONVIF_CTRL_SET_PROFILE:
		break;
		case ONVIF_CTRL_SET_SYSCONF:
			SYSCONF_save(pData);
		break;
		default:
		break;
	}

}
extern void ONVIF_dup(){
	g_OnvifConf.pConf = SYSCONF_dup();
}

SOAP_FMAC5 int SOAP_FMAC6 __dndl__Probe(struct soap *soap, struct d__ProbeType *pd__Probe, struct d__ProbeMatchesType *pd__ProbeMatches)
{
	if(soap->header){
		uuid_t uuidMessageID;
		uuid_generate(uuidMessageID);
		char strMessageID[100] = {0};
		strncpy(strMessageID, "urn:uuid:", strlen("urn:uuid:"));
		//uuid_unparse(uuidMessageID, strMessageID + strlen("urn:uuid:"));
		if(soap->header->wsa5__MessageID){
			char strOldMessageID[100] = {0};
			strcpy(strOldMessageID, soap->header->wsa5__MessageID);
			soap_dealloc(soap, soap->header->wsa5__MessageID);
			soap->header->wsa5__MessageID = NULL;
			if(!soap->header->wsa__MessageID){
				soap->header->wsa__MessageID = ONVIF_MALLOC_SIZE(char, 100);
			}
			if(!soap->header->wsa__RelatesTo){
				soap->header->wsa__RelatesTo = ONVIF_MALLOC(struct wsa__Relationship);
				soap->header->wsa__RelatesTo->__item = ONVIF_MALLOC_SIZE(char, 100);
				
			}
			strncpy(soap->header->wsa__MessageID, strMessageID, strlen(strMessageID));
			strncpy(soap->header->wsa__RelatesTo->__item, strOldMessageID, 100);
		}
		if(!soap->header->wsa__ReplyTo){
			soap->header->wsa__ReplyTo = ONVIF_MALLOC(struct wsa__EndpointReferenceType);
			soap->header->wsa__ReplyTo->Address = ONVIF_MALLOC_SIZE(char , 100);
			strncpy(soap->header->wsa__ReplyTo->Address, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous", 100);
		}
		if(!soap->header->wsa__To){
			soap->header->wsa__To = ONVIF_MALLOC_SIZE(char, 100);
			strncpy(soap->header->wsa__To, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous", 100);
		}
		if(!soap->header->wsa__Action){
			soap->header->wsa__Action = ONVIF_MALLOC_SIZE(char, 100);
			strncpy(soap->header->wsa__Action, "http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches", 100);
		}
		if(soap->header->wsa5__ReplyTo){
			soap_dealloc(soap, soap->header->wsa5__ReplyTo->Address);
			soap->header->wsa5__ReplyTo->Address = NULL;
			soap_dealloc(soap, soap->header->wsa5__ReplyTo);
			soap->header->wsa5__ReplyTo = NULL;
		}
		if(soap->header->wsa5__To){
			//soap->header->wsa__To = ONVIF_MALLOC_SIZE(char, strlen(soap->header->wsa5__To + 1));
			//strncpy(soap->header->wsa__To, soap->header->wsa5__To, strlen(soap->header->wsa5__To));
			soap_dealloc(soap, soap->header->wsa5__To);
			soap->header->wsa5__To = NULL;
		}
		if(soap->header->wsa5__Action){
			//soap->header->wsa__Action = ONVIF_MALLOC_SIZE(char, strlen(soap->header->wsa5__Action + 1));
			//strncpy(soap->header->wsa__Action, soap->header->wsa5__Action, strlen(soap->header->wsa5__Action));
			soap_dealloc(soap, soap->header->wsa5__Action);
			soap->header->wsa5__Action = NULL;
		}
	}	

	pd__ProbeMatches->__sizeProbeMatch = 1;
	pd__ProbeMatches->ProbeMatch = ONVIF_MALLOC(struct d__ProbeMatchType);
	pd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address = ONVIF_MALLOC_SIZE(char, 100);
	snprintf(pd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address, 100, "urn:uuid:11223344-5566-7788-99aa-%02x%02x%02x%02x%02x%02x", 
						g_OnvifConf.pConf->ipcam.network.mac.s1,
						g_OnvifConf.pConf->ipcam.network.mac.s2,
						g_OnvifConf.pConf->ipcam.network.mac.s3,
						g_OnvifConf.pConf->ipcam.network.mac.s4,
						g_OnvifConf.pConf->ipcam.network.mac.s5,
						g_OnvifConf.pConf->ipcam.network.mac.s6);
	char *DeviceType = ONVIF_MALLOC_SIZE(char , 50);
	snprintf(DeviceType, 50, "dn:NetworkVideoTransmitter");

	pd__ProbeMatches->ProbeMatch->Types = ONVIF_MALLOC(char *);
	pd__ProbeMatches->ProbeMatch->Types[0] = "dn:NetworkVideoTransmitter";
	pd__ProbeMatches->ProbeMatch->Scopes = ONVIF_MALLOC(struct d__ScopesType);
	pd__ProbeMatches->ProbeMatch->Scopes->__item = ONVIF_MALLOC_SIZE(char, 200);
	snprintf(pd__ProbeMatches->ProbeMatch->Scopes->__item, 200, "onvif://www.onvif.org/type/video_encoder "
													"onvif://www.onvif.org/type/audio_encoder "
													"onvif://www.onvif.org/hardware/IPC-model "
													"onvif://www.onvif.org/name/IPC "
													"onvif://www.onvif.org/location/country/china "
													);
	pd__ProbeMatches->ProbeMatch->XAddrs = ONVIF_MALLOC_SIZE(char, 50);
	snprintf(pd__ProbeMatches->ProbeMatch->XAddrs, 50, "http://%s:%d/onvif/device_service", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr),
																			g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
	pd__ProbeMatches->ProbeMatch->MetadataVersion = 1;
	
	ONVIFDEBUG("wsa__EndpointReference address:%s, device type:%s, probe scopes:%s, xaddr:%s\n", pd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address, *pd__ProbeMatches->ProbeMatch->Types, pd__ProbeMatches->ProbeMatch->Scopes->__item, pd__ProbeMatches->ProbeMatch->XAddrs);

	return SOAP_OK;
}

void onvif_fault(struct soap *soap,char *value1,char *value2)
{
	soap->fault = (struct SOAP_ENV__Fault*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Fault)));
	soap->fault->SOAP_ENV__Code = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Value = "SOAP-ENV:Sender";
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value = value1;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode = (struct SOAP_ENV__Code*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Code)));
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Value = value2;
	soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Subcode->SOAP_ENV__Subcode = NULL;
	soap->fault->faultcode = NULL;
	soap->fault->faultstring = NULL;
	soap->fault->faultactor = NULL;
	soap->fault->detail = NULL;
	//soap->fault->SOAP_ENV__Reason = NULL;
	soap->fault->SOAP_ENV__Reason = (struct SOAP_ENV__Reason*)soap_malloc(soap,(sizeof(struct SOAP_ENV__Reason)));
	soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text = (char*)soap_malloc(soap,100);
	strcpy(soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text,"The requested service is not supported by the NVT");
	soap->fault->SOAP_ENV__Node = NULL;
	soap->fault->SOAP_ENV__Role = NULL;
	soap->fault->SOAP_ENV__Detail = NULL;
}

SOAP_FMAC5 int SOAP_FMAC6 SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
	ONVIFDEBUG("SOAP_ENV__Fault\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __decpp__CreatePullPoint(struct soap *soap, struct _wsnt__CreatePullPoint *wsnt__CreatePullPoint, struct _wsnt__CreatePullPointResponse *wsnt__CreatePullPointResponse)
{
	ONVIFDEBUG("__decpp__CreatePullPoint\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __dee__GetServiceCapabilities(struct soap *soap, struct _tev__GetServiceCapabilities *tev__GetServiceCapabilities, struct _tev__GetServiceCapabilitiesResponse *tev__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("GetServiceCapabilities\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __dee__CreatePullPointSubscription(struct soap *soap, struct _tev__CreatePullPointSubscription *tev__CreatePullPointSubscription, struct _tev__CreatePullPointSubscriptionResponse *tev__CreatePullPointSubscriptionResponse)
{
	ONVIFDEBUG("__dee__CreatePullPointSubscription\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __dee__GetEventProperties(struct soap *soap, struct _tev__GetEventProperties *tev__GetEventProperties, struct _tev__GetEventPropertiesResponse *tev__GetEventPropertiesResponse)
{
	ONVIFDEBUG("dee__GetEventProperties\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __denc__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	ONVIFDEBUG("__denc__Notify\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __denf__Subscribe(struct soap *soap, struct _wsnt__Subscribe *wsnt__Subscribe, struct _wsnt__SubscribeResponse *wsnt__SubscribeResponse)
{
	ONVIFDEBUG("__denf__Subscribe\n");
	wsnt__SubscribeResponse->SubscriptionReference.Address = ONVIF_MALLOC_SIZE(char, 50);
	snprintf(wsnt__SubscribeResponse->SubscriptionReference.Address, 50, "http://%s:%d/onvif/events", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __denf__GetCurrentMessage(struct soap *soap, struct _wsnt__GetCurrentMessage *wsnt__GetCurrentMessage, struct _wsnt__GetCurrentMessageResponse *wsnt__GetCurrentMessageResponse)
{
	ONVIFDEBUG("__denf__GetCurrentMessage\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depp__GetMessages(struct soap *soap, struct _wsnt__GetMessages *wsnt__GetMessages, struct _wsnt__GetMessagesResponse *wsnt__GetMessagesResponse)
{
	ONVIFDEBUG("__depp__GetMessages\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depp__DestroyPullPoint(struct soap *soap, struct _wsnt__DestroyPullPoint *wsnt__DestroyPullPoint, struct _wsnt__DestroyPullPointResponse *wsnt__DestroyPullPointResponse)
{
	ONVIFDEBUG("__depp__DestroyPullPoint\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depp__Notify(struct soap *soap, struct _wsnt__Notify *wsnt__Notify)
{
	ONVIFDEBUG("__depp__Notify\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depps__PullMessages(struct soap *soap, struct _tev__PullMessages *tev__PullMessages, struct _tev__PullMessagesResponse *tev__PullMessagesResponse)
{
	ONVIFDEBUG("__depps__PullMessages\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depps__SetSynchronizationPoint(struct soap *soap, struct _tev__SetSynchronizationPoint *tev__SetSynchronizationPoint, struct _tev__SetSynchronizationPointResponse *tev__SetSynchronizationPointResponse)
{
	ONVIFDEBUG("__depps__SetSynchronizationPoint\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depsm__Renew(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	ONVIFDEBUG("__depsm__Renew\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depsm__Unsubscribe(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	ONVIFDEBUG("__depsm__Unsubscribe\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depsm__PauseSubscription(struct soap *soap, struct _wsnt__PauseSubscription *wsnt__PauseSubscription, struct _wsnt__PauseSubscriptionResponse *wsnt__PauseSubscriptionResponse)
{
	ONVIFDEBUG("__depsm__PauseSubscription\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __depsm__ResumeSubscription(struct soap *soap, struct _wsnt__ResumeSubscription *wsnt__ResumeSubscription, struct _wsnt__ResumeSubscriptionResponse *wsnt__ResumeSubscriptionResponse)
{

	ONVIFDEBUG("unknown%d\n", 1);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __desm__Renew(struct soap *soap, struct _wsnt__Renew *wsnt__Renew, struct _wsnt__RenewResponse *wsnt__RenewResponse)
{
	ONVIFDEBUG("unknown%d\n", 2);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __desm__Unsubscribe(struct soap *soap, struct _wsnt__Unsubscribe *wsnt__Unsubscribe, struct _wsnt__UnsubscribeResponse *wsnt__UnsubscribeResponse)
{
	ONVIFDEBUG("unknown%d\n", 3);return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __dnrd__Hello(struct soap *soap, struct d__HelloType *d__Hello, struct d__ResolveType *dn__HelloResponse)
{
	ONVIFDEBUG("types:%s\nXaddrs:%s\n", *d__Hello->Types, d__Hello->XAddrs);


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __dnrd__Bye(struct soap *soap, struct d__ByeType *d__Bye, struct d__ResolveType *dn__ByeResponse)
{
	ONVIFDEBUG("unknown%d\n", 4);


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__GetServiceCapabilities(struct soap *soap, struct _tdis__GetServiceCapabilities *tdis__GetServiceCapabilities, struct _tdis__GetServiceCapabilitiesResponse *tdis__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("GetServiceCapabilities\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__GetLayout(struct soap *soap, struct _tdis__GetLayout *tdis__GetLayout, struct _tdis__GetLayoutResponse *tdis__GetLayoutResponse)
{
	ONVIFDEBUG("unknown%d\n", 5);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__SetLayout(struct soap *soap, struct _tdis__SetLayout *tdis__SetLayout, struct _tdis__SetLayoutResponse *tdis__SetLayoutResponse)
{
	ONVIFDEBUG("unknown%d\n", 6);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__GetDisplayOptions(struct soap *soap, struct _tdis__GetDisplayOptions *tdis__GetDisplayOptions, struct _tdis__GetDisplayOptionsResponse *tdis__GetDisplayOptionsResponse)
{
	ONVIFDEBUG("unknown%d\n", 7);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__GetPaneConfigurations(struct soap *soap, struct _tdis__GetPaneConfigurations *tdis__GetPaneConfigurations, struct _tdis__GetPaneConfigurationsResponse *tdis__GetPaneConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 8);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__GetPaneConfiguration(struct soap *soap, struct _tdis__GetPaneConfiguration *tdis__GetPaneConfiguration, struct _tdis__GetPaneConfigurationResponse *tdis__GetPaneConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 9);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__SetPaneConfigurations(struct soap *soap, struct _tdis__SetPaneConfigurations *tdis__SetPaneConfigurations, struct _tdis__SetPaneConfigurationsResponse *tdis__SetPaneConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 10);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__SetPaneConfiguration(struct soap *soap, struct _tdis__SetPaneConfiguration *tdis__SetPaneConfiguration, struct _tdis__SetPaneConfigurationResponse *tdis__SetPaneConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n",  11);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__CreatePaneConfiguration(struct soap *soap, struct _tdis__CreatePaneConfiguration *tdis__CreatePaneConfiguration, struct _tdis__CreatePaneConfigurationResponse *tdis__CreatePaneConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 12);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tdis__DeletePaneConfiguration(struct soap *soap, struct _tdis__DeletePaneConfiguration *tdis__DeletePaneConfiguration, struct _tdis__DeletePaneConfigurationResponse *tdis__DeletePaneConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 13);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServices(struct soap *soap, struct _tds__GetServices *tds__GetServices, struct _tds__GetServicesResponse *tds__GetServicesResponse)
{
	ONVIFDEBUG("GetServices, include capability: %d\n", tds__GetServices->IncludeCapability);
	ONVIF_dup();

	char _IPAddr[LARGE_INFO_LENGTH];
	sprintf(_IPAddr, "http://%s:%d", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
	

	tds__GetServicesResponse->__sizeService = 6;
	tds__GetServicesResponse->Service = ONVIF_MALLOC_SIZE(struct tds__Service,tds__GetServicesResponse->__sizeService);

	tds__GetServicesResponse->Service[0].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[0].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[0].Namespace, "http://www.onvif.org/ver10/device/wsdl");
	sprintf(tds__GetServicesResponse->Service[0].XAddr,"%s%s", _IPAddr,"/onvif/device_service");
	tds__GetServicesResponse->Service[0].Capabilities = NULL;
	tds__GetServicesResponse->Service[0].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[0].Version->Major = 2;
	tds__GetServicesResponse->Service[0].Version->Minor = 1;
	tds__GetServicesResponse->Service[0].__size = 0;
	tds__GetServicesResponse->Service[0].__any = NULL;
	tds__GetServicesResponse->Service[0].__anyAttribute = NULL;

	tds__GetServicesResponse->Service[1].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[1].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[1].Namespace, "http://www.onvif.org/ver10/media/wsdl");
	sprintf(tds__GetServicesResponse->Service[1].XAddr,"%s%s", _IPAddr,"/onvif/Media");
	tds__GetServicesResponse->Service[1].Capabilities = NULL;
	tds__GetServicesResponse->Service[1].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[1].Version->Major = 2;
	tds__GetServicesResponse->Service[1].Version->Minor = 1;
	tds__GetServicesResponse->Service[1].__size = 0;
	tds__GetServicesResponse->Service[1].__any = NULL;
	tds__GetServicesResponse->Service[1].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[2].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[2].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[2].Namespace, "http://www.onvif.org/ver10/events/wsdl");
	sprintf(tds__GetServicesResponse->Service[2].XAddr,"%s%s", _IPAddr,"/onvif/Events");
	tds__GetServicesResponse->Service[2].Capabilities = NULL;
	tds__GetServicesResponse->Service[2].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[2].Version->Major = 2;
	tds__GetServicesResponse->Service[2].Version->Minor = 1;
	tds__GetServicesResponse->Service[2].__size = 0;
	tds__GetServicesResponse->Service[2].__any = NULL;
	tds__GetServicesResponse->Service[2].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[3].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[3].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[3].Namespace, "http://www.onvif.org/ver20/imaging/wsdl");
	sprintf(tds__GetServicesResponse->Service[3].XAddr,"%s%s", _IPAddr,"/onvif/Imaging");
	tds__GetServicesResponse->Service[3].Capabilities = NULL;
	tds__GetServicesResponse->Service[3].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[3].Version->Major = 2;
	tds__GetServicesResponse->Service[3].Version->Minor = 1;
	tds__GetServicesResponse->Service[3].__size = 0;
	tds__GetServicesResponse->Service[3].__any = NULL;
	tds__GetServicesResponse->Service[3].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[4].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[4].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[4].Namespace, "http://www.onvif.org/ver10/deviceIO/wsdl");
	sprintf(tds__GetServicesResponse->Service[4].XAddr,"%s%s", _IPAddr,"/onvif/DeviceIO");
	tds__GetServicesResponse->Service[4].Capabilities = NULL;
	tds__GetServicesResponse->Service[4].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[4].Version->Major = 2;
	tds__GetServicesResponse->Service[4].Version->Minor = 1;
	tds__GetServicesResponse->Service[4].__size = 0;
	tds__GetServicesResponse->Service[4].__any = NULL;
	tds__GetServicesResponse->Service[4].__anyAttribute = NULL;	

	tds__GetServicesResponse->Service[5].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[5].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[5].Namespace, "http://www.onvif.org/ver20/analytics/wsdl");
	sprintf(tds__GetServicesResponse->Service[5].XAddr,"%s%s", _IPAddr,"/onvif/Analytics");
	tds__GetServicesResponse->Service[5].Capabilities = NULL;
	tds__GetServicesResponse->Service[5].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[5].Version->Major = 2;
	tds__GetServicesResponse->Service[5].Version->Minor = 1;
	tds__GetServicesResponse->Service[5].__size = 0;
	tds__GetServicesResponse->Service[5].__any = NULL;
	tds__GetServicesResponse->Service[5].__anyAttribute = NULL;	
/*
	tds__GetServicesResponse->Service[6].XAddr = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	tds__GetServicesResponse->Service[6].Namespace = ONVIF_MALLOC_SIZE(char, MID_INFO_LENGTH);
	strcpy(tds__GetServicesResponse->Service[6].Namespace, "http://www.onvif.org/ver10/PTZ/wsdl");
	sprintf(tds__GetServicesResponse->Service[6].XAddr,"%s%s", _IPAddr,"/onvif/ptz");
	tds__GetServicesResponse->Service[6].Capabilities = NULL;
	tds__GetServicesResponse->Service[6].Version = ONVIF_MALLOC(struct tt__OnvifVersion);
	tds__GetServicesResponse->Service[6].Version->Major = 2;
	tds__GetServicesResponse->Service[6].Version->Minor = 1;
	tds__GetServicesResponse->Service[6].__size = 0;
	tds__GetServicesResponse->Service[6].__any = NULL;
	tds__GetServicesResponse->Service[6].__anyAttribute = NULL;		
*/		
	if(tds__GetServices->IncludeCapability){
		
		tds__GetServicesResponse->Service[0].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[0].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[0].Capabilities->__any,
		"<tds:Capabilities>"
			"<tds:Network IPFilter=\"false\" ZeroConfiguration=\"false\" IPVersion6=\"false\" DynDNS=\"false\" Dot11Configuration=\"false\" HostnameFromDHCP=\"false\" NTP=\"1\"></tds:Network>"
			"<tds:Security TLS1.0=\"false\" TLS1.1=\"false\" TLS1.2=\"false\" OnboardKeyGeneration=\"false\" AccessPolicyConfig=\"false\" Dot1X=\"false\" RemoteUserHandling=\"false\" X.509Token=\"false\" SAMLToken=\"false\" KerberosToken=\"false\" UsernameToken=\"false\" HttpDigest=\"false\" RELToken=\"false\"></tds:Security>"
			"<tds:System DiscoveryResolve=\"false\" DiscoveryBye=\"false\" RemoteDiscovery=\"false\" SystemBackup=\"false\" SystemLogging=\"false\" FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"false\" HttpSystemBackup=\"false\" HttpSystemLogging=\"false\" HttpSupportInformation=\"false\"></tds:System>"
		"</tds:Capabilities>"
		);

		tds__GetServicesResponse->Service[1].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[1].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[1].Capabilities->__any,
        //"<tds:Capabilities>"
            "<trt:Capabilities>"
                "<trt:ProfileCapabilities MaximumNumberOfProfiles=\"2\"></trt:ProfileCapabilities>"
                "<trt:StreamingCapabilities RTPMulticast=\"true\" RTP_TCP=\"true\" RTP_RTSP_TCP=\"true\" NonAggregateControl=\"false\"></trt:StreamingCapabilities>"
            "</trt:Capabilities>"
        //"</tds:Capabilities>"
		);	

		tds__GetServicesResponse->Service[2].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[2].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[2].Capabilities->__any,
        //"<tds:Capabilities>"
            "<tev:Capabilities WSSubscriptionPolicySupport=\"true\" WSPullPointSupport=\"true\" WSPausableSubscriptionManagerInterfaceSupport=\"false\"></tev:Capabilities>"
        //"</tds:Capabilities>"
		);			

		tds__GetServicesResponse->Service[3].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[3].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[3].Capabilities->__any,
		//"<tds:Capabilities>"
			"<timg:Capabilities/>"
		//"</tds:Capabilities>"
		);
		
		tds__GetServicesResponse->Service[4].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[4].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[4].Capabilities->__any,
		//"<tds:Capabilities>"
			"<tmd:Capabilities VideoSources=\"1\" VideoOutputs=\"0\" AudioSources=\"0\" AudioOutputs=\"0\" RelayOutputs=\"0\"></tmd:Capabilities>"
		//"</tds:Capabilities>"
		);

		tds__GetServicesResponse->Service[5].Capabilities = ONVIF_MALLOC(struct _tds__Service_Capabilities);
		tds__GetServicesResponse->Service[5].Capabilities->__any = ONVIF_MALLOC_SIZE(char, 4096);
		strcpy(tds__GetServicesResponse->Service[5].Capabilities->__any,
		//"<tds:Capabilities>"
			"<tan:Capabilities/>"
		//"</tds:Capabilities>"	
		);				
	}
	return SOAP_OK;		
}
	
SOAP_FMAC5 int SOAP_FMAC6 __tds__GetServiceCapabilities(struct soap *soap, struct _tds__GetServiceCapabilities *tds__GetServiceCapabilities, struct _tds__GetServiceCapabilitiesResponse *tds__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("GetServicesCapabilities\n");
	
	tds__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tds__DeviceServiceCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->__size = 0;
	tds__GetServiceCapabilitiesResponse->Capabilities->__any  = NULL;
	/* NETWORK */
	tds__GetServiceCapabilitiesResponse->Capabilities->Network = ONVIF_MALLOC(struct tds__NetworkCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPFilter = &nfalse;	
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->ZeroConfiguration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->IPVersion6 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->DynDNS = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->Dot11Configuration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->HostnameFromDHCP= &nfalse;	
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP= (int *)soap_malloc(soap, sizeof(int));
	*tds__GetServiceCapabilitiesResponse->Capabilities->Network->NTP= 1;
	tds__GetServiceCapabilitiesResponse->Capabilities->Network->__anyAttribute = NULL;

	/* SYSTEM */
	tds__GetServiceCapabilitiesResponse->Capabilities->System = ONVIF_MALLOC(struct tds__SystemCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryResolve = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->DiscoveryBye = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->RemoteDiscovery = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemBackup = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->FirmwareUpgrade = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->SystemLogging = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemBackup = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpFirmwareUpgrade = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSystemLogging = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->System->HttpSupportInformation = &nfalse;

	/* SECURITY */
	tds__GetServiceCapabilitiesResponse->Capabilities->Security = ONVIF_MALLOC(struct tds__SecurityCapabilities);
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e0 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e1 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->TLS1_x002e2 = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->OnboardKeyGeneration = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->AccessPolicyConfig = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->Dot1X = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RemoteUserHandling = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->X_x002e509Token = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SAMLToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->KerberosToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->UsernameToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->HttpDigest = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->RELToken = &nfalse;
	tds__GetServiceCapabilitiesResponse->Capabilities->Security->SupportedEAPMethods = NULL;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDeviceInformation(struct soap *soap, struct _tds__GetDeviceInformation *tds__GetDeviceInformation, struct _tds__GetDeviceInformationResponse *tds__GetDeviceInformationResponse)
{
	ONVIFDEBUG("GetDeviceInformation\n");
	ONVIF_dup();
	tds__GetDeviceInformationResponse->FirmwareVersion = g_OnvifConf.pConf->ipcam.info.software_version;
	tds__GetDeviceInformationResponse->HardwareId =g_OnvifConf.pConf->ipcam.info.hardware_version;
	tds__GetDeviceInformationResponse->Manufacturer = g_OnvifConf.pConf->ipcam.info.device_name;
	tds__GetDeviceInformationResponse->Model = g_OnvifConf.pConf->ipcam.info.device_model;
	tds__GetDeviceInformationResponse->SerialNumber = g_OnvifConf.pConf->ipcam.info.device_sn;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemDateAndTime(struct soap *soap, struct _tds__SetSystemDateAndTime *tds__SetSystemDateAndTime, struct _tds__SetSystemDateAndTimeResponse *tds__SetSystemDateAndTimeResponse)
{
	////Coding ÉèÖÃÏµÍ³Ê±¼ä
	ONVIFDEBUG("SetSystemDateAndTime\n");	
	
	struct tm settime;
	settime.tm_hour = tds__SetSystemDateAndTime->UTCDateTime->Time->Hour;
	settime.tm_min = tds__SetSystemDateAndTime->UTCDateTime->Time->Minute;
	settime.tm_sec = tds__SetSystemDateAndTime->UTCDateTime->Time->Second;
	settime.tm_year = tds__SetSystemDateAndTime->UTCDateTime->Date->Year;
	settime.tm_mon = tds__SetSystemDateAndTime->UTCDateTime->Date->Month;
	settime.tm_mday = tds__SetSystemDateAndTime->UTCDateTime->Date->Day;


	// ONVIFDEBUG("timezone:%s, SetSystemDateAndTime: syear=%d, smon=%d, sday=%d, shour=%d, smin=%d, ssec=%d\n", tds__SetSystemDateAndTime->TimeZone->TZ, settime.tm_year, 
	// 							settime.tm_mon, settime.tm_mday, settime.tm_hour, settime.tm_min, settime.tm_sec);


	time_t sTime;
	settime.tm_year -= 1900;
	settime.tm_mon -= 1;
	settime.tm_hour -= g_OnvifConf.pConf->ipcam.date_time.time_zone.val;
	struct timeval valTime;
	sTime = mktime(&settime);
	valTime.tv_sec = sTime;
	valTime.tv_usec = 0;
	settimeofday(&valTime, NULL);

	time_t now;
	struct tm *timenow;
	time(&now);
	timenow = localtime(&now);

	TTASK_syn_time(sTime);

	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemDateAndTime(struct soap *soap, struct _tds__GetSystemDateAndTime *tds__GetSystemDateAndTime, struct _tds__GetSystemDateAndTimeResponse *tds__GetSystemDateAndTimeResponse)
{   
	ONVIF_dup();
	time_t now;
	struct tm *timenow;
	time(&now);
	timenow = localtime(&now);

	ONVIFDEBUG("GetSystemDateAndTime: year=%04d, mon=%02d, day=%02d, hour=%02d, min=%02d, sec=%02d\n", 
		timenow->tm_year + 1900, timenow->tm_mon + 1, timenow->tm_mday,
		timenow->tm_hour, timenow->tm_min, timenow->tm_sec);

	tds__GetSystemDateAndTimeResponse->SystemDateAndTime = ONVIF_MALLOC(struct tt__SystemDateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DateTimeType = tt__SetDateTimeType__Manual;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->DaylightSavings = _false;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone = ONVIF_MALLOC(struct tt__TimeZone);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ = ONVIF_MALLOC_SIZE(char, 10);
	snprintf(tds__GetSystemDateAndTimeResponse->SystemDateAndTime->TimeZone->TZ, 10, "UTC+%02d:00", g_OnvifConf.pConf->ipcam.date_time.time_zone.val);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime = ONVIF_MALLOC(struct tt__DateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date = ONVIF_MALLOC(struct tt__Date);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time = ONVIF_MALLOC(struct tt__Time);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Year = timenow->tm_year + 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Month = timenow->tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Date->Day = timenow->tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Hour = timenow->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Minute = timenow->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->LocalDateTime->Time->Second = timenow->tm_sec;
	/*tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime = ONVIF_MALLOC(struct tt__DateTime);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date = ONVIF_MALLOC(struct tt__Date);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time = ONVIF_MALLOC(struct tt__Time);
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Year = timenow->tm_year + 1900;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Month = timenow->tm_mon + 1;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Date->Day = timenow->tm_mday;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Hour = timenow->tm_hour;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Minute = timenow->tm_min;
	tds__GetSystemDateAndTimeResponse->SystemDateAndTime->UTCDateTime->Time->Second = timenow->tm_sec;*/

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetSystemFactoryDefault(struct soap *soap, struct _tds__SetSystemFactoryDefault *tds__SetSystemFactoryDefault, struct _tds__SetSystemFactoryDefaultResponse *tds__SetSystemFactoryDefaultResponse)
{
	////Coding »Ö¸´³ö³§ÉèÖÃ
	ONVIFDEBUG("SetSystemFactoryDefault\n");
	switch(tds__SetSystemFactoryDefault->FactoryDefault){
	case tt__FactoryDefaultType__Hard:
		break;
	case tt__FactoryDefaultType__Soft:
		break;
	default:
		break;
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__UpgradeSystemFirmware(struct soap *soap, struct _tds__UpgradeSystemFirmware *tds__UpgradeSystemFirmware, struct _tds__UpgradeSystemFirmwareResponse *tds__UpgradeSystemFirmwareResponse)
{
	ONVIFDEBUG("unknown%d\n", 14);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SystemReboot(struct soap *soap, struct _tds__SystemReboot *tds__SystemReboot, struct _tds__SystemRebootResponse *tds__SystemRebootResponse)
{
	////Coding ÖØÆôÏµÍ³
	ONVIFDEBUG("SystemReboot\n");
	tds__SystemRebootResponse->Message = "System reboot finished!";
	exit(0);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RestoreSystem(struct soap *soap, struct _tds__RestoreSystem *tds__RestoreSystem, struct _tds__RestoreSystemResponse *tds__RestoreSystemResponse)
{
	ONVIFDEBUG("unknown%d\n", 15);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemBackup(struct soap *soap, struct _tds__GetSystemBackup *tds__GetSystemBackup, struct _tds__GetSystemBackupResponse *tds__GetSystemBackupResponse)
{
	ONVIFDEBUG("unknown%d\n", 16);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemLog(struct soap *soap, struct _tds__GetSystemLog *tds__GetSystemLog, struct _tds__GetSystemLogResponse *tds__GetSystemLogResponse)
{
	ONVIFDEBUG("unknown%d\n", 17);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemSupportInformation(struct soap *soap, struct _tds__GetSystemSupportInformation *tds__GetSystemSupportInformation, struct _tds__GetSystemSupportInformationResponse *tds__GetSystemSupportInformationResponse)
{
	ONVIFDEBUG("unknown%d\n", 18);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetScopes(struct soap *soap, struct _tds__GetScopes *tds__GetScopes, struct _tds__GetScopesResponse *tds__GetScopesResponse)
{
	////Coding HTTP ProbeÇëÇó»Ø¸´»áµ÷ÓÃ´Ë·½·¨
	ONVIFDEBUG("GetScopes\n");
	ONVIF_dup();
	int i;
	for(i = 0; 0 != g_OnvifConf.Scopes[i].item; ++i){

	}
	ONVIFDEBUG("Scopes count :%d\n", i);
	struct tt__Scope *s = ONVIF_MALLOC_SIZE(struct tt__Scope, i);

	for(i = 0; 0 != g_OnvifConf.Scopes[i].item; ++i){
		(s + i)->ScopeItem = ONVIF_MALLOC_SIZE(char, 50);
		(s + i)->ScopeDef = g_OnvifConf.Scopes[i].definition;
		strncpy((s + i)->ScopeItem, g_OnvifConf.Scopes[i].item, 50);
	}


	tds__GetScopesResponse->Scopes = s;
	tds__GetScopesResponse->__sizeScopes = i;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetScopes(struct soap *soap, struct _tds__SetScopes *tds__SetScopes, struct _tds__SetScopesResponse *tds__SetScopesResponse)
{
	ONVIFDEBUG("unknown%d\n", 19);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddScopes(struct soap *soap, struct _tds__AddScopes *tds__AddScopes, struct _tds__AddScopesResponse *tds__AddScopesResponse)
{
	ONVIFDEBUG("unknown%d\n", 20);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveScopes(struct soap *soap, struct _tds__RemoveScopes *tds__RemoveScopes, struct _tds__RemoveScopesResponse *tds__RemoveScopesResponse)
{
	ONVIFDEBUG("unknown%d\n", 21);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDiscoveryMode(struct soap *soap, struct _tds__GetDiscoveryMode *tds__GetDiscoveryMode, struct _tds__GetDiscoveryModeResponse *tds__GetDiscoveryModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 22);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDiscoveryMode(struct soap *soap, struct _tds__SetDiscoveryMode *tds__SetDiscoveryMode, struct _tds__SetDiscoveryModeResponse *tds__SetDiscoveryModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 23);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteDiscoveryMode(struct soap *soap, struct _tds__GetRemoteDiscoveryMode *tds__GetRemoteDiscoveryMode, struct _tds__GetRemoteDiscoveryModeResponse *tds__GetRemoteDiscoveryModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 24);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteDiscoveryMode(struct soap *soap, struct _tds__SetRemoteDiscoveryMode *tds__SetRemoteDiscoveryMode, struct _tds__SetRemoteDiscoveryModeResponse *tds__SetRemoteDiscoveryModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 25);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDPAddresses(struct soap *soap, struct _tds__GetDPAddresses *tds__GetDPAddresses, struct _tds__GetDPAddressesResponse *tds__GetDPAddressesResponse)
{
	ONVIFDEBUG("unknown%d\n", 26);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetEndpointReference(struct soap *soap, struct _tds__GetEndpointReference *tds__GetEndpointReference, struct _tds__GetEndpointReferenceResponse *tds__GetEndpointReferenceResponse)
{
	ONVIFDEBUG("unknown%d\n", 27);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRemoteUser(struct soap *soap, struct _tds__GetRemoteUser *tds__GetRemoteUser, struct _tds__GetRemoteUserResponse *tds__GetRemoteUserResponse)
{
	ONVIFDEBUG("unknown%d\n", 28);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRemoteUser(struct soap *soap, struct _tds__SetRemoteUser *tds__SetRemoteUser, struct _tds__SetRemoteUserResponse *tds__SetRemoteUserResponse)
{
	ONVIFDEBUG("unknown%d\n", 29);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetUsers(struct soap *soap, struct _tds__GetUsers *tds__GetUsers, struct _tds__GetUsersResponse *tds__GetUsersResponse)
{
	ONVIFDEBUG("unknown%d\n", 30);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateUsers(struct soap *soap, struct _tds__CreateUsers *tds__CreateUsers, struct _tds__CreateUsersResponse *tds__CreateUsersResponse)
{
	ONVIFDEBUG("unknown%d\n", 31);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteUsers(struct soap *soap, struct _tds__DeleteUsers *tds__DeleteUsers, struct _tds__DeleteUsersResponse *tds__DeleteUsersResponse)
{
	ONVIFDEBUG("unknown%d\n", 32);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetUser(struct soap *soap, struct _tds__SetUser *tds__SetUser, struct _tds__SetUserResponse *tds__SetUserResponse)
{
	ONVIFDEBUG("unknown%d\n", 33);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetWsdlUrl(struct soap *soap, struct _tds__GetWsdlUrl *tds__GetWsdlUrl, struct _tds__GetWsdlUrlResponse *tds__GetWsdlUrlResponse)
{
	ONVIFDEBUG("__tds__GetWsdlUrl\n");
	
	tds__GetWsdlUrlResponse->WsdlUrl = "http://www.onvif.org/Documents/Specifications.aspx";

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCapabilities(struct soap *soap, struct _tds__GetCapabilities *tds__GetCapabilities, struct _tds__GetCapabilitiesResponse *tds__GetCapabilitiesResponse)
{
	int _Category;
	
	ONVIF_dup();
#define ONVIF_CAPABILITIES_LEN (50)

	if(tds__GetCapabilities->Category == NULL)
	{
		tds__GetCapabilities->Category=(int *)soap_malloc(soap, sizeof(int));
		*tds__GetCapabilities->Category = tt__CapabilityCategory__All;
		_Category = tt__CapabilityCategory__All;
	}
	else
	{
		_Category = *tds__GetCapabilities->Category;
	}
	ONVIFDEBUG("__tds__GetCapabilities:Category(%d)\n", *tds__GetCapabilities->Category);

	tds__GetCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct tt__Capabilities);
	tds__GetCapabilitiesResponse->Capabilities->Analytics = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Device = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Events = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Imaging = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Media = NULL;
	tds__GetCapabilitiesResponse->Capabilities->PTZ = NULL;

	tds__GetCapabilitiesResponse->Capabilities->Extension = ONVIF_MALLOC(struct tt__CapabilitiesExtension);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO = ONVIF_MALLOC(struct tt__DeviceIOCapabilities);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr = ONVIF_MALLOC_SIZE(char,ONVIF_CAPABILITIES_LEN);
	snprintf(tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/deivceIO", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoSources = 1;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->VideoOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioSources = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->AudioOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->RelayOutputs = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__size = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__any = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->DeviceIO->__anyAttribute = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Display = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Recording = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Search = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Replay = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Receiver = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->AnalyticsDevice = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->Extensions = NULL;
	tds__GetCapabilitiesResponse->Capabilities->Extension->__size = 0;
	tds__GetCapabilitiesResponse->Capabilities->Extension->__any = NULL;
	
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Analytics))
	{
/*	
		if(_Category == tt__CapabilityCategory__Analytics){
			onvif_fault(soap,"ter:ActionNotSupported","ter:NoSuchService");
			return SOAP_FAULT;
		}
*/
		tds__GetCapabilitiesResponse->Capabilities->Analytics = ONVIF_MALLOC(struct tt__AnalyticsCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Analytics->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/analytics", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
		tds__GetCapabilitiesResponse->Capabilities->Analytics->RuleSupport = _false;
		tds__GetCapabilitiesResponse->Capabilities->Analytics->AnalyticsModuleSupport = _false;

	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Device))
	{
		tds__GetCapabilitiesResponse->Capabilities->Device = ONVIF_MALLOC(struct tt__DeviceCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Device->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/device", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
		tds__GetCapabilitiesResponse->Capabilities->Device->Network = ONVIF_MALLOC(struct tt__NetworkCapabilities);
		int *pFlag = ONVIF_MALLOC_SIZE(int , 2);
		*pFlag = 0;
		*(pFlag + 1) = 1;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPFilter = pFlag;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->ZeroConfiguration = pFlag;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->IPVersion6 = pFlag;
		tds__GetCapabilitiesResponse->Capabilities->Device->Network->DynDNS = pFlag + 1;
		tds__GetCapabilitiesResponse->Capabilities->Device->System = ONVIF_MALLOC(struct tt__SystemCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryResolve = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->DiscoveryBye = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->RemoteDiscovery = 1;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemBackup = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SystemLogging = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->FirmwareUpgrade = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions = ONVIF_MALLOC(struct tt__OnvifVersion);
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Major = 2;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->SupportedVersions->Minor = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->System->__sizeSupportedVersions = 1;
		tds__GetCapabilitiesResponse->Capabilities->Device->IO = ONVIF_MALLOC(struct tt__IOCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->Security = ONVIF_MALLOC(struct tt__SecurityCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e1 = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->TLS1_x002e2 = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->OnboardKeyGeneration = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->AccessPolicyConfig = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->X_x002e509Token = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->SAMLToken  = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->KerberosToken = 0;
		tds__GetCapabilitiesResponse->Capabilities->Device->Security->RELToken = 0;
		
	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Events))
	{	
		tds__GetCapabilitiesResponse->Capabilities->Events = ONVIF_MALLOC(struct tt__EventCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Events->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Events->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/events", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
		tds__GetCapabilitiesResponse->Capabilities->Events->WSSubscriptionPolicySupport = _false;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPullPointSupport = _false;
		tds__GetCapabilitiesResponse->Capabilities->Events->WSPausableSubscriptionManagerInterfaceSupport = _false;
	}		
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Imaging))
	{	
		tds__GetCapabilitiesResponse->Capabilities->Imaging = ONVIF_MALLOC(struct tt__ImagingCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Imaging->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/imaging", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
	}	
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__Media))
	{
		tds__GetCapabilitiesResponse->Capabilities->Media = ONVIF_MALLOC(struct tt__MediaCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->Media->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->Media->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/media", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities = ONVIF_MALLOC(struct tt__RealTimeStreamingCapabilities);
		int *pRTP = ONVIF_MALLOC_SIZE(int, 1);
		*pRTP = 0;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTPMulticast = pRTP;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORETCP = pRTP;
		tds__GetCapabilitiesResponse->Capabilities->Media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = pRTP;
	}
	if((_Category == tt__CapabilityCategory__All) || (_Category == tt__CapabilityCategory__PTZ))
	{
		if(_Category == tt__CapabilityCategory__PTZ){
			onvif_fault(soap,"ter:ActionNotSupported","ter:NoSuchService");
			return SOAP_FAULT;
		}
		/*
		tds__GetCapabilitiesResponse->Capabilities->PTZ = ONVIF_MALLOC(struct tt__PTZCapabilities);
		tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr = ONVIF_MALLOC_SIZE(char, ONVIF_CAPABILITIES_LEN);
		snprintf(tds__GetCapabilitiesResponse->Capabilities->PTZ->XAddr, ONVIF_CAPABILITIES_LEN, "http://%s:%d/onvif/ptz", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr), g_OnvifConf.pConf->ipcam.network.lan.port[0].value);
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__size = 0;
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__any = NULL;
		tds__GetCapabilitiesResponse->Capabilities->PTZ->__anyAttribute = NULL;
		*/
	}	
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDPAddresses(struct soap *soap, struct _tds__SetDPAddresses *tds__SetDPAddresses, struct _tds__SetDPAddressesResponse *tds__SetDPAddressesResponse)
{
	////Coding ÉèÖÃIPµØÖ·
	ONVIFDEBUG("SetDPAddresses\n");


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetHostname(struct soap *soap, struct _tds__GetHostname *tds__GetHostname, struct _tds__GetHostnameResponse *tds__GetHostnameResponse)
{
	////Coding »ñÈ¡Ö÷»úÃû³Æ
	ONVIFDEBUG("GetHostName\n");
	ONVIF_dup();
	tds__GetHostnameResponse->HostnameInformation = ONVIF_MALLOC(struct tt__HostnameInformation);
	tds__GetHostnameResponse->HostnameInformation->Name = ONVIF_MALLOC_SIZE(char, 30);
	gethostname(tds__GetHostnameResponse->HostnameInformation->Name, 30);
	tds__GetHostnameResponse->HostnameInformation->FromDHCP = _false;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostname(struct soap *soap, struct _tds__SetHostname *tds__SetHostname, struct _tds__SetHostnameResponse *tds__SetHostnameResponse)
{
	ONVIFDEBUG("unknown%d\n", 35);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetHostnameFromDHCP(struct soap *soap, struct _tds__SetHostnameFromDHCP *tds__SetHostnameFromDHCP, struct _tds__SetHostnameFromDHCPResponse *tds__SetHostnameFromDHCPResponse)
{
	ONVIFDEBUG("unknown%d\n", 36);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDNS(struct soap *soap, struct _tds__GetDNS *tds__GetDNS, struct _tds__GetDNSResponse *tds__GetDNSResponse)
{
	ONVIFDEBUG("__tds__GetDNS\n");
	
	char _dns[LARGE_INFO_LENGTH];	
	snprintf(_dns, LARGE_INFO_LENGTH, "%s", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_preferred_dns.in_addr));

	ONVIF_dup();
	tds__GetDNSResponse->DNSInformation = ONVIF_MALLOC(struct tt__DNSInformation);
	tds__GetDNSResponse->DNSInformation->FromDHCP = _false;
	tds__GetDNSResponse->DNSInformation->__sizeSearchDomain = 0;
	tds__GetDNSResponse->DNSInformation->SearchDomain = NULL;
	tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 0;
	tds__GetDNSResponse->DNSInformation->DNSFromDHCP = NULL;
	tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 0;
	tds__GetDNSResponse->DNSInformation->Extension = NULL;

	if((tds__GetDNSResponse->DNSInformation->FromDHCP) == 1) 
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSFromDHCP = 1;
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP = ONVIF_MALLOC(struct tt__IPAddress);
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->Type = 0; 
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
		strcpy(tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv4Address, _dns);
		tds__GetDNSResponse->DNSInformation->DNSFromDHCP->IPv6Address = NULL;
	}
	else 
	{
		tds__GetDNSResponse->DNSInformation->__sizeDNSManual = 1;
		tds__GetDNSResponse->DNSInformation->DNSManual = ONVIF_MALLOC(struct tt__IPAddress);
		tds__GetDNSResponse->DNSInformation->DNSManual->Type = tt__IPType__IPv4; 
		tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
		strcpy(tds__GetDNSResponse->DNSInformation->DNSManual->IPv4Address, _dns);
		tds__GetDNSResponse->DNSInformation->DNSManual->IPv6Address = NULL;
	}
	return SOAP_OK;
	
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDNS(struct soap *soap, struct _tds__SetDNS *tds__SetDNS, struct _tds__SetDNSResponse *tds__SetDNSResponse)
{
	ONVIFDEBUG("__tds__SetDNS\n");

	struct sockaddr_in dns_ip;
	int sys_dhcp = 0/*oget_dhcpstatus()*/;
	int value = tds__SetDNS->FromDHCP;
	char _IPv4Address[SMALL_INFO_LENGTH];

	if(tds__SetDNS->SearchDomain)
	{
		//strcpy(domainname, *(tds__SetDNS->SearchDomain));
	}
	if (sys_dhcp != value) 
	{
		//oset_dhcpstatus(value);
		if(value == 1)
		{
			return SOAP_OK;
		}
	}
	if(tds__SetDNS->__sizeSearchDomain)
	{
		//tempdnssearchdomainsize = tds__SetDNS->__sizeSearchDomain;
	}
	else
	{
		//tempdnssearchdomainsize = 0;
	}

	if(tds__SetDNS->DNSManual == NULL)
	{
		return SOAP_OK;
	}

	if(tds__SetDNS->DNSManual->Type != 0) 
	{
		onvif_fault(soap,"ter:NotSupported", "ter:InvalidIPv6Address");
		return SOAP_FAULT;
	}	

	strcpy(_IPv4Address, tds__SetDNS->DNSManual->IPv4Address);
	if(isValidIp4(_IPv4Address) == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidIPv4Address");
		return SOAP_FAULT;
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNTP(struct soap *soap, struct _tds__GetNTP *tds__GetNTP, struct _tds__GetNTPResponse *tds__GetNTPResponse)
{
	ONVIFDEBUG("unknown%d\n", 39);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNTP(struct soap *soap, struct _tds__SetNTP *tds__SetNTP, struct _tds__SetNTPResponse *tds__SetNTPResponse)
{
	ONVIFDEBUG("unknown%d\n", 40);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDynamicDNS(struct soap *soap, struct _tds__GetDynamicDNS *tds__GetDynamicDNS, struct _tds__GetDynamicDNSResponse *tds__GetDynamicDNSResponse)
{
	ONVIFDEBUG("unknown%d\n", 41);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDynamicDNS(struct soap *soap, struct _tds__SetDynamicDNS *tds__SetDynamicDNS, struct _tds__SetDynamicDNSResponse *tds__SetDynamicDNSResponse)
{
	ONVIFDEBUG("unknown%d\n", 42);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkInterfaces(struct soap *soap, struct _tds__GetNetworkInterfaces *tds__GetNetworkInterfaces, struct _tds__GetNetworkInterfacesResponse *tds__GetNetworkInterfacesResponse)
{
	ONVIFDEBUG("GetNetworkInterfaces\n");
	ONVIF_dup();
	tds__GetNetworkInterfacesResponse->__sizeNetworkInterfaces = 1;    /* required attribute of type tt:ReferenceToken */
	tds__GetNetworkInterfacesResponse->NetworkInterfaces = ONVIF_MALLOC(struct tt__NetworkInterface);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->token = "Networkinterface_default_token";
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Enabled = _true;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info = ONVIF_MALLOC(struct tt__NetworkInterfaceInfo);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress = ONVIF_MALLOC_SIZE(char, 20);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->HwAddress, 20, "%02x-%02x-%02x-%02x-%02x-%02x", g_OnvifConf.pConf->ipcam.network.mac.s1,
																						g_OnvifConf.pConf->ipcam.network.mac.s2,
																						g_OnvifConf.pConf->ipcam.network.mac.s3,
																						g_OnvifConf.pConf->ipcam.network.mac.s4,
																						g_OnvifConf.pConf->ipcam.network.mac.s5,
																						g_OnvifConf.pConf->ipcam.network.mac.s6);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = ONVIF_MALLOC(int);
	*tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->MTU = 1500;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->Info->Name = "eth0";

	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4 = ONVIF_MALLOC(struct tt__IPv4NetworkInterface);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Enabled = _true;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config = ONVIF_MALLOC(struct tt__IPv4Configuration);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->DHCP = g_OnvifConf.pConf->ipcam.network.lan.dhcp;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual = ONVIF_MALLOC(struct tt__PrefixedIPv4Address);
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address = ONVIF_MALLOC_SIZE(char, 20);
	snprintf(tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->Address, 20, "%s", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr));
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->Manual->PrefixLength = 24;
	tds__GetNetworkInterfacesResponse->NetworkInterfaces->IPv4->Config->__sizeManual = 1;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkInterfaces(struct soap *soap, struct _tds__SetNetworkInterfaces *tds__SetNetworkInterfaces, struct _tds__SetNetworkInterfacesResponse *tds__SetNetworkInterfacesResponse)
{
	ONVIFDEBUG("unknown%d\n", 43);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkProtocols(struct soap *soap, struct _tds__GetNetworkProtocols *tds__GetNetworkProtocols, struct _tds__GetNetworkProtocolsResponse *tds__GetNetworkProtocolsResponse)
{
	ONVIFDEBUG("GetNetWorkProtocols\n");
		/*g_NetworkProtocols[0].Enabled = _true;
	g_NetworkProtocols[0].Name = tt__NetworkProtocolType__HTTP;
	g_NetworkProtocols[0].Port = &g_nServerPort;
	g_NetworkProtocols[0].__sizePort = 1;
*/
	ONVIF_dup();
	tds__GetNetworkProtocolsResponse->__sizeNetworkProtocols = 1;
	tds__GetNetworkProtocolsResponse->NetworkProtocols = ONVIF_MALLOC(struct tt__NetworkProtocol);
	tds__GetNetworkProtocolsResponse->NetworkProtocols->Name = tt__NetworkProtocolType__HTTP;
	tds__GetNetworkProtocolsResponse->NetworkProtocols->Port = ONVIF_MALLOC_SIZE(int, 1);
	*tds__GetNetworkProtocolsResponse->NetworkProtocols->Port = g_OnvifConf.pConf->ipcam.network.lan.port[0].value;
	tds__GetNetworkProtocolsResponse->NetworkProtocols->__sizePort = 1;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkProtocols(struct soap *soap, struct _tds__SetNetworkProtocols *tds__SetNetworkProtocols, struct _tds__SetNetworkProtocolsResponse *tds__SetNetworkProtocolsResponse)
{
	ONVIFDEBUG("unknown%d\n", 44);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetNetworkDefaultGateway(struct soap *soap, struct _tds__GetNetworkDefaultGateway *tds__GetNetworkDefaultGateway, struct _tds__GetNetworkDefaultGatewayResponse *tds__GetNetworkDefaultGatewayResponse)
{
	ONVIFDEBUG("GetNetworkDefaultGateway\n");
		/*static char Gateway[16] = {0};
	IPADDRESS_NTOA(Gateway, g_pSysConf->ipcam.network.lan.static_gateway);
	g_NetworkGateway.__sizeIPv4Address = 1;
	static char *pGatway = &Gateway;
	g_NetworkGateway.IPv4Address = &pGatway;*/
	ONVIF_dup();
	char *address = ONVIF_MALLOC_SIZE(char, 16);
	snprintf(address, 16, "%s", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_gateway.in_addr));
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway = ONVIF_MALLOC(struct tt__NetworkGateway);
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address = ONVIF_MALLOC(char *);
	*tds__GetNetworkDefaultGatewayResponse->NetworkGateway->IPv4Address = address;
	tds__GetNetworkDefaultGatewayResponse->NetworkGateway->__sizeIPv4Address = 1;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetNetworkDefaultGateway(struct soap *soap, struct _tds__SetNetworkDefaultGateway *tds__SetNetworkDefaultGateway, struct _tds__SetNetworkDefaultGatewayResponse *tds__SetNetworkDefaultGatewayResponse)
{
	ONVIFDEBUG("SetNetWorkDefaultGateway\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetZeroConfiguration(struct soap *soap, struct _tds__GetZeroConfiguration *tds__GetZeroConfiguration, struct _tds__GetZeroConfigurationResponse *tds__GetZeroConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 45);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetZeroConfiguration(struct soap *soap, struct _tds__SetZeroConfiguration *tds__SetZeroConfiguration, struct _tds__SetZeroConfigurationResponse *tds__SetZeroConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 46);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetIPAddressFilter(struct soap *soap, struct _tds__GetIPAddressFilter *tds__GetIPAddressFilter, struct _tds__GetIPAddressFilterResponse *tds__GetIPAddressFilterResponse)
{

	ONVIFDEBUG("unknown%d\n", 47);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetIPAddressFilter(struct soap *soap, struct _tds__SetIPAddressFilter *tds__SetIPAddressFilter, struct _tds__SetIPAddressFilterResponse *tds__SetIPAddressFilterResponse)
{
	ONVIFDEBUG("un done SetIPAddressFilter\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__AddIPAddressFilter(struct soap *soap, struct _tds__AddIPAddressFilter *tds__AddIPAddressFilter, struct _tds__AddIPAddressFilterResponse *tds__AddIPAddressFilterResponse)
{
	ONVIFDEBUG("undone AddIPAddressFilter\n");
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__RemoveIPAddressFilter(struct soap *soap, struct _tds__RemoveIPAddressFilter *tds__RemoveIPAddressFilter, struct _tds__RemoveIPAddressFilterResponse *tds__RemoveIPAddressFilterResponse)
{
	ONVIFDEBUG("unknown%d\n", 48);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetAccessPolicy(struct soap *soap, struct _tds__GetAccessPolicy *tds__GetAccessPolicy, struct _tds__GetAccessPolicyResponse *tds__GetAccessPolicyResponse)
{
	ONVIFDEBUG("unknown%d\n", 49);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetAccessPolicy(struct soap *soap, struct _tds__SetAccessPolicy *tds__SetAccessPolicy, struct _tds__SetAccessPolicyResponse *tds__SetAccessPolicyResponse)
{
	ONVIFDEBUG("unknown%d\n", 50);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateCertificate(struct soap *soap, struct _tds__CreateCertificate *tds__CreateCertificate, struct _tds__CreateCertificateResponse *tds__CreateCertificateResponse)
{
	ONVIFDEBUG("unknown%d\n", 51);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificates(struct soap *soap, struct _tds__GetCertificates *tds__GetCertificates, struct _tds__GetCertificatesResponse *tds__GetCertificatesResponse)
{
	ONVIFDEBUG("unknown%d\n", 52);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificatesStatus(struct soap *soap, struct _tds__GetCertificatesStatus *tds__GetCertificatesStatus, struct _tds__GetCertificatesStatusResponse *tds__GetCertificatesStatusResponse)
{
	ONVIFDEBUG("unknown%d\n", 53);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetCertificatesStatus(struct soap *soap, struct _tds__SetCertificatesStatus *tds__SetCertificatesStatus, struct _tds__SetCertificatesStatusResponse *tds__SetCertificatesStatusResponse)
{
	ONVIFDEBUG("unknown%d\n", 54);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteCertificates(struct soap *soap, struct _tds__DeleteCertificates *tds__DeleteCertificates, struct _tds__DeleteCertificatesResponse *tds__DeleteCertificatesResponse)
{
	ONVIFDEBUG("unknown%d\n", 55);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetPkcs10Request(struct soap *soap, struct _tds__GetPkcs10Request *tds__GetPkcs10Request, struct _tds__GetPkcs10RequestResponse *tds__GetPkcs10RequestResponse)
{
	ONVIFDEBUG("unknown%d\n", 56);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificates(struct soap *soap, struct _tds__LoadCertificates *tds__LoadCertificates, struct _tds__LoadCertificatesResponse *tds__LoadCertificatesResponse)
{
	ONVIFDEBUG("unknown%d\n", 57);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetClientCertificateMode(struct soap *soap, struct _tds__GetClientCertificateMode *tds__GetClientCertificateMode, struct _tds__GetClientCertificateModeResponse *tds__GetClientCertificateModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 58);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetClientCertificateMode(struct soap *soap, struct _tds__SetClientCertificateMode *tds__SetClientCertificateMode, struct _tds__SetClientCertificateModeResponse *tds__SetClientCertificateModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 59);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetRelayOutputs(struct soap *soap, struct _tds__GetRelayOutputs *tds__GetRelayOutputs, struct _tds__GetRelayOutputsResponse *tds__GetRelayOutputsResponse)
{
	ONVIFDEBUG("GetRelayOutputs\n");
/*	static struct tt__RelayOutput RelayOutputs[0];
	static struct tt__RelayOutputSettings Properties;
	Properties.DelayTime = "PT1S";
	Properties.IdleState = tt__RelayIdleState__closed;
	Properties.Mode = tt__RelayMode__Bistable;
	RelayOutputs[0].Properties = &Properties;
	RelayOutputs[0].token = "RelayOutputs_token_1";
*/
	tds__GetRelayOutputsResponse->RelayOutputs = ONVIF_MALLOC(struct tt__RelayOutput);
	tds__GetRelayOutputsResponse->RelayOutputs->Properties = ONVIF_MALLOC(struct tt__RelayOutputSettings);
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->DelayTime = "PT1S";
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->IdleState = tt__RelayIdleState__closed;
	tds__GetRelayOutputsResponse->RelayOutputs->Properties->Mode = tt__RelayMode__Bistable;
	tds__GetRelayOutputsResponse->RelayOutputs->token = "RelayOutputs_token_1";
	tds__GetRelayOutputsResponse->__sizeRelayOutputs = 1;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputSettings(struct soap *soap, struct _tds__SetRelayOutputSettings *tds__SetRelayOutputSettings, struct _tds__SetRelayOutputSettingsResponse *tds__SetRelayOutputSettingsResponse)
{
	ONVIFDEBUG("unknown%d\n", 61);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetRelayOutputState(struct soap *soap, struct _tds__SetRelayOutputState *tds__SetRelayOutputState, struct _tds__SetRelayOutputStateResponse *tds__SetRelayOutputStateResponse)
{
	ONVIFDEBUG("unknown%d\n", 62);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SendAuxiliaryCommand(struct soap *soap, struct _tds__SendAuxiliaryCommand *tds__SendAuxiliaryCommand, struct _tds__SendAuxiliaryCommandResponse *tds__SendAuxiliaryCommandResponse)
{
	ONVIFDEBUG("unknown%d\n", 63);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCACertificates(struct soap *soap, struct _tds__GetCACertificates *tds__GetCACertificates, struct _tds__GetCACertificatesResponse *tds__GetCACertificatesResponse)
{
	ONVIFDEBUG("unknown%d\n", 64);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCertificateWithPrivateKey(struct soap *soap, struct _tds__LoadCertificateWithPrivateKey *tds__LoadCertificateWithPrivateKey, struct _tds__LoadCertificateWithPrivateKeyResponse *tds__LoadCertificateWithPrivateKeyResponse)
{
	ONVIFDEBUG("unknown%d\n", 65);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetCertificateInformation(struct soap *soap, struct _tds__GetCertificateInformation *tds__GetCertificateInformation, struct _tds__GetCertificateInformationResponse *tds__GetCertificateInformationResponse)
{
	ONVIFDEBUG("unknown%d\n", 66);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__LoadCACertificates(struct soap *soap, struct _tds__LoadCACertificates *tds__LoadCACertificates, struct _tds__LoadCACertificatesResponse *tds__LoadCACertificatesResponse)
{
	ONVIFDEBUG("unknown%d\n", 67);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__CreateDot1XConfiguration(struct soap *soap, struct _tds__CreateDot1XConfiguration *tds__CreateDot1XConfiguration, struct _tds__CreateDot1XConfigurationResponse *tds__CreateDot1XConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 68);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__SetDot1XConfiguration(struct soap *soap, struct _tds__SetDot1XConfiguration *tds__SetDot1XConfiguration, struct _tds__SetDot1XConfigurationResponse *tds__SetDot1XConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 69);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfiguration(struct soap *soap, struct _tds__GetDot1XConfiguration *tds__GetDot1XConfiguration, struct _tds__GetDot1XConfigurationResponse *tds__GetDot1XConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 70);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot1XConfigurations(struct soap *soap, struct _tds__GetDot1XConfigurations *tds__GetDot1XConfigurations, struct _tds__GetDot1XConfigurationsResponse *tds__GetDot1XConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 71);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__DeleteDot1XConfiguration(struct soap *soap, struct _tds__DeleteDot1XConfiguration *tds__DeleteDot1XConfiguration, struct _tds__DeleteDot1XConfigurationResponse *tds__DeleteDot1XConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 72);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Capabilities(struct soap *soap, struct _tds__GetDot11Capabilities *tds__GetDot11Capabilities, struct _tds__GetDot11CapabilitiesResponse *tds__GetDot11CapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 73);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetDot11Status(struct soap *soap, struct _tds__GetDot11Status *tds__GetDot11Status, struct _tds__GetDot11StatusResponse *tds__GetDot11StatusResponse)
{
	ONVIFDEBUG("unknown%d\n", 74);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__ScanAvailableDot11Networks(struct soap *soap, struct _tds__ScanAvailableDot11Networks *tds__ScanAvailableDot11Networks, struct _tds__ScanAvailableDot11NetworksResponse *tds__ScanAvailableDot11NetworksResponse)
{
	ONVIFDEBUG("unknown%d\n", 75);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__GetSystemUris(struct soap *soap, struct _tds__GetSystemUris *tds__GetSystemUris, struct _tds__GetSystemUrisResponse *tds__GetSystemUrisResponse)
{
	ONVIFDEBUG("unknown%d\n", 76);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartFirmwareUpgrade(struct soap *soap, struct _tds__StartFirmwareUpgrade *tds__StartFirmwareUpgrade, struct _tds__StartFirmwareUpgradeResponse *tds__StartFirmwareUpgradeResponse)
{
	ONVIFDEBUG("unknown%d\n", 77);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tds__StartSystemRestore(struct soap *soap, struct _tds__StartSystemRestore *tds__StartSystemRestore, struct _tds__StartSystemRestoreResponse *tds__StartSystemRestoreResponse)
{
	ONVIFDEBUG("unknown%d\n", 78);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetServiceCapabilities(struct soap *soap, struct _timg__GetServiceCapabilities *timg__GetServiceCapabilities, struct _timg__GetServiceCapabilitiesResponse *timg__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 79);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetImagingSettings(struct soap *soap, struct _timg__GetImagingSettings *timg__GetImagingSettings, struct _timg__GetImagingSettingsResponse *timg__GetImagingSettingsResponse)
{
	ONVIFDEBUG("GetImagingSettings, VideoSourceToken:%s\n", timg__GetImagingSettings->VideoSourceToken);

	int token_exist = 0;
	int i;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(timg__GetImagingSettings->VideoSourceToken, g_OnvifConf.Profile[i].VSCSourceToken) == 0)
		{
			token_exist = 1;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}

	timg__GetImagingSettingsResponse->ImagingSettings = ONVIF_MALLOC(struct tt__ImagingSettings20);
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensation20);
	//timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = ONVIF_MALLOC(enum tt__BacklightCompensationMode);
	timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
	//timg__GetImagingSettingsResponse->ImagingSettings->BacklightCompensation->__sizeMode = 1;
	timg__GetImagingSettingsResponse->ImagingSettings->Brightness = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Brightness = g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.val;
	timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->ColorSaturation = g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.val;
	timg__GetImagingSettingsResponse->ImagingSettings->Contrast = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Contrast = g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.val;
	timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = ONVIF_MALLOC(float);
	*timg__GetImagingSettingsResponse->ImagingSettings->Sharpness = g_OnvifConf.pConf->ipcam.isp.image_attr.sharpen.val;
	timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = ONVIF_MALLOC(enum tt__IrCutFilterMode);
	int ircut = 0;
	if(0 == g_OnvifConf.pConf->ipcam.isp.day_night_mode.ircut_mode){
		ircut = 2;
	}
	else if(2 == g_OnvifConf.pConf->ipcam.isp.day_night_mode.ircut_mode){
		ircut = 0;
	}
	else{
		ircut = 1;
	}
	*timg__GetImagingSettingsResponse->ImagingSettings->IrCutFilter = ircut;

	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange = ONVIF_MALLOC(struct tt__WideDynamicRange20);
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Mode = tt__WideDynamicMode__OFF/*g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.enable*/;
	timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level = ONVIF_MALLOC(float); 
	*timg__GetImagingSettingsResponse->ImagingSettings->WideDynamicRange->Level = g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.strength.val;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__SetImagingSettings(struct soap *soap, struct _timg__SetImagingSettings *timg__SetImagingSettings, struct _timg__SetImagingSettingsResponse *timg__SetImagingSettingsResponse)
{
	ONVIFDEBUG("reqVideoSourceToken:%s::rep:%s\n", timg__SetImagingSettings->VideoSourceToken,g_OnvifConf.Profile[0].VSCSourceToken);

	int token_exist = 0;
	int i;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(timg__SetImagingSettings->VideoSourceToken, g_OnvifConf.Profile[i].VSCSourceToken) == 0)
		{
			token_exist = 1;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}

	ONVIF_dup();
	int brightness = 0, saturation = 0, contrast = 0, sharpen = 0;
	if(NULL != timg__SetImagingSettings->ImagingSettings->Brightness){
		brightness = *timg__SetImagingSettings->ImagingSettings->Brightness;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->ColorSaturation){
		saturation = *timg__SetImagingSettings->ImagingSettings->ColorSaturation;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->Contrast){
		contrast = *timg__SetImagingSettings->ImagingSettings->Contrast;
	}
	if(NULL != timg__SetImagingSettings->ImagingSettings->Sharpness){
		sharpen = *timg__SetImagingSettings->ImagingSettings->Sharpness;
	}
/*	#define IMGGING_SETTINGS_MAX 255
	if(brightness > g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.max){
		brightness = brightness / IMGGING_SETTINGS_MAX * g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.max;
	}
	if(saturation > g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.max){
		saturation = saturation / IMGGING_SETTINGS_MAX * g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.max;
	}
	if(contrast > g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.max){
		contrast = contrast / IMGGING_SETTINGS_MAX * g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.max;
	}
*/
	ONVIFDEBUG("SetImagingSettings:%s, brightness:%d, saturation:%d, contrast:%d, sharpen:%d\n", 
									timg__SetImagingSettings->VideoSourceToken,
									brightness, 
									saturation, 
									contrast, 
									sharpen);	

	g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.val = brightness;
	SENSOR_brightness_set(brightness);
	g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.val = saturation;
	SENSOR_saturation_set(saturation);
	g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.val = contrast;
	SENSOR_contrast_set(contrast);
	g_OnvifConf.pConf->ipcam.isp.image_attr.sharpen.val = sharpen;
	
	//enum tt__IrCutFilterMode {tt__IrCutFilterMode__ON = 0, tt__IrCutFilterMode__OFF = 1, tt__IrCutFilterMode__AUTO = 2};
	//SYS_U32_t ircut_mode;//0:auto 1:daylight 2:night
	int ircut = 0;
	if(0 == *timg__SetImagingSettings->ImagingSettings->IrCutFilter){
		ircut = 2;
	}
	else if(2 == *timg__SetImagingSettings->ImagingSettings->IrCutFilter){
		ircut = 0;
	}
	else{
		ircut = 1;
	}
	g_OnvifConf.pConf->ipcam.isp.day_night_mode.ircut_mode = ircut;

	//g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.strength.val = *timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Level;
	g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.enable = timg__SetImagingSettings->ImagingSettings->WideDynamicRange->Mode;

	ONVIF_Ctrldata(ONVIF_CTRL_SET_SYSCONF, g_OnvifConf.pConf);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetOptions(struct soap *soap, struct _timg__GetOptions *timg__GetOptions, struct _timg__GetOptionsResponse *timg__GetOptionsResponse)
{
	ONVIFDEBUG("ImagingGetOptions, VideoSourceToken:%s\n", timg__GetOptions->VideoSourceToken);

	int token_exist = 0;
	int i;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(timg__GetOptions->VideoSourceToken, g_OnvifConf.Profile[i].VSCSourceToken) == 0)
		{
			token_exist = 1;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}

	ONVIF_dup();
	timg__GetOptionsResponse->ImagingOptions = ONVIF_MALLOC(struct tt__ImagingOptions20);
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensationOptions20);
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = ONVIF_MALLOC(enum tt__BacklightCompensationMode);
	*timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
	timg__GetOptionsResponse->ImagingOptions->BacklightCompensation->__sizeMode = 1;

	timg__GetOptionsResponse->ImagingOptions->Brightness = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Brightness->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->Brightness->Max = g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.max;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->ColorSaturation->Max = g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.max;
	timg__GetOptionsResponse->ImagingOptions->Contrast = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Contrast->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->Contrast->Max = g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.max;
	timg__GetOptionsResponse->ImagingOptions->Sharpness = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->Sharpness->Max = g_OnvifConf.pConf->ipcam.isp.image_attr.sharpen.max;
	enum tt__IrCutFilterMode *IrCutFilterMode = ONVIF_MALLOC_SIZE(enum tt__IrCutFilterMode, 3);

	for(i = 0; i < 3; ++i){
		*(i + IrCutFilterMode) = i;	
	}
	timg__GetOptionsResponse->ImagingOptions->__sizeIrCutFilterModes = i;
	timg__GetOptionsResponse->ImagingOptions->IrCutFilterModes = IrCutFilterMode;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange = ONVIF_MALLOC(struct tt__WideDynamicRangeOptions20);
	enum tt__WideDynamicMode *WideDynamicMode = ONVIF_MALLOC_SIZE(enum tt__WideDynamicMode, 2);
	for(i = 0; i < 2; ++i){
		*(i + WideDynamicMode) = i;
	}
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->__sizeMode = i;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Mode = WideDynamicMode;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level = ONVIF_MALLOC(struct tt__FloatRange);
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Min = 0;
	timg__GetOptionsResponse->ImagingOptions->WideDynamicRange->Level->Max = g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.strength.max;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Move(struct soap *soap, struct _timg__Move *timg__Move, struct _timg__MoveResponse *timg__MoveResponse)
{
	ONVIFDEBUG("__timg__Move\n");
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__Stop(struct soap *soap, struct _timg__Stop *timg__Stop, struct _timg__StopResponse *timg__StopResponse)
{
	ONVIFDEBUG("__timg__Stop\n");
	
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");

	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetStatus(struct soap *soap, struct _timg__GetStatus *timg__GetStatus, struct _timg__GetStatusResponse *timg__GetStatusResponse)
{
	ONVIFDEBUG("__timg__GetStatus\n");
	onvif_fault(soap,"ter:ActionNotSupported", "ter:NoImagingForSource");
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __timg__GetMoveOptions(struct soap *soap, struct _timg__GetMoveOptions *timg__GetMoveOptions, struct _timg__GetMoveOptionsResponse *timg__GetMoveOptionsResponse)
{
	ONVIFDEBUG("__timg__GetMoveOptions\n");
	int token_exist = 0;
	int i;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(timg__GetMoveOptions->VideoSourceToken, g_OnvifConf.Profile[i].VSCSourceToken) == 0)
		{
			token_exist = 1;
			break;
		}
	}
	if(!token_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoSource");
		return SOAP_FAULT;
	}
	
	timg__GetMoveOptionsResponse->MoveOptions = ONVIF_MALLOC(struct _timg__GetMoveOptionsResponse);
	timg__GetMoveOptionsResponse->MoveOptions->Absolute = NULL;
	timg__GetMoveOptionsResponse->MoveOptions->Relative = NULL;
	timg__GetMoveOptionsResponse->MoveOptions->Continuous = NULL;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap *soap, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 87);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap *soap, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse)
{
	tptz__GetConfigurationsResponse->__sizePTZConfiguration = 1;
	tptz__GetConfigurationsResponse->PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);
	tptz__GetConfigurationsResponse->PTZConfiguration->Name = "PTZConf";
	tptz__GetConfigurationsResponse->PTZConfiguration->UseCount = 1;
	tptz__GetConfigurationsResponse->PTZConfiguration->token = "PTZConfToken";
	tptz__GetConfigurationsResponse->PTZConfiguration->NodeToken = "PTZNodeToken";

	ONVIFDEBUG("unknown%d\n", 88);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap *soap, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse)
{
	ONVIFDEBUG("unknown%d\n", 89);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap *soap, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse)
{
	ONVIFDEBUG("unknown%d\n", 90);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap *soap, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse)
{
	ONVIFDEBUG("unknown%d\n", 91);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap *soap, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse)
{
	ONVIFDEBUG("unknown%d\n", 92);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap *soap, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse)
{
	ONVIFDEBUG("unknown%d\n", 93);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap *soap, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 94);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap *soap, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse)
{
	ONVIFDEBUG("GetNodes\n");

	tptz__GetNodesResponse->__sizePTZNode = 0;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap *soap, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse)
{
	ONVIFDEBUG("unknown%d\n", 96);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap *soap, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 97);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap *soap, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse)
{
	ONVIFDEBUG("tptz__GetConfigurationOptions:%s\n", tptz__GetConfigurationOptions->ConfigurationToken);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions = ONVIF_MALLOC(struct tt__PTZConfigurationOptions);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout = ONVIF_MALLOC(struct tt__DurationRange);
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Max = "PT2H";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Min = "PT0S";
	tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces = ONVIF_MALLOC(struct tt__PTZSpaces);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap *soap, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse)
{
	ONVIFDEBUG("unknown%d\n", 99);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap *soap, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse)
{
	ONVIFDEBUG("unknown%d\n", 100);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap *soap, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse)
{
	ONVIFDEBUG("unknown%d\n", 101);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap *soap, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse)
{
	ONVIFDEBUG("unknown%d\n", 102);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap *soap, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse)
{
	ONVIFDEBUG("unknown%d\n", 103);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap *soap, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse)
{
	ONVIFDEBUG("unknown%d\n", 104);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap *soap, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse)
{
	ONVIFDEBUG("unknown%d\n", 105);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__GetServiceCapabilities(struct soap *soap, struct _trcv__GetServiceCapabilities *trcv__GetServiceCapabilities, struct _trcv__GetServiceCapabilitiesResponse *trcv__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 106);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__GetReceivers(struct soap *soap, struct _trcv__GetReceivers *trcv__GetReceivers, struct _trcv__GetReceiversResponse *trcv__GetReceiversResponse)
{
	ONVIFDEBUG("unknown%d\n", 107);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__GetReceiver(struct soap *soap, struct _trcv__GetReceiver *trcv__GetReceiver, struct _trcv__GetReceiverResponse *trcv__GetReceiverResponse)
{
	ONVIFDEBUG("unknown%d\n", 108);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__CreateReceiver(struct soap *soap, struct _trcv__CreateReceiver *trcv__CreateReceiver, struct _trcv__CreateReceiverResponse *trcv__CreateReceiverResponse)
{
	ONVIFDEBUG("unknown%d\n", 109);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__DeleteReceiver(struct soap *soap, struct _trcv__DeleteReceiver *trcv__DeleteReceiver, struct _trcv__DeleteReceiverResponse *trcv__DeleteReceiverResponse)
{
	ONVIFDEBUG("unknown%d\n", 110);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__ConfigureReceiver(struct soap *soap, struct _trcv__ConfigureReceiver *trcv__ConfigureReceiver, struct _trcv__ConfigureReceiverResponse *trcv__ConfigureReceiverResponse)
{
	ONVIFDEBUG("unknown%d\n", 111);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__SetReceiverMode(struct soap *soap, struct _trcv__SetReceiverMode *trcv__SetReceiverMode, struct _trcv__SetReceiverModeResponse *trcv__SetReceiverModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 112);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trcv__GetReceiverState(struct soap *soap, struct _trcv__GetReceiverState *trcv__GetReceiverState, struct _trcv__GetReceiverStateResponse *trcv__GetReceiverStateResponse)
{
	ONVIFDEBUG("unknown%d\n", 113);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetServiceCapabilities(struct soap *soap, struct _trec__GetServiceCapabilities *trec__GetServiceCapabilities, struct _trec__GetServiceCapabilitiesResponse *trec__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 114);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__CreateRecording(struct soap *soap, struct _trec__CreateRecording *trec__CreateRecording, struct _trec__CreateRecordingResponse *trec__CreateRecordingResponse)
{
	ONVIFDEBUG("unknown%d\n", 115);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__DeleteRecording(struct soap *soap, struct _trec__DeleteRecording *trec__DeleteRecording, struct _trec__DeleteRecordingResponse *trec__DeleteRecordingResponse)
{
	ONVIFDEBUG("unknown%d\n", 116);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetRecordings(struct soap *soap, struct _trec__GetRecordings *trec__GetRecordings, struct _trec__GetRecordingsResponse *trec__GetRecordingsResponse)
{
	ONVIFDEBUG("unknown%d\n", 117);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__SetRecordingConfiguration(struct soap *soap, struct _trec__SetRecordingConfiguration *trec__SetRecordingConfiguration, struct _trec__SetRecordingConfigurationResponse *trec__SetRecordingConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 118);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetRecordingConfiguration(struct soap *soap, struct _trec__GetRecordingConfiguration *trec__GetRecordingConfiguration, struct _trec__GetRecordingConfigurationResponse *trec__GetRecordingConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 119);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__CreateTrack(struct soap *soap, struct _trec__CreateTrack *trec__CreateTrack, struct _trec__CreateTrackResponse *trec__CreateTrackResponse)
{
	ONVIFDEBUG("unknown%d\n", 120);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__DeleteTrack(struct soap *soap, struct _trec__DeleteTrack *trec__DeleteTrack, struct _trec__DeleteTrackResponse *trec__DeleteTrackResponse)
{
	ONVIFDEBUG("unknown%d\n", 121);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetTrackConfiguration(struct soap *soap, struct _trec__GetTrackConfiguration *trec__GetTrackConfiguration, struct _trec__GetTrackConfigurationResponse *trec__GetTrackConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 122);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__SetTrackConfiguration(struct soap *soap, struct _trec__SetTrackConfiguration *trec__SetTrackConfiguration, struct _trec__SetTrackConfigurationResponse *trec__SetTrackConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 123);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__CreateRecordingJob(struct soap *soap, struct _trec__CreateRecordingJob *trec__CreateRecordingJob, struct _trec__CreateRecordingJobResponse *trec__CreateRecordingJobResponse)
{
	ONVIFDEBUG("unknown%d\n", 124);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__DeleteRecordingJob(struct soap *soap, struct _trec__DeleteRecordingJob *trec__DeleteRecordingJob, struct _trec__DeleteRecordingJobResponse *trec__DeleteRecordingJobResponse)
{
	ONVIFDEBUG("unknown%d\n", 125);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetRecordingJobs(struct soap *soap, struct _trec__GetRecordingJobs *trec__GetRecordingJobs, struct _trec__GetRecordingJobsResponse *trec__GetRecordingJobsResponse)
{
	ONVIFDEBUG("unknown%d\n", 126);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__SetRecordingJobConfiguration(struct soap *soap, struct _trec__SetRecordingJobConfiguration *trec__SetRecordingJobConfiguration, struct _trec__SetRecordingJobConfigurationResponse *trec__SetRecordingJobConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 127);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetRecordingJobConfiguration(struct soap *soap, struct _trec__GetRecordingJobConfiguration *trec__GetRecordingJobConfiguration, struct _trec__GetRecordingJobConfigurationResponse *trec__GetRecordingJobConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 128);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__SetRecordingJobMode(struct soap *soap, struct _trec__SetRecordingJobMode *trec__SetRecordingJobMode, struct _trec__SetRecordingJobModeResponse *trec__SetRecordingJobModeResponse)
{
	ONVIFDEBUG("unknown%d\n", 129);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trec__GetRecordingJobState(struct soap *soap, struct _trec__GetRecordingJobState *trec__GetRecordingJobState, struct _trec__GetRecordingJobStateResponse *trec__GetRecordingJobStateResponse)
{
	ONVIFDEBUG("unknown%d\n", 130);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetServiceCapabilities(struct soap *soap, struct _trp__GetServiceCapabilities *trp__GetServiceCapabilities, struct _trp__GetServiceCapabilitiesResponse *trp__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 131);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayUri(struct soap *soap, struct _trp__GetReplayUri *trp__GetReplayUri, struct _trp__GetReplayUriResponse *trp__GetReplayUriResponse)
{
	ONVIFDEBUG("unknown%d\n", 132);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__GetReplayConfiguration(struct soap *soap, struct _trp__GetReplayConfiguration *trp__GetReplayConfiguration, struct _trp__GetReplayConfigurationResponse *trp__GetReplayConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 133);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trp__SetReplayConfiguration(struct soap *soap, struct _trp__SetReplayConfiguration *trp__SetReplayConfiguration, struct _trp__SetReplayConfigurationResponse *trp__SetReplayConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 134);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("nProfileCount:%d\n", g_OnvifConf.nProfileCount);

	trt__GetServiceCapabilitiesResponse->Capabilities = ONVIF_MALLOC(struct trt__Capabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities = ONVIF_MALLOC(struct trt__ProfileCapabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = ONVIF_MALLOC(int);
	*trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = g_OnvifConf.nProfileCount;

	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities = ONVIF_MALLOC(struct trt__StreamingCapabilities);
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast = &ntrue;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP = &ntrue;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = &ntrue;
	trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl = &nfalse;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
	ONVIFDEBUG("GetVideoSources\n");
	ONVIF_dup();
	trt__GetVideoSourcesResponse->__sizeVideoSources = 1;
	trt__GetVideoSourcesResponse->VideoSources = ONVIF_MALLOC(struct tt__VideoSource);
	trt__GetVideoSourcesResponse->VideoSources->token = g_OnvifConf.Profile[0].VSCSourceToken;
	trt__GetVideoSourcesResponse->VideoSources->Framerate = 25; // unspecified
	trt__GetVideoSourcesResponse->VideoSources->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
	trt__GetVideoSourcesResponse->VideoSources->Resolution->Width = 1280;
	trt__GetVideoSourcesResponse->VideoSources->Resolution->Height = 720;
	trt__GetVideoSourcesResponse->VideoSources->Imaging = ONVIF_MALLOC(struct tt__ImagingSettings);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->BacklightCompensation = ONVIF_MALLOC(struct tt__BacklightCompensation);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->BacklightCompensation->Mode = tt__BacklightCompensationMode__OFF;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->BacklightCompensation->Level = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Brightness = ONVIF_MALLOC_SIZE(float, 1);
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Brightness = g_OnvifConf.pConf->ipcam.isp.image_attr.brightness.val;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->ColorSaturation = ONVIF_MALLOC_SIZE(float, 1);
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->ColorSaturation = g_OnvifConf.pConf->ipcam.isp.image_attr.saturation.val;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Contrast = ONVIF_MALLOC_SIZE(float, 1);
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Contrast = g_OnvifConf.pConf->ipcam.isp.image_attr.contrast.val;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure = ONVIF_MALLOC(struct tt__Exposure);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Mode =  tt__ExposureMode__MANUAL;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Priority = tt__ExposurePriority__FrameRate;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window = ONVIF_MALLOC(struct tt__Rectangle);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->bottom  = ONVIF_MALLOC(float);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->top = ONVIF_MALLOC(float);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->right = ONVIF_MALLOC(float);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->left = ONVIF_MALLOC(float);
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->bottom = 1;
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->top = 0;
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->right = 1;
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Window->left = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MinExposureTime = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MaxExposureTime = 1;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MinGain = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MaxGain = 1;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MinIris = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->MaxIris = 1;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->ExposureTime = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Gain = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Exposure->Iris = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus = ONVIF_MALLOC(struct tt__FocusConfiguration);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus->AutoFocusMode = tt__AutoFocusMode__MANUAL;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus->DefaultSpeed = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus->NearLimit = 0;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Focus->FarLimit = 0;

	trt__GetVideoSourcesResponse->VideoSources->Imaging->IrCutFilter = ONVIF_MALLOC(enum tt__IrCutFilterMode);
	//enum tt__IrCutFilterMode {tt__IrCutFilterMode__ON = 0, tt__IrCutFilterMode__OFF = 1, tt__IrCutFilterMode__AUTO = 2};
	//SYS_U32_t ircut_mode;//0:auto 1:daylight 2:night
	int ircut = 0;
	if(0 == g_OnvifConf.pConf->ipcam.isp.day_night_mode.ircut_mode){
		ircut = 2;
	}
	else if(2 == g_OnvifConf.pConf->ipcam.isp.day_night_mode.ircut_mode){
		ircut = 0;
	}
	else{
		ircut = 1;
	}
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->IrCutFilter = ircut;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->Sharpness = ONVIF_MALLOC_SIZE(float, 1);
	*trt__GetVideoSourcesResponse->VideoSources->Imaging->Sharpness = g_OnvifConf.pConf->ipcam.isp.image_attr.sharpen.val;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WideDynamicRange = ONVIF_MALLOC(struct tt__WideDynamicRange);
	//enum tt__WideDynamicMode {tt__WideDynamicMode__OFF = 0, tt__WideDynamicMode__ON = 1};
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WideDynamicRange->Mode = tt__WideDynamicMode__OFF/*g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.enable*/;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WideDynamicRange->Level = g_OnvifConf.pConf->ipcam.isp.wide_dynamic_range.strength.val;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WhiteBalance = ONVIF_MALLOC(struct tt__WhiteBalance);
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WhiteBalance->Mode = tt__WhiteBalanceMode__AUTO;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WhiteBalance->CrGain = 1;
	trt__GetVideoSourcesResponse->VideoSources->Imaging->WhiteBalance->CbGain = 1;


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
	ONVIFDEBUG("GetAudioSources\n");
	ONVIF_dup();
	trt__GetAudioSourcesResponse->__sizeAudioSources = 1;
	trt__GetAudioSourcesResponse->AudioSources = ONVIF_MALLOC(struct tt__AudioSource);
	trt__GetAudioSourcesResponse->AudioSources->token = g_OnvifConf.Profile[0].ASCSourceToken;
	trt__GetAudioSourcesResponse->AudioSources->Channels = 1;
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
	ONVIFDEBUG("unknown%d\n", 138);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
	ONVIFDEBUG("name:%s,token:%s\n", trt__CreateProfile->Name,trt__CreateProfile->Token);
	int i=0,num;

	if(trt__CreateProfile->Token == NULL)
	{
		trt__CreateProfile->Token = "";//trt__CreateProfile->Name;
	}
	/* check whether profile already exist or not */
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{

		if(strcmp(trt__CreateProfile->Token, g_OnvifConf.Profile[i].ProfileToken)==0){

			onvif_fault(soap,"ter:InvalidArgVal", "ter:ProfileExists"); 
			return SOAP_FAULT;
		}
	}

	/* check the limit on number of profiles */
	num = g_OnvifConf.nProfileCount;		// total number of profiles existing in the memory
	g_OnvifConf.nProfileCount += 1;
	if(num > ONVIF_PROFILE_MAX)
	{	
		onvif_fault(soap,"ter:Action ", "ter:MaxNVTProfiles"); 
		return SOAP_FAULT;
	}

	/* save profile */

	strcpy(g_OnvifConf.Profile[num].ProfileName, trt__CreateProfile->Name);
	strcpy(g_OnvifConf.Profile[num].ProfileToken, "Profile_3"/*trt__CreateProfile->Token*/);

	trt__CreateProfileResponse->Profile = ONVIF_MALLOC(struct tt__Profile);
	trt__CreateProfileResponse->Profile->Name = g_OnvifConf.Profile[num].ProfileName;
	trt__CreateProfileResponse->Profile->token = g_OnvifConf.Profile[num].ProfileToken;
	trt__CreateProfileResponse->Profile->fixed = 0; 
	trt__CreateProfileResponse->Profile->VideoSourceConfiguration = NULL;
	trt__CreateProfileResponse->Profile->AudioSourceConfiguration = NULL;
	trt__CreateProfileResponse->Profile->VideoEncoderConfiguration = NULL;
	trt__CreateProfileResponse->Profile->AudioEncoderConfiguration = NULL;
	trt__CreateProfileResponse->Profile->VideoAnalyticsConfiguration = NULL;
	trt__CreateProfileResponse->Profile->PTZConfiguration = NULL;
	trt__CreateProfileResponse->Profile->MetadataConfiguration = NULL;
	trt__CreateProfileResponse->Profile->Extension = NULL;
	
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
	ONVIFDEBUG("GetProfile:%s\n", trt__GetProfile->ProfileToken);
	int i;
	int Ptoken_status = 0;

	if((trt__GetProfile->ProfileToken) == NULL)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:InvalidInputToken"); 
		return SOAP_FAULT;
	}
	if(strcmp(trt__GetProfile->ProfileToken, "") == 0)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;
	}/**/

	/* Check if ProfileToken Exist or Not */	
	for(i = 0; i < ONVIF_PROFILE_MAX; i++)
	{
		if(strcmp(trt__GetProfile->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_status = 1;
			break;
		}
	}
	if(!Ptoken_status)
	{
		onvif_fault(soap, "ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;
	}

	trt__GetProfileResponse->Profile = ONVIF_MALLOC(struct tt__Profile);
	trt__GetProfileResponse->Profile->Name = g_OnvifConf.Profile[i].ProfileName; 
	trt__GetProfileResponse->Profile->token = g_OnvifConf.Profile[i].ProfileToken;
	trt__GetProfileResponse->Profile->fixed = ONVIF_MALLOC(enum xsd__boolean_);
	*(trt__GetProfileResponse->Profile->fixed) = g_OnvifConf.Profile[i].Profilefixed;
	/* VideoSourceConfiguration */
	trt__GetProfileResponse->Profile->VideoSourceConfiguration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name = g_OnvifConf.Profile[i].VSCName;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->token = g_OnvifConf.Profile[i].VSCToken;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount = g_OnvifConf.Profile[i].VSCUseCount;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x = g_OnvifConf.Profile[i].Bounds.x;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y = g_OnvifConf.Profile[i].Bounds.y;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width = g_OnvifConf.Profile[i].Bounds.width;
	trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = g_OnvifConf.Profile[i].Bounds.height;
	/* AudioSourceConfiguration */
	trt__GetProfileResponse->Profile->AudioSourceConfiguration = ONVIF_MALLOC(struct tt__AudioSourceConfiguration);
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->Name = g_OnvifConf.Profile[i].ASCName;
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->token = g_OnvifConf.Profile[i].ASCToken;
	trt__GetProfileResponse->Profile->AudioSourceConfiguration->SourceToken = g_OnvifConf.Profile[i].ASCSourceToken;
	/*VideoEncoderConfiguration */
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name = g_OnvifConf.Profile[i].VECName;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token = g_OnvifConf.Profile[i].VECToken;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount = g_OnvifConf.Profile[i].VECUseCount;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout = g_OnvifConf.Profile[i].VECTimeOut;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality = g_OnvifConf.Profile[i].VECQuality;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = 0;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
	snprintf(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Port = 3703;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->TTL = 0;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->AutoStart = _false;
	trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding = g_OnvifConf.Profile[i].VECEncoding;
	/* AudioEncoderConfiguration */
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Name = g_OnvifConf.Profile[i].AECName;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->token = g_OnvifConf.Profile[i].AECToken;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Encoding = g_OnvifConf.Profile[i].AECEncoding;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->UseCount = g_OnvifConf.Profile[i].AECUseCount;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Bitrate = 8;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SampleRate = g_OnvifConf.Profile[i].AECSampleRate;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->SessionTimeout = g_OnvifConf.Profile[i].AECTimeOut;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
	snprintf(trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->Port = 3703;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->TTL = 0;
	trt__GetProfileResponse->Profile->AudioEncoderConfiguration->Multicast->AutoStart = _false;		
	/* PTZConfiguration */ 
	trt__GetProfileResponse->Profile->PTZConfiguration = NULL;
/*
	trt__GetProfileResponse->Profile->PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);
	trt__GetProfileResponse->Profile->PTZConfiguration->Name = "PTZConf";
	trt__GetProfileResponse->Profile->PTZConfiguration->UseCount = 1;
	trt__GetProfileResponse->Profile->PTZConfiguration->token = "PTZConfToken";
	trt__GetProfileResponse->Profile->PTZConfiguration->NodeToken = "PTZNodeToken";
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->x = 0.1;
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->y = 0.1;
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->space = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace";
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->x = 1;
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->space = "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace";
 	trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZTimeout = "PT1S";
 	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits = ONVIF_MALLOC(struct tt__PanTiltLimits);
 	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range = ONVIF_MALLOC(struct tt__Space2DDescription);
 	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
 	struct tt__FloatRange *pSpaceRange = ONVIF_MALLOC(struct tt__FloatRange);
 	pSpaceRange->Min = 0;
 	pSpaceRange->Max = 1;
 	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange = pSpaceRange;
 	trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange = pSpaceRange;
 	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits = ONVIF_MALLOC(struct tt__ZoomLimits);
 	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range = ONVIF_MALLOC(struct tt__Space1DDescription);
 	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
 	trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange = pSpaceRange;
*/
	/* MetadataConfiguration */
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
	ONVIFDEBUG("GetProfiles\n");

	ONVIF_dup();
	int i;
	struct tt__Profile *Profile = NULL;
	Profile = ONVIF_MALLOC_SIZE(struct tt__Profile, g_OnvifConf.nProfileCount);
	trt__GetProfilesResponse->__sizeProfiles = g_OnvifConf.nProfileCount;
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		(Profile + i)->Name = g_OnvifConf.Profile[i].ProfileName; 
		(Profile + i)->token = g_OnvifConf.Profile[i].ProfileToken;
		(Profile + i)->fixed = ONVIF_MALLOC(enum xsd__boolean_);
		*((Profile + i)->fixed) = g_OnvifConf.Profile[i].Profilefixed;
		/* VideoSourceConfiguration */
		(Profile + i)->VideoSourceConfiguration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
		(Profile + i)->VideoSourceConfiguration->Name = g_OnvifConf.Profile[i].VSCName;
		(Profile + i)->VideoSourceConfiguration->token = g_OnvifConf.Profile[i].VSCToken;
		(Profile + i)->VideoSourceConfiguration->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
		(Profile + i)->VideoSourceConfiguration->UseCount = g_OnvifConf.Profile[i].VSCUseCount;
		(Profile + i)->VideoSourceConfiguration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
		(Profile + i)->VideoSourceConfiguration->Bounds->x = g_OnvifConf.Profile[i].Bounds.x;
		(Profile + i)->VideoSourceConfiguration->Bounds->y = g_OnvifConf.Profile[i].Bounds.y;
		(Profile + i)->VideoSourceConfiguration->Bounds->width = g_OnvifConf.Profile[i].Bounds.width;
		(Profile + i)->VideoSourceConfiguration->Bounds->height = g_OnvifConf.Profile[i].Bounds.height;
		/* AudioSourceConfiguration */
		/*
		(Profile + i)->AudioSourceConfiguration = ONVIF_MALLOC(struct tt__AudioSourceConfiguration);
		(Profile + i)->AudioSourceConfiguration->Name = g_OnvifConf.Profile[i].ASCName;
		(Profile + i)->AudioSourceConfiguration->token = g_OnvifConf.Profile[i].ASCToken;
		(Profile + i)->AudioSourceConfiguration->SourceToken = g_OnvifConf.Profile[i].ASCSourceToken;
		*/
		/* VideoEncoderConfiguration */
		(Profile + i)->VideoEncoderConfiguration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
		(Profile + i)->VideoEncoderConfiguration->Name = g_OnvifConf.Profile[i].VECName;
		(Profile + i)->VideoEncoderConfiguration->token = g_OnvifConf.Profile[i].VECToken;
		(Profile + i)->VideoEncoderConfiguration->UseCount = g_OnvifConf.Profile[i].VECUseCount;
		(Profile + i)->VideoEncoderConfiguration->SessionTimeout = g_OnvifConf.Profile[i].VECTimeOut;
		(Profile + i)->VideoEncoderConfiguration->Quality = g_OnvifConf.Profile[i].VECQuality;
		(Profile + i)->VideoEncoderConfiguration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
		(Profile + i)->VideoEncoderConfiguration->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
		(Profile + i)->VideoEncoderConfiguration->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
		(Profile + i)->VideoEncoderConfiguration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
		(Profile + i)->VideoEncoderConfiguration->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
		(Profile + i)->VideoEncoderConfiguration->RateControl->EncodingInterval = 0;
		(Profile + i)->VideoEncoderConfiguration->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
		(Profile + i)->VideoEncoderConfiguration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
		(Profile + i)->VideoEncoderConfiguration->H264->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
		(Profile + i)->VideoEncoderConfiguration->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;
		(Profile + i)->VideoEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		(Profile + i)->VideoEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		(Profile + i)->VideoEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
		(Profile + i)->VideoEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf((Profile + i)->VideoEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
		(Profile + i)->VideoEncoderConfiguration->Multicast->Port = 3703;
		(Profile + i)->VideoEncoderConfiguration->Multicast->TTL = 0;
		(Profile + i)->VideoEncoderConfiguration->Multicast->AutoStart = _false;
		(Profile + i)->VideoEncoderConfiguration->Encoding = g_OnvifConf.Profile[i].VECEncoding;
		/* AudioEncoderConfiguration 
		(Profile + i)->AudioEncoderConfiguration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
		(Profile + i)->AudioEncoderConfiguration->Name = g_OnvifConf.Profile[i].AECName;
		(Profile + i)->AudioEncoderConfiguration->token = g_OnvifConf.Profile[i].AECToken;
		(Profile + i)->AudioEncoderConfiguration->Encoding = g_OnvifConf.Profile[i].AECEncoding;
		(Profile + i)->AudioEncoderConfiguration->UseCount = g_OnvifConf.Profile[i].AECUseCount;
		(Profile + i)->AudioEncoderConfiguration->Bitrate = 8;
		(Profile + i)->AudioEncoderConfiguration->SampleRate = g_OnvifConf.Profile[i].AECSampleRate;
		(Profile + i)->AudioEncoderConfiguration->SessionTimeout = g_OnvifConf.Profile[i].AECTimeOut;
		(Profile + i)->AudioEncoderConfiguration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		(Profile + i)->AudioEncoderConfiguration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		(Profile + i)->AudioEncoderConfiguration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
		(Profile + i)->AudioEncoderConfiguration->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf((Profile + i)->AudioEncoderConfiguration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
		(Profile + i)->AudioEncoderConfiguration->Multicast->Port = 3703;
		(Profile + i)->AudioEncoderConfiguration->Multicast->TTL = 0;
		(Profile + i)->AudioEncoderConfiguration->Multicast->AutoStart = _false;
		*/
		/* PTZConfiguration */
		(Profile + i)->PTZConfiguration = NULL;
/*		(Profile + i)->PTZConfiguration = ONVIF_MALLOC(struct tt__PTZConfiguration);
		(Profile + i)->PTZConfiguration->Name = "PTZConf";
		(Profile + i)->PTZConfiguration->UseCount = 1;
		(Profile + i)->PTZConfiguration->token = "PTZConfToken";
		(Profile + i)->PTZConfiguration->NodeToken = "PTZNodeToken";
	 	(Profile + i)->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultAbsoluteZoomPositionSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultRelativePanTiltTranslationSpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultRelativeZoomTranslationSpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace="http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultContinuousZoomVelocitySpace="http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace";
// 	PTZConfiguration[1].DefaultPTZSpeed=&PTZSpeed[1];
	// 	PTZSpeed[1].PanTilt=&PanTilt[1];
	// 	PanTilt[1].space="http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace";
	// 	PanTilt[1].x=0.1000000;
	// 	PanTilt[1].y=0.1000000;
	// 	PTZSpeed[1].Zoom=&Zoom[1];
	// 	Zoom[1].space="http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace";
	// 	Zoom[1].x=1.000000;
	// 	PTZConfiguration[1].DefaultPTZTimeout="PT1S";
	// 	PTZConfiguration[1].PanTiltLimits=&PanTiltLimits[1];
	// 	PanTiltLimits[1].Range=&PTRange[1];
	// 	PTRange[1].URI="http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	// 	PTZConfiguration[1].ZoomLimits=&ZoomLimits[1];
	// 	ZoomLimits[1].Range=&ZRange[1];
	// 	ZRange[1].URI="http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace";
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed = ONVIF_MALLOC(struct tt__PTZSpeed);
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->PanTilt = ONVIF_MALLOC(struct tt__Vector2D);
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->PanTilt->x = 0.1;
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->PanTilt->y = 0.1;
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->PanTilt->space = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace";
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->Zoom = ONVIF_MALLOC(struct tt__Vector1D);
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->Zoom->x = 1;
	 	(Profile + i)->PTZConfiguration->DefaultPTZSpeed->Zoom->space = "http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace";
	 	(Profile + i)->PTZConfiguration->DefaultPTZTimeout = "PT1S";
	 	(Profile + i)->PTZConfiguration->PanTiltLimits = ONVIF_MALLOC(struct tt__PanTiltLimits);
	 	(Profile + i)->PTZConfiguration->PanTiltLimits->Range = ONVIF_MALLOC(struct tt__Space2DDescription);
	 	(Profile + i)->PTZConfiguration->PanTiltLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	 	struct tt__FloatRange *pSpaceRange = ONVIF_MALLOC(struct tt__FloatRange);
	 	pSpaceRange->Min = 0;
	 	pSpaceRange->Max = 1;
	 	(Profile + i)->PTZConfiguration->PanTiltLimits->Range->XRange = pSpaceRange;
	 	(Profile + i)->PTZConfiguration->PanTiltLimits->Range->YRange = pSpaceRange;
	 	(Profile + i)->PTZConfiguration->ZoomLimits = ONVIF_MALLOC(struct tt__ZoomLimits);
	 	(Profile + i)->PTZConfiguration->ZoomLimits->Range = ONVIF_MALLOC(struct tt__Space1DDescription);
	 	(Profile + i)->PTZConfiguration->ZoomLimits->Range->URI = "http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace";
	 	(Profile + i)->PTZConfiguration->ZoomLimits->Range->XRange = pSpaceRange;
*/		
	}	
	
	trt__GetProfilesResponse->Profiles = Profile;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
	ONVIFDEBUG("PToken:%s,CToken:%s.\n", trt__AddVideoEncoderConfiguration->ProfileToken,trt__AddVideoEncoderConfiguration->ConfigurationToken);

	int Ptoken_exist = 0;
	int i = 0;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(trt__AddVideoEncoderConfiguration->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = 1;
			break;
		}
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;		
	}

	strcpy(g_OnvifConf.Profile[i].VECToken, trt__AddVideoEncoderConfiguration->ConfigurationToken);

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
	ONVIFDEBUG("PToken:%s,CToken:%s.\n", trt__AddVideoSourceConfiguration->ProfileToken,trt__AddVideoSourceConfiguration->ConfigurationToken);

	int Ptoken_exist = 0;
	int i = 0;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
ONVIFDEBUG("(%d)PToken:%s,VSCToken:%s.\n",i, g_OnvifConf.Profile[i].ProfileToken,g_OnvifConf.Profile[i].VSCToken);		

		if(strcmp(trt__AddVideoSourceConfiguration->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = 1;
			break;
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;		
	}

	//if(strcmp(trt__AddVideoSourceConfiguration->ConfigurationToken, g_OnvifConf.Profile[i].VSCToken) == 0)
	//{
		//onvif_fault(soap,"ter:Action", "ter:ConfigurationConflict"); 
		//return SOAP_FAULT;			
	//}else{
		ONVIFDEBUG("exit(%d)PToken(i:%d):%d\n",Ptoken_exist,i,g_OnvifConf.Profile[i].VSCUseCount);
		g_OnvifConf.Profile[i].VSCUseCount += 1;
		strcpy(g_OnvifConf.Profile[i].VSCToken, trt__AddVideoSourceConfiguration->ConfigurationToken);
	//}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap *soap, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 142);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap *soap, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 143);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap *soap, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
	ONVIFDEBUG("undone AddPTZConf, ProfileToken:%s, ConfigurationToken:%s\n", trt__AddPTZConfiguration->ProfileToken, trt__AddPTZConfiguration->ConfigurationToken);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap *soap, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 145);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 146);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap *soap, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 147);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap *soap, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 148);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 149);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 150);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap *soap, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 151);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap *soap, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 152);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap *soap, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 153);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:PTZNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 154);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 155);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap *soap, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 156);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap *soap, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 157);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
	ONVIFDEBUG("unknown%d\n", 158);

	int index;
	int Ptoken_exist = 0;
	/* check whether profile already exist or not */
	for(index = 0; index < ONVIF_PROFILE_MAX; index++)
	{	
		if(strcmp(trt__DeleteProfile->ProfileToken, g_OnvifConf.Profile[index].ProfileToken) == 0)
		{
			Ptoken_exist = 1;
			break;		   	
		}
	}

	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile"); 
		return SOAP_FAULT;
	}

	g_OnvifConf.nProfileCount -= 1;
	memset(g_OnvifConf.Profile[index].ProfileName,0,ONVIF_NAME_TOKEN_MAX);
	memset(g_OnvifConf.Profile[index].ProfileToken,0,ONVIF_NAME_TOKEN_MAX);
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
	ONVIFDEBUG("GetVideoSourceConfigurations\n");
	ONVIF_dup();
	int i = 0;
	trt__GetVideoSourceConfigurationsResponse->Configurations = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);

	trt__GetVideoSourceConfigurationsResponse->Configurations->Name = g_OnvifConf.Profile[i].VSCName;
	trt__GetVideoSourceConfigurationsResponse->Configurations->token = g_OnvifConf.Profile[i].VSCToken;
	trt__GetVideoSourceConfigurationsResponse->Configurations->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
	trt__GetVideoSourceConfigurationsResponse->Configurations->UseCount = g_OnvifConf.Profile[i].VSCUseCount;
	trt__GetVideoSourceConfigurationsResponse->Configurations->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetVideoSourceConfigurationsResponse->Configurations->Bounds->x = g_OnvifConf.Profile[i].Bounds.x;
	trt__GetVideoSourceConfigurationsResponse->Configurations->Bounds->y = g_OnvifConf.Profile[i].Bounds.y;
	trt__GetVideoSourceConfigurationsResponse->Configurations->Bounds->width = g_OnvifConf.Profile[i].Bounds.width;
	trt__GetVideoSourceConfigurationsResponse->Configurations->Bounds->height = g_OnvifConf.Profile[i].Bounds.height;

	trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = 1;	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
	ONVIFDEBUG("GetVideoEncoderConfigurations\n");
	ONVIF_dup();
	int i;
	struct tt__VideoEncoderConfiguration *VideoEncoderConf = ONVIF_MALLOC_SIZE(struct tt__VideoEncoderConfiguration, g_OnvifConf.nProfileCount);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		(VideoEncoderConf + i)->Name = g_OnvifConf.Profile[i].VECName;
		(VideoEncoderConf + i)->token = g_OnvifConf.Profile[i].VECToken;
		(VideoEncoderConf + i)->UseCount = g_OnvifConf.Profile[i].VECUseCount;
		(VideoEncoderConf + i)->SessionTimeout = g_OnvifConf.Profile[i].VECTimeOut;
		(VideoEncoderConf + i)->Quality = g_OnvifConf.Profile[i].VECQuality;
		(VideoEncoderConf + i)->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
		(VideoEncoderConf + i)->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
		(VideoEncoderConf + i)->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
		(VideoEncoderConf + i)->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
		(VideoEncoderConf + i)->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
		(VideoEncoderConf + i)->RateControl->EncodingInterval = 0;
		(VideoEncoderConf + i)->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
		(VideoEncoderConf + i)->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
		(VideoEncoderConf + i)->H264->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
		(VideoEncoderConf + i)->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;
		(VideoEncoderConf + i)->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		(VideoEncoderConf + i)->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		(VideoEncoderConf + i)->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
		(VideoEncoderConf + i)->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf((VideoEncoderConf + i)->Multicast->Address->IPv4Address, 16, "235.255.255.255");
		(VideoEncoderConf + i)->Multicast->Port = 3703;
		(VideoEncoderConf + i)->Multicast->TTL = 0;
		(VideoEncoderConf + i)->Multicast->AutoStart = _false;
		(VideoEncoderConf + i)->Encoding = g_OnvifConf.Profile[i].VECEncoding;		
	}	

	trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = g_OnvifConf.nProfileCount;
	trt__GetVideoEncoderConfigurationsResponse->Configurations = VideoEncoderConf;


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
	ONVIFDEBUG("GetAudioSourceConfigurations\n");
	int i;
	struct tt__AudioSourceConfiguration *AudioSourceConf = ONVIF_MALLOC_SIZE(struct tt__AudioSourceConfiguration, g_OnvifConf.nProfileCount);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		(AudioSourceConf + i)->Name = g_OnvifConf.Profile[i].ASCName;
		(AudioSourceConf + i)->token = g_OnvifConf.Profile[i].ASCToken;
		(AudioSourceConf + i)->SourceToken = g_OnvifConf.Profile[i].ASCSourceToken;
	}


	trt__GetAudioSourceConfigurationsResponse->__sizeConfigurations = g_OnvifConf.nProfileCount;
	trt__GetAudioSourceConfigurationsResponse->Configurations = AudioSourceConf;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap *soap, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
	ONVIFDEBUG("GetAudioEncoderConfigurations\n");
	int i;
	struct tt__AudioEncoderConfiguration *AudioEncoderConf = ONVIF_MALLOC_SIZE(struct tt__AudioEncoderConfiguration, g_OnvifConf.nProfileCount);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		(AudioEncoderConf + i)->Name = g_OnvifConf.Profile[i].AECName;
		(AudioEncoderConf + i)->token = g_OnvifConf.Profile[i].AECToken;
		(AudioEncoderConf + i)->Encoding = g_OnvifConf.Profile[i].AECEncoding;
		(AudioEncoderConf + i)->UseCount = g_OnvifConf.Profile[i].AECUseCount;
		(AudioEncoderConf + i)->Bitrate = 8;
		(AudioEncoderConf + i)->SampleRate = g_OnvifConf.Profile[i].AECSampleRate;
		(AudioEncoderConf + i)->SessionTimeout = g_OnvifConf.Profile[i].AECTimeOut;
		(AudioEncoderConf + i)->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		(AudioEncoderConf + i)->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		(AudioEncoderConf + i)->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
		(AudioEncoderConf + i)->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf((AudioEncoderConf + i)->Multicast->Address->IPv4Address, 16, "235.255.255.255");
		(AudioEncoderConf + i)->Multicast->Port = 3703;
		(AudioEncoderConf + i)->Multicast->TTL = 0;
		(AudioEncoderConf + i)->Multicast->AutoStart = _false;
	}
	trt__GetAudioEncoderConfigurationsResponse->Configurations = AudioEncoderConf;
	trt__GetAudioEncoderConfigurationsResponse->__sizeConfigurations = g_OnvifConf.nProfileCount;
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 160);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 161);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap *soap, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 162);
	onvif_fault(soap,"ter:ActionNotSupported", "ter:AudioIn/OutNotSupported"); 
	return SOAP_FAULT;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap *soap, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 163);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap *soap, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
	ONVIFDEBUG("CToken:%s\n", trt__GetVideoSourceConfiguration->ConfigurationToken);
	ONVIF_dup();
	int i;
	trt__GetVideoSourceConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		if(0 == strcmp(trt__GetVideoSourceConfiguration->ConfigurationToken, g_OnvifConf.Profile[i].VSCToken)){
			trt__GetVideoSourceConfigurationResponse->Configuration->Name = g_OnvifConf.Profile[i].VSCName;
			trt__GetVideoSourceConfigurationResponse->Configuration->token = g_OnvifConf.Profile[i].VSCToken;
			trt__GetVideoSourceConfigurationResponse->Configuration->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
			trt__GetVideoSourceConfigurationResponse->Configuration->UseCount = g_OnvifConf.Profile[i].VSCUseCount;
			trt__GetVideoSourceConfigurationResponse->Configuration->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
			trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->x = g_OnvifConf.Profile[i].Bounds.x;
			trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->y = g_OnvifConf.Profile[i].Bounds.y;
			trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->width = g_OnvifConf.Profile[i].Bounds.width;
			trt__GetVideoSourceConfigurationResponse->Configuration->Bounds->height = g_OnvifConf.Profile[i].Bounds.height;			
		}		
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap *soap, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
	ONVIFDEBUG("CToken:%s\n", trt__GetVideoEncoderConfiguration->ConfigurationToken);
	ONVIF_dup();
	int i;
	trt__GetVideoEncoderConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		if(0 == strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken, g_OnvifConf.Profile[i].VECToken)){
		trt__GetVideoEncoderConfigurationResponse->Configuration->Name = g_OnvifConf.Profile[i].VECName;
		trt__GetVideoEncoderConfigurationResponse->Configuration->token = g_OnvifConf.Profile[i].VECToken;
		trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount = g_OnvifConf.Profile[i].VECUseCount;
		trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout = g_OnvifConf.Profile[i].VECTimeOut;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Quality = g_OnvifConf.Profile[i].VECQuality;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
		trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = 0;
		trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
		trt__GetVideoEncoderConfigurationResponse->Configuration->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
		trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
		trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4;
		snprintf(trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Port = 3703;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->TTL = 0;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->AutoStart = _false;
		trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding = g_OnvifConf.Profile[i].VECEncoding;		
		}		
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap *soap, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 166);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap *soap, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
	ONVIFDEBUG("GetAudioEncoderConfiguration:%s\n", trt__GetAudioEncoderConfiguration->ConfigurationToken);
		ONVIF_dup();
	int i;
	trt__GetAudioEncoderConfigurationResponse->Configuration = ONVIF_MALLOC(struct tt__AudioEncoderConfiguration);
	for(i = 0; i < g_OnvifConf.nProfileCount; ++i){
		if(0 == strcmp(trt__GetAudioEncoderConfiguration->ConfigurationToken, g_OnvifConf.Profile[i].VECToken)){
			trt__GetAudioEncoderConfigurationResponse->Configuration->Name = g_OnvifConf.Profile[i].AECName;
			trt__GetAudioEncoderConfigurationResponse->Configuration->token = g_OnvifConf.Profile[i].AECToken;
			trt__GetAudioEncoderConfigurationResponse->Configuration->Encoding = g_OnvifConf.Profile[i].AECEncoding;
			trt__GetAudioEncoderConfigurationResponse->Configuration->UseCount = g_OnvifConf.Profile[i].AECUseCount;
			trt__GetAudioEncoderConfigurationResponse->Configuration->Bitrate = 8;
			trt__GetAudioEncoderConfigurationResponse->Configuration->SampleRate = g_OnvifConf.Profile[i].AECSampleRate;
			trt__GetAudioEncoderConfigurationResponse->Configuration->SessionTimeout = g_OnvifConf.Profile[i].AECTimeOut;
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address = ONVIF_MALLOC(struct tt__IPAddress);
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, 16);
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->Type = tt__IPType__IPv4;
			snprintf(trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address, 16, "235.255.255.255");
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->Port = 3703;
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->TTL = 0;
			trt__GetAudioEncoderConfigurationResponse->Configuration->Multicast->AutoStart = _false;
		}		
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 168);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 169);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap *soap, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 170);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap *soap, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 171);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 172);
	char _IPAddr[LARGE_INFO_LENGTH];
	int i = 0;
	int flag = 0;
	int Ptoken_exist = 0;

	sprintf(_IPAddr, "http://%03d.%03d.%d.%d/onvif/services", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr));
	
	/* VideoEncoderConfiguration */
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(trt__GetCompatibleVideoEncoderConfigurations->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoEncoderConfigurationsResponse->__sizeConfigurations = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations = ONVIF_MALLOC(struct tt__VideoEncoderConfiguration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Name = g_OnvifConf.Profile[i].VECName;//"VideoEncoderConfiguration";
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->UseCount = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->token = g_OnvifConf.Profile[i].VECToken;//"VideoEncoderConfiguration";
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Encoding = g_OnvifConf.Profile[i].VECEncoding;// {tt__VideoEncoding__JPEG = 0, onv__VideoEncoding__MPEG4 = 1, tt__VideoEncoding__H264 = 2}
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Quality = g_OnvifConf.Profile[i].VECQuality; // float
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->SessionTimeout = g_OnvifConf.Profile[i].VECTimeOut;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution = ONVIF_MALLOC(struct tt__VideoResolution);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution->Width = g_OnvifConf.Profile[i].VECWidth;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Resolution->Height = g_OnvifConf.Profile[i].VECHeight;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl = ONVIF_MALLOC(struct tt__VideoRateControl);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->FrameRateLimit = g_OnvifConf.Profile[i].VECfps;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->EncodingInterval = g_OnvifConf.Profile[i].VECH264Gop;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->RateControl->BitrateLimit = g_OnvifConf.Profile[i].VECbps;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4 = ONVIF_MALLOC(struct tt__Mpeg4Configuration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->MPEG4->Mpeg4Profile = 0;//{onv__Mpeg4Profile__SP = 0, onv__Mpeg4Profile__ASP = 1}
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264 = ONVIF_MALLOC(struct tt__H264Configuration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264->GovLength = g_OnvifConf.Profile[i].VECH264Gop;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->H264->H264Profile = g_OnvifConf.Profile[i].VECH264Profile;//Baseline = 0, Main = 1, High = 3
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast = ONVIF_MALLOC(struct tt__MulticastConfiguration);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Port = 80;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->TTL = 1;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->AutoStart = 0; 
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address  = ONVIF_MALLOC(struct tt__IPAddress);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->Type = 0;
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address = ONVIF_MALLOC_SIZE(char, LARGE_INFO_LENGTH);
	strcpy(trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv4Address, _IPAddr);
	trt__GetCompatibleVideoEncoderConfigurationsResponse->Configurations->Multicast->Address->IPv6Address = NULL;

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
	ONVIFDEBUG("Ptoken:%s\n",trt__GetCompatibleVideoSourceConfigurations->ProfileToken);
	int i=0;
	int Ptoken_exist = NOT_EXIST;

	for(i=0;i< g_OnvifConf.nProfileCount;i++)
	{
		if(strcmp(trt__GetCompatibleVideoSourceConfigurations->ProfileToken,g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoConfig");
		return SOAP_FAULT;
	}

	trt__GetCompatibleVideoSourceConfigurationsResponse->__sizeConfigurations = 1; //MPEG4 | H264 | JPEG
	/* VideoSourceConfiguration */
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations = ONVIF_MALLOC(struct tt__VideoSourceConfiguration);
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Name = g_OnvifConf.Profile[i].VSCName;//"JPEG";
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->UseCount = 1;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->token = g_OnvifConf.Profile[i].VSCToken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->SourceToken = g_OnvifConf.Profile[i].VSCSourceToken;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds = ONVIF_MALLOC(struct tt__IntRectangle);
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->x = g_OnvifConf.Profile[i].Bounds.x;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->y = g_OnvifConf.Profile[i].Bounds.y;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->width = g_OnvifConf.Profile[i].Bounds.width;
	trt__GetCompatibleVideoSourceConfigurationsResponse->Configurations->Bounds->height = g_OnvifConf.Profile[i].Bounds.height;

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 174);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
	ONVIFDEBUG("ProfileToken:%s\n", trt__GetCompatibleAudioSourceConfigurations->ProfileToken);

	trt__GetCompatibleAudioSourceConfigurationsResponse->__sizeConfigurations = 2;

	/* AudioSourceConfiguration */
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations = ONVIF_MALLOC_SIZE(struct tt__AudioSourceConfiguration, trt__GetCompatibleAudioSourceConfigurationsResponse->__sizeConfigurations);
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].Name, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].token, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[0].SourceToken, "G711");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].Name, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].UseCount = 1;
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].token, "AAC");
	trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetCompatibleAudioSourceConfigurationsResponse->Configurations[1].SourceToken, "AAC");

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 176);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 177);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 178);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
	ONVIFDEBUG("unknown%d\n", 179);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 180);

	int i;
	int j = 0;
	int flg = 0;
	int Ptoken_exist = NOT_EXIST;
	int ret;
	int num_token = 0;
	unsigned char codec_combo;
	unsigned char codec_res;
	unsigned char mode = 0;
	int _width;
	int _height;
	 
	if(trt__SetVideoSourceConfiguration->Configuration->Bounds != NULL)
	{
		_width = trt__SetVideoSourceConfiguration->Configuration->Bounds->width;
		_height = trt__SetVideoSourceConfiguration->Configuration->Bounds->height;
	}
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(g_OnvifConf.Profile[j].VSCToken, g_OnvifConf.Profile[i].VSCToken)==0);
			{
				flg = 1;		
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}

	if(trt__SetVideoSourceConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i <= g_OnvifConf.nProfileCount; i++)
		{
			if(strcmp(trt__SetVideoSourceConfiguration->Configuration->token, g_OnvifConf.Profile[i].VSCToken) == 0)
			{
				Ptoken_exist = EXIST;
				break;
			}
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist) 
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}

	/* check for width and height */
	if(!((_width >= 320 && _width <= 1920) && (_height >= 192 && _height <= 1080)))
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(trt__SetVideoSourceConfiguration->Configuration->token != NULL)
	{
		for(i = 0; i< g_OnvifConf.nProfileCount; i++)
		{
			Ptoken_exist = NOT_EXIST;
			if(strcmp(trt__SetVideoSourceConfiguration->Configuration->token, g_OnvifConf.Profile[i].VSCToken) == 0)
			{
				Ptoken_exist = EXIST;
			}

			if(Ptoken_exist)
			{
				strcpy(g_OnvifConf.Profile[i].VSCToken, trt__SetVideoSourceConfiguration->Configuration->token);
				strcpy(g_OnvifConf.Profile[i].VSCName, trt__SetVideoSourceConfiguration->Configuration->Name);
				strcpy(g_OnvifConf.Profile[i].VSCSourceToken, trt__SetVideoSourceConfiguration->Configuration->SourceToken);
				g_OnvifConf.Profile[i].VSCUseCount = trt__SetVideoSourceConfiguration->Configuration->UseCount;	
				if(trt__SetVideoSourceConfiguration->Configuration->Bounds != NULL)
				{
					g_OnvifConf.Profile[i].Bounds.x = trt__SetVideoSourceConfiguration->Configuration->Bounds->x;
					g_OnvifConf.Profile[i].Bounds.y = trt__SetVideoSourceConfiguration->Configuration->Bounds->y;
					g_OnvifConf.Profile[i].Bounds.width = _width;
					g_OnvifConf.Profile[i].Bounds.height = _height;
				}
				//ret = ControlSystemData(SFIELD_SET_VIDEOSOURCE_CONF, (void *)&set_video_source_t, sizeof(set_video_source_t));
			}
		}
	}
/*	if(ret != SUCCESS)
	{		
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}*/
	if(g_OnvifConf.Profile[i].VECEncoding == 0)
	{
		codec_combo = 2;
		if((_width == 1600) && (_height == 1200)) // 2MP = 1600x1200
		{
			codec_res = 1;                        
		}
		else if((_width == 2048) && (_height == 1536)) // 3MP = 2048x1536
		{
			codec_res = 3;
		}
		else if(((_width == 2592) || (_height == 2560)) && (_height == 1920)) // 5MP = 2592x1920
		{
			codec_res = 5;
		}
		else 
		{
			codec_res = 1;		
		}
	}
	else if(g_OnvifConf.Profile[i].VECEncoding == 1)
	{
		codec_combo = 1;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
		{
			codec_res = 0;
		}
		else if((_width == 720) && (_height == 480)) // D1 = 720x480
		{
			codec_res = 1;
		}
		else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
		{
			codec_res = 2;
		}
		else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
		{
			codec_res = 3;
		}
		else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
		{
			codec_res = 4;
		}
		else codec_res = 0;
	}	
	else if(g_OnvifConf.Profile[i].VECEncoding == 2)
	{
		codec_combo = 0;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
		{
			codec_res = 0;
		}
		else if((_width == 720) && (_height == 480)) // D1 = 720x480
		{
			codec_res = 1;
		}
		else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
		{
			codec_res = 2;
		}
		else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
		{
			codec_res = 3;
		}
		else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
		{
			codec_res = 4;
		}
	}
	else
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:UnabletoSetParams");
		return SOAP_FAULT;
	}

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
	ONVIFDEBUG("encoding:%d\n",trt__SetVideoEncoderConfiguration->Configuration->Encoding);

    int i=0;
	int Ptoken_exist = NOT_EXIST;
	int ret;
    unsigned char codec_combo = 0;
    unsigned char codec_res = 0;
    unsigned char mode = 0;
    unsigned char encodinginterval = 0;
    unsigned char frameratelimit = 0;
    int _width = 0;
    int _height = 0;
    int bitrate = 0;
	int govlength = 0;
	
	if(trt__SetVideoEncoderConfiguration->Configuration != NULL)
	{
		if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
		{
			_width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
			_height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
		}
	}
	if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
	{
		encodinginterval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
		bitrate = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
        	//bitrate /= 1000;
		bitrate *= 1000;
		frameratelimit = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
	}
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
        {
		if(strcmp(trt__SetVideoEncoderConfiguration->Configuration->token, g_OnvifConf.Profile[i].VECToken)==0)
      	{
  			Ptoken_exist = EXIST;
			break;
		}
	}
	/* if ConfigurationToken does not exist */
	if(!Ptoken_exist) 
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoVideoSource");
		return SOAP_FAULT;
	}
	if( ( (_width % 2) != 0 || (_height % 2) != 0 ) || ((_width * _height) % 16 != 0))	
	{
		onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
		return SOAP_FAULT;
	}
	
    if(trt__SetVideoEncoderConfiguration->Configuration != NULL)
    {
		strcpy(g_OnvifConf.Profile[i].VECName, trt__SetVideoEncoderConfiguration->Configuration->Name);
		strcpy(g_OnvifConf.Profile[i].VECToken, trt__SetVideoEncoderConfiguration->Configuration->token); 
		g_OnvifConf.Profile[i].VECUseCount = trt__SetVideoEncoderConfiguration->Configuration->UseCount;
		g_OnvifConf.Profile[i].VECQuality = trt__SetVideoEncoderConfiguration->Configuration->Quality;
		g_OnvifConf.Profile[i].VECEncoding = trt__SetVideoEncoderConfiguration->Configuration->Encoding;
    	if(trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
    	{
			g_OnvifConf.Profile[i].VECWidth = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
			g_OnvifConf.Profile[i].VECHeight = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
    	}
		if(trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
		{
			g_OnvifConf.Profile[i].VECfps = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
			g_OnvifConf.Profile[i].VECH264Gop = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
			g_OnvifConf.Profile[i].VECbps = bitrate;
		}
		else
		{
			g_OnvifConf.Profile[i].VECfps = 0;//trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
			g_OnvifConf.Profile[i].VECH264Gop = 0;//trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
			g_OnvifConf.Profile[i].VECbps = 0;//bitrate;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
		{
			ONVIFDEBUG("trt__SetVideoEncoderConfiguration->Configuration->MPEG4.\n");
		}
		else
		{
			ONVIFDEBUG("trt__SetVideoEncoderConfiguration->Configuration->MPEG4=NULL.\n");
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
		{
			g_OnvifConf.Profile[i].VECH264Gov = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
			g_OnvifConf.Profile[i].VECH264Profile = trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile;
		}
		else
		{
			ONVIFDEBUG("trt__SetVideoEncoderConfiguration->Configuration->H264.\n");
		}
		
		strcpy(g_OnvifConf.Profile[i].VECTimeOut, trt__SetVideoEncoderConfiguration->Configuration->SessionTimeout);
		if(trt__SetVideoEncoderConfiguration->Configuration->Multicast != NULL)
		{
			ONVIFDEBUG("trt__SetVideoEncoderConfiguration->Configuration->Multicast.\n");
		}
	}

	if (trt__SetVideoEncoderConfiguration->Configuration->Encoding == 0)
	{
		codec_combo = 2;
		if((_width == 1600) && (_height == 1200)) // 2MP = 1600x1200
        {
                codec_res = 1;                        
        }
        else if((_width == 2048) && (_height == 1536)) // 3MP = 2048x1536
        {
                codec_res = 3;
        }
        else if(((_width == 2592) || (_height == 2560)) && (_height == 1920)) // 5MP = 2592x1920
        {
                codec_res = 5;
        }
        else if(((_width == 704) && (_height == 576))) // 5MP = 2592x1920
        {
                codec_res = 5;
        }    
        else if(((_width == 320) && (_height == 240))) // 5MP = 2592x1920
        {
                codec_res = 5;
        }  
        else if(((_width == 352) && (_height == 288))) // 5MP = 2592x1920
        {
                codec_res = 5;
        }         
        else 
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;	
		}
	}
	else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == 1)
	{
		codec_combo = 1;
		if((_width == 1280) && (_height == 720)) // 720 = 1280x720
        {
            codec_res = 0;
        }
        else if((_width == 720) && (_height == 480)) // D1 = 720x480
        {
            codec_res = 1;
        }
        else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
        {
            codec_res = 2;
        }
        else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
        {
            codec_res = 3;
        }
        else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
        {
            codec_res = 4;
        }
        else 
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;	
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
		{
            govlength = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->GovLength;
		}
        if((govlength<0) || (govlength>30))
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:ConfigModify");
			return SOAP_FAULT;
		}
	}	
	else if(trt__SetVideoEncoderConfiguration->Configuration->Encoding == 2)
	{
		codec_combo = 0;
		if((_width == 640) && (_height == 360)) // 720 = 1280x720
		{
			codec_combo = 0;
			codec_res = 0;
		}		
		else if((_width == 1280) && (_height == 720)) // 720 = 1280x720
		{
			codec_combo = 0;
			codec_res = 0;
		}
		else if((_width == 720) && (_height == 480)) // D1 = 720x480
		{
			codec_combo = 0;
			codec_res = 1;
		}
		else if((_width == 1280) && (_height == 960)) // SXVGA = 1280x960
		{
			codec_combo = 0;
			codec_res = 2;
		}
		else if((_width == 1920) && (_height == 1080)) // 1080 = 1920x1080
		{
			codec_combo = 0;
			codec_res = 3;
		}
		else if((_width == 1280) && (_height == 720)) // 720MAX60 = 1280x720
		{
			codec_combo = 0;
			codec_res = 4;
		}
		else if((_width == 1600) && (_height == 1200)) // 2MP 1600x1200
		{
			codec_combo = 2;
            codec_res = 0;
		}
		else if((_width == 2048) && (_height == 1536)) // 3MP 2048x1536
		{
			codec_combo = 2;
            codec_res = 2;
		}
		else if((_width == 2592) && (_height == 1920)) // 5MP 2592x1920
		{
			codec_combo = 2;
            codec_res = 4;
		}
		else
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
			return SOAP_FAULT;
		}
		if(trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
		{
            govlength = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
		}
        if((govlength<0) || (govlength>50))
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
			return SOAP_FAULT;
		}
    }
		
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap *soap, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 182);
	
	char _Token[SMALL_INFO_LENGTH]; 
	char _Name[SMALL_INFO_LENGTH]; 
	char _SourceToken[SMALL_INFO_LENGTH]; 
	int i;
	int flag = 0;
	unsigned char _Encoding;

	strcpy(_SourceToken, trt__SetAudioSourceConfiguration->Configuration->SourceToken);
	strcpy(_Token, trt__SetAudioSourceConfiguration->Configuration->token);
	strcpy(_Name, trt__SetAudioSourceConfiguration->Configuration->Name);

	if(strcmp(_Token,"G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token,"AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap,"ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}
	
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(_SourceToken, g_OnvifConf.Profile[i].ASCSourceToken) == 0)
		{
			flag = EXIST;
			break;
		}
	}
	if(!flag)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	if(strcmp(_SourceToken, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_SourceToken, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap,"ter:InvalidArgVal", "ter:ConfigModify");
		return SOAP_FAULT;
	}
	//if(oSysInfo->audio_config.codectype != _Encoding)
	//{
	//	ControlSystemData(SFIELD_SET_AUDIO_ENCODE, (void *)&_Encoding, sizeof(_Encoding));
	//}
	strcpy(g_OnvifConf.Profile[i].ASCName, _Name);
	strcpy(g_OnvifConf.Profile[i].ASCToken, _Token);
	strcpy(g_OnvifConf.Profile[i].ASCSourceToken, _SourceToken);
	flag = NOT_EXIST;
	for(i = 0; i <= g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(_Token, g_OnvifConf.Profile[i].ASCToken) == 0)
		{
			flag = EXIST;
			//Add_Audio_Conf.position = i;
			break;
		}
	}
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap *soap, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 183);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap *soap, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 184);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 185);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap *soap, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 186);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap *soap, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
	ONVIFDEBUG("unknown%d\n", 187);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
	ONVIFDEBUG("undone GetVideoSourceConfigurationOptions, CToken:%s, PToken:%s\n", trt__GetVideoSourceConfigurationOptions->ConfigurationToken, trt__GetVideoSourceConfigurationOptions->ProfileToken);

	int i;
	int num_token = 0;
	int j;
	int flag = NOT_EXIST;
	int flg = 0;
	int index = 0;
	int num = 0;
	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		for(j = 0; j <= i; j++)
		{
			if(strcmp(g_OnvifConf.Profile[j].VSCToken,g_OnvifConf.Profile[i].VSCToken) == 0);
			{
				flg = 1;		
				break;
			}
		}
		if(flg == 0)
		{
			num_token++;
		}
	}
	if(trt__GetVideoSourceConfigurationOptions->ConfigurationToken != NULL)
	{
		for(i = 0; i <= g_OnvifConf.nProfileCount ; i++)
		{
			if(strcmp(trt__GetVideoSourceConfigurationOptions->ConfigurationToken,g_OnvifConf.Profile[i].VSCToken) == 0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}
		
	if(trt__GetVideoSourceConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i <= g_OnvifConf.nProfileCount; i++)
		{
			if(strcmp(trt__GetVideoSourceConfigurationOptions->ProfileToken, g_OnvifConf.Profile[i].ProfileToken)==0);
			{
				flag = EXIST;
				index = j;
				break;
			}
		}
	}
	
	if(!flag)
	{
		return SOAP_FAULT;
	}
	else
	{
		trt__GetVideoSourceConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__VideoSourceConfigurationOptions);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange = ONVIF_MALLOC(struct tt__IntRectangleRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->XRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Min = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->YRange->Max = 0;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Min = 320;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->WidthRange->Max = 1920;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Min = 192;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->BoundsRange->HeightRange->Max = 1080;
		trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable = (char **)soap_malloc(soap, sizeof(char *) * 1);
		if(trt__GetVideoSourceConfigurationOptions->ProfileToken == NULL && trt__GetVideoSourceConfigurationOptions->ConfigurationToken == NULL)
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = num_token;
			for(i = 0;i < g_OnvifConf.nProfileCount; i++)
			{
				for(j = 0; j < i; j++)
				{
					if(strcmp(g_OnvifConf.Profile[j].VSCToken, g_OnvifConf.Profile[i].VSCToken) == 0)
					{
						flg = 1;
						break;
					}
				}
				if(flg == 0)
				{

				trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
				strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[num], g_OnvifConf.Profile[i].VSCSourceToken);
				num++;
				}
				flg = 0;
			}
		}
		else
		{
			trt__GetVideoSourceConfigurationOptionsResponse->Options->__sizeVideoSourceTokensAvailable = 1;
			trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0] = (char *)soap_malloc(soap, sizeof(char) * SMALL_INFO_LENGTH);
			strcpy(trt__GetVideoSourceConfigurationOptionsResponse->Options->VideoSourceTokensAvailable[0], g_OnvifConf.Profile[index].VSCSourceToken);
		}
	}

	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
	ONVIFDEBUG("GetVideoEncoderConfigurationOptions, CToken:%s, PToken:%s\n", trt__GetVideoEncoderConfigurationOptions->ConfigurationToken, trt__GetVideoEncoderConfigurationOptions->ProfileToken);
	int index;
	char JPEG_profile = 0;
	char MPEG4_profile = 0;
	char H264_profile =0;
	char PToken[SMALL_INFO_LENGTH];
	char CToken[SMALL_INFO_LENGTH];

	if(trt__GetVideoEncoderConfigurationOptions->ProfileToken)
	{
		strcpy(PToken, trt__GetVideoEncoderConfigurationOptions->ProfileToken);
	}
	if(trt__GetVideoEncoderConfigurationOptions->ConfigurationToken)
	{
		strcpy(CToken, trt__GetVideoEncoderConfigurationOptions->ConfigurationToken);
	}

	for(index = 0; index < g_OnvifConf.nProfileCount; index++)
	{
		if((strcmp(PToken, g_OnvifConf.Profile[index].ProfileToken) == 0) || (strcmp(CToken, g_OnvifConf.Profile[index].VECToken) == 0))
		{
			if(g_OnvifConf.Profile[index].VECEncoding == 0)
			{	
				JPEG_profile = 1;
			}
			else if(g_OnvifConf.Profile[index].VECEncoding == 1)
			{	
				MPEG4_profile = 1;
			}
			else if(g_OnvifConf.Profile[index].VECEncoding == 2)
			{	
				H264_profile = 1;
			}
		}
	}
ONVIFDEBUG("VECEncoding:%d\n",g_OnvifConf.Profile[index].VECEncoding);
	ONVIF_dup();
	trt__GetVideoEncoderConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__VideoEncoderConfigurationOptions);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange = ONVIF_MALLOC(struct tt__IntRange);
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Max = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[index].stream[0].quality.max;
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->QualityRange->Min = 0;

	//if(JPEG_profile)
	//{
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG  = ONVIF_MALLOC(struct tt__JpegOptions); 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->__sizeResolutionsAvailable = 3; // 2MP(1600x1200) | 3MP(2048x1536) | 5MP(2592x1920)
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable  = ONVIF_MALLOC_SIZE(struct tt__VideoResolution,trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->__sizeResolutionsAvailable); 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[0].Width  = 320; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[0].Height = 240; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[1].Width  = 352; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[1].Height = 288; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[2].Width  = 704; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->ResolutionsAvailable[2].Height = 576; 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange  = ONVIF_MALLOC(struct tt__IntRange); 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Min = 1; //dummy
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->FrameRateRange->Max = 25; //dummy
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange  = ONVIF_MALLOC(struct tt__IntRange); 
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Min = 1; //dummy
	trt__GetVideoEncoderConfigurationOptionsResponse->Options->JPEG->EncodingIntervalRange->Max = 400; //dummy
//}

	if(H264_profile)
	{
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264 = ONVIF_MALLOC(struct tt__H264Options);
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeResolutionsAvailable = 1;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable = ONVIF_MALLOC(struct tt__VideoResolution);
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable->Width = g_OnvifConf.Profile[index].VECWidth;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->ResolutionsAvailable->Height = g_OnvifConf.Profile[index].VECHeight;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Min = 0;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->GovLengthRange->Max = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].gop;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Min = 0;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->FrameRateRange->Max = g_OnvifConf.pConf->ipcam.vin[0].enc_h264[0].stream[0].fps;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange = ONVIF_MALLOC(struct tt__IntRange);
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Min = 0;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->EncodingIntervalRange->Max = 1;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->__sizeH264ProfilesSupported = 1;
		trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported = ONVIF_MALLOC(int);
		*trt__GetVideoEncoderConfigurationOptionsResponse->Options->H264->H264ProfilesSupported = 0; //Baseline = 0, Main = 1, High = 3}
	}

	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap *soap, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
	ONVIFDEBUG("unknown%d\n", 189);

	int token_exist = 0, profile_exist = 0;
	int i = 0;
	int index = 0;
	int _Encoding;
	char _Token[SMALL_INFO_LENGTH] = "";

	/* Response */
	if(trt__GetAudioSourceConfigurationOptions->ProfileToken != NULL)
	{
		for(i = 0; i < g_OnvifConf.nProfileCount; i++)
		{
			if(strcmp(trt__GetAudioSourceConfigurationOptions->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
			{
				profile_exist = EXIST;
				index = i;
				break;
			}
		}
		if(!profile_exist)
		{
			onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
			return SOAP_FAULT;
		}
	}

	if(trt__GetAudioSourceConfigurationOptions->ConfigurationToken != NULL)
	{
		strcpy(_Token,trt__GetAudioSourceConfigurationOptions->ConfigurationToken);
	}

	if(_Token[0] == 0 || strcmp(_Token, "G711") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "G726") == 0)
	{
		_Encoding = 1;
	}
	else if(strcmp(_Token, "AAC") == 0)
	{
		_Encoding = 2;
	}
	else
	{ 
		onvif_fault(soap, "ter:NoConfig", "ter:InvalidArgValue");
		return SOAP_FAULT;
	}

	trt__GetAudioSourceConfigurationOptionsResponse->Options = ONVIF_MALLOC(struct tt__AudioSourceConfigurationOptions); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable = 3; 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable = ONVIF_MALLOC_SIZE(char, trt__GetAudioSourceConfigurationOptionsResponse->Options->__sizeInputTokensAvailable);
	
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0] = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH);
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[0], "G711");
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1] = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH); 
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[1], "G726"); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2] = ONVIF_MALLOC_SIZE(char, SMALL_INFO_LENGTH); 
	strcpy(trt__GetAudioSourceConfigurationOptionsResponse->Options->InputTokensAvailable[2], "AAC"); 
	trt__GetAudioSourceConfigurationOptionsResponse->Options->Extension = NULL;
	return SOAP_OK; 
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
	ONVIFDEBUG("unknown, GetAudioEncoderConfigurationOptions\n");


	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
	ONVIFDEBUG("unknown%d\n", 191);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap *soap, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
	ONVIFDEBUG("unknown%d\n", 192);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap *soap, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
	ONVIFDEBUG("unknown%d\n", 193);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
	ONVIFDEBUG("GetGuaranteedNumberOfVideoEncoderInstances\n");

	trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse->TotalNumber = 1;

	return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
	char file[10] = {0};
	int i;
	int TokenExit = NOT_EXIST;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(trt__GetStreamUri->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			TokenExit = EXIST;
			break;
		}
	}
	if(!TokenExit)
    {
        onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
        return SOAP_FAULT;
    }

	if(trt__GetStreamUri->StreamSetup != NULL)
	{
		if(trt__GetStreamUri->StreamSetup->Stream == 1)
		{
			onvif_fault(soap,"ter:InvalidArgVal","ter:InvalidStreamSetup");
			return SOAP_FAULT;
		}
	}
	
	if(0 == strcmp(trt__GetStreamUri->ProfileToken, g_OnvifConf.Profile[0].ProfileToken)){
		snprintf(file, sizeof(file), "ch0_0.264");
	}
	else{
		snprintf(file, sizeof(file), "ch0_1.264");
	}
	trt__GetStreamUriResponse->MediaUri = ONVIF_MALLOC(struct tt__MediaUri);
	trt__GetStreamUriResponse->MediaUri->Uri = ONVIF_MALLOC_SIZE(char , 50);
	trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = _false;
	trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = _false;
	trt__GetStreamUriResponse->MediaUri->Timeout = "PT100S";
	snprintf(trt__GetStreamUriResponse->MediaUri->Uri, 50, "rtsp://%s:%d/%s", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr),
																			g_OnvifConf.pConf->ipcam.network.lan.port[0].value,
																			file);
	ONVIFDEBUG("GetStreamUri, stream setup:%p, profile token:%s, stream uri:%s\n", trt__GetStreamUri->StreamSetup, trt__GetStreamUri->ProfileToken,
																	trt__GetStreamUriResponse->MediaUri->Uri);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap *soap, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
	ONVIFDEBUG("unknown%d\n", 195);

	int i = 0;
    int Ptoken_exist = 0;

	for(i = 0; i < g_OnvifConf.nProfileCount; i++)
	{
		if(strcmp(trt__StartMulticastStreaming->ProfileToken, g_OnvifConf.Profile[i].ProfileToken) == 0)
		{
			Ptoken_exist = EXIST;
			break;		
		}
	}
	if(!Ptoken_exist)
	{
		onvif_fault(soap,"ter:InvalidArgVal", "ter:NoProfile");
		return SOAP_FAULT;
	}
	
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap *soap, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
	ONVIFDEBUG("unknown%d\n", 196);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
	ONVIFDEBUG("SetSynchronizationPoint\n");
	ONVIFDEBUG("unknown%d\n", 197);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
	ONVIFDEBUG("GetSnapshotUri\n");

	char file[20] = {0};
	strcpy(file,"snapshot");
	trt__GetSnapshotUriResponse->MediaUri = ONVIF_MALLOC(struct tt__MediaUri);
	trt__GetSnapshotUriResponse->MediaUri->Uri = ONVIF_MALLOC_SIZE(char , 50);
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot = _false;
	trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = _false;
	trt__GetSnapshotUriResponse->MediaUri->Timeout = "PT100S";
	snprintf(trt__GetSnapshotUriResponse->MediaUri->Uri, 50, "http://%s:%d/%s", inet_ntoa(g_OnvifConf.pConf->ipcam.network.lan.static_ip.in_addr),
																			g_OnvifConf.pConf->ipcam.network.lan.port[0].value,
																			file);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetServiceCapabilities(struct soap *soap, struct _tse__GetServiceCapabilities *tse__GetServiceCapabilities, struct _tse__GetServiceCapabilitiesResponse *tse__GetServiceCapabilitiesResponse)
{
	ONVIFDEBUG("unknown%d\n", 199);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSummary(struct soap *soap, struct _tse__GetRecordingSummary *tse__GetRecordingSummary, struct _tse__GetRecordingSummaryResponse *tse__GetRecordingSummaryResponse)
{
	ONVIFDEBUG("unknown%d\n", 200);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingInformation(struct soap *soap, struct _tse__GetRecordingInformation *tse__GetRecordingInformation, struct _tse__GetRecordingInformationResponse *tse__GetRecordingInformationResponse)
{
	ONVIFDEBUG("unknown%d\n", 201);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMediaAttributes(struct soap *soap, struct _tse__GetMediaAttributes *tse__GetMediaAttributes, struct _tse__GetMediaAttributesResponse *tse__GetMediaAttributesResponse)
{
	ONVIFDEBUG("unknown%d\n", 202);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindRecordings(struct soap *soap, struct _tse__FindRecordings *tse__FindRecordings, struct _tse__FindRecordingsResponse *tse__FindRecordingsResponse)
{
	ONVIFDEBUG("unknown%d\n", 203);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetRecordingSearchResults(struct soap *soap, struct _tse__GetRecordingSearchResults *tse__GetRecordingSearchResults, struct _tse__GetRecordingSearchResultsResponse *tse__GetRecordingSearchResultsResponse)
{
	ONVIFDEBUG("unknown%d\n", 204);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindEvents(struct soap *soap, struct _tse__FindEvents *tse__FindEvents, struct _tse__FindEventsResponse *tse__FindEventsResponse)
{
	ONVIFDEBUG("unknown%d\n", 205);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetEventSearchResults(struct soap *soap, struct _tse__GetEventSearchResults *tse__GetEventSearchResults, struct _tse__GetEventSearchResultsResponse *tse__GetEventSearchResultsResponse)
{
	ONVIFDEBUG("unknown%d\n", 206);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindPTZPosition(struct soap *soap, struct _tse__FindPTZPosition *tse__FindPTZPosition, struct _tse__FindPTZPositionResponse *tse__FindPTZPositionResponse)
{
	ONVIFDEBUG("unknown%d\n", 207);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetPTZPositionSearchResults(struct soap *soap, struct _tse__GetPTZPositionSearchResults *tse__GetPTZPositionSearchResults, struct _tse__GetPTZPositionSearchResultsResponse *tse__GetPTZPositionSearchResultsResponse)
{
	ONVIFDEBUG("unknown%d\n", 208);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetSearchState(struct soap *soap, struct _tse__GetSearchState *tse__GetSearchState, struct _tse__GetSearchStateResponse *tse__GetSearchStateResponse)
{
	ONVIFDEBUG("unknown%d\n", 209);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__EndSearch(struct soap *soap, struct _tse__EndSearch *tse__EndSearch, struct _tse__EndSearchResponse *tse__EndSearchResponse)
{
	ONVIFDEBUG("unknown%d\n", 210);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__FindMetadata(struct soap *soap, struct _tse__FindMetadata *tse__FindMetadata, struct _tse__FindMetadataResponse *tse__FindMetadataResponse)
{
	ONVIFDEBUG("unknown%d\n", 211);return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __tse__GetMetadataSearchResults(struct soap *soap, struct _tse__GetMetadataSearchResults *tse__GetMetadataSearchResults, struct _tse__GetMetadataSearchResultsResponse *tse__GetMetadataSearchResultsResponse)
{
	ONVIFDEBUG("unknown%d\n", 212);return SOAP_OK;
}

