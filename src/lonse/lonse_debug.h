
#ifndef __LONSE_DEBUG_H__
#define __LONSE_DEBUG_H__

#include <string.h>
#include <strings.h>

#define LONSE_DEBUG (1)
#if LONSE_DEBUG
#define LONSE_SYNTAX "1;36"
#define LONSE_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		printf("\033["LONSE_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define LONSE_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["LONSE_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define LONSE_TRACE(fmt...)
#define LONSE_ASSERT(exp, fmt...)
#endif


#endif //__LONSE_DEBUG_H__

