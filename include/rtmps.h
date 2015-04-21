#ifndef	RTMPS_H_
#define	RTMPS_H_

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

extern SPOOK_SESSION_PROBE_t RTMPS_probe(const void *msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t RTMPS_loop(uint32_t *trigger, int sock, time_t *read_pts, const void *msg, ssize_t msg_sz);



#endif
