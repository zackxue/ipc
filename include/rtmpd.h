
#ifndef	__RTMPD_H__
#define	__RTMPD_H__
#ifdef __cpluscplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <netinet/tcp.h>

#include "spook/spook.h"

extern SPOOK_SESSION_PROBE_t RTMPD_probe(const void *msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t RTMPD_loop(bool* trigger, int sock, time_t *read_pts);

#ifdef __cpluscplus
};
#endif
#endif //__RTMPD_H__

