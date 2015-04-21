#ifndef __RTSP_CLIENT_H__
#define __RTSP_CLIENT_H__

#define RTSPC_STATE_INIT				(0)
#define RTSPC_STATE_BUFFER_FILLING		(1)
#define RTSPC_STATE_BUFFER_AVAILABLE	(2)


extern void *RTSPC_NETWORK_proc(void *param);
extern void *RTSPC_DECODE_proc(void *param);
extern int RTSPC_daemon(void **rtsp,char *url,char *user,char *pwd,int bInterleaved,int buffer_time,int chn,int *trigger);
extern int RTSPC_test(int argc,char *argv[]);

#endif


