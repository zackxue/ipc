
#ifndef __HLS_DEBUG_H__
#define __HLS_DEBUG_H__

#include <string.h>

#define HLS_DEBUG (1)
#if HLS_DEBUG
#define HLS_SYNTAX "1;35"
#define HLS_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		printf("\033["HLS_SYNTAX"m[%12s:%4d pid=(%08x,%08x)]\033[0m ", bname, __LINE__, getpid(), pthread_self());\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define HLS_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["HLS_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define HLS_TRACE(fmt...)
#define HLS_ASSERT(exp, fmt...)
#endif

#endif //__HLS_DEBUG_H__

