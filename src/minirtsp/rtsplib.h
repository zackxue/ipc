/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtsplib.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : rtsp  utils , reference to rfc2326
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#ifndef __RTSPLIB_H__
#define __RTSPLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vlog.h"
#include "authentication.h"
#include "sdplib.h"
#include "rtplib.h"
#include "rtcplib.h"
#include "netstream.h"

/****************************************************************
* configure mirco
****************************************************************/
#define RTSP_BUF_SIZE			(1024*16)
#define RTSP_CLIENT_PORT_BEGIN	(5200)
#define RTSP_SERVER_PORT_BEGIN	(7200)
#define RTSP_CHANNEL_BEGIN		(0)

#define RTSP_USER_AGENT		"minirtsp (by kaga)"
#define RTSP_ALLOW_METHODS	"SETUP,OPTIONS,DESCRIBE,PLAY,TEARDOWN"
/*************************************************************************
* const micro relative to rtsp, must not modified
**************************************************************************/
#define RTSP_DEFAULT_PORT	(554)
#define RTSP_VERSION		"RTSP/1.0"

#define RTSP_MODE_PLAY		(0)
#define RTSP_MODE_RECORD	(1)

// transport
#define RTSP_RTP_OVER_UDP	(FALSE)
#define RTSP_RTP_OVER_RTSP	(TRUE)// interleaved mode
#define RTSP_RTP_AUTO		(2)// using rtp over udp first ,if failed try rtp over rtsp

// stream type
#define RTSP_STREAM_VIDEO	(0x01) 
#define RTSP_STREAM_AUDIO	(0x02)

typedef enum{
	RTSP_STATE_INIT,
	RTSP_STATE_READY,
	RTSP_STATE_PLAYING,
	RTSP_STATE_RECORDING,
	RTSP_STATE_CNT
}RtspState_t;

typedef enum{
	RTSP_METHOD_DESCRIBE = 0,
	RTSP_METHOD_ANNOUNCE,
	RTSP_METHOD_GET_PARAMETER,
	RTSP_METHOD_OPTIONS,
	RTSP_METHOD_PAUSE,
	RTSP_METHOD_PLAY,
	RTSP_METHOD_RECORD,
	RTSP_METHOD_REDIRECT,
	RTSP_METHOD_SETUP,	//8	
	RTSP_METHOD_SET_PARAMETER,
	RTSP_METHOD_TEARDOWN,
	RTSP_METHOD_CNT
}RtspMethod_t;

typedef struct _RtspPacket
{
	int cseq;
	int body_size;
	char *body;
}RtspPacket_t;

typedef struct _RtspTransport
{
	int b_interleavedMode;
	int transport;	// udp or tcp
	int client_port;
	int server_port;
	int channel;
	int cast_type;//unicast or multicast
	int work_mode;//record or play
	uint32_t ssrc;
}RtspTransport_t;

typedef struct _Rtsp
{
	// rtsp role , client or server
	int role;
	int stream_type; // invalid for rtsp player
	int buffer_time;//unit : ms, only valid for rtsp player
	int toggle;// run or not
	//
	int sock;
	int trigger;
	//uri
	char ip_me[20];
	int port;
	char stream[64];
	
	//transport
	char peername[20];
	int b_interleavedMode;
	int low_transport;	// udp or tcp
	int client_port;
	int server_port;
	int channel;
	int cast_type;//unicast or multicast
	int work_mode;//record or play
	uint32_t ssrc;
	
	RtspState_t state;
	char session_id[32];
	uint32_t session_timeout;

	int cseq;
	char allow_method[128];
	int payload_size;
	char payload[RTSP_BUF_SIZE];

	Authentication_t *auth;	
	int bLogin;		// login success or not
	char user[32];
	char pwd[32];
	//
	RtspStream_t s;

	SessionDesc_t *sdp;
	Rtp_t *rtp_video;
	Rtp_t *rtp_audio;
	Rtcp_t *rtcp_audio;
	Rtcp_t *rtcp_video;
}Rtsp_t;

// 
#define RTSP_MAX_STREAM		(64)
#define RTSP_MAX_STREAM_LEN	(16)
typedef struct _streamtable
{
	int entries;
	char stream[RTSP_MAX_STREAM][RTSP_MAX_STREAM_LEN+1];
	char media[RTSP_MAX_STREAM][RTSP_MAX_STREAM_LEN+1];
}RtspStreamTable_t;


// global interface for rtsp client and rtsp server
// rtsp player
extern Rtsp_t* RTSP_connect_server(char *url,// FORMAT: rtsp://ip:port/stream_name
	char *user,char *pwd, // user login
	int bInterleaved,//RTSP_RTP_OVER_UDP or RTSP_RTP_OVER_RTSP OR RTSP_RTP_OVER_AUTO
	int iBufferTime);// unit : millisecond
// rtsp server
extern Rtsp_t* RTSP_SERVER_init(int fd,// rtsp socket
	int bAudio);// send audio or not
//common
//extern int RTSP_init(Rtsp_t *r,int sock,int role);
extern int RTSP_destroy(Rtsp_t *r);

extern int RTSP_read_message(Rtsp_t *r);
extern int RTSP_parse_message(Rtsp_t *r,
	fRtpParsePacket fRTP,//for interleaved mode, only use for rtsp client,if not use,give it NULL
	fRtcpParsePacket fRTCP);// for interleaved mode, support for client ans server,it give it NULL,it would not parse rtcp packet
// rtsp stream table
extern int RTSP_add_stream(const char *stream_name,const char *media_name);
extern int RTSP_remove_stream(const char *stream_name);
extern int RTSP_find_stream(const char *stream_name,char *media_name);

#ifdef __cplusplus
}
#endif
#endif

