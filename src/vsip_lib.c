

#include "vsip_lib.h"
#include "vsip.h"
#include "ifconf.h"
#include "generic.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"
#include "sysconf.h"
#include "app_debug.h"


#define VENC_HISILICON_H264 (1)
#define VENC_STANDARD_H264 (2)
#define VENC_STANDARD_MPEG4 (3)
#define VENC_STANDARD_MJPEG (4)
#define VENC_STANDARD_H263 (5)

#define AENC_HISILICON_G711_U (1)
#define AENC_HISILICON_G711_A (2)
#define AENC_HISILICON_G726_16K (3)
#define AENC_HISILICON_G726_24K (4)
#define AENC_HISILCION_G726_32K (5)
#define AENC_HISILCION_G726_40K (6)
#define AENC_G726_16K (7)
#define AENC_G726_24K (8)
#define AENC_G726_32K (9)
#define AENC_G726_40K (10)
#define AENC_G711_A (11)
#define AENC_G711_U (12)
#define AENC_G723 (13)
#define AENC_PCM (14)
#define AENC_ADPCM (15)
#define AENC_ADPCM_DVI (16)
#define AENC_ADPCM_IMA (17)
#define AENC_AAC (18)
#define AENC_G729 (19)
#define AENC_G723_1 (20)
#define AENC_G722 (21)
#define AENC_MP3 (22)
#define AENC_OTG (23)

typedef struct VSIP_STREAM_TASK
{
	bool task_trigger;
	pthread_t task_tid;
	char stream_name[32];
	int venc_ch;

}VSIP_STREAM_TASK_t;

typedef struct VSIP_JUAN
{
	char eth[16];
	// for IpServer
	pthread_t server_tid;
	// for stream task
	int n_stream_task;
	VSIP_STREAM_TASK_t* stream_task;

}VSIP_JUAN_t;
static VSIP_JUAN_t _vsip =
{
	.eth = "",
	.server_tid = (pthread_t)NULL,
	//
	.n_stream_task = 0,
};
static VSIP_JUAN_t* _p_vsip = NULL;

static void* vsip_stream_task(void* arg)
{
	int ret = 0;
	VSIP_STREAM_TASK_t* const this_task = (VSIP_STREAM_TASK_t*)arg;
	//int const mediabuf_id = MEDIABUF_lookup_byname(this_task->stream_name);
	int const mediabuf_id = this_task->venc_ch;
	lpMEDIABUF_USER mediabuf_user = MEDIABUF_attach(mediabuf_id);

	APP_TRACE("Stream(%s) task (%08x,%08x) begin", this_task->stream_name, (uint32_t)getpid(), (uint32_t)pthread_self());
	MEDIABUF_sync(mediabuf_user);
	while(this_task->task_trigger)
	{
		bool out_success = false;
		// attaching to media buf and send out stream here
		if(0 == MEDIABUF_out_lock(mediabuf_user)){
			const lpSDK_ENC_BUF_ATTR attr = NULL;
			ssize_t out_size = 0;
			if(0 == MEDIABUF_out(mediabuf_user, (void*)&attr, NULL, &out_size)){
				void* const frame_ptr = (void*)(attr + 1);
				ssize_t const frame_size = attr->data_sz;
				if(kSDK_ENC_BUF_DATA_H264 == attr->type){
					// sending video
					ret = SendVideoStreamToServer(this_task->venc_ch, frame_size, attr->h264.keyframe ? 1 : 0, frame_ptr, 0);
					if(0 != ret){
						// FIXME:
					}
				}else if(kSDK_ENC_BUF_DATA_G711A == attr->type){
					// sending audio
					//ret = SendAudioStreamToServer(0,len + m_Offset,Buf,m_Offset);
				}
				out_success = true;
			}
			MEDIABUF_out_unlock(mediabuf_user);
		}
		if(out_success){
			usleep(30000);
		}
	}
	APP_TRACE("Stream task (%08x,%08x) end", (uint32_t)getpid(), (uint32_t)pthread_self());
	pthread_exit(NULL);
}

static void vsip_stream_task_start(VSIP_STREAM_TASK_t* task)
{
	int ret = 0;
	if(!task->task_tid){
		task->task_trigger = true;
		ret = pthread_create(&task->task_tid, NULL, vsip_stream_task, (void*)task);
		APP_ASSERT(0 == ret, "Create stream task failed!");
	}
}

static void vsip_stream_task_stop(VSIP_STREAM_TASK_t* task)
{
	if(task->task_tid){
		task->task_trigger = false;
		pthread_join(task->task_tid, NULL);
		task->task_tid = (pthread_t)NULL;
	}
}


static void GlobalReset()
{
	APP_TRACE("I want you to reboot!");
}

static void ControlVideoChn(int chn,int status)
{
	APP_TRACE("Control video in @ %d status %d", chn, status);
}

static void ControlAudioInChn(int chn, int status)
{
	APP_TRACE("Control audio in @ %d status %d", chn, status);
}

static void ControlAudioOutChn(int chn,int status)
{
	APP_TRACE("Control audio out @ %d status %d", chn, status);
}

static int GetDeviceInitParamMacAddr(struct VSWC_MACINTERFACE_CONTEXT* mac_ctx)
{
	ifconf_interface_t irf;
	ifconf_get_interface(_p_vsip->eth, &irf);
//
// mac value example
// mac: 00-EA-21-3B-39-28
//   VSWC_MACINTERFACE_CONTEXT::m_Mac1 = 0x3928;
//   VSWC_MACINTERFACE_CONTEXT::m_Mac2 = 0x213B;
//   VSWC_MACINTERFACE_CONTEXT::m_Mac3 = 0x00EA;
//
	mac_ctx->m_Mac1 = irf.hwaddr.s_b5 << 8 | irf.hwaddr.s_b6;
	mac_ctx->m_Mac2 = irf.hwaddr.s_b3 << 8 | irf.hwaddr.s_b4;
	mac_ctx->m_Mac3 = irf.hwaddr.s_b1 << 8 | irf.hwaddr.s_b2;

	APP_TRACE("Get MAC %s", ifconf_hw_ntoa(irf.hwaddr));
	return 0;
}

static int GetDeviceInitParam(Device_Param *pDeviceParam)
{
	int chn;
	pDeviceParam->m_ManuID = 0x02;
	pDeviceParam->m_VideoChnNum = 1;
	pDeviceParam->m_bTwoStream = 1;
	pDeviceParam->m_pVideoType = calloc(pDeviceParam->m_VideoChnNum *(pDeviceParam->m_bTwoStream + 1),sizeof(int));
	pDeviceParam->m_VideoBufLen = 200*1024;
	for(chn = 0;chn < pDeviceParam->m_VideoChnNum;++chn){
		if(pDeviceParam->m_bTwoStream){
			pDeviceParam->m_pVideoType[chn * 2] = VENC_HISILICON_H264;
			pDeviceParam->m_pVideoType[chn * 2] = VENC_HISILICON_H264;
		}else{
			pDeviceParam->m_pVideoType[chn] = VENC_HISILICON_H264;
		}
	}

	pDeviceParam->m_AudioInChnNum = 0;
	if(pDeviceParam->m_AudioInChnNum > 0){
		pDeviceParam->m_pAudioInType = calloc(pDeviceParam->m_AudioInChnNum,sizeof(int));
		for(chn = 0;chn < pDeviceParam->m_AudioInChnNum;++chn)
		{
	//		if(chn > 8 )
				*(pDeviceParam->m_pAudioInType + chn) = 10;
	//		else
	//			*(pDeviceParam->m_pAudioInType + chn) = chn;
		}
	}

	pDeviceParam->m_AudioOutChnNum = 0;
	if(pDeviceParam->m_AudioOutChnNum > 0){
		pDeviceParam->m_pAudioOutType = calloc(pDeviceParam->m_AudioOutChnNum,sizeof(int));
		for(chn = 0;chn < pDeviceParam->m_AudioOutChnNum;++chn)
		{
			*(pDeviceParam->m_pAudioOutType + chn) = 10;
		}
	}

	pDeviceParam->m_AlarmInNum = 0;
	pDeviceParam->m_AlarmOutNum = 0;

	pDeviceParam->m_bDeviceStorage = 0;

	pDeviceParam->m_bUseResend = 1;
	pDeviceParam->m_bUserMemMode = 0;

	GetDeviceInitParamMacAddr(&pDeviceParam->m_MacAddress);
	return 0;
}

//static void GetDeviceType(char *DeviceType)
//{
//	memcpy(DeviceType,"H264C5000T4-D",13);
//}

static void Send485Data(unsigned char *bufaddr,int len)
{
	int i = 0;
	APP_TRACE("RS485:");
	for(i = 0; i < len; ++i){
		printf("%02X ", i, bufaddr[i]);
	}
}

static int AdjustTime(rtc_time_t settime)
{
	APP_TRACE("Adjust time %04d/%02d/%02d %02d:%02d:%02d %d",
		settime.year, settime.month, settime.date,
    	settime.hour, settime.minute, settime.second, settime.weekday);
	return 0;
}

static int SetAlarmOut(int chn,int value)
{
	APP_TRACE("Set alarm out @ %d status %d", chn, value);
	return 0;
}

static int GetServerParamFlag()
{
	// always readable
	return 1;
}

static int WriteServerParamToFlash(unsigned char *addr,int len)
{
	SYSCONF_t* sys_conf = SYSCONF_dup();
	APP_ASSERT(len < sizeof(sys_conf->ipcam.vsip_param), "No space in sysconf for VSIP param");
	memcpy(sys_conf->ipcam.vsip_param.buf, addr, len);
	SYSCONF_save(sys_conf); // save to flash

	APP_TRACE("Write VSIP param to flash size = %d", len);
	return 0;
}

static int ReadServerParamFromFlash(unsigned char *addr,int len)
{
	SYSCONF_t* sys_conf = SYSCONF_dup();
	APP_ASSERT(len < sizeof(sys_conf->ipcam.vsip_param), "No space in sysconf for VSIP param");
	memcpy(addr, sys_conf->ipcam.vsip_param.buf, len);
	
	APP_TRACE("Read VSIP param from flash size = %d", len);
	return 0;
}

static int ModifyDeviceParam (int ID ,int chn,int value)
{
	APP_TRACE("Modify device Param ID %d chn %d value %d", ID, chn, value);
	switch(ID)
	{
		case DEVICEIP :
		{
//			printf("ip %x\n",ntohl(value));
		}
		break;

		case DEVICENETMASK:
		{
//			printf("netmask %x\n",ntohl(value));
		}
		break;

		case DEVICEGATEWAY:
		{
//			printf("gateway %x\n",ntohl(value));
		}
		break;

		case VADSTD:
		{
		}
		break;

		case VADBRIGHTNESS:
		{
		}
		break;

		case VADCONTRAST:
		{

		}
		break;

		case VADSATURATE:
		{
		}
		break;

		case VADHUE:
		{
		}
		break;

		case VENCRESOLUTION	:
		{
		}
		break;

		case VENCFRAMERATE:
		{
		}
		break;

		case VENCBITRATE:
		{
		}
		break;

		case VENCGOP:
		{
		}
		break;

		case VENCQUALITY	:
		{
		}
		break;

		case VENCCBR:
		{
		}
		break;

		case AENCSAMPLERATE:
		{
		}
		break;

		case AENCCHANNEL:
		{
		}
		break;

		case AENCDATABIT	:
		{
		}
		break;

		case ADECSAMPLERATE:
		{
		}
		break;

		case ADECCHANNEL:
		{
		}
		break;

		case ADECDATABIT:
		{
		}
		break;

		case RS485BAUDRATE:
		{
		}
		break;

		case RS232BAUDRATE:
		{
		}
		break;

		default:
		{
		}
		break;
	}
	return 0;
}

static int GetDeviceParamNetwork(int ID,int chn,int *pvalue)
{
//
// only for
//   ID = DEVICEIP, DEVICENETMASK, DEVICEGATEWAY
//
	int ret = 0;
	ifconf_interface_t ifconf_irf;
	char ipv4[32] = {""};

	ret = ifconf_get_interface(_p_vsip->eth, &ifconf_irf);
	if(DEVICEIP == ID){
		sprintf(ipv4, "%d.%d.%d.%d", ifconf_irf.ipaddr.s_b1, ifconf_irf.ipaddr.s_b2, ifconf_irf.ipaddr.s_b3, ifconf_irf.ipaddr.s_b4);
		*pvalue = inet_addr(ipv4);
		APP_TRACE("IP %s", ipv4);
	}else if(DEVICENETMASK == ID){
		sprintf(ipv4, "%d.%d.%d.%d", ifconf_irf.netmask.s_b1, ifconf_irf.netmask.s_b2, ifconf_irf.netmask.s_b3, ifconf_irf.netmask.s_b4);
		*pvalue = inet_addr(ipv4);
		APP_TRACE("Netmask %s", ipv4);
	}else if(DEVICEGATEWAY == ID){
		sprintf(ipv4, "%d.%d.%d.%d", ifconf_irf.gateway.s_b1, ifconf_irf.gateway.s_b2, ifconf_irf.gateway.s_b3, ifconf_irf.gateway.s_b4);
		*pvalue = inet_addr(ipv4);
		APP_TRACE("Gateway %s", ipv4);
	}else{
		return -1;
	}
	return 0;
}

static int GetDeviceParam(int ID,int chn,int *pvalue)
{
	int ret = 0;
//	APP_TRACE("Get device Param ID %d chn %d value %d", ID, chn);
	switch(ID){
	case DEVICEIP:
	case DEVICENETMASK:
	case DEVICEGATEWAY:
		{
			ret = GetDeviceParamNetwork(ID, chn, pvalue);
		}
		break;

		case VADSTD:
		{
			*pvalue = PAL;
		}
		break;

		case VADBRIGHTNESS:
		{
			*pvalue = 0;
		}
		break;

		case VADCONTRAST:
		{
			*pvalue = 0;
		}
		break;

		case VADSATURATE:
		{
			*pvalue = 0;
		}
		break;

		case VADHUE:
		{
			*pvalue = 0;
		}
		break;

		case VENCRESOLUTION	:
		{
			*pvalue = PIXEL2M;
		}
		break;

		case VENCFRAMERATE:
		{
			*pvalue = 20;
		}
		break;

		case VENCBITRATE:
		{
			*pvalue = 100;
		}
		break;

		case VENCGOP:
		{
			*pvalue = 50;
		}
		break;

		case	VENCQUALITY	:
		{
			*pvalue = 4;
		}
		break;

		case	VENCCBR:
		{
			if(chn == 15)
			*pvalue = 3;
			else
			*pvalue = 0;
		}
		break;

		case	AENCSAMPLERATE:
		{
//			if(chn%2)
				*pvalue = 8000;
//			else
//				*pvalue = 16000;
		}
		break;

		case	AENCCHANNEL:
		{
//			if(chn%2)
				*pvalue = 1;
//			else
//				*pvalue = 2;
		}
		break;

		case AENCDATABIT	:
		{
//			if(chn%2)
				*pvalue = 16;
//			else
//				*pvalue = 8;
		}
		break;

		case ADECSAMPLERATE:
		{
				*pvalue = 8000;
		}
		break;

		case	ADECCHANNEL:
		{
			*pvalue = 1;
		}
		break;

		case ADECDATABIT:
		{
			*pvalue = 16;
		}
		break;

		case RS485BAUDRATE:
		{
			*pvalue = 4800;
		}
		break;

		case RS485STOPBIT:
		{
			*pvalue = 1;
		}
		break;

		case RS485DATABIT:
		{
			*pvalue = 8;
		}
		break;

		case RS485CHECKNUM:
		{
			*pvalue = 'N';
		}
		break;

		case	RS232BAUDRATE	:
		{
			*pvalue = 9600;
		}
		break;

		case RS232STOPBIT:
		{
			*pvalue = 1;
		}
		break;

		case RS232DATABIT:
		{
			*pvalue = 8;
		}
		break;

		case RS232CHECKNUM:
		{
			*pvalue = 'N';
		}
		break;

		default:
		{
		}
		break;
	}
	return 0;
}

static void RequestIDR(int chn)
{
	APP_TRACE("Request IDR @ %d", chn);
}

static int SendAudioDataToDec(unsigned char *addr,int len)
{
	printf("audio len = %d\n",len);
	FILE *paudiofile = NULL;
//	printf("open file \n");
	paudiofile = fopen("./test.au","ab+");
//	printf("write file \n");
	fwrite(addr,len,1,paudiofile);
//	printf("close file \n");
	fclose(paudiofile);

	return 0;
}

//static int UserProgramUpdate(unsigned char *buf,int len)
//{
//	printf("len = %d\n",len);
//	return 0;
//}
// mossback 

static int vsip_start_server()
{
	int i = 0;
	int ret = 0;
	if(_p_vsip){
		if(!_p_vsip->server_tid){
			SEVER_FPTR l_ServerFPTR;

			l_ServerFPTR.pGlobalReset = GlobalReset;
			//l_ServerFPTR.pGetDeviceType = GetDeviceType;
			l_ServerFPTR.pGetDeviceInitParam = GetDeviceInitParam;
			l_ServerFPTR.pSend485Data = Send485Data;
			l_ServerFPTR.pAdjustTime = AdjustTime;
			l_ServerFPTR.pSetAlarmOut = SetAlarmOut;
			l_ServerFPTR.pGetServerParamFlag = GetServerParamFlag;
			l_ServerFPTR.pWriteServerParamToFlash = WriteServerParamToFlash;
			l_ServerFPTR.pReadServerParamFromFlash = ReadServerParamFromFlash;
			l_ServerFPTR.pModifyDeviceParam = ModifyDeviceParam;
			l_ServerFPTR.pGetDeviceParam = GetDeviceParam;
			l_ServerFPTR.pRequestIDR = RequestIDR;
			l_ServerFPTR.pControlVideoChn = ControlVideoChn;
			l_ServerFPTR.pControlAudioInChn = ControlAudioInChn;
			l_ServerFPTR.pControlAudioOutChn = ControlAudioOutChn;
			l_ServerFPTR.pSendAudioDataToDec = SendAudioDataToDec;

			ret = pthread_create(&_p_vsip->server_tid, NULL, (void *)Ipserver,(void *)&l_ServerFPTR);
			APP_ASSERT(0 == ret , "Start server failed");
			sleep(5);

			// start stream task
			// FIXME:
			_p_vsip->n_stream_task = 2;
			_p_vsip->stream_task = calloc(sizeof(VSIP_STREAM_TASK_t) * _p_vsip->n_stream_task, 1);
			for(i = 0; i < _p_vsip->n_stream_task; ++i){
				// start task here
				if(0 == i){
					strcpy(_p_vsip->stream_task[i].stream_name, "720p.264");
				}else{
					strcpy(_p_vsip->stream_task[i].stream_name, "360p.264");
				}
				_p_vsip->stream_task[i].venc_ch = i;
				vsip_stream_task_start(_p_vsip->stream_task + i);
			}

			return 0;
		}
	}
	return -1;
}

static int vsip_stop_server()
{
	if(_p_vsip){
		if(_p_vsip->server_tid){
			int i = 0;
			for(i = 0; i < _p_vsip->n_stream_task; ++i){
				// stop task here
				vsip_stream_task_stop(_p_vsip->stream_task + i);
			}
			free(_p_vsip->stream_task);
			_p_vsip->stream_task = NULL;
			_p_vsip->n_stream_task = 0;

			// stop IpServer
			ExitIpserver();
			pthread_join(_p_vsip->server_tid, NULL);
			_p_vsip->server_tid = (pthread_t)NULL;
			return 0;
		}
	}
	return -1;
}

int VSIPLIB_init(const char* eth)
{
	if(!_p_vsip){
		// init the pointer
		STRUCT_ZERO(_vsip);
		_p_vsip = &_vsip;
		// let me know which eth i focus
		strncpy(_p_vsip->eth, eth, sizeof(_p_vsip->eth));
		// start the api ipserver
		vsip_start_server();

		return 0;
	}
	return -1;
}

void VSIPLIB_destroy()
{
	if(_p_vsip){
		// stop the api ipserver
		vsip_stop_server();

		// release the pointer
		_p_vsip = NULL;
	}
}

