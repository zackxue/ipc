
#ifndef __DDNS_H__
#define __DDNS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define DDNS_UPDATE_PERIOD	(30)
//#define DDNS_DEBUG

#ifdef DDNS_DEBUG
#define DDNS_TRACE_ATTR "40;33"
#define DDNS_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["DDNS_TRACE_ATTR"mDDNS->[%s: %d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define DDNS_ASSERT(exp, fmt...) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["DDNS_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s: %d] ", #exp, basename(bname), __LINE__);\
			printf(fmt);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define DDNS_TRACE(fmt...)
#define DDNS_ASSERT(exp, fmt...)
#endif
/*
enum{
	DDNS_DYNDNS,
	DDNS_3322,
	DDNS_CHANGEIP,
	DDNS_NOIP
};*/

typedef struct DDNS_PARA
{
	//char provider[32];
	int provider;
	union
	{
		// 3322, dyndns, noip,changeip
		struct
		{
			char register_url[64];
			char username[32];
			char password[32];
		}_3322, dyndns, noip, changeip;
		
		// popdvr, skybest, dvrtop
		struct
		{
			char zero[64];
			char username[32];
			char password[32];
		}popdvr, skybest, dvrtop;
		
	};
}DDNS_PARA_t;

typedef enum DDNS_STAT
{
	DDNS_UNKNOW_ERROR,
	DDNS_SUSPEND,
	DDNS_GET_LOCAL_HOST,
	DDNS_GET_DDNS_HOST,
	DDNS_GET_WAN_IP,
	DDNS_UPDATE,
	DDNS_TRYING_CONNECT,
	DDNS_SERVICE_INVAILD,
	DDNS_REGISTER_OK,
	DDNS_UPDATE_OK,
}DDNS_STAT_t;

extern int DDNS_start(DDNS_PARA_t para);
extern int DDNS_restart(DDNS_PARA_t para);

extern void DDNS_quit();

extern DDNS_STAT_t DDNS_get_stat();

#ifdef __cplusplus
};
#endif
#endif //__DDNS_H__

