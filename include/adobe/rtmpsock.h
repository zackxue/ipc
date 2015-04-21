#ifndef __RTMP_SOCK_H__
#define __RTMP_SOCK_H__

#include "rtmplib.h"

int RTMP_SOCK_init(int fd);
int RTMP_SOCK_init2(char *ip,int port);
int RTMP_SOCK_init3();
int RTMP_SOCK_read(Rtmp_t *r,char *buf,int size);
int RTMP_SOCK_write(Rtmp_t *r,char *buf,int size);

#endif
