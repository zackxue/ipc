#ifndef __RTSPDEF_H__
#define __RTSPDEF_H__

#ifdef __cplusplus
extern "C" {
#endif	

#include "gnu_win.h"

/**************************************************************************
* configure mirco
**************************************************************************/
#define RTSP_SOCK_TIMEOUT			(3000) //  unit: ms , socket send and receive timeout
#define RTSP_PLAYER_BUFFER_TIME		(2500)	// unit: ms
#define RTSP_PBUFFER_READY_POINT 	(0.5) // the factor of while circle buffer is available to get data
										// for example,0.5 is meaning to 50%
	
#define TIMESTAMP_FIX_FACTOR		(3)//时钟漂移和时间戳错误修正因子
#define RTSP_BUFFER_ENTER_KEYFRAME_FIRST //流绥冲如果264视频，让进入缓冲的第一帧为idr


#define RTSP_DISABLE_PLAYER_BUFFER_INLOCAL	
#define RTSP_ENABLE_AUDIO			(FALSE) //rtsp server 使用使用音频
#define RTSP_ENABLE_RTCP			(FALSE)
#define RTSP_ENABLE_AUTHTICATION	(FALSE)	// only vaild for rtsp server
#define IPCAM_SOLUTION
	
/* *****************************************************************
*  circle buffer used rate threshold
	_____________________________________________________
* 	|            |                                            |               |          |
	-----------------------------------------------------
	0		LEVEL1					   LEVEL2		LEVEL3	1.0     
* case 1:  0~level1 , used rate is too small , play in slow speed
   case 2:  level1~level2, used rate is normal, play in normal speed
   case 3:  level2~level3, used rate is too big, play in fast speed
   case 4:  level3~level4, used rate is too too big, only play idr
********************************************************************/
#define CBUFFER_USED_RATE_LEVEL1	(0.2)  
#define CBUFFER_USED_RATE_LEVEL2	(0.7)
#define CBUFFER_USED_RATE_LEVEL3	(0.9)

/*************************************************************************
* const micro relative to rtsp module, must not modified
**************************************************************************/
#define H264_TS_TO_MILLISECOND(TS)	((TS)/90)//(1000/90000*TS)
#define G711_TS_TO_MILLISECOND(TS)  ((TS)/8)//(1000/8000*TS)

#define RTSP_INTERLEAVED_MAGIC	'$'//0X24

#define RTSP_NET_PROC	(0x01)
#define RTSP_DEC_PROC	(0x02)
#define RTSPC_RUNNING	(RTSP_NET_PROC | RTSP_DEC_PROC)

#define RTSP_CLIENT				(0)
#define RTSP_SERVER				(1)

#define RTSP_RET_OK				(0)
#define RTSP_RET_FAIL			(-1)

#ifndef true
#define true	(1)
#endif
#ifndef false
#define false	(0)
#endif

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define ABS_MINUS(A,B) (((A) > (B)) ? ((A)-(B)) : ((B)-(A)))
#define MAXAB(A,B) (((A) > (B)) ? (A) : (B))
#define MAXABC(A,B,C) ((MAXAB((A),(B)) > (C)) ? MAXAB((A),(B)) : (C))

//typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int 	uint32_t;
#ifdef _WIN32
typedef unsigned __int64 uint64_t;
#else
typedef unsigned long long uint64_t;
#endif

typedef struct _thread_args
{
	void *data;
	void *LParam;
	int RParam;
}ThreadArgs_t;


#ifdef __cplusplus
}
#endif
#endif

