
#ifndef __LIVEVIEW_H__
#define __LIVEVIEW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>


extern int LVIEW_init();
extern void LVIEW_destroy();
extern void LVIEW_run();
extern void LVIEW_stop();
extern void LVIEW_apply();
#ifdef __cplusplus
};
#endif
#endif //__LIVEVIEW_H__

