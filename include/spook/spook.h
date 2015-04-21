
#ifndef __SPOOK_H__
#define __SPOOK_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

typedef enum SPOOK_SESSION_PROBE
{
	SPOOK_PROBE_NULL = -1,
	SPOOK_PROBE_MATCH = 0,
	SPOOK_PROBE_UNDETERMIN,
	SPOOK_PROBE_MISMATCH,
}SPOOK_SESSION_PROBE_t;

typedef enum SPOOK_SESSION_LOOP
{
	SPOOK_LOOP_FAILURE = -1,
	SPOOK_LOOP_SUCCESS = 0,
}SPOOK_SESSION_LOOP_t;

typedef SPOOK_SESSION_PROBE_t (*SPOOK_TASK_PROBE_FUNC)(const void* msg, ssize_t msg_sz);
typedef SPOOK_SESSION_LOOP_t (*SPOOK_TASK_LOOP_FUNC)(bool* trigger, int sock, time_t* pts);

extern int SPOOK_init(uint32_t port);
extern void SPOOK_destroy();

extern int SPOOK_add_session(const char* name, SPOOK_TASK_PROBE_FUNC do_probe, SPOOK_TASK_LOOP_FUNC do_loop);
extern int SPOOK_remove_session(const char* name);

extern void SPOOK_addrlist_as_black();
extern void SPOOK_addrlist_as_white();

extern bool SPOOK_addrlist_check(const char* addr);
extern int SPOOK_addrlist_add(const char* addr);
extern void SPOOK_addrlist_remove(const char* addr);
extern void SPOOK_addrlist_clear();

#ifdef _TEST_SPOOK

SPOOK_SESSION_PROBE_t DUMMY_probe(const void* msg, ssize_t msg_sz)
{
	if(0 == strncasecmp(msg, "dummy", strlen("dummy"))){
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t DUMMY_loop(uint32_t* trigger, int sock, time_t* read_pts, const void* msg, ssize_t msg_sz)
{
	int count = 10;
	do
	{
		*read_pts = time(NULL);
		DUMMY_TRACE("dummy is laughing:\"hohoho!\" @ %u", (uint32_t)*read_pts);
		sleep(1);
	}while(*trigger && count--);
	return SPOOK_LOOP_SUCCESS;
}


int main(int argc, char** argv)
{
	
	SPOOK_init(10080);
	SPOOK_add_session("dummy", DUMMY_probe, DUMMY_loop);

	printf("== Please press any keys twice to quit ==\r\n");
	getchar();
	getchar();

	SPOOK_remove_session("dummy");
	SPOOK_destroy();
	return 0;
}
#endif

#ifdef __cplusplus
};
#endif
#endif //__SPOOK_H__

