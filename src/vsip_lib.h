
#ifndef VSIP_LIB_H_
#define VSIP_LIB_H_S
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>

extern int VSIPLIB_init(const char* eth);
extern void VSIPLIB_destroy();

#ifdef __cplusplus
}
#endif
#endif //VSIP_LIB_H_

