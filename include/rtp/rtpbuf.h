#ifndef __RTP_BUF_H__
#define __RTP_BUF_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include "rtspdef.h"
#include "gnu_win.h"
#include "rtplink.h"

#define ENABLE_BUFFER_OVERWRITE // when buffer is full,overwrite the oldest frame
//#define ENABLE_BUFFER_IGNORE	// when buffer is full, ignore this frame (not push it to circle buffer)
#define SEQUENCE_WRONG_ALLOW	

#define RTPBUF_USED_RATE_FACTOR		(0.4) //流缓冲使用率计算的反馈因子，
									//调节此值来改变反馈的灵敏度

#define CBUFFER_RET_OK		(0)
#define CBUFFER_RET_FAIL	(-1)
#define CBUFFER_RET_NODATA	(1)
#define CBUFFER_RET_FULL	(2)

#define FREE_RET_FAIL			(-1)
#define FREE_RET_OK				(0)
#define FREE_MEET_IDR			(1)
#define FREE_MEET_EMPTY_BUFFER	(2)


#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef RET_OK
#define RET_OK 	(0)
#endif
#ifndef RET_FAIL
#define RET_FAIL (-1)
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define CIC_WAITTIMEOUT		(5*1000*1000) //5s

typedef void* CP_HCIRCLEBUFFER;  
typedef void (*pfn_CircleBufferUninitialise)(CP_HCIRCLEBUFFER bBuffer);
//
typedef void (*pfn_CircleBufferWrite)(CP_HCIRCLEBUFFER bBuffer, const void* pSourceBuffer, const unsigned int iNumBytes);
typedef int (*pfn_CircleBufferRead)(CP_HCIRCLEBUFFER bBuffer, void* pDestBuffer, const unsigned int iBytesToRead, unsigned int* pbBytesRead);
typedef unsigned int (*pfn_CircleGetUsedSpace)(CP_HCIRCLEBUFFER bBuffer);
typedef unsigned int (*pfn_CircleGetFreeSpace)(CP_HCIRCLEBUFFER bBuffer);
typedef float (*pfn_CircleGetInSpeed)(CP_HCIRCLEBUFFER bBuffer);

typedef void (*pfn_CircleFlush)(CP_HCIRCLEBUFFER bBuffer);
// get a frame
typedef int (*pfn_GetNextFrame)(CP_HCIRCLEBUFFER bBuffer,void *out,int *out_size,unsigned int *timestamp);
typedef int (*pfn_GetNextIFrame)(CP_HCIRCLEBUFFER bBuffer,void *out,int *out_size,unsigned int *timestamp);
typedef int (*pfn_GetNextFrimeTimestamp)(CP_HCIRCLEBUFFER bBuffer,unsigned int *timestamp);
// insert a rtp packet
typedef int (*pfn_AddRtpFrame)(CP_HCIRCLEBUFFER bBuffer,RtpFrameInfo_t *info);
typedef int (*pfn_IsAvailable)(CP_HCIRCLEBUFFER bBuffer);
typedef int (*pfn_Destroy)(CP_HCIRCLEBUFFER bBuffer);

typedef struct _CPs_CircleBuffer
{
    pfn_CircleBufferWrite Write;
    pfn_CircleBufferRead Read;
    pfn_CircleFlush Flush;
    pfn_CircleGetUsedSpace GetUsedSize;
    pfn_CircleGetFreeSpace GetFreeSize;
	pfn_CircleGetInSpeed GetInSpeed;
	//
	pfn_GetNextFrame NextFrame;
	pfn_GetNextIFrame NextIFrame;
	pfn_GetNextFrimeTimestamp NextFrameTS;
	pfn_AddRtpFrame AddRtpFrame;
	pfn_IsAvailable IsAvailable;
	//
	pfn_Destroy Destroy;

    unsigned char* m_pBuffer;
    unsigned int m_iBufferSize;
    unsigned int m_iReadCursor;
    unsigned int m_iWriteCursor;
    EventHandle_t m_evtDataAvailable;
    Lock_t m_csCircleBuffer;
	int m_bRealloc;
	int m_iBufferTime; // unit : millisecond
	int m_iFrameCnt;
	MillisecondTimer_t m_StartTime;
	unsigned int m_iTotalBytes;
	int m_bAvailable;//
	float m_fInSpeed;
	float m_fUsedRate;

	DLink_t *m_FrameInfo;
} CircleBuffer_t;
//
////////////////////////////////////////////////////////////////////////////////
CircleBuffer_t* CIRCLEBUFFER_new(const unsigned int iBufferSize,const int buffer_time);

#ifdef __cplusplus
}
#endif

#endif

