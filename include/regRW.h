#ifndef __REGRW_H_
#define __REGRW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "spook.h"

extern SPOOK_SESSION_PROBE_t SENSOR_REGRW_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t SENSOR_REGRW_loop(bool* trigger, int sock, time_t *read_pts);

#ifdef __cplusplus
};
#endif

#endif //__REGRW_H_
