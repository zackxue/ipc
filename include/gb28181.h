#ifndef __GB28181__H__
#define __GB28181__H__

#ifdef __cplusplus
extern "C"
{
#endif

#define GB28181_DEFAULT_DEV_PORT	(5060)

#define GB28181_MAX_VIDEO_CHN				(32)
#define GB28181_MAX_ALARM_CHN				(16)
typedef struct _gb28181_conf
{
	char SipServerIp[20];
	unsigned short SipServerPort;
	char SipServerId[32];
	char LoginDomain[32];
	unsigned int AliveTime;
	unsigned int HeartBeatTime;
	//
	char UserId[32];
	char UserPwd[32];
	//
	int VideoNum;
	int AlarmNum;
	char VideoId[GB28181_MAX_VIDEO_CHN][32];
	char AlarmId[GB28181_MAX_ALARM_CHN][32];
}Gb28181Conf_t;


int GB28181_start();
int GB28181_stop();
int GB28181_resart();
int GB28181_configure(const void *str);

#ifdef __cplusplus
}
#endif
#endif

