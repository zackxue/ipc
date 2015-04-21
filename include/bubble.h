
#ifndef __BUBBLE_HEAD_FILE__
#define __BUBBLE_HEAD_FILE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "spook.h"
#include "httpd.h"


typedef enum{
	CMD_PTZ_UP = 0,
	CMD_PTZ_DOWN,
	CMD_PTZ_LEFT,
	CMD_PTZ_RIGHT,
	CMD_PTZ_AUTO,
	CMD_PTZ_FOCUSFAR,
	CMD_PTZ_FOCUSNEAR,
	CMD_PTZ_ZOOMIN,
	CMD_PTZ_ZOOMOUT,
	CMD_PTZ_IRISOPEN,
	CMD_PTZ_IRISCLOSE,
	CMD_PTZ_AUX1,
	CMD_PTZ_AUX2,

	CMD_PTZ_CNT,
}REMOTE_PTZ_CMD;


extern SPOOK_SESSION_PROBE_t BUBBLE_probe(const void *msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t BUBBLE_loop(bool *trigger, int sock, time_t *read_pts);
extern int BUBBLE_over_http_cgi(HTTPD_SESSION_t* http_session);

extern int BUBBLE_init();
extern void BUBBLE_destroy();

#endif

