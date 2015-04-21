/*
 * rtspd_rtcp.h
 *
 *  Created on: 2012-6-18
 *      Author: root
 */

#ifndef RTSPD_RTCP_H_
#define RTSPD_RTCP_H_

#include "spook/rtspd.h"

#define  	RTP_VERSION  2 
#define 	RTP_MAX_SDES 255 		/* maximum text length for SDES */

#define 	IP_UDP_HDR_SIZE  28
#define 	IP_TCP_HDR_SIZE  4

typedef enum {
	RTCP_SR = 200,
	RTCP_RR = 201,
	RTCP_SDES = 202,
	RTCP_BYE = 203,
	RTCP_APP = 204
} rtcp_type_t;

typedef enum {
	RTCP_SDES_END = 0,
	RTCP_SDES_CNAME = 1,
	RTCP_SDES_NAME = 2,
	RTCP_SDES_EMAIL = 3,
	RTCP_SDES_PHONE = 4,
	RTCP_SDES_LOC = 5,
	RTCP_SDES_TOOL = 6,
	RTCP_SDES_NOTE = 7,
	RTCP_SDES_PRIV = 8
} rtcp_sdes_type_t;

// RTCP  common  header  word 
typedef struct {
	uint8_t count:5; /* varies by packet type */
	uint8_t p:1; /* padding flag */
	uint8_t version:2; /* protocol version */
	uint8_t pt:8; /* RTCP packet type */
	uint16_t length; /* pkt len in words, w/o this word */
} rtcp_common_t;

//Source  report  block 
typedef struct {
	uint32_t ssrc; /* data source being reported */
	uint32_t fraction:8; /* fraction lost since last SR/RR */
	uint32_t lost:24; /* cumul. no. pkts lost (signed!) */
	uint32_t last_seq; /* extended last seq. no. received */
	uint32_t jitter; /* interarrival jitter */
	uint32_t lsr; /* last SR packet from this source */
	uint32_t dlsr; /* delay since last SR packet */
} rtcp_src_t;

//SDES item
typedef struct {
	uint8_t type; /* type of item (rtcp_sdes_type_t) */
	uint8_t length; /* length of item (in octets) */
	char data[1]; /* text, not null-terminated */
} rtcp_sdes_item_t;

//One RTCP packet
typedef struct {
	rtcp_common_t common; /* common header */
	union {
		/* sender report (SR) */
		struct {
			uint32_t ssrc; /* sender generating this report */
			uint32_t ntp_sec; /* NTP timestamp */
			uint32_t ntp_frac;
			uint32_t rtp_ts; /* RTP timestamp */
			uint32_t psent; /* packets sent */
			uint32_t osent; /* octets sent */
			rtcp_src_t src[1]; /* variable-length list */
		} sr;
		/* Receiver report (RR) */
		struct {
			uint32_t ssrc; /* receiver generating this report */
			rtcp_src_t src[1]; /* variable-length list */
		} rr;
		/* source description (SDES) */
		struct rtcp_sdes {
			uint32_t ssrc; /* first SSRC/CSRC */
			rtcp_sdes_item_t item[1]; /* list of SDES items */
		} sdes;
		/* BYE */
		struct {
			uint32_t src[1]; /* list of sources */
			/* can't express trailing text for reason */
		} bye;
	};
} rtcp_t;

typedef struct rtcp_sdes rtcp_sdes_t;

// Per-source state information
typedef struct {
	uint16_t max_seq; /* highest seq. number seen */
	uint32_t cycles; /* shifted count of seq. number cycles */
	uint32_t base_seq; /* base seq number */
	uint32_t bad_seq; /* last 'bad' seq numbe r+1*/
	uint32_t probation; /* sequ. packets till source is valid */
	uint32_t received; /* packets received */
	uint32_t expected_prior; /* packet expected at last interval */
	uint32_t received_prior; /* packet received at last interval */
	uint32_t transit; /* relative trans time for prev pkt */
	uint32_t jitter; /* estimated jitter */
	/* ... */
} source;

extern void RTCP_init(RtspdSession_t* session);
extern int RTCP_handle(RtspdSession_t* session);
extern void RTCP_schedule(RtspdSession_t* session);

#endif /* RTSPD_RTCP_H_ */
