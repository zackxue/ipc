#ifndef __MACRO_DEF_HEAD_FILE__
#define __MACRO_DEF_HEAD_FILE__

#define STRUCT_MEMBER_POS(t,m)  ((unsigned long)(&(((t *)(0))->m)))

#define BUBBLE_DEBUG (1)
#if BUBBLE_DEBUG
#define BUBBLE_TRACE_ATTR "40;1;31"
#define BUBBLE_TRACE(fmt...) \
	do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033["BUBBLE_TRACE_ATTR"m[%s:%4d] ", basename(bname), __LINE__);\
		printf(fmt);\
		printf("\033[0m\r\n");\
	}while(0)

#define BUBBLE_ASSERT(exp) \
	do{\
		if(!(exp)){\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033["BUBBLE_TRACE_ATTR"m");\
			printf("Assertion( %s ) @ [%s:%4d] ", #exp, basename(bname), __LINE__);\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define BUBBLE_TRACE(fmt...)
#define BUBBLE_ASSERT(exp, fmt...)
#endif

#endif
