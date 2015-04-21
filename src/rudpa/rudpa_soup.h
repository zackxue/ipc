/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa_soup.h
 * Describle: the api&& some struct expose
 * History: 
 * Last modified: 2013-03-28 17:42
=============================================================*/
#ifndef RUDPA_SOUP_H
#define RUDPA_SOUP_H

#include "rudp_session.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"


typedef enum _SoupCmd
{
	SoupCmdStart = __LINE__,
	SoupCmdPtz = 0,
	SoupCmdSeekStream,
	SoupCmdReqStream,
	SoupCmdAuth,
	SoupCmdDevinfo,
	SoupCmdEnd = __LINE__,
}SoupCmd;

typedef enum _ErrorSoup
{
	ES_SUCCESS,	
	ES_UNSUPPORT,
	ES_PASSWD,
	ES_NOPERMISSION,
	ES_HEADERROR,
	ES_UNDEF = 1024,	
}ErrorSoup;

#define MAX_SOUP_CMD (SoupCmdEnd - SoupCmdStart - 1)


typedef struct _tagSoupFrameHead{
	uint32_t magic;			// magic number 固定为 0x534f55ff , "SOU·“
	uint32_t version;		// 版本信息，当前版本为1.0.0.0，固定为0x01000000
	uint32_t frametype;	// 码流帧类型，当前版本支持三种类型：0x00--音频 0x01--视频I帧 0x02--视频P帧
	uint32_t framesize;	// 码流帧的裸数据长度
	uint64_t pts;				// 帧时间戳，64位数据，精度到微秒
	uint32_t externsize;	// 扩展数据大小，当前版本为0
	union{
		struct _tagVideoParam{
			uint32_t width;	// 视频宽
			uint32_t height;	// 视频高
			uint32_t enc;	// 视频编码，四个ASIIC字符表示，当前支持的有"H264"
		}v;
		struct _tagAudioParam{
			uint32_t samplerate;	// 音频采样率
			uint32_t samplewidth;	// 音频采样位宽
			uint32_t enc;	// 音频编码，四个ASIIC字符表示，当前支持的有"G711"
		}a;
	}_U;
}SoupFrameHead;



typedef struct _SoupData
{
	char *soup_version;
	uint32_t soup_cmd;
	char *ptz_chl;
	char **ptz_param;	
	char *settings_method;
	char * settings_vin;
	uint32_t settings_stream_No;
	uint32_t streamreq_ch;
	uint32_t streamreq_stream_No;
	char *streamreq_opt;
	char *auth_usr;
	char *auth_psw;
	char *soup_ticket;
	uint32_t soup_error;
	uint32_t sd_camcnt;
}SoupData;

typedef struct _Soup
{

	int (*PackHead)(void *,const lpSDK_ENC_BUF_ATTR);
	int (*DataProc)(char *,int ,SoupData*);
	char* (*PackPkt)(SoupData *);
}Soup;


extern Soup * CreateNewSoup(void*);
extern char *GetStreamName(int stream_No);
extern char * PackSoupPkt(SoupData *thiz);


#endif /*end of rudpa_soup.h*/

