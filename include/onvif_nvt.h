
#ifndef __ONVIF_NVT_H__
#define __ONVIF_NVT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "spook/spook.h"

extern SPOOK_SESSION_PROBE_t ONVIF_nvt_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t ONVIF_nvt_loop(uint32_t* trigger, int sock, time_t* read_pts);

#ifdef __cplusplus
};
#endif

#endif //__ONVIF_NVT_H__

