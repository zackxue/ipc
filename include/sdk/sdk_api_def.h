

#ifndef SDK_API_DEF_H_
#define SDK_API_DEF_H_


#if 1
#define SDK_TRACE_ATTR "40;1;31"
#define SDK_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["SDK_TRACE_ATTR"mSDK->[%s:%4d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define SDK_ASSERT(exp) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["SDK_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s:%4d] ", #exp, basename(bname), __LINE__);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#define SDK_CHECK(exp) do{SDK_ASSERT(SDK_SUCCESS == (exp));}while(0)

#else
#define SDK_TRACE(fmt...)
#define SDK_ASSERT(exp, fmt...)
#endif

#endif //SDK_API_DEF_H_


