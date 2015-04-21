
#ifndef __USRM_DEBUG_H__
#define __USRM_DEBUG_H__

#include <string.h>

#define USRM_DEBUG (1)
#if USRM_DEBUG
#define USRM_SYNTAX "1;31"
#define USRM_TRACE(fmt, arg...) \
	do{\
		const char* bname = strdupa(basename(__FILE__));\
		printf("\033["USRM_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define USRM_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = strdupa(basename(__FILE__));\
			printf("\033["USRM_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define USRM_TRACE(fmt...)
#define USRM_ASSERT(exp, fmt...)
#endif

#endif //__USRM_DEBUG_H__

