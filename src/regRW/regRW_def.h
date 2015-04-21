#ifndef __REGRW_DEF_H_
#define __REGRW_DEF_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RegRWSession
{
	bool* trigger;
	int is_init;
	int sock;

#define  REGRW_SESSION_RECV_BUF_SZ (2 * 1024)
	uint8_t recv_buf[REGRW_SESSION_RECV_BUF_SZ];
	ssize_t recv_sz;
	
}RegRWSession_t;

enum _regRWType{
	REGRW_TYPE_MT9D131 = 0,
	REGRW_TYPE_JUANSN,
	REGRW_TYPE_CNT,
};

typedef struct _PackHead
{
    char cHeadChar[4];  //0xbbddccaa
    int uiLength;  
    int iPackType;
	int iOperation;//0:read  1:write
}PackHead_t;

typedef struct _mt9d131reg
{
	unsigned int value;
	int page;
	unsigned char addr;
}Mt9d131reg_t;

typedef struct _juansn
{
	char sn[32];
}Juansn_t;

#ifdef __cplusplus
};
#endif

#endif //__REGRW_DEF_H_

