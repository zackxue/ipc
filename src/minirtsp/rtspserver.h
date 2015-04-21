#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif	


extern void *RTSPS_proc(void *arg);
//
extern int RTSPS_start();
extern void RTSPS_stop();
extern int RTSPS_test(int argc,char *argv[]);


#ifdef __cplusplus
}
#endif
#endif

