#ifndef __RTSPD_H__
#define __RTSPD_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NOCROSS
//#include "spook.h"
#include "spook/spook.h"

SPOOK_SESSION_PROBE_t MINIRTSP_probe(const void* msg, ssize_t msg_sz);
SPOOK_SESSION_LOOP_t MINIRTSP_loop(uint32_t* trigger, int sock, time_t* read_pts, const void* msg, ssize_t msg_sz);

#endif

/****************************************************************
* rtsp player 
*****************************************************************/
#define RTSP_PLAYER_RECONNECT
#define RTSP_PLAYLIST_MAX_ITEM	(32)
#define RTSP_PLAYITEM_MAX_CHAR	(100)

typedef struct _rtp_playitem
{
	char url[RTSP_PLAYITEM_MAX_CHAR];
	int transport; // rtp over udp or rtp over rtsp or auto detect
	int buffertime;
	int channel;// bind channel
	char user[32];
	char pwd[32];
	void *rtsp;	// rtsp instance
	int toggle;
}RtspPlayItem_t;

typedef struct _rtsp_playlist
{
	int entries;
	RtspPlayItem_t items[RTSP_PLAYLIST_MAX_ITEM];
}RtspPlayList_t;

extern int RTSP_PLAYLIST_add(char *url,char *user,char *pwd,int transport,int buffer_time,int channel);
extern int RTSP_PLAYLIST_remove(char *url);
extern int RTSP_PLAYER_start();
extern int RTSP_PLAYER_stop();
extern int RTSP_PLAYER_test();


#ifdef __cplusplus
}
#endif


#endif
