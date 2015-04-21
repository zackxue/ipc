


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "http_util.h"
#include "spook/spook.h"

#ifndef WEB_SERVER_H_
#define WEB_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kWEBS_SERVER_NAME "JAWS/1.0"
#define kWEBS_USER_AGENT_NAME kWEBS_SERVER_NAME" "__DATE__

#define kWEBS_KEEP_ALIVE_DURATION (60)

extern int WEBS_init(const char* resource_dir);
extern void WEBS_destroy();

extern SPOOK_SESSION_PROBE_t WEBS_probe(const void* msg, size_t msg_sz);
extern SPOOK_SESSION_LOOP_t WEBS_loop(bool* trigger, int sock, time_t* read_pts);


#ifdef __cplusplus
}
#endif
#endif //WEB_SERVER_H_

