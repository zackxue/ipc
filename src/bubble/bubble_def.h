#ifndef __BUBBLE_DEF_HEAD_FILE__
#define __BUBBLE_DEF_HEAD_FILE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define USER_AUTH_CNT       32
#define MAX_USERNAME_LEN    (20)
#define MAX_PASSWORD_LEN    (20)

enum _enPackType{
    PT_MSGPACK,
    PT_MEDIAPACK,
    PT_HEARTBEATPACK,
    PT_OPENCHL = 4,
    PT_OPENSTREAM = 0x0A,

    PT_CNT,
};

enum _enMediaType{
    MT_AUDIO,
    MT_IDR,
    MT_PSLICE,

    MT_CNT,
};

enum _enMsgType{
    MSGT_USERVRF = 0,
    MSGT_CHLREQ,
    MSGT_PTZ,
    MSGT_USERVRF_B,
    MSGT_CHLREQ_B,

    MSGT_CNT,
};

#pragma pack(1)
typedef struct _tagPackHead{
    char          cHeadChar;     // always be 0xaa
    unsigned int  uiLength;      // length of pack except cHeadChar & uiLength,in network sequence
    char          cPackType;     // package type,can be PT_MSGPACK, PT_MEDIAPACK or PT_HEARTBEATPACK
    unsigned int  uiTicket;      // the creating time of the package,in microsecond,in network sequence
    char          pData[1];      // package data ,can be MsgPackData or MediaPackData struct
}PackHead,*lpPackHead;

typedef struct _tagMediaPackData{
    unsigned int  uiLength;      // length of pData, in network sequence
    char          cMediaType;    // type of data, can be value of _enMediaType type
    char          cId;           // channel number
    char          pData[1];      // media data
}MediaPackData,* lpMediaPackData;

typedef struct _tagMsgPackData{
    unsigned int  uiLength;      // length of package except uiLength 
    char          cMsgType[4];   // type of message,can be value of _enMsgType type
    char          pMsg[4];       // content of message,can be UserVrfB,ChlReqB
}MsgPackData,*lpMsgPackData;

typedef struct _tagBubbleOpenStream{
	unsigned int uiChannel;
	unsigned int uiStreamId;
	unsigned int uiOpened;
	unsigned int uiReverse;
}BubbleOpenStream,*lpBubbleOpenStream;

typedef struct _tagOpenChlData{
    char         	cChl;
	char			bopened;
}OpenChlData,*lpOpenChlData;

typedef struct _tagUserVrf{
    unsigned char sUserName[MAX_USERNAME_LEN];
    unsigned char sPassWord[MAX_PASSWORD_LEN];
}UserVrf,*lpUserVrf;

typedef struct _tagUserVrfB{
    char          bVerify;
    unsigned char Reverse[3];
    char          bAuth[USER_AUTH_CNT];
}UserVrfB,*lpUserVrfB;

typedef struct _tagChlReqB{
    int           nChannelNum;
}ChlReqB,*lpChlReqB;

#pragma pack(4)






#endif
