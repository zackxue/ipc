#ifndef _GB28181_DEBUG_H__
#define _GB28181_DEBUG_H__

#include <assert.h>
//#include <libgen.h>


#ifdef __cplusplus
extern "C" {
#endif


#define GB28181_DEBUG_LEVEL 1

#define GB_PRINT(syntax,fmt,arg...)  \
	do{	\
		printf("\033[%sm""[GB28181 |%s:%d]\033[0m",syntax,basename(__FILE__),__LINE__); \
		printf(fmt,##arg); \
	 }while(0);


#if GB28181_DEBUG_LEVEL <= 1
#define GB_TRACE(fmt, arg...) GB_PRINT("0",fmt,##arg)
#else
#define GB_TRACE(fmt, arg...)
#endif

#if GB28181_DEBUG_LEVEL <= 2
#define GB_NOTIFY(fmt, arg...) GB_PRINT("32",fmt,##arg)
#else
#define GB_NOTIFY(fmt, arg...)
#endif



#if GB28181_DEBUG_LEVEL <= 3
#define GB_ERROR(fmt, arg...) GB_PRINT("31",fmt,##arg)
#else
#define GB_ERROR(fmt, arg...)
#endif

#define GB_ASSERT(exp, fmt, arg...) \
	{\
	if(!(exp)){\
				printf("[GB28181 | %s:%d]",basename(__FILE__),__LINE__);\
				printf(fmt, ##arg);\
				printf("\r\n");\
				exit(-1);\
			}\
	}

#define TRACE(fmt,arg...) GB_TRACE(fmt,##arg)

#define FALSE (0)
#define TRUE  (1)

#endif

