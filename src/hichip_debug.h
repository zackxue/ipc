
#ifndef __HICHIP_DEBUG_H__
#define __HICHIP_DEBUG_H__

#define PARTY3RD_DEBUG (1)
#if PARTY3RD_DEBUG
#define PARTY3RD_TRACE_ATTR "1;36"
#define PARTY3RD_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["PARTY3RD_TRACE_ATTR"mPARTY3RD->[%s:%4d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define PARTY3RD_ASSERT(exp, fmt...) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["PARTY3RD_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s:%4d] ", #exp, basename(bname), __LINE__);\
			printf(fmt);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define PARTY3RD_TRACE(fmt...)
#define PARTY3RD_ASSERT(exp, fmt...)
#endif

#define HICHIP_DEBUG (1)
#if HICHIP_DEBUG
#define HICHIP_SYNTAX "1;36"
#define HICHIP_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		printf("\033["HICHIP_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define HICHIP_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["HICHIP_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define HICHIP_TRACE(fmt...)
#define HICHIP_ASSERT(exp, fmt...)
#endif


#endif //__HICHIP_DEBUG_H__

