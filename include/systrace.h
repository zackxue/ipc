
#ifndef __SYSTRACE_H__
#define __SYSTRACE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>

// bitmask bit type defination
typedef unsigned long long TRACE_BITHOT;

#define TRACE_ATTR_NONE "0"
#define TRACE_ATTR_HIGHLIGHT "1"
#define TRACE_ATTR_UNDERLINE "4"
#define TRACE_ATTR_FLICKER "5"
#define TRACE_ATTR_INVERT "7"
#define TRACE_ATTR_BLANKING "8"
// debug trace color foreground
#define TRACE_ATTR_FG_BLACK "30"
#define TRACE_ATTR_FG_RED "31"
#define TRACE_ATTR_FG_GREEN "32"
#define TRACE_ATTR_FG_YELLOW "33"
#define TRACE_ATTR_FG_BLUE "34"
#define TRACE_ATTR_FG_PURPLE "35"
#define TRACE_ATTR_FG_DARKGREEN "36"
#define TRACE_ATTR_FG_WHITE "37"
// debug trace color background
#define TRACE_ATTR_BG_BLACK "40"
#define TRACE_ATTR_BG_RED "41"
#define TRACE_ATTR_BG_GREEN "42"
#define TRACE_ATTR_BG_YELLOW "43"
#define TRACE_ATTR_BG_BLUE "44"
#define TRACE_ATTR_BG_PURPLE "45"
#define TRACE_ATTR_BG_DARKGREEN "46"
#define TRACE_ATTR_BG_WHITE "47"

#define RTSPD_BITSHIFT (1<<0)
#define AVENC_BITSHIFT (1<<1)
#define FRAMBUF_BITSHIFT (1<<2)

#define TRACE_DEFAULT_BITMASK (RTSPD_BITSHIFT | AVENC_BITSHIFT | FRAMBUF_BITSHIFT)

#if TRUE
#define TRACE(fmt...) PRINTF(fmt)
#else
#define TRACE(fmt...)
#endif

#define TRACE_EX(cond, attr, title, fmt...) \
	do{\
		if(TRACE_check(cond)){\
			TRACE("\033["attr"m%s->[%s:%d]", title, __func__,__LINE__);\
			TRACE(fmt);\
			TRACE("\033[0m\r\n");\
		}\
	}while(0)

// functions
#define RTSPD_TRACE(fmt...) TRACE_EX(RTSPD_BITSHIFT, TRACE_ATTR_BG_BLACK";"TRACE_ATTR_HIGHLIGHT";"TRACE_ATTR_FG_PURPLE, "RTSPD", fmt)
#define AVENC_TRACE(fmt...) TRACE_EX(AVENC_BITSHIFT, TRACE_ATTR_BG_BLACK";"TRACE_ATTR_HIGHLIGHT";"TRACE_ATTR_FG_GREEN, "AVENC", fmt)
#define FRAMBUF_TRACE(fmt...) TRACE_EX(FRAMBUF_BITSHIFT, TRACE_ATTR_BG_BLACK";"TRACE_ATTR_FG_PURPLE, "FRAMBUF", fmt)

// system assertion
#define SYS_ASSERT(exp, fmt...) \
	do{\
		if(!(exp)){\
			TRACE("\033["TRACE_ATTR_BG_BLACK";"TRACE_ATTR_HIGHLIGHT";"TRACE_ATTR_FG_BLUE"m");\
			TRACE("Assertion( %s ) @ [%s:%d] ", #exp, __func__,__LINE__);\
			TRACE(fmt);\
			TRACE("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

extern int PRINTF(const char* fmt, ...);
extern int TRACE_check(TRACE_BITHOT bithot);
extern void TRACE_set(TRACE_BITHOT bithot);
extern void TRACE_clear(TRACE_BITHOT bithot);
extern void TRACE_dup(FILE* fp);

#ifdef __cplusplus
};
#endif
#endif //__SYSTRACE_H__

