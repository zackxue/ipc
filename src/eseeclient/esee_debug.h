
#ifndef __ESEE_DEBUG_H__
#define __ESEE_DEBUG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <time.h>
#include <sys/time.h>


#define ESEE_SYNTAX "1;34"
//#define _DEPRESS_TESTING
#ifdef _DEPRESS_TESTING
#define ESEE_SYNTAX_ON() \
	do{ printf("\033["ESEE_SYNTAX"m"); }while(0)

#define ESEE_SYNTAX_OFF() \
	do{ printf("\033[0m\r\n"); }while(0)

#define ESEE_TRACE(fmt...) \
	do{\
		char* bname = strdupa(basename(__FILE__));\
		ESEE_SYNTAX_ON(); \
		printf("ESEECLIENT->[%s:%4d] ", bname, __LINE__);\
		printf(fmt);\
		ESEE_SYNTAX_OFF(); \
	}while(0)

#define ESEE_TRACE_TIME() \
	do{\
		time_t tt = time(NULL); \
		struct tm* p_tm = localtime(&tt); \
		printf("[%04d/%02d/%02d %02d:%02d:%02d]\r\n", \
			p_tm->tm_year + 1900, p_tm->tm_mon+1, p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec); \
	}while(0)

#define ESEE_TRACE_REQUEST(fmt...) \
	do{\
		printf("\033[1;31m"); \
		ESEE_TRACE_TIME(); \
		printf(fmt);printf("\033[0m\r\n");\
	}while(0)

#define ESEE_TRACE_RESPONSE(fmt...) \
	do{\
		printf("\033[1;33m"); \
		ESEE_TRACE_TIME(); \
		printf(fmt);printf("\033[0m\r\n");\
	}while(0)
	
#else
#define ESEE_SYNTAX_ON()\
	do{ printf("\033["ESEE_SYNTAX"m"); }while(0)
#define ESEE_SYNTAX_OFF()\
	do{ printf("\033[0m\r\n"); }while(0)
#define ESEE_TRACE(fmt...)

#define ESEE_TRACE_TIME()
#define ESEE_TRACE_REQUEST(fmt...)
#define ESEE_TRACE_RESPONSE(fmt...)
#endif
#ifdef __cplusplus
};
#endif
#endif //__ESEE_DEBUG_H__

