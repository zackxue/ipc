
#ifndef __SYSTEM_STATUS_H__
#define __SYSTEM_STATUS_H__
#ifdef _cplusplus
extern "C" {
#endif

#include <stdio.h>

extern float cpu_get_status();

typedef struct _connect_status
{
	char client_ip[32];
	int stream_count;
	char stream_size[3][16];
	int connect_count;
	time_t connect_time_start;
	time_t last_connect_time;
}CONNECT_status_t;

typedef struct _system_status
{
	float cpu_use;
	float cpu_idle;
	int connect_count;
	CONNECT_status_t connect_status[16];
}SYSTEM_status_t;

#ifdef _cplusplus
};
#endif
#endif //__SYSTEM_STATUS_H__


