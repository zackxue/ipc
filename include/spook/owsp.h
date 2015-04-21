/*
 * protocol_owsp.h
 *
 *  Created on: 2012-6-13
 *      Author: ted
 */

#ifndef PROTOCOL_OWSP_H_
#define PROTOCOL_OWSP_H_

#include <stdint.h>

#include "spook/spook.h"

extern SPOOK_SESSION_PROBE_t OWSP_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t OWSP_loop(bool *trigger, int sock, time_t *read_pts);

#endif /* PROTOCOL_OWSP_H_ */

