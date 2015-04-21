#ifndef __RUDPA_DEBUG_HEAD_FILE__
#define __RUDPA_DEBUG_HEAD_FILE__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
//#include <libgen.h>

#define _RUDPA_DEBUG_LEVEL 3

#define _RUDPA_PRINT(syntax,fmt,arg...)  \
	do{	\
		printf("\033[%sm""[RUDPA |%s:%d:%s ]\033[0m",syntax,basename(__FILE__),__LINE__,__FUNCTION__); \
		printf(fmt,##arg); \
	 }while(0);

#if _RUDPA_DEBUG_LEVEL <= 1
#define _RUDPA_DEBUG(fmt, arg...) _RUDPA_PRINT("34",fmt,##arg)
#else
#define _RUDPA_DEBUG(fmt,arg...)
#endif

#if _RUDPA_DEBUG_LEVEL <= 2
#define _RUDPA_STUB(fmt, arg...) _RUDPA_PRINT("33",fmt,##arg)
#else
#define _RUDPA_STUB(fmt,arg...)
#endif


#if _RUDPA_DEBUG_LEVEL <= 3
#define _RUDPA_TRACE(fmt, arg...) _RUDPA_PRINT("32",fmt,##arg)
#else
#define _RUDPA_TRACE(fmt, arg...)
#endif


#if _RUDPA_DEBUG_LEVEL <= 4
#define _RUDPA_ERROR(fmt, arg...) _RUDPA_PRINT("31",fmt,##arg)
#else
#define _RUDPA_ERROR(fmt, arg...)

#endif

#define _RUDPA_ASSERT(exp, fmt, arg...) \
	{\
	if(!(exp)){\
				printf("[RUDPA | %s:%d|%s]",basename(__FILE__),__LINE__,__FUNCTION__);\
				printf(fmt, ##arg);\
				printf("\r\n");\
			}\
	}

#define DBG(fmt,arg...) _RUDPA_DEBUG(fmt,##arg)

#define TRACE(fmt,arg...) _RUDPA_TRACE(fmt,##arg)

#endif
