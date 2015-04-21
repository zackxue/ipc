/*
 *  Version 4.01
 *  Time: 20100308
 */
 
#ifndef __OWSP_DEF_H_INCLUDED__
#define __OWSP_DEF_H_INCLUDED__


#define STR_LEN_32		32
#define STR_LEN_16		16

#define MAX_TLV_LEN		65535		//最大TLV长为(64K-1)

//版本信息
#define VERSION_MAJOR	4
#define VERSION_MINOR	4

//
//  如果已经定义
//
#ifndef u_int32
typedef unsigned long	u_int32;
typedef unsigned short	u_int16;
typedef unsigned char	u_int8;
#endif

//#pragma pack(push)
//#pragma pack(4)

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM								0x0001
#endif 

#ifndef WAVE_FORMAT_MPEGLAYER3
#define WAVE_FORMAT_MPEGLAYER3					    0x0055	// ISO/MPEG Layer 3 格式标记
#endif                                      
                                            
#ifndef WAVE_FORMAT_QUALCOMM_PUREVOICE      
#define WAVE_FORMAT_QUALCOMM_PUREVOICE	    	    0x0150
#endif                                      
                                            
//AMR_NB CBR wave format                    
#ifndef WAVE_FORMAT_AMR_CBR                 
#define WAVE_FORMAT_AMR_CBR 						0x7A21 
#endif                                      
                                            
//AMR VBR Not support yet                   
#ifndef WAVE_FORMAT_AMR_VBR                 
#define WAVE_FORMAT_AMR_VBR 						0x7A22
#endif                                      
                                            
//AMR_WB Wave format                        
#ifndef WAVE_FORMAT_VOICEAGE_AMR_WB         
#define WAVE_FORMAT_VOICEAGE_AMR_WB			        0xA104
#endif                                      
                                            
#define CODEC_H264									0x34363248	//H264


/* TLV 类型命令字 */

#define TLV_T_VERSION_INFO_ANSWER			   39
#define TLV_T_VERSION_INFO_REQUEST		   40
#define TLV_T_LOGIN_REQUEST					     41
#define TLV_T_LOGIN_ANSWER						   42		//0x2A
#define TLV_T_TOTAL_CHANNEL						   43		//NOT USED
#define TLV_T_SENDDATA_REQUEST				     44		//通道请求
#define TLV_T_SENDDATA_ANSWER					 45		//通道请求应答
#define TLV_T_TOTAL_CHANEL_ANSWER				 46		//Not used
#define TLV_T_SUSPENDSENDDATA_REQUEST		     47		//停止发送数据
#define TLV_T_SUSPENDSENDDATA_ANSWER		     48
#define TLV_T_DEVICE_KEEP_ALIVE					 49		//心跳包
#define TLV_T_DEVICE_FORCE_EXIT					 50		
#define TLV_T_CONTROL_REQUEST					 51		//云台等控制请求
#define TLV_T_CONTROL_ANSWER					 52		//云台等响应
#define TLV_T_RECORD_REQUEST				     53		//录像请求
#define TLV_T_RECORD_ANSWER						 54
#define TLV_T_DEVICE_SETTING_REQUEST			 55		//设备参数设置请求
#define TLV_T_DEVICE_SETTING_ANSWER				 56		//设备参数设置应答
#define TLV_T_KEEP_ALIVE_ANSWER					 57		//心跳包响应
#define TLV_T_DEVICE_RESET						 58		//通知设备重启
#define TLV_T_DEVICE_RESET_ANSWER				 59		//设备接收到重启命令后的响应，通常不用发出
#define TLV_T_ALERT_REQUEST     				 60   //报警请求，由设备发出
#define TLV_T_ALERT_ANSWER      				 61   //报警请求回应，由服务器发出，通常可以不用发出
#define TLV_T_ALERT_SEND_PHOTO    			     62   //报警后，设备采集当时的图片，发送到服务器
#define TLV_T_ALERT_SEND_PHOTO_ANSWER 	         63   //设备发送MSG_CMD_ALERT_SEND_PHOTO后，服务器的回应
#define TLV_T_CHANNLE_REQUEST		    		 64   		//切换到另一通道
#define TLV_T_CHANNLE_ANSWER					 65   		//切换另一通道应答
#define TLV_T_SUSPEND_CHANNLE_REQUEST		     66   		//挂起某一通道
#define TLV_T_SUSPEND_CHANNLE_ANSWER			 67   		//应答
#define TLV_T_VALIDATE_REQUEST					 68   		//程序验证请求
#define TLV_T_VALIDATE_ANSWER					 69   		//应答
#define TLV_T_DVS_INFO_REQUEST					 70			//设备DVS通知连接方设备信息请求
#define TLV_T_DVS_INFO_ANSWER					 71			//
#define TLV_T_PHONE_INFO_REQUEST					 72			//手机通知连接方手机信息请求
#define TLV_T_PHONE_INFO_ANSWER					 73			//
#define TLV_T_RECORDFILE_SEARCH_REQUEST		 74			//录像搜索请求
#define TLV_T_RECORDFILE_SEARCH_ANSWER		 75			//
#define TLV_T_RECORDFILE_PLAY_REQUEST			 76			//录像播放请求
#define TLV_T_RECORDFILE_PLAY_ANSWER			 77			//
#define TLV_T_XML_COMMAND				 78			//XML格式命令，由xml中的cmd字段来表示是什么消息

#define TLV_T_DEVICEINFO_REQUEST		 79			//设备信息查询
#define TLV_T_DEVICEINFO_ANSWER 		 80			//设备信息回复

//xml命令
#define MSG_TYPE_CMD_LOGIN					"LOGIN"			//xml的LOGIN命令(请求与应答)
#define MSG_VERSION						1			//xml方式协议版本


//vod & live
#define TLV_T_AUDIO_INFO							 0x61   //97		音频信息, 表示V为音频信息
#define TLV_T_AUDIO_DATA							 0x62   //98		音频数据, 表示V为音频数据
#define TLV_T_VIDEO_FRAME_INFO					     0x63   //99    视频帧信息, 表示V的数据描述帧信息
#define TLV_T_VIDEO_IFRAME_DATA					     0x64   //100   视频关键帧数据，表示V的数据为关键帧
#define TLV_T_VIDEO_PFRAME_DATA					     0x66   //102   视频P帧(参考帧)数据, 表示V的数据为参考帧
#define TLV_T_VIDEO_FRAME_INFO_EX				     0x65   //101   扩展视频帧信息支持>=64KB的视频帧
#define TLV_T_STREAM_FORMAT_INFO				     0xC7   //199		流格式信息 ,描述视频属性,音频属性
#define TLV_T_STREAM_FORMAT_INFO_V3					 0xC8   //200

//vod
#define TLV_T_STREAM_FILE_INDEX						 213
#define TLV_T_STREAM_FILE_ATTRIBUTE				     214
#define TLV_T_STREAM_FILE_END					     0x0000FFFF


/* response result */
#define _RESPONSECODE_SUCC						 0x01		//	成功
#define _RESPONSECODE_USER_PWD_ERROR			 0x02		//  用户名或密码错
#define _RESPONSECODE_PDA_VERSION_ERROR			 0x04		//	版本不一致
#define _RESPONSECODE_MAX_USER_ERROR			 0x05	
#define _RESPONSECODE_DEVICE_OFFLINE			 0x06		//	设备已经离线
#define _RESPONSECODE_DEVICE_HAS_EXIST			 0x07		//  设备已经存在
#define _RESPONSECODE_DEVICE_OVERLOAD				 0x08		//  设备性能超载(设备忙)
#define _RESPONSECODE_INVALID_CHANNLE				 0x09		//  设备不支持的通道
#define _RESPONSECODE_PROTOCOL_ERROR				0X0A		//协议解析出错
#define _RESPONSECODE_NOT_START_ENCODE			0X0B		//未启动编码
#define _RESPONSECODE_TASK_DISPOSE_ERROR		0X0C		//任务处理过程出错
#define _RESPONSECODE_TIME_ERROR				0x0D		//搜索时间跨天
#define _RESPONSECODE_OVER_INDEX_ERROR				0x0E		//索引超出范围
#define _RESPONSECODE_MEMORY_ERROR			0x0F		//内存分配失败
#define _RESPONSECODE_QUERY_ERROR			0x10		//搜索失败
#define _RESPONSECODE_NO_USER_ERROR			0x11		//没有此用户
#define _RESPONSECODE_NOW_EXITING			0x12		//用户正在退出
#define	_RESPONSECODE_GET_DATA_FAIL			0x13		//获取数据失败


//流数据类型
typedef enum _OWSP_StreamDataType
{
	OWSP_SDT_VIDEO_ONLY			= 0,
	OWSP_SDT_AUDIO_ONLY			= 1,
	OWSP_SDT_VIDEO_AUDIO			= 2
} OWSP_StreamDataType;

//云台控制码,取值范围为0~255
 typedef enum _OWSP_ACTIONCode
 {
   OWSP_ACTION_MD_STOP      = 0,            // 停止运动
   OWSP_ACTION_ZOOMReduce=5,
   OWSP_ACTION_ZOOMADD=6,
   OWSP_ACTION_FOCUSADD=7,    //焦距
   OWSP_ACTION_FOCUSReduce=8,
   OWSP_ACTION_MD_UP=9,                    // 向上
   OWSP_ACTION_MD_DOWN=10,              // 向下
   OWSP_ACTION_MD_LEFT=11,              // 向左
   OWSP_ACTION_MD_RIGHT=12,            // 向右
   OWSP_ACTION_Circle_Add = 13,    //光圈
   OWSP_ACTION_Circle_Reduce = 14,    //
   OWSP_ACTION_AUTO_CRUISE = 15,			//自动巡航
   OWSP_ACTION_GOTO_PRESET_POSITION = 16, 	//跳转预置位
   OWSP_ACTION_SET_PRESET_POSITION = 17, 	//设置预置位点
   OWSP_ACTION_CLEAR_PRESET_POSITION = 18, //清除预置位点
   OWSP_ACTION_ACTION_RESET = 20,

   OWSP_ACTION_TV_SWITCH = 128,		//切换视频源,消息参数为INT*,1--TV, 2--SV,3--CV1, 4--CV2 
   OWSP_ACTION_TV_TUNER = 129,		//切换频道, 消息参数为INT*, 为频道号
   OWSP_ACTION_TV_SET_QUALITY  = 130,		//画质设置, 亮度,色度,饱和度,对比度结构体
 } OWSP_ACTIONCode;

//报警种类，目前只支持探头报警，也就是ATC_INFRARED
typedef enum _AlertTypeCode
{
	ATC_VIDEO = 0,//视频帧预测
	ATC_DEVICE_SERSTART = 1,	/* 设备启动 */	
	ATC_MOTION = 2,						/* 移动侦测报警 */
	ATC_VIDEOLOST = 3,				/* 视频丢失报警 */
	ATC_DISKFULL = 4,					/* 硬盘满报警 */
	ATC_HIDEALARM=5,					/* 视频遮挡报警 */	
	ATC_STOP = 6,							/* 服务器停止 */
	ATC_SDERROR = 7,         	/* SD卡异常*/
	ATC_INFRARED = 20					//开关量探头（比如红外探头）
}AlertTypeCode;

//报警级别，主要针对帧预测，开光量探头报警时该值统一为0，目前只支持探头报警，当触发报警时，发送报警级别ALC_ALERT，当报警停止时，发送ALC_STOP
typedef enum _AlertLevelCode
{
	ALC_ALERT = 0,//报警，警报级别最高，通常用户开关量探头
	ALC_LEVEL1 = 10,//1级警告，AlertLevelCode的值越大，警告级别越低
	ALC_STOP = 255//报警停止，发送停止信息
}AlertLevelCode;


/* the common packet header, must be placed in front of any packets. */
typedef struct _OwspPacketHeader
{
	u_int32 packet_length;		//length of the following packet, donot include this header
	u_int32 packet_seq;			//packet sequence 包序号,每发送一个包就自增
	
} OwspPacketHeader;

/////////////////////////////////////////////////////////////////////////
//For TLV 
//////////////////////////////////////////////////////////////////////////
struct _TLV_Header {
	u_int16 tlv_type;
	u_int16 tlv_len;
};
//__attribute ((packed));
typedef struct _TLV_Header  TLV_HEADER;

/* version info: remote -> streaming server.  No response need */
// TLV_T: TLV_T_VERSION_INFO_ANSWER
// TLV_L: sizeof(TLV_V_VersionInfoRequest)
typedef struct _TLV_V_VersionInfoRequest
{
	u_int16   versionMajor;		// major version
	u_int16   versionMinor;		// minor version
}TLV_V_VersionInfoRequest;

// TLV_T: TLV_T_VERSION_INFO_ANSWER
// TLV_L: sizeof(TLV_V_VersionInfoResponse)
typedef struct _TLV_V_VersionInfoResponse
{
	u_int16 result;				//result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;
}TLV_V_VersionInfoResponse;

/* login request: remote -> streaming server */
typedef struct _TLV_V_LoginRequest
{
	char userName[STR_LEN_32];			//用户名, 后面不足部分为数字0      (为ASCII字符串)
	char password[STR_LEN_16];			//密码, 后面不足部分为数字0        (为ASCII字符串) 
	u_int32 deviceId;					//设备ID. CS模式下由服务器统一分配, P2P模式下为固定值
	u_int8  flag;						//should be set to 1 to be compatible with the previous version.
	u_int8  channel;					//channel, 0xFF 表示组合通道 同TLV_V_ChannelRequest中相同
	u_int8  reserve[2];				//用于组合通道： reserve[0] 初始通道号(从0开始)   reserver[1] 通道数目		
} TLV_V_LoginRequest;

//For HHDigital
// if reserve0 == 3 ,the data indicate phoneID (char*)
#define HKSSERVER_FLAG_ID							1
#define SZSTREAMING_FLAT_ID						2
#define HHDIGITAL_FLAG_ID							3

/* login response, streaming server -> remote */
typedef struct _TLV_V_LoginResponse
{
	u_int16 result;				//result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;
} TLV_V_LoginResponse;

 
/* send data request, streaming server -> remote. 
 * Now this command is ignored, the remote will send data to server actively and immidietely after logining.*/
typedef struct _TLV_V_SendDataRequest
{
	u_int32 deviceId;			//device id generating by the remote device
	u_int8  videoChannel;	
	u_int8  audioChannel;   
	u_int16 reserve;
} TLV_V_SendDataRequest;

/* send data response, remote -> streaming server */
typedef struct _TLV_V_SendDataResponse
{
	u_int16 result;			//result of send data request
	u_int16 reserve;
} TLV_V_SendDataResponse;

/* suspend sending data request, streaming server -> remote */
typedef struct _TLV_V_SuspendSendDataRequest
{
	u_int32 deviceId;			//device id generating by the remote device
	u_int8  videoChannel;
	u_int8  audioChannel; 
	u_int16 reserve;
} TLV_V_SuspendSendDataRequest;

/* suspend sending data response, remote -> streaming server */
typedef struct _TLV_V_SuspendSendDataResponse
{
	u_int16 result;			//result of send data request
	u_int16 reserve;
} TLV_V_SuspendSendDataResponse;



/* specify the format of video, this info is sent to server immidiately after StreamDataFormat*/
typedef struct _OWSP_VideoDataFormat
{
	u_int32 codecId;			//FOUR CC code，’H264’
	u_int32 bitrate;			//bps
	u_int16 width;				//image widht
	u_int16 height;				//image height
	u_int8 framerate;			//fps
	u_int8 colorDepth;			//should be 24 bits 
	u_int16 reserve;		

} OWSP_VideoDataFormat;

/* specify the format of audio, this info is sent to server immidiately after StreamDataFormat or VideoDataFormat*/
typedef struct _OWSP_AudioDataFormat
{
	u_int32 samplesPerSecond;		//samples per second
	u_int32 bitrate;			//bps
	u_int16 waveFormat;			//wave format, such as WAVE_FORMAT_PCM,WAVE_FORMAT_MPEGLAYER3
	u_int16 channelNumber;		//audio channel number
	u_int16 blockAlign;			//block alignment defined by channelSize * (bitsSample/8)
	u_int16 bitsPerSample;			//bits per sample
	u_int16 frameInterval;		//interval between frames, in milliseconds
	u_int16 reserve;

} OWSP_AudioDataFormat;

/* this format should be sent to the server before any other stream data,
 Plus if any format of video/audio has changed, it should send this info to server at the first time.
 followed by VideoDataFormat/AudioDataFormat*/
typedef struct _TLV_V_StreamDataFormat
{
	u_int8 videoChannel;					//视频通道号
	u_int8 audioChannel;					//音频通道号
	u_int8 dataType;							//流数据类型, 取值见StreamDataType
	u_int8 reserve;								//保留
	OWSP_VideoDataFormat videoFormat;	//视频格式
	OWSP_AudioDataFormat audioFormat;  //音频格式
} TLV_V_StreamDataFormat;


/* 视频帧信息 TLV */
typedef struct _TLV_V_VideoFrameInfo
{
	u_int8  channelId;			//通道ID
	u_int8  reserve;				//备用
	u_int16 checksum;				//校验和.目前为0未用
	u_int32 frameIndex;			//视频帧序号
	u_int32 time;				    //时间戳.
} TLV_V_VideoFrameInfo;

//之后是视频数据TLV, V部分是视频编码后的Raw Data.

/* 扩展视频帧信息 TLV, 当视频数据>64K时使用 */
typedef struct _TLV_V_VideoFrameInfoEx
{
	u_int8  channelId;			//通道ID
	u_int8  reserve;				//备用
	u_int16 checksum;				//校验和.目前为0未用
	u_int32 frameIndex;			//视频帧序号
	u_int32 time;				    //时间戳.
	u_int32 dataSize;				//视频数据长度
} TLV_V_VideoFrameInfoEx;

//之后是若干个视频数据TLV, V部分是视频编码后的Raw Data.


/* 音频信息 TLV */
typedef struct _TLV_V_AudioInfo
{
	u_int8 channelId;			//channel id
	u_int8  reserve;			//备用
	u_int16 checksum;			//checksum of audio data.
	u_int32 time;					// specify the time when this audio data is created.
} TLV_V_AudioInfo;

//之后是音频数据TLV, V部分就是音频编码后的Raw Data.

/* 扩展控制协议, 包括云台及TV控制 */
typedef struct _TLV_V_ControlRequest
{
		u_int32 deviceId;			// device id generating by the remote device
		u_int8  channel;			// channel id 
		u_int8  cmdCode;			// 控制命令字，参见_PTZCode
		u_int16 size;				// 控制参数数据长度,如果size==0 表示无控制参数
} TLV_V_ControlRequest;

//u_int8 * data;		//array of data followed.
// size = sizeof(PTZArgData);
//如果是上下左右，牵涉到速度的话，有水平速度arg1，垂直速度arg2；
//如果是预置位的话，操作第几个预制位使用arg1标明
//如果是清除预置位，操作第几个预置位使用arg1标明，如果0xffffffff表示清除全部
//如果是自动巡航，arg1=1表示启动，0表示停止
typedef struct _ControlArgData
{		
		u_int32 arg1;
		u_int32 arg2;
		u_int32 arg3;
		u_int32 arg4;
} ControlArgData;

/* 云台请求响应 */
typedef struct _TLV_V_ControlResponse
{
	u_int16 result;				//result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;
} TLV_V_ControlResponse;

/* 
  通道请求协议 
  Streaming server -> Device
*/
typedef struct _TLV_V_ChannelRequest
{
	u_int32 deviceId;
	u_int8  sourceChannel;	//源通道ID
	u_int8  destChannel;		//切换的目的通道ID
	u_int8	reserve[2];		
} TLV_V_ChannelRequest;

/*
  通道请求响应
  Device -> Streaming server
  message: MSG_CMD_CHANNEL_RESPONSE
*/
typedef struct _TLV_V_ChannelResponse
{
  u_int16 result;					//result of request. _RESPONSECODE_SUCC - succeeded, others - failed
  u_int8	currentChannel;	//如果不支持的通道，则返回当前通道号
	u_int8 reserve;	
} TLV_V_ChannelResponse;


/* 
	通道挂起协议 
  Streaming server -> Device 
  message: MSG_CMD_CHANNEL_SUSPEND
*/
typedef struct _TLV_V_ChannelSuspendRequest
{
	u_int8  channelId;	//Chanel id
	u_int8  reserve[3];	
} TLV_V_ChannelSuspendRequest;

/*
  通道挂起响应
  Device -> Streaming server
  message: MSG_CMD_SUSPEND_RESPONSE
*/
typedef struct _TLV_V_ChannelSuspendResponse
{
  u_int16 result;				//result of request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;	
} TLV_V_ChannelSuspendResponse;


/*
  心跳包
  Device -> Streaming server
  message: MSG_CMD_DEVICE_KEEP_ALIVE
*/
typedef struct _TLV_V_KeepAliveRequest
{
	u_int8  channelID;	//Channel id
	u_int8  reserve[3];	
} TLV_V_KeepAliveRequest;

/*
  心跳包响应
  Streaming server -> Device
  message: MSG_CMD_KEEP_ALIVE_ANSWER
*/
typedef struct _TLV_V_KeepAliveResponse
{
  u_int16 result;				//result of request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;	
} TLV_V_KeepAliveResponse;

/* 扩展报警协议，设备发到服务器 */
typedef struct _TLV_V_AlertRequest
{
  u_int32  	deviceId;   	// device id generating by the remote device
  u_int8   	channelId;   	// channel id 
  u_int8  	alertType;   	// 报警种类，参见 _AlertTypeCode
  u_int8  	alertLevel;   // 报警级别，参见 _AlertLevelCode
	u_int8  	reserve;    	//保留
  u_int8   	localTime[14];			//报警时本地时间字符串，格式为yyyymmddhhmmss,如"20080919132059"代表2008年9月19日13点20分59秒，时间精度为秒
  u_int16  	size;     		// array of data size followed，default =  0
} TLV_V_AlertRequest;

/* 报警请求响应，服务器发送到设备 */
typedef struct _TLV_V_AlertResponse
{
 u_int16 result;    //result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
 u_int16 reserve;
} TLV_V_AlertResponse;


/* 日期定义 */
typedef struct _OWSP_DATE
{
	u_int16 	m_year;			//年,2009
	u_int8		m_month;		//月,1-12
	u_int8		m_day;			//日,1-31
}OWSP_DATE;

/* 时间定义 */
typedef struct _OWSP_TIME
{
	u_int8		m_hour;			//0-23
	u_int8		m_minute;		//0-59
	u_int8		m_second;		//0-59
	u_int8		m_microsecond;		//毫秒	(0-249)   该值需要乘以4映射到0-1000ms
}OWSP_TIME;


/* DVS报告设备信息 */
typedef struct _TLV_V_DVSInfoRequest
{
	char		companyIdentity[STR_LEN_16];			//公司识别码,最多16个字符,后面不足部分为数字0      (为ASCII字符串)
	char   		equipmentIdentity[STR_LEN_16];			//设备识别码,本字段中为DVS的物理地址,即MAC地址,后面不足部分为数字0  (为ASCII字符串)
	char		equipmentName[STR_LEN_16];				//设备名称,最多16个字符,后面不足部分为数字0        (为ASCII字符串)
	char		equipmentVersion[STR_LEN_16];			//设备的软件版本,最多16个字符, 后面不足部分为数字0 (为ASCII字符串)
	OWSP_DATE	equipmentDate;							//设备的出厂日期20090120 
	u_int8		channleNumber;			//设备支持多少个通道
	u_int8		reserve1;						//保留 更改成报警类型//byte 1在家模式，2离家模式，3睡眠模式
	u_int8		reserve2;						//保留
	u_int8		reserve3;						//保留
} TLV_V_DVSInfoRequest;

/* DVS报告设备信息应答 */
typedef struct _TLV_V_DVSInfoResponse
{
	u_int16 result;    //result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;
} TLV_V_DVSInfoResponse;


/* 手机报告设备信息 */
typedef struct _TLV_V_PhoneInfoRequest
{
	u_int8   	equipmentIdentity[STR_LEN_16];		//设备识别码,本字段中为DVS的物理地址,即MAC地址
	u_int8   	equipmentOS[STR_LEN_16];						//手机的操作系统
	u_int8		reserve1;						//保留
	u_int8		reserve2;						//保留
	u_int8		reserve3;						//保留
	u_int8		reserve4;						//保留
} TLV_V_PhoneInfoRequest;

/* 手机报告设备信息应答 */
typedef struct _TLV_V_PhoneInfoResponse
{
	u_int16 result;    //result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;
} TLV_V_PhoneInfoResponse;

/* 获取录像列表请求 */
typedef struct _TLV_V_RecordFileSearchRequest
{
	u_int8	searchType;				//搜索类型  1-表示根据开始和结束时间搜索   2-按月搜索(startDate参数中的月）
	u_int8  filetype;//搜索文件类型  1：常规录像(定时录像) 2：报警录像(移动报警录像)，3 ：所有录像 4、手动录像 5、探头报警录像
	u_int8  reserve1;
	u_int8  reserve0;
	u_int16 indexStart;				//当前页中启始索引
	u_int16 countOfPage;			//当前页返回的最大记录数
	u_int32	channelMask32;		//0xffffffff 表示全部通道 搜索某一个通道就只需要在相应的位上赋值1其他的赋值0即可
	OWSP_DATE  startDate;			//开始日期
	OWSP_TIME  startTime;			//开始时间
	OWSP_DATE  endDate;				//结束日期
	OWSP_TIME  endTime;				//结束日期
	u_int32 argSize;					//附件长度
}TLV_V_RecordFileSearchRequest;

/* 获取录像列表应答 */
//文件列表可以xml或者结构体数组返回
typedef struct _TLV_V_RecordFileSearchResponse
{	
	u_int16 result;		//0表示成功
        u_int16 wReserve;	
	u_int8	argType	;	//附件类型， 1-表示是search_file_setting数组，2-表示xml(ascii), 3-表示xml(utf-8)  4-表示按月历返回search_file_month_setting数组
	u_int8  reserve;
	u_int16 totalCount;	//记录总数
	u_int32 argSize;	//附件长度
}TLV_V_RecordFileSearchResponse;
#if 0
typedef struct	FileInfoSearchRet_Tag
{
	INT32	fileSize;
	INT32 	recType;
	INT32	channel;							
	INT8	fileName[ 128 ];
	UINT8	start_hour;
	UINT8	start_min;
	UINT8	start_sec;
	UINT8	end_hour;
	UINT8 	end_min;
	UINT8	end_sec;
	UINT8 	reserve[ 2 ];
}search_file_setting;
#endif
/*
typedef struct	FileInfoSearchMonthRet_Tag
{
	UINT8   month;
	UINT8	day;			
	UINT16 	count;
}search_file_month_setting;
*/
typedef struct	FileInfoSearchMonthRet_Tag
{
	u_int8   month;
	u_int8	 reserve[3];	
	u_int32  calendar;//bit0-bit30表示1号到31号，如果bit位为1表示有录像，为0表示没有录像
}search_file_month_setting;

//回放命令字
typedef enum _PlaybackCommandCode
{
	PCC_STOP =  1,	//停止
	PCC_PAUSE = 2,	//暂停
	PCC_RESUME = 3, //继续
	PCC_PLAY =4,	//播放
	PCC_SEEK =5     //搜索
}PlaybackCommandCode;

/* 播放或者下载录像 */
//argSize > 0 且argType=1 时，argData为文件的全路径字符串
typedef struct _TLV_V_RecordFilePlayRequest
{
	u_int32		channel;	//通道号BIT位表示
 	u_int8		playCmdCode;  // PlaybackCommandCode
	u_int8		offsetUnit;		//偏移单位, 1-表示byte, 2-表示时间(表示播放某一时间段)
	u_int8		argType;			//附件类型，1-表示文件全路径
	u_int8		reserve;
	OWSP_DATE  startDate;			//开始日期
	OWSP_TIME  startTime;			//开始时间
	OWSP_DATE  endDate;				//结束日期
	OWSP_TIME  endTime;				//结束日期
	u_int32		offsetPos;		//偏移位置
	u_int32   argSize;			//附件长度
}TLV_V_RecordFilePlayRequest;


/* 获取录像列表应答 */
typedef struct _TLV_V_RecordFilePlayResponse
{
		u_int16 result;				//result of login request. _RESPONSECODE_SUCC - succeeded, others - failed
		u_int8	playStatus;			//播放状态   0-正常 1-结束
		u_int8  argType;
		u_int32 reserve;
		u_int32 argSize;
}TLV_V_RecordFilePlayResponse;

//////////////////新增获取和设置设备信息协议///////////////////

//#define TLV_T_DEVICEINFO_REQUEST		 79			//设备信息查询
//#define TLV_T_DEVICEINFO_ANSWER		 80			//设备信息回复

typedef struct _TLV_V_DeviceInfoRequest {
	u_int32	channelMask32;//0xffffffff 表示全部通道 搜索某一个通道就只需要在相应的位上赋值1其他的赋值0即可 如果是0表示只是查询设备信息不查询具体通道
	u_int8 	reserve[ 4 ];
}TLV_V_DeviceInfoRequest;

typedef struct _TLV_V_DeviceInfoResponse
{
	u_int32	channelMask32;		//0xffffffff 表示全部通道 搜索某一个通道就只需要在相应的位上赋值1其他的赋值0即可 如果是0表示只是查询设备信息不查询具体通道
	TLV_V_DVSInfoRequest DVSInfoRequest;	
	u_int32 argSize;//附件大小 附件不为0时候 附件内容就是_TLV_V_ChannelInfoResponse 数组
}TLV_V_DeviceInfoResponse;

typedef struct	_TLV_V_ChannelInfo
{							
	char	ChannelName[ 128 ];
	int     BitRate;//单位bit
	u_int8	Framesize;//byte 1 2 3 4 5 分别对应qqvga qcif qvga cif d1
	u_int8	FrameRate;//帧率 1-25
	u_int8 	reserve[ 2 ];
}TLV_V_ChannelInfo;

/////////////////////////////////////////////////////////////////////////
//For TLV 
//////////////////////////////////////////////////////////////////////////
//struct _TLV {
//    u_int16 tlv_type;
//    u_int16 tlv_len;
//    u_int8  tlv_data[0];
//} __attribute ((packed));
//typedef struct _TLV  TLV;


#define FILEFLAG	"DSV"
//////////////////////////////////////////////////////////////////////////
//For vod streaming  only Keyframe 
//////////////////////////////////////////////////////////////////////////
typedef struct _TLV_V_FileAttributeData
{
	u_int32 totalframes;
	u_int32 totaltimes;
}TLV_V_FileAttributeData;

typedef struct _TLV_V_StreamIndexDataHeader
{
	u_int32 count;			//
	u_int32 reserve;		//在文件
	u_int32 datasize;		//
}TLV_V_StreamIndexDataHeader;

typedef struct _TLV_V_StreamIndexData
{
	u_int32 timestamp;
	u_int32 pos;		//在文件
}TLV_V_StreamIndexData;

typedef struct _TLV_V_StreamEndData
{
	u_int32 timestamp;
	u_int16 result;				//result of request. _RESPONSECODE_SUCC - succeeded, others - failed
	u_int16 reserve;	
} TLV_V_StreamEndData;

typedef struct _TLV_V_StreamFileDataHeader
{
	u_int32 timestamp;
}TLV_V_StreamFileDataHeader;


//#pragma pack(pop)

#endif
