
#ifndef __LIVE555_H__
#define __LIVE555_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#define LIVE555_DEBUG (1)
#if LIVE555_DEBUG
#define LIVE555_SYNTAX "37;1;32"
#define LIVE555_TRACE(fmt, arg...) \
	do{\
		const char* bname = basename(strdupa(__FILE__));\
		printf("\033["LIVE555_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define LIVE555_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* bname = basename(strdupa(__FILE__));\
			printf("\033["LIVE555_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
			exit(1);\
		}\
	}while(0)

#else
#define LIVE555_TRACE(fmt...)
#define LIVE555_ASSERT(exp, fmt...)
#endif

extern int LIVE555_init(uint16_t port, const char* description);
extern void LIVE555_destroy();

extern int LIVE555_add_stream(const char* stream_name);
extern int LIVE555_remove_stream(const char* stream_name);

extern int LIVE555_start();
extern void LIVE555_stop();

#include "spook/spook.h"

extern SPOOK_SESSION_PROBE_t LIVE555_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t LIVE555_loop(bool* trigger, int sock, time_t* read_pts);

#ifdef _TEST_LIVE555_LITE
int main()
{
	LIVE555_init(554, "");
	LIVE555_start();
	getchar();
	getchar();
	LIVE555_stop();
	LIVE555_destroy();
	return 0;
}
#endif
#ifdef __cplusplus
}
#endif
#endif //__LIVE555_H__

