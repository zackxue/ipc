
#ifndef __RTSPD_DEBUG_H__
#define __RTSPD_DEBUG_H__

#define RTSPD_DEBUG (1)
#if RTSPD_DEBUG
#define RTSPD_TRACE_ATTR "40;1;31"
#define RTSPD_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["RTSPD_TRACE_ATTR"mRTSPD->[%s:%4d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define RTSPD_ASSERT(exp) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["RTSPD_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s:%4d] ", #exp, basename(bname), __LINE__);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#define RTSPD_TRACE_REQUEST(fmt...) \
	do{\
		printf("\033[1;35m");\
		printf(fmt);\
		printf("\033[0m");\
		fflush(stdout);\
	}while(0)

#define RTSPD_TRACE_RESPONSE(fmt...) \
	do{\
		printf("\033[40;1;34m");\
		printf(fmt);\
		printf("\033[0m");\
		fflush(stdout);\
	}while(0)

#else
#define RTSPD_TRACE(fmt...)
#define RTSPD_ASSERT(exp, fmt...)
#define RTSPD_TRACE_REQUEST(fmt...) 
#define RTSPD_TRACE_RESPONSE(fmt...)
#endif

#endif //__RTSPD_DEBUG_H__

