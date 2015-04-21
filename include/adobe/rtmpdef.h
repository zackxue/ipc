#ifndef __RTMP_DEF_H__
#define __RTMP_DEF_H__

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>

//#define RTMP_STREAM_NOCPY

#define	FLV_VIDEO_TAG_HEADER_SIZE	(5)
#define FLV_AUDIO_TAG_HEADER_SIZE	(1)


#define RTMP_RET_OK			(0)
#define RTMP_RET_FAIL			(-1)
#define RTMP_TRUE				(1)
#define RTMP_FALSE				(0)

#ifndef NULL
#define NULL ((void *)0)
#endif

#endif

