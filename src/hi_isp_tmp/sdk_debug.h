
#ifndef __SOC_DEBUG_H__
#define __SOC_DEBUG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>


#define SOC_DEBUG (1)
#if SOC_DEBUG
#define SOC_SYNTAX "1;32"
#define SOC_TRACE(fmt, arg...) \
	do{\
		const char* const bname = __FILE__;\
		printf("\033["SOC_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define SOC_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* const bname = __FILE__;\
			printf("\033["SOC_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#define SOC_CHECK(exp, fmt...) \
	do{\
		int ret = exp;\
		if(HI_SUCCESS != ret){\
			const char* const bname = __FILE__;\
			printf("\033["SOC_SYNTAX"m");\
			printf("%s @ [%s: %d] err: 0x%08x <%s>", #exp, bname, __LINE__, ret, SOC_strerror(ret));\
			printf("\033[0m\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define SOC_TRACE(fmt...)
#define SOC_ASSERT(exp, fmt, arg...)
#define SOC_CHECK(exp, fmt...)
#endif

#ifdef __cplusplus
};
#endif
#endif //__SOC_DEBUG_H__

