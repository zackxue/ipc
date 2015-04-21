
#ifndef __BUBBLE_H__
#define __BUBBLE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "spook.h"
#include "httpd.h"

// for spook session entry
extern SPOOK_SESSION_PROBE_t BUBBLE_probe(const void *msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t BUBBLE_loop(bool *trigger, int sock, time_t *read_pts);

// for http cgi entry
extern int BUBBLE_over_http_cgi(HTTPD_SESSION_t* http_session);

#endif //__BUBBLE_H__
