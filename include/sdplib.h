/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : sdplib.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : session description protocal  utils , reference to rfc4566
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/
#ifndef __SDPLIB_H__
#define __SDPLIB_H__
	
#ifdef __cplusplus
	extern "C" {
#endif


/***********************************************************************
*configure
***********************************************************************/
#define SDP_BUFFER_SIZE				(1024*5)
#define SDP_MAX_MEDIA_LEVEL			(2)
#define SDP_MAX_ATTR_NUM			(8)
#define SDP_MAX_ATTR_NAME_SIZE		(32)
#define SDP_MAX_ATTR_VALUE_SIZE		(256)
#define SDP_DEFAULT_SESSION_VER		(1)
#define SDP_DEFAULT_SESSION_NAME	"default session"
#define SDP_DEFAULT_SESSION_INFO	"minisdp"
#define	SDP_DEFAULT_EMAIL			"default@email.com (default)"
#define SDP_DEFAULT_PHONE			"88888888"
#define SDP_DEFAULT_VIDEO_CONTROL	"track=video"
#define SDP_DEFAULT_AUDIO_CONTROL	"track=audio"

/**********************************************************************
* sdp specification mirco
**********************************************************************/
#define SDP_MEDIA_TYPE				"application/sdp"
#define SDP_VERSION					"0"
#define SDP_NOTSUPPORT_USERID		"-"

#define SDP_NETTYPE_INTERNET		"IN"

#define SDP_ADDR_TYPE_IPV4			"IP4"
#define SDP_ADDR_TYPE_IPV6			"IP6"

#define SDP_MEDIA_TYPE_VIDEO		"video"
#define SDP_MEDIA_TYPE_AUDIO		"audio"
#define SDP_MEDIA_TYPE_TEXT			"text"
#define SDP_MEDIA_TYPE_APPLICATION	"application"
#define SDP_MEDIA_TYPE_MESSAGE		"message"

#define SDP_MEDIA_PROTOCAL_RTP_AVP	"RTP/AVP"
#define SDP_MEDIA_PROTOCAL_RTP_SAVP	"RTP/SAVP"

#define SDP_ENCRYPT_TYPE_CLEAR		"clear"		// clear:<encryption key>
#define SDP_ENCRYPT_TYPE_BASE64		"base64"	// base64:<encoded encryption key>
#define SDP_ENCRYPT_TYPE_URI		"uri"		// uri:<uri to obtain key>	
#define SDP_ENCRYPT_TYPE_PROMPT		"prompt"	// prompt   // no key

#define SDP_SPEC_VERSION			0
#define SDP_SPEC_ORIGIN				1
#define SDP_SPEC_SESSION_NAME		2
#define SDP_SPEC_SESSION_INFO		3
#define SDP_SPEC_URI				4
#define SDP_SPEC_EMAIL				5
#define SDP_SPEC_PHONE				6
#define SDP_SPEC_CONNECTION			7
#define SDP_SPEC_BANDWIDTH			8
#define SDP_SPEC_TIMING				9
#define SDP_SPEC_REPEAT_TIME		10
#define SDP_SPEC_TIME_ZONE			11
#define SDP_SPEC_ENCRYPT_KEY		12
#define SDP_SPEC_ATTRIBUTE			13
#define SDP_SPEC_MEDIA_DESC			14
#define SDP_SPEC_SSRC				15
#define SDP_SPEC_CNT				16


#define SDP_ATTR_CATEGORY			(0)
#define SDP_ATTR_KEY_WORDS			(1)
#define SDP_ATTR_TOOL				(2)
#define SDP_ATTR_PACKET_TIME		(3)
#define SDP_ATTR_MAX_PTIME			(4)
#define SDP_ATTR_RTP_MAP			(5)
#define SDP_ATTR_RECVONLY			(6)
#define SDP_ATTR_SENDRECV			(7)
#define SDP_ATTR_SENDONLY			(8)
#define SDP_ATTR_INACTIVE			(9)
#define SDP_ATTR_ORIENTATION		(10)
#define SDP_ATTR_CONFERENCE_TYPE	(11)
#define SDP_ATTR_CHARSET			(12)
#define SDP_ATTR_SDP_LANG			(13)
#define SDP_ATTR_LANG				(14)
#define SDP_ATTR_FRAME_RATE			(15)
#define SDP_ATTR_QUALITY			(16)
#define SDP_ATTR_FMTP				(17)
#define SDP_ATTR_CONTROL			(18)
#define SDP_ATTR_RANGE				(19)
#define SDP_ATTR_ENTITY_TAG			(20)
#define SDP_ATTR_FRAME_SIZE			(21)

#define SDP_PAYLOAD_TYPE_DYNAMIC	(96)
#define SDP_PAYLOAD_TYPE_ULAW		(0)
#define SDP_PAYLOAD_TYPE_ALAW		(8)


#define SDP_MEDIA_H264_FREQ			(90000)
#define SDP_MEDIA_G711_FREQ			(8000)

typedef struct _fmtp_attr
{
	unsigned short type;
	unsigned short packet_mode;
	unsigned char profile_levle_id[4];// 3 bytes
	int sps_size;
	unsigned char sps[64];
	int pps_size;
	unsigned char pps[64];
}FmtpAttr_t;

typedef struct _rtpmap_attr
{
	int payload_type;
	char codec_type[16];
	unsigned int freq;
}RtpMapAttr_t;

typedef struct _attrubute
{
	char name[SDP_MAX_ATTR_NAME_SIZE];
	union{
		char value[SDP_MAX_ATTR_VALUE_SIZE];
		FmtpAttr_t fmtp;
		RtpMapAttr_t rtpmap;
	};
}Attribute_t;

typedef struct _media_desc
{
	unsigned int spec_flag;
	// m= <media> <port> <proto> <fmt> ...
	struct{
		char type[16];
		int port;
		char protocal[16];
		int format;
	}media_n;
	// i=*
	char media_info[128];
	// c=*
	struct{
		char nettype[8];
		char addrtype[8];
		char addr[32];
	}conn_info;
	// k=*
	char encryt_key[128];
	// y=*
	unsigned int ssrc;
	// a=*
	int attr_num;
	Attribute_t attri[SDP_MAX_ATTR_NUM];
}MediaDesc_t;

typedef struct _session_desc
{	
	char *buffer;
	unsigned int spec_flag;
	// v=
	char version[8];
	// o= <username> <session id> <session version> <net type> <address type> <unicast-address>
	struct{
		char user_name[32];
		unsigned int session_id;
		int session_ver;
		char nettype[8];
		char addrtype[8];
		char addr[20];
	}originer;
	// s=
	char session_name[32];
	// i=*
	char session_info[128];
	// u=*
	char uri[128];
	// e=* <email> {(user name)}
	char email[64];
	// p=*
	char phone[32];
	// c=* <nettype> <addrtype> <connect-address>
	struct{
		char nettype[8];
		char addrtype[8];
		char addr[20];
	}conn_info;
	// b=*  //zero or more bandwidth information lines
	char bandwidth[128];
	// z=*
	char zone[32];
	// k=*
	struct{
		char key[128];
	}encrypt_key;
	// a=*
	int attr_num;
	Attribute_t attri[SDP_MAX_ATTR_NUM];
	// t= <start time> <end time>
	struct{
		unsigned int start;
		unsigned int end;
	}time_active;
	// r=*
	char repeat_time[64];
	// m=*
	int media_num;
	MediaDesc_t media[SDP_MAX_MEDIA_LEVEL];
}SessionDesc_t;

extern SessionDesc_t *SDP_new_default(char *session_name,char *ip);// new a sdp ,without media description
extern int SDP_add_h264(SessionDesc_t *sdp,char *control);// add a media type of h264 to existing sdp
extern int SDP_add_g711(SessionDesc_t *sdp,char *control);// add a media type of g711 to existing sdp
extern int SDP_encode(SessionDesc_t *sdp);  // genrate the session description string from a sdp
extern SessionDesc_t* SDP_decode(char *src);// generate a instance of sdp from sdp session description string
extern int SDP_get_attr(SessionDesc_t *sdp,int attr,void *out);// out must type of Attrubute_t
extern int SDP_get_media_attr(SessionDesc_t *sdp,char *media_type,int attr,void *out);// out must type of Attrubute_t
extern int SDP_cleanup(SessionDesc_t *sdp); // destroy an existing sdp
//
extern unsigned int SDP_get_ssrc(SessionDesc_t *sdp);
extern int SDP_get_h264_info(SessionDesc_t *sdp,int *payloadtype,char *ip,int *port);

#ifdef __cplusplus
}
#endif
#endif


