#ifndef ESEE_PROTOCOL_H
#define ESEE_PROTOCOL_H

#include <string.h>
#include <stdbool.h>
#include "rudp_session.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG_SIGN_ESEE 0
#define TAG_SIGN_HEAD 1
#define TAG_SIGN_HEADSUB 2
#define TAG_SIGN_BODY 1
#define TAG_SIGN_BODYSUB 2

#define ET(tag) ESEE_TAG_##tag

typedef enum _EseeTagSign
{
	ESEE_TAG_START = __LINE__,
	ESEE_TAG_ESEE = 0,
	ESEE_TAG_HEAD ,
	ESEE_TAG_CMD ,
	ESEE_TAG_TICK ,
	ESEE_TAG_PKTNUM ,
	ESEE_TAG_PKTNO ,
	ESEE_TAG_SN ,
	ESEE_TAG_ID ,
	ESEE_TAG_PWD ,
	ESEE_TAG_STATUS ,
	ESEE_TAG_CHANNEL ,
	ESEE_TAG_ERRCMD ,
	ESEE_TAG_ERRINFO ,
	ESEE_TAG_ECODE ,
	ESEE_TAG_INTERIP ,
	ESEE_TAG_EXTERIP ,
	ESEE_TAG_INTERPORT ,
	ESEE_TAG_EXTERPORT ,
	ESEE_TAG_PORT ,
	ESEE_TAG_DATAPORT ,
	ESEE_TAG_PHONEPORT ,
	ESEE_TAG_URL ,
	ESEE_TAG_VERSION ,
	ESEE_TAG_VENDOR ,
	ESEE_TAG_RANDOM,
	ESEE_TAG_NODES,
	ESEE_TAG_NODE,
	ESEE_TAG_LENGTH , 
	ESEE_TAG_DATA ,
	ESEE_TAG_DVRIP,
	ESEE_TAG_DVRPORT,
	ESEE_TAG_CLIENTIP ,
	ESEE_TAG_CLIENTPORT ,
	ESEE_TAG_TURNSERVERS ,
	ESEE_TAG_TURNSERVER ,
	ESEE_TAG_END = __LINE__,
}EseeTagSign;

#define ESEE_MAX_TAG 	(ESEE_TAG_END - ESEE_TAG_START -1)


typedef struct _EseeTag{
	int	TagOrder;
	const char* TagName;
	const char* TagType;
	char *TagTxt;
	const int	TagSign;
}EseeTag;

typedef struct _CmdPack{
	int PackOrder;
	const char* PackName;
	const char* PackType;
	char* PackTag[ESEE_MAX_TAG];
}CmdPack;

typedef struct _Tag{
	char* TagName;
	char* Content;
}Tag;


typedef enum _EseeCmd
{
	ESEE_CMD_START = __LINE__,
	SRequestIdentify = 10000, 
	SRequestLogin, 
	SHeartbeat, 
	SRequestAllport,
	SRequestDataport, 
	SRequestPhoneport,
	SRequestInterip,
	SRequestUrl,
	SRequestUpdate,
	SResponseIdentify = 11000,
	SResponseLogin,
	SResponseHeart, 
	SResponseAllport,
	SResponseDataport, 
	SResponsePhoneport,
	SResponseInterip,
	SResponseUrl,
	SResponseUpdate,
	SResponseClientInfo,
	SResponseErrorInfo = 11100, 
	CRequestLogin = 20001,
	CRequestUpdate,
	CResponseLogin = 21001,
	CResponseUpdate,
	CResponseErrorInfo = 21100,
	STurnAuth = 10010, 
	SResponseTurn = 11010,
	STurnReady = 10011,
	STurnAck = 11011, 
	CResponseReadyTurn = 21012,
	CRequestTraverse = 20101,
	CResponseTraversal= 21101,
	STraversalReq = 11101,
	SConfirmTraversal = 10101,
	CResponseConfirm = 21102,
	SHoleClient = 30101,
	CHoleDevice = 31101,
	ESEE_CMD_END = __LINE__,
}EseeCmd;


#define ESEE_MAX_CMD (ESEE_CMD_END - ESEE_CMD_START -1)
extern EseeTag TagTable[ESEE_MAX_TAG];
extern CmdPack CmdPackTable[ESEE_MAX_CMD];

#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
#define COPYSTRING(dest, src) do{ if(src && strlen(src)>0 ){ dest = strdup(src); } else{ dest = ""; } }while(0)
#define EQUAL(src, dest) ( strlen(src) == strlen(dest) && 0 == strcmp(src, dest))
#define FREE(ptr) do{ if(ptr && strlen(ptr) > 0) free(ptr); }while(0)

//given a sting of xml data, parse it and output tagbuffer which must be freed by user, return the numbers of tag
int ReadProtocol(char* stream);
//given a packname and tick, return a string of xml data which must be freed by user
char* WriteProtocol(void* packName, void* tick);

void SetTagTxt(const char* tagName, void* tagValue);

bool Parse(void* xml,int *buffsize);
bool FindTag(char* target);
char** GetPack(char* packName);
char* GetCmdByPackName(char* packName);
char** GetPackByCmd(char* cmd);

#ifdef __cplusplus
}
#endif

#endif /*end the esee protocol */
