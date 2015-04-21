/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtcplib.h
  Version       : Initial Draft
  Author        : kejiazhw@gmail.com(kaga)
  Created       : 2013/04/25
  Last Modified : 2013/04/25
  Description   : Real Time Control Protocal  utils , reference to rfc3605
 
  History       : 
  1.Date        : 2013/04/25
    	Author      : kaga
 	Modification: Created file	
******************************************************************************/

#ifndef __RTCPLIB_H__
#define __RTCPLIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rtspdef.h"
#include "rtplib.h"

/**************************************************************************
* configure mirco
**************************************************************************/
#define RTCP_MAX_SRC 			(1)
#define RTCP_MAX_PACKETS 		(3)
#define RTCP_MAX_DES_ITEMS		(8) /* we must configure this manually */
#define RTCP_DEFAULT_SDES_TOOL			"minirtsp"

/*************************************************************************
* const micro relative to rtcp, must not modified
**************************************************************************/
#define RTCP_VERSION		(2)

//rtcp packet types
#define RTCP_PACKET_FIR		(192)//full intra-frame request
#define RTCP_PACKET_NACK	(193)//negative acknowledgement
#define RTCP_PACKET_SR		(200)//sender report
#define RTCP_PACKET_RR		(201)//reciever report
#define RTCP_PACKET_SDES	(202)//source description
#define RTCP_PACKET_BYE		(203)//indeicate end of participation
#define RTCP_PACKET_APP		(204)//application specific functions
#define RTCP_PACKET_XR		(207)//rtcp extension

#define RTCP_MIN_TRANS_FREQ	(5000)//it should increase as number of receivers increases

//rtcp description items
#define RTCP_SDES_CNAME		(1)
#define RTCP_SDES_NAME		(2)
#define RTCP_SDES_EMAIL		(3)
#define RTCP_SDES_PHONE		(4)
#define RTCP_SDES_LOC		(5)
#define RTCP_SDES_TOOL		(6)
#define RTCP_SDES_NOTE		(7)
#define RTCP_SDES_PRIV		(8)


// RTCP  common  header 
typedef struct _rtcp_common{
	uint8_t count:5; /* varies by packet type */
	uint8_t p:1; /* padding flag */
	uint8_t version:2; /* protocol version */
	uint8_t pt; /* RTCP packet type */
	uint16_t length; /* pkt len in words, w/o this word */
}rtcp_common_t;


//Source  report  block 
typedef struct _rtcp_src{
	uint32_t ssrc; /* data source being reported */
	uint32_t fraction:8; /* fraction lost since last SR/RR */
	uint32_t lost:24; /* cumul. no. pkts lost (signed!) */
	uint32_t last_seq; /* extended last seq. no. received */
	uint32_t jitter; /* interarrival jitter */
	uint32_t lsr; /* last SR packet from this source */
	uint32_t dlsr; /* delay since last SR packet */
}rtcp_src_t;

//SDES item
typedef struct _rtcp_sdes_item{
	uint8_t type; /* type of item (rtcp_sdes_type_t) */
	uint8_t length; /* length of item (in octets) */
	char data[32]; /* text, not null-terminated */
} rtcp_sdes_item_t;


typedef struct _rtcp_sr
{
	rtcp_common_t common; /* common header */
	uint32_t ssrc; /* sender generating this report */
	uint32_t ntp_sec; /* NTP timestamp */
	uint32_t ntp_frac;
	uint32_t rtp_ts; /* RTP timestamp */
	uint32_t psent; /* packets sent */
	uint32_t osent; /* octets sent */
	rtcp_src_t src[RTCP_MAX_SRC]; /* variable-length list */
}RtcpSR_t;

typedef struct _rtcp_rr
{
	rtcp_common_t common; /* common header */
	uint32_t ssrc; /* receiver generating this report */
	rtcp_src_t src[RTCP_MAX_SRC]; /* variable-length list */
}RtcpRR_t;

typedef struct _rtcp_sdes
{
	rtcp_common_t common; /* common header */
	uint32_t ssrc; /* first SSRC/CSRC */
	rtcp_sdes_item_t item[RTCP_MAX_DES_ITEMS]; /* list of SDES items */
}RtcpSDES_t;

typedef struct _rtcp_bye
{
	rtcp_common_t common; /* common header */
	uint32_t src[RTCP_MAX_SRC]; /* list of sources */
}RtcpBYE_t;


//One RTCP packet
typedef struct _rtcp_packet
{
	union {
		RtcpSR_t sr;
		RtcpRR_t rr;
		RtcpSDES_t sdes;
		RtcpBYE_t bye;
	};
}RtcpPacket_t;

#define RTCP_ONE_PACKET_SIZE	(sizeof(RtcpPacket_t))

typedef struct __rtcp_packets
{
	int cnt;
	//char buffer[RTCP_MAX_PACKETS][RTCP_ONE_PACKET_SIZE];
	void *buffer;
	int buf_size[RTCP_MAX_PACKETS];
}RtcpPacketEx_t;


typedef struct __rtcp
{
	int role;
	int bAlive;
	// transport
	int interleaved;
	int sock;
	int chn_port_c;
	int chn_port_s;
	char peername[20];
	//
	uint32_t ssrc;
	uint32_t src_id[1];
	//
	Rtp_t *rtp;
	uint32_t last_sr_ts;  		// to mark the sender report's timestamp
								// use to compute the round-trip time
	MillisecondTimer_t rtt_timer; // to mark the time of received SR(receiver)
								// or to mark the time to sending SR(sender)
								// use for compute the round-trip time
	//MilliSecond_t last_recv_sr_ts;	// to mark the time  received SR(receiver)
									// use for compute jitter
	MillisecondTimer_t jitter_timer; // use for compute  jitter	
								// to mark the time between two SR(sender)
								// to mark the time between receiving two SR(reiceiver)
	uint32_t jitter;
	MillisecondTimer_t trans_timer; // use to control the rtcp packet transmission interval
	//
	RtcpPacketEx_t packet;
}Rtcp_t;

typedef int (*fRtcpParsePacket)(Rtcp_t *rtcp,void *payload,int size);


extern int RTCP_init(Rtcp_t **r,
	int role,	// act as client or server
	uint32_t src_id,// src id
	int protocal,/* udp or tcp */
	int cast_type,/* unicast or multicast */
	int interleaved, /* true or false */
	int rtsp_sock, /* if interleaved , rtp sock use this */
	int chn_port_s,/* rtp port or channel if in interleaved mode */
	int chn_port_c,
	Rtp_t *rtp);
extern int RTCP_destroy(Rtcp_t *rtcp);

//extern int RTCP_add_source(Rtcp_t *rtcp);
//extern int RTCP_del_source(Rtcp_t *rtcp);

extern int RTCP_handle_packet(Rtcp_t *rtcp,void *payload,int payload_size);
extern int RTCP_process(Rtcp_t *rtcp);

#if RTSP_ENABLE_RTCP == TRUE
	#define RTCP_HANDLE	RTCP_handle_packet
#else
	#define RTCP_HANDLE	NULL
#endif

#ifdef __cplusplus
}
#endif
#endif

