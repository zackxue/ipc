
#ifndef __UNISTRUCT_DEBUG_H__
#define __UNISTRUCT_DEBUG_H__

#include <string.h>
#include <libgen.h>

#define UNISTRUCT_DEBUG (1)
#if UNISTRUCT_DEBUG
#define UNISTRUCT_SYNTAX "37;1;35"
#define UNISTRUCT_TRACE(fmt, arg...) \
	do{\
		char* const file_name = strdupa(__FILE__);\
		char* const bname = strdupa(basename(file_name));\
		printf("\033["UNISTRUCT_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define UNISTRUCT_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			char* const file_name = strdupa(__FILE__);\
			char* const bname = strdupa(basename(file_name));\
			printf("\033["UNISTRUCT_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define UNISTRUCT_TRACE(fmt...)
#define UNISTRUCT_ASSERT(exp, fmt...)
#endif


#endif //__UNISTRUCT_DEBUG_H__

