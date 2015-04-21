/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, ANTS, Inc, All Rights Reserved.
*
*	Project Name:ServerCoreV2
*	File Name:AntsMidLayerSDK.h
*
*	Writed by ItmanLee at 2012 - 11 - 05 Ants,WuHan,HuBei,China
*/
#ifndef __ANTSMIDLAYERSDK_H__
#define __ANTSMIDLAYERSDK_H__
#if (defined(_WIN32)||defined(WIN32))
#include <Windows.h>
#else
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef int LONG;
#ifndef BOOL
#define BOOL int
#else
typedef int BOOL;
#endif
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned int* LPDWORD; 
typedef void*	LPVOID;

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef NULL
#define NULL 	0
#endif
#endif

/** @入参*/
#ifndef IN
#define IN
#endif

/** @出参*/
#ifndef OUT
#define OUT
#endif

//!宏定义
#define ANTS_MID_MAX_NAMELEN			    16		//!DVR本地登陆名
#define ANTS_MID_MAX_RIGHT			    	32		//!设备支持的权限（1-12表示本地权限，13-32表示远程权限）
#define ANTS_MID_NAME_LEN			    		32    //!用户名长度
#define ANTS_MID_PASSWD_LEN			    	16    //!密码长度
#define ANTS_MID_SERIALNO_LEN		    	48    //!序列号长度
#define ANTS_MID_MACADDR_LEN			    6     //!MAC地址长度
#define ANTS_MID_MAX_ETHERNET		    	2     //!设备可配以太网络
#define ANTS_MID_MAX_NETWORK_CARD    	4     //!设备可配最大网卡数目
#define ANTS_MID_PATHNAME_LEN		    	128   //!路径长度

#define ANTS_MID_MAX_TIMESEGMENT		  8	   	//!设备最大时间段数

#define ANTS_MID_MAX_SHELTERNUM				4     //!设备最大遮挡区域数
#define ANTS_MID_MAX_DAYS							7     //!每周天数
#define ANTS_MID_PHONENUMBER_LEN			32    //!PPPOE拨号号码最大长度

#define ANTS_MID_MAX_DISKNUM					8			//!设备最大硬盘数
#define ANTS_MID_MAX_DISKNUM_V2					24

#define ANTS_MID_MAX_WINDOW						32    //设备本地显示最大播放窗口数
#define ANTS_MID_MAX_VGA							4     //设备最大可接VGA数

#define ANTS_MID_MAX_USERNUM					8     //设备最大用户数
#define ANTS_MID_MAX_EXCEPTIONNUM			32    //!设备最大异常处理数
#define ANTS_MID_MAX_LINK							6     //!设备单通道最大视频流连接数

#define ANTS_MID_MAX_STRINGNUM				1			//!设备最大OSD字符块数
#define ANTS_MID_MAX_HD_GROUP					8	    //!设备最大硬盘组数

#define ANTS_MID_MAX_WIFI_ESSID_SIZE	    				32      //WIFI的SSID号长度
#define ANTS_MID_MAX_WIFI_ENCODING_TOKEN					32      //WIFI密锁最大字节数
#define ANTS_MID_MAX_WIFI_WEP_KEY_COUNT						4
#define ANTS_MID_MAX_WIFI_WEP_KEY_LENGTH					33
#define ANTS_MID_MAX_WIFI_WPA_PSK_KEY_LENGTH			63
#define ANTS_MID_MIN_WIFI_WPA_PSK_KEY_LENGTH			8
#define ANTS_MID_MAX_WIFI_AP_COUNT								20
#define ANTS_MID_WIFI_MACADDR_LEN									6

#define ANTS_MID_MAX_3G_DEVICE_DESC_LEN						32
#define ANTS_MID_MAX_3G_DEVICE_NUM								200
#define ANTS_MID_MAX_MANAGERHOST_NUM							2

#define ANTS_MID_MAX_SERIAL_NUM										64	    //最多支持的透明通道路数
#define ANTS_MID_MAX_DDNS_NUMS	        					10      //设备最大可配ddns数
#define ANTS_MID_MAX_DOMAIN_NAME		    					64			//最大域名长度
#define ANTS_MID_MAX_EMAIL_ADDR_LEN	    					48      //最大email地址长度
#define ANTS_MID_MAX_EMAIL_PWD_LEN								32      //最大email密码长度

#define ANTS_MID_MAX_NFS_DISK									8
#define ANTS_MID_MAX_NET_DISK 								16			//!最大网络硬盘个数

#define ANTS_MID_MAXPROGRESS		        			100     //!回放时的最大百分率
#define ANTS_MID_MAX_SERIALNUM	        			2       //!设备支持的串口数 1-232， 2-485

#define ANTS_MID_MAX_PRESET										256			//!设备支持的云台预置点数
#define ANTS_MID_MAX_TRACK										256			//!设备支持的云台轨迹数
#define ANTS_MID_MAX_CRUISE										256			//!设备支持的云台巡航数

#define ANTS_MID_MAX_CRUISE_PRESET_NUMS				32 	    //!一条巡航最多的巡航点

#define ANTS_MID_MAX_SERIAL_PORT							8       //!设备支持232串口数
#define ANTS_MID_MAX_PREVIEW_MODE							8       //!设备支持最大预览模式数目 1画面,4画面,9画面,16画面....
#define ANTS_MID_LOG_INFO_LEN									11840   //!日志附加信息
#define ANTS_MID_DESC_LEN											32      //!云台描述字符串长度
#define ANTS_MID_DESC_LEN_64									64
#define ANTS_MID_PTZ_PROTOCOL_NUM							200     //!最大支持的云台协议数

#define ANTS_MID_MAX_AUDIO										2       //!语音对讲通道数
#define ANTS_MID_MAX_CHANNUM									16      //!设备最大通道数
#define ANTS_MID_MAX_CHANNUM_V2								64      //!设备最大通道数
#define ANTS_MID_MAX_ALARMIN									16      //!设备最大报警输入数
#define ANTS_MID_MAX_ALARMIN_V2								64      //!设备最大报警输入数
#define ANTS_MID_MAX_ALARMOUT									4       //!设备最大报警输出数

#define ANTS_MID_MAX_RECORD_FILE_NUM					20      //!每次删除或者刻录的最大文件数

#define ANTS_MID_MAX_FORTIFY_NUM							10   		//!最大布防个数
#define ANTS_MID_MAX_INTERVAL_NUM							4    		//!最大时间间隔个数

#define	ANTS_MID_MAX_IPCNUM										32
#define	ANTS_MID_MAX_IPCNUM_V2								36

#define ANTS_MID_MAX_NODE_NUM				         	256  //!节点个数
#define ANTS_MID_MAX_ABILITYTYPE_NUM			  	12   //!最大能力项

/**********************云台控制命令 begin*************************/	
#define ANTS_MID_LIGHT_PWRON				2		/* 接通灯光电源 */
#define ANTS_MID_WIPER_PWRON				3		/* 接通雨刷开关 */
#define ANTS_MID_FAN_PWRON					4		/* 接通风扇开关 */
#define ANTS_MID_HEATER_PWRON				5		/* 接通加热器开关 */
#define ANTS_MID_AUX_PWRON1					6		/* 接通辅助设备开关 */
#define ANTS_MID_AUX_PWRON2					7		/* 接通辅助设备开关 */
#define ANTS_MID_SET_PRESET					8		/* 设置预置点 */
#define ANTS_MID_CLE_PRESET					9		/* 清除预置点 */

#define ANTS_MID_ZOOM_IN						11	/* 焦距以速度SS变大(倍率变大) */
#define ANTS_MID_ZOOM_OUT						12	/* 焦距以速度SS变小(倍率变小) */
#define ANTS_MID_FOCUS_NEAR      		13  /* 焦点以速度SS前调 */
#define ANTS_MID_FOCUS_FAR      	 	14  /* 焦点以速度SS后调 */
#define ANTS_MID_IRIS_OPEN      		15  /* 光圈以速度SS扩大 */
#define ANTS_MID_IRIS_CLOSE      		16  /* 光圈以速度SS缩小 */

#define ANTS_MID_TILT_UP						21	/* 云台以SS的速度上仰 */
#define ANTS_MID_TILT_DOWN					22	/* 云台以SS的速度下俯 */
#define ANTS_MID_PAN_LEFT						23	/* 云台以SS的速度左转 */
#define ANTS_MID_PAN_RIGHT					24	/* 云台以SS的速度右转 */
#define ANTS_MID_UP_LEFT						25	/* 云台以SS的速度上仰和左转 */
#define ANTS_MID_UP_RIGHT						26	/* 云台以SS的速度上仰和右转 */
#define ANTS_MID_DOWN_LEFT					27	/* 云台以SS的速度下俯和左转 */
#define ANTS_MID_DOWN_RIGHT					28	/* 云台以SS的速度下俯和右转 */
#define ANTS_MID_PAN_AUTO						29	/* 云台以SS的速度左右自动扫描 */

#define ANTS_MID_FILL_PRE_SEQ				30	/* 将预置点加入巡航序列 */
#define ANTS_MID_SET_SEQ_DWELL			31	/* 设置巡航点停顿时间 */
#define ANTS_MID_SET_SEQ_SPEED			32	/* 设置巡航速度 */
#define ANTS_MID_CLE_PRE_SEQ				33	/* 将预置点从巡航序列中删除 */
#define ANTS_MID_STA_MEM_CRUISE			34	/* 开始记录轨迹 */
#define ANTS_MID_STO_MEM_CRUISE			35	/* 停止记录轨迹 */
#define ANTS_MID_RUN_CRUISE					36	/* 开始轨迹 */
#define ANTS_MID_RUN_SEQ						37	/* 开始巡航 */
#define ANTS_MID_STOP_SEQ						38	/* 停止巡航 */
#define ANTS_MID_GOTO_PRESET				39	/* 快球转到预置点 */
#define ANTS_MID_FILL_SEQ_CRUISE		40	/* 将巡航序列设置到云台中 */
/**********************云台控制命令 end*************************/	

/*************************************************
回放时播放控制命令宏定义 具体支持查看函数说明和代码 Begin
**************************************************/	
#define ANTS_MID_PLAYSTART					1//开始播放
#define ANTS_MID_PLAYSTOP						2//停止播放
#define ANTS_MID_PLAYPAUSE					3//暂停播放
#define ANTS_MID_PLAYRESTART				4//恢复播放
#define ANTS_MID_PLAYFAST						5//快放
#define ANTS_MID_PLAYSLOW						6//慢放
#define ANTS_MID_PLAYNORMAL					7//正常速度
#define ANTS_MID_PLAYFRAME					8//单帧放
#define ANTS_MID_PLAYSTARTAUDIO			9//打开声音
#define ANTS_MID_PLAYSTOPAUDIO			10//关闭声音
#define ANTS_MID_PLAYAUDIOVOLUME		11//调节音量
#define ANTS_MID_PLAYSETPOS					12//改变文件回放的进度
#define ANTS_MID_PLAYGETPOS					13//获取文件回放的进度
#define ANTS_MID_PLAYGETTIME				14//获取当前已经播放的时间(按文件回放的时候有效)
#define ANTS_MID_PLAYGETFRAME				15//获取当前已经播放的帧数(按文件回放的时候有效)
#define ANTS_MID_GETTOTALFRAMES			16//获取当前播放文件总的帧数(按文件回放的时候有效)
#define ANTS_MID_GETTOTALTIME				17//获取当前播放文件总的时间(按文件回放的时候有效)
#define ANTS_MID_THROWBFRAME				20//丢B帧
#define ANTS_MID_SETSPEED						24//设置码流速度
#define ANTS_MID_KEEPALIVE					25//保持与设备的心跳(如果回调阻塞，建议2秒发送一次)
#define ANTS_MID_PLAYSETTIME				26//按绝对时间定位
#define ANTS_MID_PLAYGETTOTALLEN		27//获取按时间回放对应时间段内的所有文件的总长度
#define ANTS_MID_PLAYSETDISPLAYZOOM	30//设置回放区域局部放大
/*************************************************
回放时播放控制命令宏定义 具体支持查看函数说明和代码 End
**************************************************/	

/*************************设备编码能力集 begin*******************************/
#define COMPRESSIONCFG_ABILITY  0x400    //获取压缩参数能力获取
typedef enum{
	ANTS_MID_COMPRESSION_STREAM_ABILITY = 0,
	ANTS_MID_MAIN_RESOLUTION_ABILITY,
	ANTS_MID_SUB_RESOLUTION_ABILITY,
	ANTS_MID_EVENT_RESOLUTION_ABILITY,
	ANTS_MID_FRAME_ABILITY,
	ANTS_MID_BITRATE_TYPE_ABILITY,
	ANTS_MID_BITRATE_ABILITY
}ANTS_MID_COMPRESSION_ABILITY_TYPE;
/*************************设备编码能力集 end*******************************/

/*************************帧类型定义begin*******************************/
typedef enum {
	AntsMidMainStream=0x00,
	AntsMidSubStream=0x01,
	AntsMidThirdStream=0x02
}eANTS_MID_STREAMTYPE;

typedef enum {
	//!主码流帧类型
	AntsMidPktError=0x00,
	AntsMidPktIFrames=0x01,
	AntsMidPktAudioFrames=0x08,
	AntsMidPktPFrames=0x09,
	AntsMidPktBBPFrames=0x0a,
	AntsMidPktMotionDetection=0x0b,
	AntsMidPktDspStatus=0x0c,
	AntsMidPktOrigImage=0x0d,
	AntsMidPktSysHeader=0x0e,
	AntsMidPktBPFrames=0x0f,
	AntsMidPktSFrames=0x10,
	//!子码流帧类型
	AntsMidPktSubSysHeader=0x11,
	AntsMidPktSubIFrames=0x12,
	AntsMidPktSubPFrames=0x13,
	AntsMidPktSubBBPFrames=0x14,
	//!智能分析信息帧类型
	AntsMidPktVacEventZones=0x15,
	AntsMidPktVacObjects=0x16,
	//!第三码流帧类型
	AntsMidPktThirdSysHeader=0x17,
	AntsMidPktThirdIFrames=0x18,
	AntsMidPktThirdPFrames=0x19,
	AntsMidPktThirdBBPFrames=0x1a
}eANTS_MID_FRAME_TYPE;
/*************************帧类型定义end*******************************/

/*************************参数配置命令 begin*******************************/
//用于SetParameter和GetParameter,注意其对应的配置结构
typedef enum{
	ANTS_MID_CFG=0x200,
	ANTS_MID_GET_DEVICECFG,						//!获取设备参数 +
	ANTS_MID_SET_DEVICECFG,						//!设置设备参数
	
	ANTS_MID_GET_NETCFG,							//!获取网络参数 +
	ANTS_MID_SET_NETCFG,							//!设置网络参数
	
	ANTS_MID_GET_PICCFG,							//!获取图象参数 +
	ANTS_MID_SET_PICCFG,							//!设置图象参数
	
	ANTS_MID_GET_PICCFG_V2,						//!获取图象参数(64路扩展) +
	ANTS_MID_SET_PICCFG_V2,						//!设置图象参数(64路扩展)
	
	ANTS_MID_GET_COMPRESSCFG,					//!获取压缩参数 +
	ANTS_MID_SET_COMPRESSCFG,					//!设置压缩参数
	
	ANTS_MID_GET_RECORDCFG,						//!获取录像时间参数 +
	ANTS_MID_SET_RECORDCFG,						//!设置录像时间参数
	
	ANTS_MID_GET_DECODERCFG,					//!获取解码器参数 +
	ANTS_MID_SET_DECODERCFG,					//!设置解码器参数
	
	ANTS_MID_GET_RS232CFG,						//!获取232串口参数 +
	ANTS_MID_SET_RS232CFG,						//!设置232串口参数
	
	ANTS_MID_GET_ALARMINCFG,					//!获取报警输入参数 +
	ANTS_MID_SET_ALARMINCFG,					//!设置报警输入参数
	
	ANTS_MID_GET_ALARMINCFG_V2,				//!获取报警输入参数(64路扩展) +
	ANTS_MID_SET_ALARMINCFG_V2,				//!设置报警输入参数(64路扩展)
	
	ANTS_MID_GET_ALARMOUTCFG,					//!获取报警输出参数 +
	ANTS_MID_SET_ALARMOUTCFG,					//!设置报警输出参数
	
	ANTS_MID_GET_TIMECFG,							//!获取DVR时间 +
	ANTS_MID_SET_TIMECFG,							//!设置DVR时间
	
	ANTS_MID_GET_USERCFG,							//!获取用户参数 +
	ANTS_MID_SET_USERCFG,							//!设置用户参数
	
	ANTS_MID_GET_USERCFG_V2,					//!获取用户参数(64路扩展) +
	ANTS_MID_SET_USERCFG_V2,					//!设置用户参数(64路扩展)
	
	ANTS_MID_GET_EXCEPTIONCFG,				//!获取异常参数 +
	ANTS_MID_SET_EXCEPTIONCFG,				//!设置异常参数
	
	ANTS_MID_GET_ZONEANDDST,					//!获取时区和夏时制参数 +
	ANTS_MID_SET_ZONEANDDST,					//!设置时区和夏时制参数
	
	ANTS_MID_GET_SHOWSTRING,					//!获取叠加字符参数 +
	ANTS_MID_SET_SHOWSTRING,					//!设置叠加字符参数
	
	ANTS_MID_GET_EVENTCOMPCFG,				//!获取事件触发录像参数 +
	ANTS_MID_SET_EVENTCOMPCFG,				//!设置事件触发录像参数
	
	ANTS_MID_GET_AUTOREBOOT,					//!获取自动维护参数 +
	ANTS_MID_SET_AUTOREBOOT,					//!设置自动维护参数
	
	ANTS_MID_GET_NETAPPCFG,						//!获取网络应用参数 NTP/DDNS/EMAIL +
	ANTS_MID_SET_NETAPPCFG,						//!设置网络应用参数 NTP/DDNS/EMAIL
	
	ANTS_MID_GET_NTPCFG,							//!获取网络应用参数 NTP +
	ANTS_MID_SET_NTPCFG,							//!设置网络应用参数 NTP
	
	ANTS_MID_GET_DDNSCFG,							//!获取网络应用参数 DDNS +
	ANTS_MID_SET_DDNSCFG,							//!设置网络应用参数 DDNS
	
	ANTS_MID_GET_EMAILCFG,						//!获取网络应用参数 EMAIL +
	ANTS_MID_SET_EMAILCFG,						//!设置网络应用参数 EMAIL
	
	ANTS_MID_GET_HDCFG,								//!获取硬盘管理配置参数 +
	ANTS_MID_SET_HDCFG,								//!设置硬盘管理配置参数
	
	ANTS_MID_GET_HDGROUP_CFG,					//!获取盘组管理配置参数 +
	ANTS_MID_SET_HDGROUP_CFG,					//!设置盘组管理配置参数
	
	ANTS_MID_GET_HDGROUP_CFG_V2,			//!获取盘组管理配置参数(64路扩展)+
	ANTS_MID_SET_HDGROUP_CFG_V2,			//!设置盘组管理配置参数(64路扩展)
	
	ANTS_MID_GET_COMPRESSCFG_AUD,			//!获取设备语音对讲编码参数 +
	ANTS_MID_SET_COMPRESSCFG_AUD,			//!设置设备语音对讲编码参数
	
	ANTS_MID_GET_SNMPCFG,							//!获取设备SNMP配置参数 +
	ANTS_MID_SET_SNMPCFG,							//!设置设备SNMP配置参数
	
	ANTS_MID_GET_NETCFG_MULTI,				//!获取设备多网卡配置参数 +
	ANTS_MID_SET_NETCFG_MULTI,				//!设置设备多网卡配置参数
	
	ANTS_MID_GET_NFSCFG,							//!获取设备NFS配置参数 +
	ANTS_MID_SET_NFSCFG,							//!设置设备NFS配置参数
	
	ANTS_MID_GET_NET_DISKCFG,					//!获取设备网络硬盘配置参数 +
	ANTS_MID_SET_NET_DISKCFG,					//!设置设备网络硬盘配轩参数
	
	ANTS_MID_GET_IPCCFG,							//获取IPC配置参数 +
	ANTS_MID_SET_IPCCFG,							//设置IPC配置参数 
	
	ANTS_MID_GET_IPCCFG_V2,						//设置IPC配置参数(64路扩展)  
	ANTS_MID_SET_IPCCFG_V2,						//获取IPC配置参数(64路扩展)+

	ANTS_MID_GET_WIFI_CFG,						//!获取IP监控设备无线参数
	ANTS_MID_SET_WIFI_CFG,						//!设置IP监控设备无线参数 +
	
	ANTS_MID_GET_WIFI_WORKMODE,				//!获取IP监控设备网口工作模式参数
	ANTS_MID_SET_WIFI_WORKMODE,				//!设置IP监控设备网口工作模式参数 +
	
	ANTS_MID_GET_3G_CFG,							//!获取3G配置参数
	ANTS_MID_SET_3G_CFG,							//!设置3G配置参数 +
	
	ANTS_MID_GET_MANAGERHOST_CFG, 		//!获取主动注册管理主机配置参数 +
	ANTS_MID_SET_MANAGERHOST_CFG,			//!设置主动注册管理主机配置参数
	
	ANTS_MID_GET_RTSPCFG,							//!获取RTSP配置参数 +
	ANTS_MID_SET_RTSPCFG,							//!设置RTSP配置参数
	
	ANTS_MID_GET_VIDEOEFFECT,
	ANTS_MID_SET_VIDEOEFFECT,
	
	ANTS_MID_GET_MOTIONCFG,
	ANTS_MID_SET_MOTIONCFG,
	
	ANTS_MID_GET_MOTIONCFG_V2,
	ANTS_MID_SET_MOTIONCFG_V2,
	
	ANTS_MID_GET_SHELTERCFG,
	ANTS_MID_SET_SHELTERCFG,
	
	ANTS_MID_GET_HIDEALARMCFG,
	ANTS_MID_SET_HIDEALARMCFG,
	
	ANTS_MID_GET_VIDEOLOSTCFG,
	ANTS_MID_SET_VIDEOLOSTCFG,
	
	ANTS_MID_GET_OSDCFG,
	ANTS_MID_SET_OSDCFG,
	
	ANTS_MID_GET_VIDEOFORMAT,
	ANTS_MID_SET_VIDEOFORMAT,

	ANTS_MID_GET_NVRWORKMODE,
	ANTS_MID_SET_NVRWORKMODE,
	
	ANTS_MID_GET_NETDEVCONNETCTCFG,
	ANTS_MID_SET_NETDEVCONNETCTCFG,
	
	ANTS_MID_GET_DEVCHANNAME_CFG,
	ANTS_MID_SET_DEVCHANNAME_CFG,

	ANTS_MID_GET_DEVCHANNAME_CFG_V2,
	ANTS_MID_SET_DEVCHANNAME_CFG_V2,

	ANTS_MID_GET_HDCFG_V2,
	ANTS_MID_SET_HDCFG_V2,

	ANTS_MID_GET_SENSOR_CFG,
	ANTS_MID_SET_SENSOR_CFG,

	ANTS_MID_GET_3GDEVICE_CFG=0x600,	//!获取设备支持3G上网设备集合	
	ANTS_MID_GET_AP_INFO_LIST,				//!获取无线网络资源参数
	ANTS_MID_GET_USERINFO,						//!获取当前用户信息
	ANTS_MID_GET_USERINFO_V2,
	ANTS_MID_GET_PTZCFG,							//!获取设备支持PTZ协议集合
	ANTS_MID_GET_WORKSTATUS,					//!获取设备当前工作状态
	ANTS_MID_GET_WORKSTATUS_V2,
	ANTS_MID_GET_COMPRESS_ABILITY,		//!获取设备压缩能力集合
	ANTS_MID_GET_WORKSTATUS_V3,

}ANTS_MID_COMMAND;
/*************************参数配置命令 end*******************************/

/*******************报警信息结构 begin*********************/
typedef struct {
	DWORD dwAlarmType;															/*0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问, 9-视频信号异常，10-录像异常*/
	DWORD dwAlarmInputNumber;												/*报警输入端口*/
	BYTE byAlarmOutputNumber[ANTS_MID_MAX_ALARMOUT];/*触发的输出端口，哪一位为1表示对应哪一个输出*/
	BYTE byAlarmRelateChannel[ANTS_MID_MAX_CHANNUM];/*触发的录像通道，哪一位为1表示对应哪一路录像, dwAlarmRelateChannel[0]对应第1个通道*/
	BYTE byChannel[ANTS_MID_MAX_CHANNUM];						/*dwAlarmType为2或3,6,9,10时，表示哪个通道，dwChannel[0]位对应第1个通道*/
	BYTE byDiskNumber[ANTS_MID_MAX_DISKNUM];				/*dwAlarmType为1,4,5时,表示哪个硬盘, dwDiskNumber[0]位对应第1个硬盘*/
}ANTS_MID_ALARMINFO,*LPANTS_MID_ALARMINFO;

typedef struct {
	DWORD dwAlarmType;																	/*0-信号量报警,1-硬盘满,2-信号丢失,3－移动侦测,4－硬盘未格式化,5-读写硬盘出错,6-遮挡报警,7-制式不匹配, 8-非法访问, 9-视频信号异常，10-录像异常*/
	DWORD dwAlarmInputNumber;														/*报警输入端口*/
	BYTE byAlarmOutputNumber[ANTS_MID_MAX_ALARMOUT];		/*触发的输出端口，哪一位为1表示对应哪一个输出*/
	BYTE byAlarmRelateChannel[ANTS_MID_MAX_CHANNUM_V2];	/*触发的录像通道，哪一位为1表示对应哪一路录像, dwAlarmRelateChannel[0]对应第1个通道*/
	BYTE byChannel[ANTS_MID_MAX_CHANNUM_V2];						/*dwAlarmType为2或3,6,9,10时，表示哪个通道，dwChannel[0]位对应第1个通道*/
	BYTE byDiskNumber[ANTS_MID_MAX_DISKNUM];						/*dwAlarmType为1,4,5时,表示哪个硬盘, dwDiskNumber[0]位对应第1个硬盘*/
}ANTS_MID_ALARMINFO_V2,*LPANTS_MID_ALARMINFO_V2;
/*******************报警信息结构 end*********************/

/*******************编码能力集结构 begin*********************/
typedef struct{
	int iValue;
	BYTE byDescribe[ANTS_MID_DESC_LEN];
	DWORD dwFreeSpace;
	BYTE byRes[12];
}ANTS_MID_DESC_NODE,*LPANTS_MID_DESC_NODE;

typedef struct{
	DWORD dwAbilityType;
	BYTE byRes[32];
	DWORD dwNodeNum;
	ANTS_MID_DESC_NODE struDescNode[ANTS_MID_MAX_NODE_NUM];
}ANTS_MID_ABILITY_LIST,*LPANTS_MID_ABILITY_LIST;

typedef struct{
	DWORD dwSize;
	DWORD dwAbilityNum;
	ANTS_MID_ABILITY_LIST struAbilityNode[ANTS_MID_MAX_ABILITYTYPE_NUM];
}ANTS_MID_COMPRESSIONCFG_ABILITY,*LPANTS_MID_COMPRESSIONCFG_ABILITY;
/*******************编码能力集结构 end*********************/

/*******************参数配置结构、参数 begin*********************/
//!校时结构参数
typedef struct{
	DWORD dwYear;			//年
	DWORD dwMonth;		//月
	DWORD dwDay;			//日
	DWORD dwHour;			//时
	DWORD dwMinute;		//分
	DWORD dwSecond;		//秒
}ANTS_MID_TIME, *LPANTS_MID_TIME;

//!时间段(子结构)
typedef struct{
	//!开始时间
  BYTE byStartHour;
	BYTE byStartMin;
	//!结束时间
	BYTE byStopHour;
	BYTE byStopMin;
}ANTS_MID_SCHEDTIME,*LPANTS_MID_SCHEDTIME;

typedef struct{
	BYTE byBrightness;  	/*亮度,0-255*/
	BYTE byContrast;    	/*对比度,0-255*/	
	BYTE bySaturation;  	/*饱和度,0-255*/
	BYTE byHue;    				/*色调,0-255*/
}ANTS_MID_COLOR,*LPANTS_MID_COLOR;

//!报警和异常处理结构(子结构)(多处使用)
typedef struct{
	DWORD dwHandleType;		/*处理方式,处理方式的"或"结果*/
												/*0x00: 无响应*/
												/*0x01: 监视器上警告*/
												/*0x02: 声音警告*/
												/*0x04: 上传中心*/
												/*0x08: 触发报警输出*/
												/*0x10: Jpeg抓图并上传EMail*/
	BYTE byRelAlarmOut[ANTS_MID_MAX_ALARMOUT];	//报警触发的输出通道,报警触发的输出,为1表示触发该输出
}ANTS_MID_HANDLEEXCEPTION,*LPANTS_MID_HANDLEEXCEPTION;

//!信号丢失报警(子结构)
typedef struct {
	BYTE byEnableHandleVILost;	/* 是否处理信号丢失报警 */
	BYTE byRes[3];
	ANTS_MID_HANDLEEXCEPTION strVILostHandleType;	/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//布防时间
}ANTS_MID_VILOST, *LPANTS_MID_VILOST;

//移动侦测(子结构)
typedef struct {
	BYTE byMotionScope[64][96];/* 侦测区域,0-16位,表示12行,共有16*12个小宏块,为1表示是移动侦测区域,0-表示不是 */
	BYTE byMotionSensitive;/* 移动侦测灵敏度, 0 - 5,越高越灵敏,oxff关闭 */
	BYTE byEnableHandleMotion;/* 是否处理移动侦测 0－否 1－是*/ 
	BYTE byPrecision;/* 移动侦测算法的进度: 0--16*16, 1--32*32, 2--64*64 ... (暂时固定为0)*/
	char reservedData;	
	ANTS_MID_HANDLEEXCEPTION struMotionHandleType;/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* 布防时间 */
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM];/* 报警触发的录象通道*/
}ANTS_MID_MOTION, *LPANTS_MID_MOTION;

//!移动侦测(子结构)
typedef struct {
	BYTE byMotionScope[64][96];/* 侦测区域,0-16位,表示12行,共有16*12个小宏块,为1表示是移动侦测区域,0-表示不是 */
	BYTE byMotionSensitive;/* 移动侦测灵敏度, 0 - 5,越高越灵敏,oxff关闭 */
	BYTE byEnableHandleMotion;/* 是否处理移动侦测 0－否 1－是*/ 
	BYTE byPrecision;/* 移动侦测算法的进度: 0--16*16, 1--32*32, 2--64*64 ... (暂时固定为0)*/
	char reservedData;	
	ANTS_MID_HANDLEEXCEPTION struMotionHandleType;/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* 布防时间 */
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM_V2];/* 报警触发的录象通道*/
}ANTS_MID_MOTION_V2, *LPANTS_MID_MOTION_V2;

//!遮挡报警(子结构)  区域大小704*576
typedef struct {
	DWORD dwEnableHideAlarm;/* 是否启动遮挡报警 ,0-否,1-低灵敏度 2-中灵敏度 3-高灵敏度*/
	WORD wHideAlarmAreaTopLeftX;/* 遮挡区域的x坐标 */
	WORD wHideAlarmAreaTopLeftY;/* 遮挡区域的y坐标 */
	WORD wHideAlarmAreaWidth;/* 遮挡区域的宽 */
	WORD wHideAlarmAreaHeight;/* 遮挡区域的高*/
	ANTS_MID_HANDLEEXCEPTION strHideAlarmHandleType;/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//布防时间
}ANTS_MID_HIDEALARM,*LPANTS_MID_HIDEALARM;

//!遮挡区域(子结构)
typedef struct {
	WORD wHideAreaTopLeftX;/* 遮挡区域的x坐标 */
	WORD wHideAreaTopLeftY;/* 遮挡区域的y坐标 */
	WORD wHideAreaWidth;/* 遮挡区域的宽 */
	WORD wHideAreaHeight;/*遮挡区域的高*/
}ANTS_MID_SHELTER,*LPANTS_MID_SHELTER;

typedef struct {
	//遮挡区域大小704*576
	DWORD dwEnableHide;/* 是否启动遮挡 ,0-否,1-是*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];	
}ANTS_MID_SHELTERCFG,*LPANTS_MID_SHELTERCFG;

typedef struct {
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	//!显示通道名
	DWORD dwShowChanName;//预览的图象上是否显示通道名称,0-不显示,1-显示 区域大小704*576
	WORD wShowNameTopLeftX;/* 通道名称显示位置的x坐标 */
	WORD wShowNameTopLeftY;/* 通道名称显示位置的y坐标 */
	//!OSD
	DWORD dwShowOsd;// 预览的图象上是否显示OSD,0-不显示,1-显示 区域大小704*576
	WORD wOSDTopLeftX;/* OSD的x坐标 */
	WORD wOSDTopLeftY;/* OSD的y坐标 */
	BYTE byOSDType;/* OSD类型(主要是年月日格式) */
									/* 0: XXXX-XX-XX 年月日 */
									/* 1: XX-XX-XXXX 月日年 */
									/* 2: XXXX年XX月XX日 */
									/* 3: XX月XX日XXXX年 */
									/* 4: XX-XX-XXXX 日月年*/
									/* 5: XX日XX月XXXX年 */
	BYTE byDispWeek;/*是否显示星期 */
	BYTE byOSDAttrib;/*OSD属性:透明，闪烁 (保留)*/
	BYTE byHourOSDType;/*OSD小时制:0-24小时制,1-12小时制 */
}ANTS_MID_OSDCFG,*LPANTS_MID_OSDCFG;

//!通道图象结构
typedef struct{
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	DWORD dwVideoFormat;/*视频制式 1-NTSC 2-PAL*/
	ANTS_MID_COLOR struColor;//图像参数
	char reservedData [60];/*保留*/
	//!显示通道名
	DWORD dwShowChanName;// 预览的图象上是否显示通道名称,0-不显示,1-显示 区域大小704*576
	WORD wShowNameTopLeftX;/* 通道名称显示位置的x坐标 */
	WORD wShowNameTopLeftY;/* 通道名称显示位置的y坐标 */
	//!视频信号丢失报警
	ANTS_MID_VILOST struVILost;
	ANTS_MID_VILOST struRes;/*保留*/
	//!移动侦测
	ANTS_MID_MOTION	struMotion;
	//!遮挡报警
	ANTS_MID_HIDEALARM struHideAlarm;
	//!遮挡  区域大小704*576
	DWORD dwEnableHide;/* 是否启动遮挡 ,0-否,1-是*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];
	//!OSD
	DWORD dwShowOsd;//!预览的图象上是否显示OSD,0-不显示,1-显示 区域大小704*576
	WORD wOSDTopLeftX;/* OSD的x坐标 */
	WORD wOSDTopLeftY;/* OSD的y坐标 */
	BYTE byOSDType;		/* OSD类型(主要是年月日格式) */
										/* 0: XXXX-XX-XX 年月日 */
										/* 1: XX-XX-XXXX 月日年 */
										/* 2: XXXX年XX月XX日 */
										/* 3: XX月XX日XXXX年 */
										/* 4: XX-XX-XXXX 日月年*/
										/* 5: XX日XX月XXXX年 */
	BYTE byDispWeek;/* 是否显示星期 */
	BYTE byOSDAttrib;/* OSD属性:透明，闪烁 (保留)*/
  BYTE byHourOSDType;/* OSD小时制:0-24小时制,1-12小时制 */
	BYTE byRes[64];
}ANTS_MID_PICCFG, *LPANTS_MID_PICCFG;

//!通道图象结构
typedef struct{
	DWORD dwSize;
	BYTE sChanName[ANTS_MID_NAME_LEN];
	DWORD dwVideoFormat;/*视频制式 1-NTSC 2-PAL*/
	ANTS_MID_COLOR struColor;//图像参数
	char reservedData[60];/*保留*/
	//!显示通道名
	DWORD dwShowChanName;// 预览的图象上是否显示通道名称,0-不显示,1-显示 区域大小704*576
	WORD wShowNameTopLeftX;/* 通道名称显示位置的x坐标 */
	WORD wShowNameTopLeftY;/* 通道名称显示位置的y坐标 */

	//!视频信号丢失报警
	ANTS_MID_VILOST struVILost;
	ANTS_MID_VILOST struRes;/*保留*/

	//!移动侦测
	ANTS_MID_MOTION_V2 struMotion;

	//!遮挡报警
	ANTS_MID_HIDEALARM struHideAlarm;

	//!遮挡区域大小704*576
	DWORD dwEnableHide;/* 是否启动遮挡 ,0-否,1-是*/
	ANTS_MID_SHELTER struShelter[ANTS_MID_MAX_SHELTERNUM];
	//!OSD
	DWORD dwShowOsd;// 预览的图象上是否显示OSD,0-不显示,1-显示 区域大小704*576
	WORD wOSDTopLeftX;/* OSD的x坐标 */
	WORD wOSDTopLeftY;/* OSD的y坐标 */
	BYTE byOSDType;/* OSD类型(主要是年月日格式) */
								/* 0: XXXX-XX-XX 年月日 */
								/* 1: XX-XX-XXXX 月日年 */
								/* 2: XXXX年XX月XX日 */
								/* 3: XX月XX日XXXX年 */
								/* 4: XX-XX-XXXX 日月年*/
								/* 5: XX日XX月XXXX年 */
	BYTE byDispWeek;/* 是否显示星期 */
	BYTE byOSDAttrib;/* OSD属性:透明，闪烁 (保留)*/
  BYTE byHourOSDType;/* OSD小时制:0-24小时制,1-12小时制 */
	BYTE byRes[64];
}ANTS_MID_PICCFG_V2,*LPANTS_MID_PICCFG_V2;

//!码流压缩参数(子结构)
typedef struct {
	BYTE byStreamType;//码流类型 0-视频流, 1-复合流, 表示事件压缩参数时最高位表示是否启用压缩参数
	BYTE byResolution;//分辨率0-DCIF 1-CIF, 2-QCIF, 3-4CIF, 4-2CIF 5（保留）,
	                  //16-VGA（640*480）, 17-UXGA（1600*1200）, 18-SVGA （800*600）,
	                  //19-HD720p（1280*720）,20-XVGA,  21-HD900p, 27-HD1080i, 
	                  //28-2560*1920, 29-1600*304, 30-2048*1536, 31-2448*2048	
	BYTE byBitrateType;//码率类型 0:变码率, 1:定码率
	BYTE byPicQuality;//图象质量 0-最好 1-次好 2-较好 3-一般 4-较差 5-差
	DWORD dwVideoBitrate;//视频码率 0-保留 1-16K 2-32K 3-48k 4-64K 5-80K 6-96K 7-128K 8-160k 9-192K 10-224K 11-256K 12-320K
											//13-384K 14-448K 15-512K 16-640K 17-768K 18-896K 19-1024K 20-1280K 21-1536K 22-1792K 23-2048K
											//最高位(31位)置成1表示是自定义码流, 0-30位表示码流值。
	DWORD dwVideoFrameRate;//帧率 0-全部; 1-1/16; 2-1/8; 3-1/4; 4-1/2; 5-1; 6-2; 7-4; 8-6; 9-8; 10-10; 11-12; 12-16; 13-20; 14-15; 15-18; 16-22,17-25;
	WORD wIntervalFrameI;//I帧间隔
	BYTE byIntervalBPFrame;//0-BBP帧; 1-BP帧; 2-单P帧 (保留)
 	BYTE byres1;//保留
 	BYTE byVideoEncType;//视频编码类型 0 私有h264;1标准h264; 2标准mpeg4; 3-M-JPEG
 	BYTE byAudioEncType;//音频编码类型 0-OggVorbis;1-G711_U;2-G711_A
 	BYTE byres[10];//这里保留音频的压缩参数
}ANTS_MID_COMPRESSION_INFO, *LPANTS_MID_COMPRESSION_INFO;

//!通道压缩参数
typedef struct {
	DWORD dwSize;
	ANTS_MID_COMPRESSION_INFO	struNormHighRecordPara;//录像
	ANTS_MID_COMPRESSION_INFO	struRes;//保留 char reserveData[28];
  ANTS_MID_COMPRESSION_INFO	struEventRecordPara;//事件触发压缩参数
	ANTS_MID_COMPRESSION_INFO	struNetPara;//网传(子码流)
}ANTS_MID_COMPRESSIONCFG, *LPANTS_MID_COMPRESSIONCFG;

//!时间段录像参数配置(子结构)
typedef struct {
	ANTS_MID_SCHEDTIME struRecordTime;
	BYTE byRecordType;//0:定时录像，1:移动侦测，2:报警录像，3:动测|报警，4:动测&报警, 5:命令触发, 6:手动录像
	char reservedData[3];
}ANTS_MID_RECORDSCHED, *LPANTS_MID_RECORDSCHED;

//!全天录像参数配置(子结构)
typedef struct {
	WORD wAllDayRecord;/* 是否全天录像 0-否 1-是*/
	BYTE byRecordType;/* 录象类型 0:定时录像，1:移动侦测，2:报警录像，3:动测|报警，4:动测&报警 5:命令触发*/
	char reservedData;
}ANTS_MID_RECORDDAY, *LPANTS_MID_RECORDDAY;

//!通道录像参数配置
typedef struct {
	DWORD dwSize;
	DWORD dwRecord;/*是否录像 0-否 1-是*/
	ANTS_MID_RECORDDAY struRecAllDay[ANTS_MID_MAX_DAYS];
	ANTS_MID_RECORDSCHED struRecordSched[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];
	DWORD dwRecordTime;/* 录象延时长度 0-5秒， 1-20秒， 2-30秒， 3-1分钟， 4-2分钟， 5-5分钟， 6-10分钟*/
	DWORD dwPreRecordTime;/* 预录时间 0-不预录 1-5秒，2-10秒，3-15秒，4-20秒，5-25秒，6-30秒 7-0xffffffff(尽可能预录) */
	DWORD dwRecorderDuration;/* 录像保存的最长时间 */
	BYTE byRedundancyRec;/*是否冗余录像,重要数据双备份：0/1*/
	BYTE byAudioRec;/*录像时复合流编码时是否记录音频数据：国外有此法规*/
	BYTE byReserve[10];	
}ANTS_MID_RECORD,*LPANTS_MID_RECORD;

//!通道解码器(云台)参数配置
typedef struct {
	DWORD dwSize;
	DWORD dwBaudRate;//波特率(bps)，0－50，1－75，2－110，3－150，4－300，5－600，6－1200，7－2400，8－4800，9－9600，10－19200， 11－38400，12－57600，13－76800，14－115.2k;
	BYTE byDataBit;//数据有几位 0－5位，1－6位，2－7位，3－8位;
	BYTE byStopBit;//停止位 0－1位，1－2位;
	BYTE byParity;//校验 0－无校验，1－奇校验，2－偶校验;
	BYTE byFlowcontrol;//0－无，1－软流控,2-硬流控
	WORD wDecoderType;//解码器类型  NET_DVR_IPC_PROTO_LIST中得到
	WORD wDecoderAddress;/*解码器地址:0 - 255*/
	BYTE bySetPreset[ANTS_MID_MAX_PRESET];/* 预置点是否设置,0-没有设置,1-设置*/
	BYTE bySetCruise[ANTS_MID_MAX_CRUISE];/* 巡航是否设置: 0-没有设置,1-设置 */
	BYTE bySetTrack[ANTS_MID_MAX_TRACK];/* 轨迹是否设置,0-没有设置,1-设置*/
}ANTS_MID_DECODERCFG,*LPANTS_MID_DECODERCFG;

//!DVR设备参数
typedef struct{
	DWORD dwSize;
	BYTE sDVRName[ANTS_MID_NAME_LEN];//DVR名称
	DWORD dwDVRID;//DVR ID,用于遥控器
	DWORD dwRecycleRecord;//是否循环录像,0:不是; 1:是

	//!以下不可设置
	BYTE sSerialNumber[ANTS_MID_SERIALNO_LEN];//序列号
	DWORD dwSoftwareVersion;//软件版本号,高16位是主版本,低16位是次版本
	DWORD dwSoftwareBuildDate;//软件生成日期,0xYYYYMMDD
	DWORD dwDSPSoftwareVersion;//DSP软件版本,高16位是主版本,低16位是次版本
	DWORD dwDSPSoftwareBuildDate;// DSP软件生成日期,0xYYYYMMDD
	DWORD dwPanelVersion;//前面板版本,高16位是主版本,低16位是次版本
	DWORD dwHardwareVersion;//硬件版本,高16位是主版本,低16位是次版本
	BYTE byAlarmInPortNum;//DVR报警输入个数
	BYTE byAlarmOutPortNum;//DVR报警输出个数
	BYTE byRS232Num;//DVR 232串口个数
	BYTE byRS485Num;//DVR 485串口个数
	BYTE byNetworkPortNum;//网络口个数
	BYTE byDiskCtrlNum;//DVR 硬盘控制器个数
	BYTE byDiskNum;//DVR 硬盘个数
	BYTE byDVRType;//DVR类型, 1:DVR 2:Result 3:DVS ......
	BYTE byChanNum;//DVR 通道个数
	BYTE byStartChan;//起始通道号,例如DVS-1,DVR - 1
	BYTE byDecodeChans;//DVR 解码路数
	BYTE byVGANum;//VGA口的个数
	BYTE byUSBNum;//USB口的个数
	BYTE byAuxoutNum;//辅口的个数
	BYTE byAudioNum;//语音口的个数
	BYTE byIPChanNum;//最大数字通道数
}ANTS_MID_DEVICECFG,*LPANTS_MID_DEVICECFG;

//!IP地址
typedef struct{		
	char sIpV4[16];/* IPv4地址 */
	BYTE byIPv6[128];/* 保留 */
}ANTS_MID_IPADDR,*LPANTS_MID_IPADDR;

//!PPP参数配置(子结构)
typedef struct {
	ANTS_MID_IPADDR struRemoteIP;//远端IP地址
	ANTS_MID_IPADDR struLocalIP;//本地IP地址
	char sLocalIPMask[16];//本地IP地址掩码
	BYTE sUsername[ANTS_MID_NAME_LEN];/* 用户名 */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* 密码 */
	BYTE byPPPMode;//PPP模式, 0－主动，1－被动
	BYTE byRedial;//是否回拨 ：0-否,1-是
	BYTE byRedialMode;//回拨模式,0-由拨入者指定,1-预置回拨号码
	BYTE byDataEncrypt;//数据加密,0-否,1-是
	DWORD dwMTU;//MTU
	char sTelephoneNumber[ANTS_MID_PHONENUMBER_LEN];//电话号码
}ANTS_MID_PPPCFG,*LPANTS_MID_PPPCFG;

//!RS232串口参数配置
typedef struct{
    DWORD dwBaudRate;/*波特率(bps)，0－50，1－75，2－110，3－150，4－300，5－600，6－1200，7－2400，8－4800，9－9600，10－19200， 11－38400，12－57600，13－76800，14－115.2k;*/
    BYTE byDataBit;/* 数据有几位 0－5位，1－6位，2－7位，3－8位 */
    BYTE byStopBit;/* 停止位 0－1位，1－2位 */
    BYTE byParity;/* 校验 0－无校验，1－奇校验，2－偶校验 */
    BYTE byFlowcontrol;/* 0－无，1－软流控,2-硬流控 */
    DWORD	dwWorkMode;/* 工作模式，0－232串口用于PPP拨号，1－232串口用于参数控制，2－透明通道 */
}ANTS_MID_SINGLE_RS232;

//!RS232串口参数配置
typedef struct {
	DWORD dwSize;
  ANTS_MID_SINGLE_RS232 struRs232;
	BYTE byRes[84]; 
	ANTS_MID_PPPCFG struPPPConfig;
}ANTS_MID_RS232CFG,*LPANTS_MID_RS232CFG;

//!报警输入参数配置
typedef struct {
	DWORD dwSize;
	BYTE sAlarmInName[ANTS_MID_NAME_LEN];/* 名称 */
	BYTE byAlarmType;//报警器类型,0：常开,1：常闭
	BYTE byAlarmInHandle;/* 是否处理 0-不处理 1-处理*/
  BYTE byRes1[2];
	ANTS_MID_HANDLEEXCEPTION struAlarmHandleType;/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//布防时间
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM];//报警触发的录象通道,为1表示触发该通道
	BYTE byEnablePreset[ANTS_MID_MAX_CHANNUM];/* 是否调用预置点 0-否,1-是*/
	BYTE byPresetNo[ANTS_MID_MAX_CHANNUM];/* 调用的云台预置点序号,一个报警输入可以调用多个通道的云台预置点, 0xff表示不调用预置点。*/
	BYTE byRes2[192];/* 保留 */
	BYTE byEnableCruise[ANTS_MID_MAX_CHANNUM];/* 是否调用巡航 0-否,1-是*/
	BYTE byCruiseNo[ANTS_MID_MAX_CHANNUM];/* 巡航 */
	BYTE byEnablePtzTrack[ANTS_MID_MAX_CHANNUM];/* 是否调用轨迹 0-否,1-是*/
	BYTE byPTZTrack[ANTS_MID_MAX_CHANNUM];/* 调用的云台的轨迹序号 */
  BYTE byRes3[16];
}ANTS_MID_ALARMINCFG,*LPANTS_MID_ALARMINCFG;

//!报警输入参数配置
typedef struct {
	DWORD dwSize;
	BYTE sAlarmInName[ANTS_MID_NAME_LEN];/* 名称 */
	BYTE byAlarmType;//报警器类型,0：常开,1：常闭
	BYTE byAlarmInHandle;/* 是否处理 0-不处理 1-处理*/
  BYTE byRes1[2];
	ANTS_MID_HANDLEEXCEPTION struAlarmHandleType;/* 处理方式 */
	ANTS_MID_SCHEDTIME struAlarmTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];//布防时间
	BYTE byRelRecordChan[ANTS_MID_MAX_CHANNUM_V2];//报警触发的录象通道,为1表示触发该通道
	BYTE byEnablePreset[ANTS_MID_MAX_CHANNUM_V2];/* 是否调用预置点 0-否,1-是*/
	BYTE byPresetNo[ANTS_MID_MAX_CHANNUM_V2];/* 调用的云台预置点序号,一个报警输入可以调用多个通道的云台预置点, 0xff表示不调用预置点。*/
	BYTE byRes2[192];/* 保留 */
	BYTE byEnableCruise[ANTS_MID_MAX_CHANNUM_V2];/* 是否调用巡航 0-否,1-是*/
	BYTE byCruiseNo[ANTS_MID_MAX_CHANNUM_V2];/* 巡航 */
	BYTE byEnablePtzTrack[ANTS_MID_MAX_CHANNUM_V2];/* 是否调用轨迹 0-否,1-是*/
	BYTE byPTZTrack[ANTS_MID_MAX_CHANNUM_V2];/* 调用的云台的轨迹序号 */
  BYTE byRes3[16];
}ANTS_MID_ALARMINCFG_V2,*LPANTS_MID_ALARMINCFG_V2;

//!DVR报警输出
typedef struct {
	DWORD dwSize;
	BYTE sAlarmOutName[ANTS_MID_NAME_LEN];/* 名称 */
	DWORD dwAlarmOutDelay;/* 输出保持时间(-1为无限，手动关闭) */
												//0-5秒,1-10秒,2-30秒,3-1分钟,4-2分钟,5-5分钟,6-10分钟,7-手动
	ANTS_MID_SCHEDTIME struAlarmOutTime[ANTS_MID_MAX_DAYS][ANTS_MID_MAX_TIMESEGMENT];/* 报警输出激活时间段 */
  BYTE byRes[16];
}ANTS_MID_ALARMOUTCFG,*LPANTS_MID_ALARMOUTCFG;

//!单用户参数(子结构)
typedef struct{
	BYTE sUserName[ANTS_MID_NAME_LEN];/* 用户名 */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* 密码 */
	BYTE byLocalRight[ANTS_MID_MAX_RIGHT];/* 本地权限 */
														/*数组0: 本地控制云台*/
														/*数组1: 本地手动录象*/
														/*数组2: 本地回放*/
														/*数组3: 本地设置参数*/
														/*数组4: 本地查看状态、日志*/
														/*数组5: 本地高级操作(升级，格式化，重启，关机)*/
													  /*数组6: 本地查看参数 */
													  /*数组7: 本地管理模拟和IP camera */
													  /*数组8: 本地备份 */
													  /*数组9: 本地关机/重启 */
	BYTE byRemoteRight[ANTS_MID_MAX_RIGHT];/* 远程权限 */	
														/*数组0: 远程控制云台*/
														/*数组1: 远程手动录象*/
														/*数组2: 远程回放 */
														/*数组3: 远程设置参数*/
														/*数组4: 远程查看状态、日志*/
														/*数组5: 远程高级操作(升级，格式化，重启，关机)*/
														/*数组6: 远程发起语音对讲*/
														/*数组7: 远程预览*/
														/*数组8: 远程请求报警上传、报警输出*/
														/*数组9: 远程控制，本地输出*/
														/*数组10: 远程控制串口*/	
												    /*数组11: 远程查看参数 */
												    /*数组12: 远程管理模拟和IP camera */
												    /*数组13: 远程关机/重启 */
	BYTE byLocalPreviewRight[ANTS_MID_MAX_CHANNUM];/* 本地可以预览的通道 1-有权限，0-无权限*/
	BYTE byNetPreviewRight[ANTS_MID_MAX_CHANNUM];/* 远程可以预览的通道 1-有权限，0-无权限*/
	BYTE byLocalPlaybackRight[ANTS_MID_MAX_CHANNUM];/* 本地可以回放的通道 1-有权限，0-无权限*/
	BYTE byNetPlaybackRight[ANTS_MID_MAX_CHANNUM];/* 远程可以回放的通道 1-有权限，0-无权限*/
	BYTE byLocalRecordRight[ANTS_MID_MAX_CHANNUM];/* 本地可以录像的通道 1-有权限，0-无权限*/
	BYTE byNetRecordRight[ANTS_MID_MAX_CHANNUM];/* 远程可以录像的通道 1-有权限，0-无权限*/
	BYTE byLocalPTZRight[ANTS_MID_MAX_CHANNUM];/* 本地可以PTZ的通道 1-有权限，0-无权限*/
	BYTE byNetPTZRight[ANTS_MID_MAX_CHANNUM];/* 远程可以PTZ的通道 1-有权限，0-无权限*/
	BYTE byLocalBackupRight[ANTS_MID_MAX_CHANNUM];/* 本地备份权限通道 1-有权限，0-无权限*/
	ANTS_MID_IPADDR struUserIP;/* 用户IP地址(为0时表示允许任何地址) */
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];/* 物理地址 */
	BYTE byPriority;/* 优先级，0xff-无，0--低，1--中，2--高 */
                  /*
                  无……表示不支持优先级的设置
                  低……默认权限:包括本地和远程回放,本地和远程查看日志和状态,本地和远程关机/重启
                  中……包括本地和远程控制云台,本地和远程手动录像,本地和远程回放,语音对讲和远程预览
                        本地备份,本地/远程关机/重启
                  高……管理员
                  */
	BYTE byRes[1];	
}ANTS_MID_USER_INFO,*LPANTS_MID_USER_INFO;

//单用户参数(子结构)
typedef struct{
	BYTE sUserName[ANTS_MID_NAME_LEN];/* 用户名 */
	BYTE sPassword[ANTS_MID_PASSWD_LEN];/* 密码 */
	BYTE byLocalRight[ANTS_MID_MAX_RIGHT];/* 本地权限 */
															/*数组0: 本地控制云台*/
															/*数组1: 本地手动录象*/
															/*数组2: 本地回放*/
															/*数组3: 本地设置参数*/
															/*数组4: 本地查看状态、日志*/
															/*数组5: 本地高级操作(升级，格式化，重启，关机)*/
													    /*数组6: 本地查看参数 */
													    /*数组7: 本地管理模拟和IP camera */
													    /*数组8: 本地备份 */
													    /*数组9: 本地关机/重启 */
	BYTE byRemoteRight[ANTS_MID_MAX_RIGHT];/* 远程权限 */	
															/*数组0: 远程控制云台*/
															/*数组1: 远程手动录象*/
															/*数组2: 远程回放 */
															/*数组3: 远程设置参数*/
															/*数组4: 远程查看状态、日志*/
															/*数组5: 远程高级操作(升级，格式化，重启，关机)*/
															/*数组6: 远程发起语音对讲*/
															/*数组7: 远程预览*/
															/*数组8: 远程请求报警上传、报警输出*/
															/*数组9: 远程控制，本地输出*/
															/*数组10: 远程控制串口*/	
													    /*数组11: 远程查看参数 */
													    /*数组12: 远程管理模拟和IP camera */
													    /*数组13: 远程关机/重启 */
	BYTE byLocalPreviewRight[ANTS_MID_MAX_CHANNUM_V2];/* 本地可以预览的通道 1-有权限，0-无权限*/
	BYTE byNetPreviewRight[ANTS_MID_MAX_CHANNUM_V2];/* 远程可以预览的通道 1-有权限，0-无权限*/
	BYTE byLocalPlaybackRight[ANTS_MID_MAX_CHANNUM_V2];/* 本地可以回放的通道 1-有权限，0-无权限*/
	BYTE byNetPlaybackRight[ANTS_MID_MAX_CHANNUM_V2];/* 远程可以回放的通道 1-有权限，0-无权限*/
	BYTE byLocalRecordRight[ANTS_MID_MAX_CHANNUM_V2];/* 本地可以录像的通道 1-有权限，0-无权限*/
	BYTE byNetRecordRight[ANTS_MID_MAX_CHANNUM_V2];/* 远程可以录像的通道 1-有权限，0-无权限*/
	BYTE byLocalPTZRight[ANTS_MID_MAX_CHANNUM_V2];/* 本地可以PTZ的通道 1-有权限，0-无权限*/
	BYTE byNetPTZRight[ANTS_MID_MAX_CHANNUM_V2];/* 远程可以PTZ的通道 1-有权限，0-无权限*/
	BYTE byLocalBackupRight[ANTS_MID_MAX_CHANNUM_V2];/* 本地备份权限通道 1-有权限，0-无权限*/
	ANTS_MID_IPADDR struUserIP;/* 用户IP地址(为0时表示允许任何地址) */
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];/* 物理地址 */
	BYTE byPriority;/* 优先级，0xff-无，0--低，1--中，2--高 */
                  /*
                  无……表示不支持优先级的设置
                  低……默认权限:包括本地和远程回放,本地和远程查看日志和状态,本地和远程关机/重启
                  中……包括本地和远程控制云台,本地和远程手动录像,本地和远程回放,语音对讲和远程预览
                        本地备份,本地/远程关机/重启
                  高……管理员
                  */
	BYTE byRes[1];	
}ANTS_MID_USER_INFO_V2,*LPANTS_MID_USER_INFO_V2;

//!DVR用户参数
typedef struct{
	DWORD dwSize;
	ANTS_MID_USER_INFO struUser[ANTS_MID_MAX_USERNUM];
}ANTS_MID_USER,*LPANTS_MID_USER;

//!DVR用户参数
typedef struct{
	DWORD dwSize;
	ANTS_MID_USER_INFO_V2 struUser[ANTS_MID_MAX_USERNUM];
}ANTS_MID_USER_V2,*LPANTS_MID_USER_V2;

typedef struct{
	char sUserName[ANTS_MID_NAME_LEN];
	DWORD LoginTime;
	BOOL bLocal;
	ANTS_MID_IPADDR LoginIP;
}ANTS_MID_LOGINUSERINFO,*LPANTS_MID_LOGINUSERINFO;

//!DVR异常参数
typedef struct {
	DWORD dwSize;
	ANTS_MID_HANDLEEXCEPTION struExceptionHandleType[ANTS_MID_MAX_EXCEPTIONNUM];
	/*数组0-盘满,1- 硬盘出错,2-网线断,3-局域网内IP 地址冲突, 4-非法访问, 5-输入/输出视频制式不匹配, 6-视频信号异常, 7-录像异常*/
}ANTS_MID_EXCEPTION,*LPANTS_MID_EXCEPTION;

//!时间点(子结构)
typedef struct {
	DWORD dwMonth;//月0-11表示1-12个月
	DWORD dwWeekNo;//第几周0－第1周 1－第2周 2－第3周 3－第4周 4－最后一周
	DWORD dwWeekDate;//星期几0－星期日 1－星期一 2－星期二 3－星期三 4－星期四 5－星期五 6－星期六
	DWORD dwHour;//小时	开始时间0－23 结束时间1－23
	DWORD dwMin;//分0－59
}ANTS_MID_TIMEPOINT,*LPANTS_MID_TIMEPOINT;

//!夏令时参数
typedef struct {
	DWORD dwSize;
	BYTE byRes1[16];//保留
	DWORD dwEnableDST;//是否启用夏时制 0－不启用 1－启用
	BYTE byDSTBias;//夏令时偏移值，30min, 60min, 90min, 120min, 以分钟计，传递原始数值
	BYTE byRes2[3];
	ANTS_MID_TIMEPOINT struBeginPoint;//夏时制开始时间
	ANTS_MID_TIMEPOINT struEndPoint;//夏时制停止时间
}ANTS_MID_ZONEANDDST,*LPANTS_MID_ZONEANDDST;

//!单字符参数(子结构)
typedef struct {
	WORD wShowString;// 预览的图象上是否显示字符,0-不显示,1-显示 区域大小704*576,单个字符的大小为32*32
	WORD wStringSize;/* 该行字符的长度，不能大于44个字符 */
	WORD wShowStringTopLeftX;/* 字符显示位置的x坐标 */
	WORD wShowStringTopLeftY;/* 字符名称显示位置的y坐标 */
	char sString[44];/* 要显示的字符内容 */
}ANTS_MID_SHOWSTRINGINFO,*LPANTS_MID_SHOWSTRINGINFO;

//!叠加字符
typedef struct {
	DWORD dwSize;
	ANTS_MID_SHOWSTRINGINFO struStringInfo[ANTS_MID_MAX_STRINGNUM];/* 要显示的字符内容 */
}ANTS_MID_SHOWSTRING,*LPANTS_MID_SHOWSTRING;

//!本地硬盘信息配置(子结构)
typedef struct{
    DWORD dwHDNo;/*硬盘号, 取值0~ANTS_MID_MAX_DISKNUM-1*/
    DWORD dwCapacity;/*硬盘容量(不可设置)*/
    DWORD dwFreeSpace;/*硬盘剩余空间(不可设置)*/
    DWORD dwHdStatus;/*硬盘状态(不可设置) HD_STAT*/
    BYTE byHDAttr;/*0-默认, 1-冗余; 2-只读*/
		BYTE byHDType;/*0-本地硬盘,1-ESATA硬盘,2-NAS硬盘,3-iSCSI硬盘 4-Array虚拟磁盘*/
		BYTE byRes1[2];
    DWORD dwHdGroup;/*属于哪个盘组 1-ANTS_MID_MAX_HD_GROUP*/
    BYTE byRes2[120];
}ANTS_MID_SINGLE_HD,*LPANTS_MID_SINGLE_HD;

typedef struct{
    DWORD dwSize;
    DWORD dwHDCount;/*硬盘数(不可设置)*/
    ANTS_MID_SINGLE_HD struHDInfo[ANTS_MID_MAX_DISKNUM];//硬盘相关操作都需要重启才能生效；
}ANTS_MID_HDCFG,*LPANTS_MID_HDCFG;

//!本地盘组信息配置
typedef struct{
    DWORD dwHDGroupNo;/*盘组号(不可设置) 1-ANTS_MID_MAX_HD_GROUP*/        
    BYTE byHDGroupChans[ANTS_MID_MAX_CHANNUM];/*盘组对应的录像通道, 0-表示该通道不录象到该盘组，1-表示录象到该盘组*/
    BYTE byRes[8];
}ANTS_MID_SINGLE_HDGROUP,*LPANTS_MID_SINGLE_HDGROUP;

//!本地盘组信息配置
typedef struct{
    DWORD dwHDGroupNo;/*盘组号(不可设置) 1-ANTS_MID_MAX_HD_GROUP*/        
    BYTE byHDGroupChans[ANTS_MID_MAX_CHANNUM_V2];/*盘组对应的录像通道, 0-表示该通道不录象到该盘组，1-表示录象到该盘组*/
    BYTE byRes[8];
}ANTS_MID_SINGLE_HDGROUP_V2,*LPANTS_MID_SINGLE_HDGROUP_V2;

typedef struct{
    DWORD dwSize;
    DWORD dwHDGroupCount;/*盘组总数(不可设置)*/
    ANTS_MID_SINGLE_HDGROUP struHDGroupAttr[ANTS_MID_MAX_HD_GROUP];//硬盘相关操作都需要重启才能生效；
}ANTS_MID_HDGROUP_CFG,*LPANTS_MID_HDGROUP_CFG;

typedef struct{
    DWORD dwSize;
    DWORD dwHDGroupCount;/*盘组总数(不可设置)*/
    ANTS_MID_SINGLE_HDGROUP_V2 struHDGroupAttr[ANTS_MID_MAX_HD_GROUP];//硬盘相关操作都需要重启才能生效；
}ANTS_MID_HDGROUP_CFG_V2,*LPANTS_MID_HDGROUP_CFG_V2;

//!语音对讲参数
typedef struct{
	BYTE byAudioEncType;//音频编码类型 0-OggVorbis;1-G711_U;2-G711_A;3-G726(默认)
	BYTE byres[7];//这里保留音频的压缩参数 
}ANTS_MID_COMPRESSION_AUDIO,*LPANTS_MID_COMPRESSION_AUDIO;

//!自动维护参数
typedef struct{
	BYTE byAutoRebootMode;//自动维护模式:0--不维护，1--每天定时维护，2--每周定时维护，3--单次维护
	DWORD dwSingleTime;//单次维护时间:time_t类型
	DWORD dwEveryDayTime;//每天维护时间:0-7位是分钟，8-15位是小时
	BOOL bWeeklyDay[ANTS_MID_MAX_DAYS];//每周7天是否启动维护:0--星期天，1--星期一，依次往后
	DWORD dwWeeklyTime[ANTS_MID_MAX_DAYS];//每周维护时间:0-7位是分钟，8-15位是小时
}ANTS_MID_AUTOREBOOT,*LPANTS_MID_AUTOREBOOT;

//!PPPOE结构
typedef struct {
	DWORD dwPPPOE;//0-不启用,1-启用
	BYTE sPPPoEUser[ANTS_MID_NAME_LEN];//PPPoE用户名
	char sPPPoEPassword[ANTS_MID_PASSWD_LEN];//PPPoE密码
	ANTS_MID_IPADDR	struPPPoEIP;//PPPoE IP地址
}ANTS_MID_PPPOECFG,*LPANTS_MID_PPPOECFG;

/*网络数据结构(子结构)*/
typedef struct {
	ANTS_MID_IPADDR struDVRIP;//DVR IP地址
	ANTS_MID_IPADDR struDVRIPMask;//DVR IP地址掩码
	DWORD dwNetInterface;//网络接口1-10MBase-T 2-10MBase-T全双工 3-100MBase-TX 4-100M全双工 5-10M/100M自适应 6-100M/1000M自适应
	WORD wDVRPort;//端口号
	WORD wMTU;//增加MTU设置,默认1500
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];//物理地址
	BYTE byRes[2];//保留对齐
}ANTS_MID_ETHERNET,*LPANTS_MID_ETHERNET;

//!网络配置结构
typedef struct{
	DWORD dwSize;
	ANTS_MID_ETHERNET struEtherNet[ANTS_MID_MAX_ETHERNET];//以太网口
	ANTS_MID_IPADDR struRes1[2];/*保留*/
	ANTS_MID_IPADDR struAlarmHostIpAddr;/* 报警主机IP地址 */
	WORD wRes2[2];/* 保留 */
	WORD wAlarmHostIpPort;/* 报警主机端口号 */
	BYTE byUseDhcp;/* 是否启用DHCP 0xff-无效 0-不启用 1-启用*/
	BYTE byRes3;
	ANTS_MID_IPADDR struDnsServer1IpAddr;/* 域名服务器1的IP地址 */
	ANTS_MID_IPADDR	struDnsServer2IpAddr;/* 域名服务器2的IP地址 */
	BYTE byIpResolver[ANTS_MID_MAX_DOMAIN_NAME];	/* IP解析服务器域名或IP地址 */
	WORD wIpResolverPort;/* IP解析服务器端口号 */
	WORD wHttpPortNo;/* HTTP端口号 */
	ANTS_MID_IPADDR struMulticastIpAddr;/* 多播组地址 */
	ANTS_MID_IPADDR struGatewayIpAddr;/* 网关地址 */
	ANTS_MID_PPPOECFG struPPPoE;	
	char szManagerHostIpV4[32];/*主动注册服务器IP地址0-不启用1-启用*/
	WORD wManagerHostPort;/*主动注册服务器端口*/
	BYTE byUseManagerHost;/*是否启用主动注册服务0-不启用1-启用*/
	BYTE byRes[29];
}ANTS_MID_NETCFG,*LPANTS_MID_NETCFG;

//!NTP
typedef struct {
	BYTE sNTPServer[64];/* Domain Name or IP addr of NTP server */
	WORD wInterval;/* adjust time interval(hours) */
	BYTE byEnableNTP;/* enable NPT client 0-no，1-yes*/
	signed char cTimeDifferenceH;/* 与国际标准时间的 小时偏移-12 ... +13 */
	signed char cTimeDifferenceM;/* 与国际标准时间的 分钟偏移0, 30, 45*/
	BYTE res1;
	WORD wNtpPort;/* ntp server port 设备默认为123*/
	BYTE res2[8];
}ANTS_MID_NTPPARA,*LPANTS_MID_NTPPARA;

//!DDNS
typedef struct {
	BYTE byEnableDDNS;
	BYTE byHostIndex;/* 0-私有DDNS 1－Dyndns 2－PeanutHull(花生壳) 3- NO-IP 4-qdns*/
	BYTE byRes1[2];
  struct{    
		BYTE sUsername[ANTS_MID_NAME_LEN];/* DDNS账号用户名*/
		BYTE sPassword[ANTS_MID_PASSWD_LEN];/* 密码 */
		BYTE sDomainName[ANTS_MID_MAX_DOMAIN_NAME];/* 设备配备的域名地址 */
		BYTE sServerName[ANTS_MID_MAX_DOMAIN_NAME];/* DDNS协议对应的服务器地址，可以是IP地址或域名 */
		WORD wDDNSPort;/* 端口号 */
		BYTE byRes[10];
  }struDDNS[ANTS_MID_MAX_DDNS_NUMS];
	BYTE byRes2[16];
}ANTS_MID_DDNSPARA,*LPANTS_MID_DDNSPARA;

//!网络参数配置
typedef struct {
	DWORD dwSize;
	char sDNSIp[16];/* DNS服务器地址 */
	ANTS_MID_NTPPARA struNtpClientParam;/* NTP参数 */
	ANTS_MID_DDNSPARA struDDNSClientParam;/* DDNS参数 */
	BYTE res[464];/* 保留 */
}ANTS_MID_NETAPPCFG,*LPANTS_MID_NETAPPCFG;

/*EMAIL参数结构*/
typedef struct{		
	DWORD dwSize;
	BYTE sAccount[ANTS_MID_NAME_LEN];/* 账号*/ 
	BYTE sPassword[ANTS_MID_MAX_EMAIL_PWD_LEN];/*密码 */
	struct{
		BYTE sName[ANTS_MID_NAME_LEN];/* 发件人姓名 */
		BYTE sAddress[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* 发件人地址 */
	}struSender;

	BYTE sSmtpServer[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* smtp服务器 */
	BYTE sPop3Server[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* pop3服务器 */

	struct{
		BYTE sName[ANTS_MID_NAME_LEN];/* 收件人姓名 */
		BYTE sAddress[ANTS_MID_MAX_EMAIL_ADDR_LEN];/* 收件人地址 */
	}struReceiver[3];/* 最多可以设置3个收件人 */

	BYTE byAttachment;/* 是否带附件 */
	BYTE bySmtpServerVerify;/* 发送服务器要求身份验证 */
  BYTE byMailInterval;/* mail interval */
	BYTE byEnableSSL;//ssl是否启用
	WORD wSmtpPort;//gmail的465，普通的为25     
	BYTE byRes[74];//保留
}ANTS_MID_EMAILCFG,*LPANTS_MID_EMAILCFG;

typedef struct{
	DWORD dwSize;//!结构长度
	BYTE byEnable;//!0-禁用SNMP，1-表示启用SNMP
	BYTE byRes1[3];//!保留
	WORD wVersion;//!snmp 版本  v1 = 1, v2 =2, v3 =3，设备目前不支持 v3
	WORD wServerPort;//!snmp消息接收端口，默认 161
	BYTE byReadCommunity[ANTS_MID_NAME_LEN];//!读共同体，最多31,默认"public"
	BYTE byWriteCommunity[ANTS_MID_NAME_LEN];//!写共同体,最多31 字节,默认 "private"
	BYTE byTrapHostIP[ANTS_MID_DESC_LEN_64];//!自陷主机ip地址描述，支持IPV4 IPV6和域名描述    
	WORD wTrapHostPort;//!trap主机端口
	BYTE byRes2[102];//!保留
}ANTS_MID_SNMPCFG,*LPANTS_MID_SNMPCFG;

typedef struct{
	ANTS_MID_IPADDR struDVRIP;//!DVR IP地址
	ANTS_MID_IPADDR struDVRIPMask;//!DVR IP地址掩码
	DWORD	dwNetInterface;//!网络接口1-10MBase-T 2-10MBase-T全双工 3-100MBase-TX 4-100M全双工 5-10M/100M自适应
	BYTE byRes1[2];
	WORD wMTU;//!增加MTU设置，默认1500。
	BYTE byMACAddr[ANTS_MID_MACADDR_LEN];//!物理地址，只用于显示
	BYTE byRes2[2];//!保留
	BYTE byUseDhcp;//!是否启用DHCP 
	BYTE byRes3[3];
	ANTS_MID_IPADDR struGatewayIpAddr;//!网关地址 
	ANTS_MID_IPADDR struDnsServer1IpAddr;//!域名服务器1的IP地址 
	ANTS_MID_IPADDR struDnsServer2IpAddr;//!域名服务器2的IP地址 
}ANTS_MID_ETHERNET_MULTI,*LPANTS_MID_ETHERNET_MULTI;

typedef struct{
	DWORD dwSize;
	BYTE byDefaultRoute;//!默认路由，0表示struEtherNet[0]，1表示struEtherNet[1]
	BYTE byNetworkCardNum;//!设备实际可配置的网卡数目
	BYTE byRes[2];//!保留
	ANTS_MID_ETHERNET_MULTI struEtherNet[ANTS_MID_MAX_NETWORK_CARD];	//!以太网口
	ANTS_MID_IPADDR struManageHost1IpAddr;//!主管理主机IP地址 
	ANTS_MID_IPADDR struManageHost2IpAddr;//!辅管理主机IP地址 
	ANTS_MID_IPADDR struAlarmHostIpAddr;//!报警主机IP地址 
	WORD wManageHost1Port;//!主管理主机端口号 
	WORD wManageHost2Port;//!辅管理主机端口号 
	WORD wAlarmHostIpPort;//!报警主机端口号 
	BYTE byRes4[2];
	BYTE byIpResolver[ANTS_MID_MAX_DOMAIN_NAME];	//!IP解析服务器域名或IP地址 
	WORD wIpResolverPort;				//!IP解析服务器端口号 
	WORD wDvrPort;						//!通讯端口 默认8000 
	WORD wHttpPortNo;					//!HTTP端口号 
	BYTE byRes2[6];
	ANTS_MID_IPADDR struMulticastIpAddr;//!多播组地址
	ANTS_MID_PPPOECFG struPPPoE;
	BYTE byRes3[24];
}ANTS_MID_NETCFG_MULTI,*LPANTS_MID_NETCFG_MULTI;

typedef struct{
	DWORD dwSize;//!长度
	WORD wPort;//!RTSPrtsp服务器侦听端口
	BYTE byReserve[54];//!预留
}ANTS_MID_RTSPCFG,*LPANTS_MID_RTSPCFG;

typedef struct{
	char sNfsHostIPAddr[16];
	BYTE sNfsDirectory[ANTS_MID_PATHNAME_LEN];	// ANTS_PATHNAME_LEN = 128
}ANTS_MID_SINGLE_NFS, *LPANTS_MID_SINGLE_NFS;

typedef struct{
	DWORD dwSize;
	ANTS_MID_SINGLE_NFS struNfsDiskParam[ANTS_MID_MAX_NFS_DISK];
}ANTS_MID_NFSCFG,*LPANTS_MID_NFSCFG;

typedef struct{
	BYTE byNetDiskType;//!网络硬盘类型, 0-NFS,1-iSCSI
	BYTE byRes1[3];//!保留
	ANTS_MID_IPADDR struNetDiskAddr;//!网络硬盘地址
	BYTE sDirectory[ANTS_MID_PATHNAME_LEN];//!ANTS_PATHNAME_LEN = 128
	WORD wPort;//!iscsi有端口，现在为默认
	BYTE byRes2[66];//!保留
}ANTS_MID_SINGLE_NET_DISK_INFO,*LPANTS_MID_SINGLE_NET_DISK_INFO;

typedef struct{
	DWORD dwSize;
	ANTS_MID_SINGLE_NET_DISK_INFO struNetDiskParam[ANTS_MID_MAX_NET_DISK];
}ANTS_MID_NET_DISKCFG, *LPANTS_MID_NET_DISKCFG;

typedef struct{
	char sFileName[100];//!图片名
	ANTS_MID_TIME struTime;//!图片的时间
	DWORD dwFileSize;//!图片的大小
	char sCardNum[32];//!卡号
}ANTS_MID_FIND_PICTURE,*LPANTS_MID_FIND_PICTURE;

typedef struct{
	ANTS_MID_TIME strLogTime;
	DWORD dwMajorType;//!主类型
	DWORD dwMinorType;//!次类型
	BYTE sPanelUser[ANTS_MID_MAX_NAMELEN];//!操作面板的用户名
	BYTE sNetUser[ANTS_MID_MAX_NAMELEN];//!网络操作的用户名
	ANTS_MID_IPADDR struRemoteHostAddr;//!远程主机地址
	DWORD dwParaType;//!参数类型,9000设备MINOR_START_VT/MINOR_STOP_VT时，表示语音对讲的端子号
	DWORD dwChannel;//!通道号
	DWORD dwDiskNumber;//!硬盘号
	DWORD dwAlarmInPort;//!报警输入端口
	DWORD dwAlarmOutPort;//!报警输出端口
	DWORD dwInfoLen;
	char sInfo[ANTS_MID_LOG_INFO_LEN];
}ANTS_MID_LOG,*LPANTS_MID_LOG;

typedef struct{
	ANTS_MID_TIME strLogTime;
	DWORD dwMajorType;//!主类型
	DWORD dwMinorType;//!次类型
	BYTE sPanelUser[ANTS_MID_MAX_NAMELEN];//!操作面板的用户名
	BYTE sNetUser[ANTS_MID_MAX_NAMELEN];//!网络操作的用户名
	ANTS_MID_IPADDR	struRemoteHostAddr;//!远程主机地址
	DWORD wParaType;//!参数类型,9000设备MINOR_START_VT/MINOR_STOP_VT时，表示语音对讲的端子号
	DWORD dwChannel;//!通道号
	DWORD dwDiskNumber;//!硬盘号
	DWORD dwAlarmInPort;//!报警输入端口
	DWORD dwAlarmOutPort;//!报警输出端口
	DWORD dwInfoLen;
	char sInfo[4];
}ANTS_MID_LOG_V2,*LPANTS_MID_LOG_V2;

typedef struct{
	DWORD dwVolume;
	DWORD dwFreeSpace;
	DWORD dwHardDiskStatic;
}ANTS_MID_DISKSTATE,*LPANTS_MID_DISKSTATE;

typedef struct{
	BYTE byRecordStatic;//!通道是否在录像,0-不录像,1-录像
	BYTE bySignalStatic;//!连接的信号状态,0-正常,1-信号丢失
	BYTE byHardwareStatic;//!通道硬件状态,0-正常,1-异常,例如DSP死掉
	BYTE byRes1;
	DWORD dwBitRate;//!实际码率
	DWORD dwLinkNum;//!客户端连接的个数
	ANTS_MID_IPADDR struClientIP[ANTS_MID_MAX_LINK];//!客户端的IP地址
	DWORD dwIPLinkNum;//!如果该通道为IP接入，那么表示IP接入当前的连接数
	BYTE byRes[12];
}ANTS_MID_CHANNELSTATE,*LPANTS_MID_CHANNELSTATE;

typedef struct{
	DWORD dwDeviceStatic;//!设备的状态,0-正常,1-CPU占用率太高,超过85%,2-硬件错误,例如串口死掉
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM];	//!通道的状态
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN];//!报警端口的状态,0-没有报警,1-有报警
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!报警输出端口的状态,0-没有输出,1-有报警输出
	DWORD dwLocalDisplay;//!本地显示状态,0-正常,1-不正常
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!表示语音通道的状态 0-未使用，1-使用中, 0xff无效
	BYTE byRes[10];
}ANTS_MID_WORKSTATE,*LPANTS_MID_WORKSTATE;

typedef struct{
	DWORD dwDeviceStatic;//!设备的状态,0-正常,1-CPU占用率太高,超过85%,2-硬件错误,例如串口死掉
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM_V2];	//!通道的状态
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN_V2];//!报警端口的状态,0-没有报警,1-有报警
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!报警输出端口的状态,0-没有输出,1-有报警输出
	DWORD dwLocalDisplay;//!本地显示状态,0-正常,1-不正常
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!表示语音通道的状态 0-未使用，1-使用中, 0xff无效
	BYTE byRes[10];
}ANTS_MID_WORKSTATE_V2,*LPANTS_MID_WORKSTATE_V2;

typedef struct{
	DWORD dwDeviceStatic;//!设备的状态,0-正常,1-CPU占用率太高,超过85%,2-硬件错误,例如串口死掉
	ANTS_MID_DISKSTATE struHardDiskStatic[ANTS_MID_MAX_DISKNUM_V2];
	ANTS_MID_CHANNELSTATE struChanStatic[ANTS_MID_MAX_CHANNUM_V2];	//!通道的状态
	BYTE byAlarmInStatic[ANTS_MID_MAX_ALARMIN_V2];//!报警端口的状态,0-没有报警,1-有报警
	BYTE byAlarmOutStatic[ANTS_MID_MAX_ALARMOUT];//!报警输出端口的状态,0-没有输出,1-有报警输出
	DWORD dwLocalDisplay;//!本地显示状态,0-正常,1-不正常
	BYTE byAudioChanStatus[ANTS_MID_MAX_AUDIO];//!表示语音通道的状态 0-未使用，1-使用中, 0xff无效
	BYTE byRes[10];
}ANTS_MID_WORKSTATE_V3,*LPANTS_MID_WORKSTATE_V3;

typedef struct{
	LONG lChannel;
	DWORD dwFileType;
	DWORD dwIsLocked;
	DWORD dwUseCardNo;
	BYTE sCardNumber[32];
	ANTS_MID_TIME struStartTime;
	ANTS_MID_TIME struStopTime;
}ANTS_MID_FILECOND,*LPANTS_MID_FILECOND;

typedef struct{
  char sFileName[100];
  ANTS_MID_TIME struStartTime;
  ANTS_MID_TIME struStopTime;
  DWORD dwFileSize;
  char sCardNum[32];
  BYTE byLocked;
  BYTE byFileType;
  BYTE byRes[2];
}ANTS_MID_FINDDATA,*LPANTS_MID_FINDDATA;

typedef struct{
	LONG lChannel;
	ANTS_MID_TIME struStartTime;
	ANTS_MID_TIME struStopTime;
	BYTE byDiskDes[ANTS_MID_PATHNAME_LEN];
	BYTE byWithPlayer;
	BYTE byRes[35];
}ANTS_MID_BACKUP_TIME_PARAM,*LPANTS_MID_BACKUP_TIME_PARAM;

typedef struct{
	WORD wPicSize;//!图片尺寸：0-CIF，1-QCIF，2-D1，3-UXGA(1600x1200)，
								//!4-SVGA(800x600)，5-HD720p(1280x720)，6-VGA，7-XVGA，8-HD900p，
								//!9-HD1080，10-2560*1920，11-1600*304，12-2048*1536，13-2448*2048，
								//!14-2448*1200，15-2448*800，16-XGA(1024*768)，17-SXGA(1280*1024)，18-WD1(960*576/960*480),19-1080i 
	WORD wPicQuality;//!图片质量系数：0-最好，1-较好，2-一般 
}ANTS_MID_JPEGPARA,*LPANTS_MID_JPEGPARA;

typedef struct{
	BYTE Output[ANTS_MID_MAX_ALARMOUT];
}ANTS_MID_ALARMOUTSTATUS,*LPANTS_MID_ALARMOUTSTATUS;

typedef struct{
	DWORD dwType;
	BYTE byDescribe[ANTS_MID_DESC_LEN];
}ANTS_MID_PTZ_PROTOCOL,*LPANTS_MID_PTZ_PROTOCOL;

typedef struct{
	DWORD dwSize;
	ANTS_MID_PTZ_PROTOCOL struPtz[ANTS_MID_PTZ_PROTOCOL_NUM];
	DWORD dwPtzNum;
	BYTE byRes[8];
}ANTS_MID_PTZCFG,*LPANTS_MID_PTZCFG;

typedef struct{
	BYTE PresetNum;
	BYTE Dwell;
	BYTE Speed;
	BYTE Reserve;
}ANTS_MID_CRUISE_POINT,*LPANTS_MID_CRUISE_POINT;

typedef struct{
	ANTS_MID_CRUISE_POINT struCruisePoint[32];
}ANTS_MID_CRUISE_RET,*LPANTS_MID_CRUISE_RET;

//网络端结构体
typedef struct{
	char sDVRIP[16];
	DWORD dwDVRPort;
	DWORD dwChannel;
	DWORD dwTransProtocol;
	DWORD dwTransMode;
	DWORD dwLinkProtocol;//!0-TCP;1-UDP
	DWORD dwEXMode;//!是否启用增强连接模式
	char sUserName[ANTS_MID_NAME_LEN];
	char sPassword[ANTS_MID_PASSWD_LEN];
	char sRtspMain[ANTS_MID_PATHNAME_LEN];
	char sRtspAux[ANTS_MID_PATHNAME_LEN];
	char byRes[16];//!是否启用增强连接模式
}ANTS_MID_IPCINFO,*LPANTS_MID_IPCINFO;

typedef struct{
	DWORD	dwSize;
	ANTS_MID_IPCINFO struIPCInfos[ANTS_MID_MAX_IPCNUM];
}ANTS_MID_IPCCFG,*LPANTS_MID_IPCCFG;

typedef struct{
	DWORD	dwSize;
	ANTS_MID_IPCINFO struIPCInfos[ANTS_MID_MAX_IPCNUM_V2];
}ANTS_MID_IPCCFG_V2,*LPANTS_MID_IPCCFG_V2;

typedef struct {
	char  sSsid[ANTS_MID_MAX_WIFI_ESSID_SIZE];
	DWORD dwMode;/* 0 mange 模式;1 ad-hoc模式，参见NICMODE */
	DWORD dwSecurity;/*0 不加密；1 wep加密；2 wpa-psk;3 wpa-Enterprise，参见WIFISECURITY*/
	DWORD dwChannel;/*1-11表示11个通道*/
	DWORD dwSignalStrength;/*0-100信号由最弱变为最强*/
	DWORD dwSpeed;/*速率,单位是0.01mbps*/
}ANTS_MID_AP_INFO,*LPANTS_MID_AP_INFO;

typedef struct {
	DWORD dwSize;
	DWORD dwCount;/*无线AP数量，不超过20*/
	ANTS_MID_AP_INFO struApInfo[ANTS_MID_MAX_WIFI_AP_COUNT];
}ANTS_MID_AP_INFO_LIST,*LPANTS_MID_AP_INFO_LIST;

typedef struct {	
	char sIpAddress[16];/*IP地址*/
	char sIpMask[16];/*掩码*/	
	BYTE byMACAddr[ANTS_MID_WIFI_MACADDR_LEN];/*物理地址，只用来显示*/
	BYTE bRes[2];
	DWORD dwEnableDhcp;/*是否启动dhcp  0不启动 1启动*/
	DWORD dwAutoDns;/*如果启动dhcp是否自动获取dns,0不自动获取 1自动获取；对于有线如果启动dhcp目前自动获取dns*/	
	char sFirstDns[16];/*第一个dns域名*/
	char sSecondDns[16];/*第二个dns域名*/
	char sGatewayIpAddr[16];/* 网关地址*/
	BYTE bRes2[8];
}ANTS_MID_WIFIETHERNET,*LPANTS_MID_WIFIETHERNET;

typedef struct {
	/*wifi网口*/
	ANTS_MID_WIFIETHERNET struEtherNet;
	/*SSID*/
	char sEssid[ANTS_MID_MAX_WIFI_ESSID_SIZE];
	/* 0 mange 模式;1 ad-hoc模式，参见*/
	DWORD dwMode;
	/*0 不加密；1 wep加密；2 wpa-psk; */
	DWORD dwSecurity;
	union{
		struct _tagkey{
			/*0 -开放式 1-共享式*/
			DWORD dwAuthentication;
			/* 0 -64位；1- 128位；2-152位*/
			DWORD dwKeyLength;
			/*0 16进制;1 ASCI */
			DWORD dwKeyType;
			/*0 索引：0---3表示用哪一个密钥*/
			DWORD dwActive;
			char sKeyInfo[ANTS_MID_MAX_WIFI_WEP_KEY_COUNT][ANTS_MID_MAX_WIFI_WEP_KEY_LENGTH];
		}wep;
		struct {
			/*8-63个ASCII字符*/
			DWORD dwKeyLength;
			char sKeyInfo[ANTS_MID_MAX_WIFI_WPA_PSK_KEY_LENGTH];
			char sRes;
		}wpa_psk;
	}key;
}ANTS_MID_WIFI_CFG_EX,*LPANTS_MID_WIFI_CFG_EX;

//!wifi配置结构
typedef struct {
	DWORD dwSize;
	ANTS_MID_WIFI_CFG_EX struWifiCfg;
}ANTS_MID_WIFI_CFG,*LPANTS_MID_WIFI_CFG;

//!wifi工作模式
typedef struct {
	DWORD dwSize;
	DWORD dwNetworkInterfaceMode; /*0 自动切换模式　1 有线模式*/
}ANTS_MID_WIFI_WORKMODE,*LPANTS_MID_WIFI_WORKMODE;

//!3G
typedef struct {
	DWORD dwSize;
	BOOL bEnable;//!是否启用3G上网功能
	char szAPNAddr[32];//!APN地址
	char szTelePhone[32];//!拔号号码
	char szIPAddr[32];//!3G IP地址
	DWORD dwWorkMode;/*!
				3G工作模式
				00-表示与ADSL网络并行工作
				01-表示ADSL网络断开030秒后工作
				02-表示ADSL网络断开035秒后工作
				03-表示ADSL网络断开040秒后工作
				04-表示ADSL网络断开045秒后工作
				05-表示ADSL网络断开050秒后工作
				06-表示ADSL网络断开055秒后工作
				07-表示ADSL网络断开060秒后工作
				08-表示ADSL网络断开065秒后工作
				09-表示ADSL网络断开070秒后工作
				10-表示ADSL网络断开075秒后工作
				11-表示ADSL网络断开080秒后工作
				12-表示ADSL网络断开085秒后工作
				13-表示ADSL网络断开090秒后工作
				14-表示ADSL网络断开095秒后工作					
				*/
	DWORD dwDeviceType;//!设备类型
				/*
				0-ZTE MF100 WCDMA
				1-HUAWEI E156G WCDMA
				2-VITION E1916 CDMA2000
				*/
	BYTE byRes[128];	
}ANTS_MID_3G_CFG,*LPANTS_MID_3G_CFG;

//!流媒体服务器基本配置
typedef struct {
	BOOL bEnableManagerHost;
	char szManagerHost[128];
	WORD wManagerHostPort;
	BYTE byRes[2];
}ANTS_MID_MANAGERHOST,*LPANTS_MID_MANAGERHOST;

typedef struct {
	DWORD dwSize;
	ANTS_MID_MANAGERHOST struManagerHosts[ANTS_MID_MAX_MANAGERHOST_NUM];
	BYTE byRes[128];
}ANTS_MID_MANAGERHOST_CFG,*LPANTS_MID_MANAGERHOST_CFG;

//!3G上网卡类型及描述
typedef struct {
	DWORD dwType;//!3G类型值
	BYTE byDescribe[ANTS_MID_MAX_3G_DEVICE_DESC_LEN];
	BYTE byISPDescribe[16];
}ANTS_MID_3G_DEVICE,*LPANTS_MID_3G_DEVICE;

typedef struct {
	DWORD dwSize;
	DWORD dw3GDevNum;
	ANTS_MID_3G_DEVICE stru3Gs[ANTS_MID_MAX_3G_DEVICE_NUM];
	BYTE byRes[256];
}ANTS_MID_3GDEVICE_CFG,*LPANTS_MID_3GDEVICE_CFG;

typedef struct{
	char szIP[16];
	char szMask[16];
	char szGateWay[16];
	char szDns1[16];
	char szDns2[16];
	BYTE byMacAddr[ANTS_MID_MACADDR_LEN];
	BYTE byRes[14];
}ANTS_MID_DISCOVERY_INFO,*LPANTS_MID_DISCOVERY_INFO;

typedef struct{
	ANTS_MID_DISCOVERY_INFO struDiscoveryInfos[3];//0-eth0 1-wifi 2-3g
	WORD wPorts[4];//0-Private 1-Http 2-Rtsp 3-保留
	BYTE bySerialNo[ANTS_MID_SERIALNO_LEN];
	char szName[ANTS_MID_NAME_LEN];
	char szPwd[ANTS_MID_PASSWD_LEN];
	char szDeviceType[32];//!DVR-04 DVR-08 DVR-16 NVR-04 NVR-08 NVR-16  IPC
	BYTE byRes[32];	
}ANTS_MID_DISCOVERYCFG,*LPANTS_MID_DISCOVERYCFG;

//!获取NVR/DVR/IPC单通道名称
typedef struct{
	DWORD dwSize;
	char szChanName[ANTS_MID_NAME_LEN];
	BYTE byRes[4];
}ANTS_MID_DEVCHANNELNAME_CFG,*LPANTS_MID_DEVCHANNELNAME_CFG;

//!获取NVR/DVR/IPC多通道名称
typedef struct{
	DWORD dwSize;
	DWORD dwChanNum;
	char szChanName[ANTS_MID_MAX_CHANNUM_V2][ANTS_MID_NAME_LEN];
}ANTS_MID_DEVCHANNELNAME_CFG_V2,*LPANTS_MID_DEVCHANNELNAME_CFG_V2;

typedef struct{
	DWORD dwSize;
	DWORD dwHDCount;/*硬盘数(不可设置)*/
	ANTS_MID_SINGLE_HD struHDInfo[ANTS_MID_MAX_DISKNUM_V2];//硬盘相关操作都需要重启才能生效；
}ANTS_MID_HDCFG_V2, *LPANTS_MID_HDCFG_V2;

typedef struct{
	LONG lDayNightMode;// -1--不支持;0--外部红外控制;1--自动模式;2--强制白天;3--强制黑夜
	LONG lDelay ;// 自动转换延迟，自动模式有效。0-30
	LONG lNighttoDayThreshold ;// 自动转换黑夜到白天的阈值0-255，默认0xEE
	LONG lDaytoNightThreshold ;// 自动转换白天到黑夜的阈值0-255，默认0x57
}ANTS_MID_SENSOR_DAYNIGHTMODE,*LPANTS_MID_SENSOR_DAYNIGHTMODE;

typedef struct{
	DWORD dwSize;
	DWORD dwValidMask;// 相应位0-无效，1-有效; 
	// bit0 - DayNightMode,bit1-lMinorMode,bit2-lGainMode,bit3-lAntiflickerMode,
	// bit4-lPicQualityMode,bit5-lWBMode   ,bit6-lBacklightMode,bit7-lShutterMode
	// bit8-lIrisMode         ,bit9-lSharpnessMode,bit10-l3DNRMode
	ANTS_MID_SENSOR_DAYNIGHTMODE DayNightMode;
	LONG lMinorMode ;//镜像 -1--不支持;0--正常;1--水平翻转;2--垂直翻转;3--180°翻转
	LONG lGainMode ;//增益 -1--不支持;0--低;1--中;2--高
	LONG lAntiflickerMode ;//抗闪 -1--不支持;0--关;1--开
	LONG lPicQualityMode ;//图像效果 -1--不支持;0--正常;1--艳丽;2--自然
	LONG lWBMode ;//白平衡 -1--不支持;0--自动白平衡;1--室内模式;2--室外模式
	LONG lBacklightMode;// 背光补偿模式-1--不支持; 0--关闭;1--BLC;2--HBLC
	LONG lShutterMode;// 快门模式 -1--不支持; 0--自动快门;
				/*其他模式:
				0x01:1/30(1/25), 
				0x02:1/60(1/50), 
				0x03:Flicker, 
				0x04:1/250, 
				0x05:1/500, 
				0x06:1/1000, 
				0x07:1/2000, 
				0x08:1/5000, 
				0x09:1/10000, 
				0x0A:1/50000, 
				0x0B:x2, 
				0x0C:x4, 
				0x0D:x6, 
				0x0E:x8, 
				0x0F:x10, 
				0x10:x15, 
				0x11:x20,
				0x12:x25,
				0x13:x30 */
	LONG lIrisMode;// 镜头光圈模式:-1--不支持; 0--自动光圈;1--手动或固定光圈
	LONG lSharpnessMode;// 锐度模式: -1--不支持;0--关闭;1--打开
	LONG lSharpnessLevel;// 0-100
	LONG l3DNRMode;// 3D降噪模式 : -1--不支持;0--关闭;1--打开
	LONG l3DNRLevel;// 0-100
	LONG lRes[5];
}ANTS_MID_SENSOR_CFG,*LPANTS_MID_SENSOR_CFG;
/*******************参数配置结构、参数 end*********************/

/********************************SDK接口函数声明*********************************/
#ifdef __cplusplus
extern "C"{
#endif

typedef void(*fVoiceStreamCallBack)(IN LONG lVoiceHandle,IN BYTE *lpBuffer,IN DWORD dwSize,IN BYTE byAudioFlag,IN LPVOID lpUser);

//!Added by ItmanLee at 2013-01-10
typedef BOOL (*fCheckPassword)(IN const char *lpUserName,IN const char *lpPassword);

//!参数获取与设置
typedef BOOL (*fSetParameter)(IN DWORD dwCommand,IN DWORD dwChannel,IN LPVOID lpBuffer,IN DWORD dwSize);
typedef BOOL (*fGetParameter)(IN DWORD dwCommand,IN DWORD dwChannel,OUT LPVOID lpBuffer,IN DWORD dwInSize,OUT DWORD *lpOutSize);

//!PTZ操作
typedef BOOL (*fPtzControl)(IN DWORD dwChannel,IN DWORD dwPtzCommand,IN DWORD dwStop,IN DWORD dwSpeed);
typedef BOOL (*fPtzPreset)(IN DWORD dwChannel,IN DWORD dwPresetCommand,IN DWORD dwPresetIndex);
typedef BOOL (*fPtzTrack)(IN DWORD dwChannel,IN DWORD dwTrackCommand);
typedef BOOL (*fPtzCruise)(IN DWORD dwChannel,IN DWORD dwCruiseCommand,IN BYTE byCruiseRoute,IN BYTE byCruisePoint,IN WORD wInput);
typedef BOOL (*fPtzTrans)(IN DWORD dwChannel,IN BYTE *lpBuffer,IN DWORD dwSize);
typedef BOOL (*fPtz3D)(IN DWORD dwChannel,IN DWORD dwXPoint,IN DWORD dwYPoint,IN DWORD dwScale);
typedef BOOL (*fGetPtzCruise)(IN DWORD dwChannel,IN DWORD dwCruiseRoute,OUT LPANTS_MID_CRUISE_RET lpCruiseRet);

//!Jpeg抓图操作
typedef BOOL (*fCaptureJpeg)(IN DWORD dwChannel,IN LPANTS_MID_JPEGPARA lpJpegParam,OUT BYTE *lpBuffer,IN DWORD dwInSize,OUT DWORD *lpOutSize);

//!获取通道实时主码流及子码流信息
typedef BOOL (*fGetChannelBitRate)(IN DWORD dwChannel,OUT DWORD *lpMainStreamBitRate,OUT DWORD *lpSubStreamBitRate);

//!报警输出状态获取与设置
typedef BOOL (*fGetAlarmOut)(OUT LPANTS_MID_ALARMOUTSTATUS lpAlarmOutStatus);
typedef BOOL (*fSetAlarmOut)(IN LONG lAlarmOutPort,IN LONG lAlarmOutStatic);

typedef BOOL (*fGetAlarmInfo)(OUT DWORD *lpCommand,OUT BYTE *lpBuffer,OUT DWORD *lpBufLen);

//!对讲操作
typedef LONG (*fStartVoice)(IN DWORD dwVoiceChannel,IN BOOL bNeedCBNOEncData,IN fVoiceStreamCallBack fVoiceStream,IN void *lpUser);
typedef BOOL (*fStopVoice)(IN LONG lVoiceHandle);
typedef BOOL (*fSendVoiceData)(IN LONG lVoiceHandle,IN BYTE *lpBuffer,IN DWORD dwSize);

//!设备重启/关机/恢复默认值/保存参数操作
typedef BOOL (*fReboot)( );

//!获取SDK版本及错误码
typedef DWORD (*fGetSDKVersion)( );
typedef DWORD (*fGetLastError)( );

#ifdef __cplusplus
}
#endif
#endif

