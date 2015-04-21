
#ifndef __SYSCONF_DEBUG_H__
#define __SYSCONF_DEBUG_H__

#include <string.h>
#include <libgen.h>

#define SYSCONF_DEBUG (1)
#if SYSCONF_DEBUG
#define SYSCONF_SYNTAX "37;1;35"
#define SYSCONF_TRACE(fmt, arg...) \
	do{\
		char* const file_name = strdupa(__FILE__);\
		char* const bname = strdupa(basename(file_name));\
		printf("\033["SYSCONF_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define SYSCONF_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			char* const file_name = strdupa(__FILE__);\
			char* const bname = strdupa(basename(file_name));\
			printf("\033["SYSCONF_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define SYSCONF_TRACE(fmt...)
#define SYSCONF_ASSERT(exp, fmt...)
#endif


#endif //__SYSCONF_DEBUG_H__

