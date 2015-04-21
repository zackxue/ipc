
#ifndef __APP_DEBUG_H__
#define __APP_DEBUG_H__

#define APP_DEBUG (1)
#if APP_DEBUG
#define APP_SYNTAX "1;35"
#define APP_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		printf("\033["APP_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define APP_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["APP_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define APP_TRACE(fmt...)
#define APP_ASSERT(exp, fmt...)
#endif

#endif //__APP_DEBUG_H__

