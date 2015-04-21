#ifndef __JASTCONF_H__
#define __JASTCONF_H__

#ifdef __cplusplus
extern "C" {
#endif	

#include "gnu_win.h"

#define JAST_RET_OK		(0)
#define JAST_RET_FAIL	(-1)

//#define JAST_USE_BROADCAST

#ifndef true
#define true	(1)
#endif
#ifndef false
#define false	(0)
#endif

#ifndef TRUE
#define TRUE	(1)
#endif
#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int 	uint32_t;
#ifdef _WIN32
typedef unsigned __int64 uint64_t;
#else
typedef unsigned long long uint64_t;
#endif


#ifdef __cplusplus
}
#endif
#endif

