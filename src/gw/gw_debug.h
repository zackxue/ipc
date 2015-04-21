
#ifndef __GW_DEBUG_H__
#define __GW_DEBUG_H__

#define GW_DEBUG (1)
#if GW_DEBUG
#define GW_TRACE_ATTR "40;35"
#define GW_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["GW_TRACE_ATTR"mGW->[%s: %d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define GW_ASSERT(exp, fmt...) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["GW_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s: %d] ", #exp, basename(bname), __LINE__);\
			printf(fmt);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define GW_TRACE(fmt...)
#define GW_ASSERT(exp, fmt...)
#endif

#endif //__GW_DEBUG_H__

