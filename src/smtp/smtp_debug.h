
#ifndef __SMTP_DEBUG_H__
#define __SMTP_DEBUG_H__

#include <string.h>

#define SMTP_DEBUG (1)
#if SMTP_DEBUG
#define SMTP_SYNTAX "1;35"
#define SMTP_TRACE(fmt, arg...) \
		do{\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["SMTP_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
			printf(fmt, ##arg);\
			printf("\r\n");\
		}while(0)
	
#define SMTP_ASSERT(exp, fmt, arg...) \
		do{\
			if(!(exp)){\
				const char* bname = basename(strdupa(__FILE__));\
				printf("\033["SMTP_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
				printf(fmt, ##arg);\
				printf("\r\n");\
				exit(1);\
			}\
		}while(0)
	
#else
#define SMTP_TRACE(fmt...)
#define SMTP_ASSERT(exp, fmt...)
#endif

#endif //__SMTP_DEBUG_H__

