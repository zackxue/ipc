#ifndef _VSIP_H_
#define _VSIP_H_

#ifdef __cplusplus
extern "C" {
#endif

struct VSWC_MACINTERFACE_CONTEXT
{
	unsigned int m_Mac1;
	unsigned int m_Mac2;
	unsigned int m_Mac3;
};

typedef struct
{
	unsigned int m_ManuID;
	
	unsigned int m_VideoChnNum;
	unsigned int *m_pVideoType;
	unsigned int m_VideoBufLen;

	unsigned int m_AudioInChnNum;
	unsigned int *m_pAudioInType;
	
	unsigned int m_AudioOutChnNum;
	unsigned int *m_pAudioOutType;
	
	unsigned int m_AlarmInNum;
	unsigned int m_AlarmOutNum;
	
	unsigned int m_bDeviceStorage;
	unsigned int m_bTwoStream;
	
	unsigned int m_bUseResend;
	unsigned int m_bUserMemMode;
	
	struct VSWC_MACINTERFACE_CONTEXT m_MacAddress;
}Device_Param;

typedef struct
{
    	unsigned int  year;
    	unsigned int  month;
    	unsigned int  date;
    	unsigned int  hour;
    	unsigned int  minute;
    	unsigned int  second;
    	unsigned int  weekday;
} rtc_time_t;

#define NTSC     0x01
#define PAL      0x02


enum 
{
	DEVICEIP	=  0		, 			//设备IP地址
	DEVICENETMASK		,				//设备网络掩码
	DEVICEGATEWAY		,				//设备网关
	VADSTD						,				//视频制式
	VADBRIGHTNESS		,				//视频亮度
	VADCONTRAST 		,				//视频对比度
	VADSATURATE 		,				//视频饱和度
	VADHUE						,				//视频色温
	VENCRESOLUTION	,				//视频分辨率
	VENCFRAMERATE		,				//视频编码帧率
	VENCBITRATE			,				//视频编码码率
	VENCGOP						,				//视频I帧间隔
	VENCQUALITY			,				//视频编码质量
	VENCCBR						,				//VBR,CBR设置
	AENCSAMPLERATE	,				//音频编码采样率
	AENCCHANNEL			,				//音频编码通道数
	AENCDATABIT			,				//音频编码采样位数
	ADECSAMPLERATE	,				//音频解码采样率
	ADECCHANNEL			,				//音频解码通道数
	ADECDATABIT			,				//音频解码采样位数
	RS485BAUDRATE		,				//485波特率
	RS485STOPBIT        ,
	RS485DATABIT        ,
	RS485CHECKNUM       ,
	RS232BAUDRATE		,				//232
	RS232STOPBIT        ,
	RS232DATABIT        ,
	RS232CHECKNUM
};

enum
{
	QCIF = 0,//QQVGA = 0
	CIF,//QVGA = 0
	HD1,
	D1,//VGA = 0
	PIXEL1M,
	PIXEL2M,
	PIXEL3M,
	PIXEL4M,
	PIXEL5M
};

enum
{
	IOPORTALARM = 1, //IO口输入报警
	MDALARM	,	  				//移动侦测报警
	VIDEOLOST      	//视频丢失报警
};

typedef struct
{
	void (*pGlobalReset)(int sel);
///	void (*pGetDeviceType)(char *DeviceType);
	int (*pGetDeviceInitParam)(Device_Param *pDeviceParam);
	void (*pSend485Data)(unsigned char *bufaddr,int len);
	int (*pAdjustTime)(rtc_time_t settime);
	int (*pSetAlarmOut)(int chn,int value);

	int	(*pGetServerParamFlag)(void);
	int	(*pWriteServerParamToFlash)(unsigned char *addr,int len);
	int	(*pReadServerParamFromFlash)(unsigned char *addr,int len);
	int	(*pModifyDeviceParam)(int ID ,int chn,int value);
	int	(*pGetDeviceParam)(int ID,int chn,int *pvalue);

	void	(*pRequestIDR)(int chn);
	void (*pControlVideoChn)(int chn,int status);
	void (*pControlAudioInChn)(int chn,int status);
	void (*pControlAudioOutChn)(int chn,int status);
	int (*pSendAudioDataToDec)(unsigned char *addr,int len);
}SEVER_FPTR;


extern int	Ipserver(SEVER_FPTR *pServerFPtr);
extern int	ExitIpserver(void);
extern int	SendAlarmToServer(int style,int channel,int alarm);
extern int	SendVideoStreamToServer(int chn,int len,int iflag,unsigned char *bufaddr,unsigned int useroffset);
extern int SendAudioStreamToServer(int chn,int len,unsigned char *bufaddr,unsigned int useroffset);
extern int GetPlatVersion(char *pPlatVer,char *pbuildtime);

#ifdef __cplusplus
}
#endif

#endif
