
#ifndef __OWSP_DEBUG_H__
#define __OWSP_DEBUG_H__

#define OWSP_DEBUG (1)
#if OWSP_DEBUG
#define OWSP_TRACE_ATTR "40;1;31"
#define OWSP_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["OWSP_TRACE_ATTR"m[%s:%4d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define OWSP_ASSERT(exp) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["OWSP_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s:%4d] ", #exp, basename(bname), __LINE__);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define OWSP_TRACE(fmt...)
#define OWSP_ASSERT(exp, fmt...)
#endif

#endif //__OWSP_DEBUG_H__

