
#ifndef __VLOG_H__
#define __VLOG_H__

#include <stdio.h>
#include <stdarg.h>
//#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    /* Enable this to get full debugging output */
    /* #define _DEBUG */

#ifdef _DEBUG
#undef NODEBUG
#endif

typedef enum
{ 
	VLOG_CRIT=0,
	VLOG_ERROR,
	VLOG_WARNING,
	VLOG_INFO,
    VLOG_DEBUG,
    VLOG_DEBUG2,
    VLOG_ALL
} VLOG_Level_t;

extern VLOG_Level_t debuglevel;

typedef void (fVLOG_Callback)(int level, const char *fmt, va_list);
void VLOG_SetCallback(fVLOG_Callback *cb);
void VLOG_SetOutput(FILE *file);

#ifdef __GNUC__
void VLOG_Printf(const char *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
void VLOG_Status(const char *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
void VLOG(int level, const char *format, ...) __attribute__ ((__format__ (__printf__, 2, 3)));
#else
void VLOG_Printf(const char *format, ...);
void VLOG_Status(const char *format, ...);
void VLOG(int level, const char *format, ...);
#endif
void VLOG_Hex(int level, const unsigned char *data, unsigned long len);
void VLOG_HexString(int level, const unsigned char *data, unsigned long len);
void VLOG_SetLevel(VLOG_Level_t lvl);
VLOG_Level_t VLOG_GetLevel(void);

#ifdef __cplusplus
}
#endif

#endif
