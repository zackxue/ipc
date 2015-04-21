/******************************************************************************

  Copyright (C), 2013-2020, GuangZhou JUAN Electronics Co., Ltd.

 ******************************************************************************
  File Name    : rtcplib.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sock.h"
#include "rtspdef.h"
#include "rtcplib.h"
#include "vlog.h"


static const char * const rtcpDesItems[]=
{
	"",
	"CNAME",
	"NAME",
	"EMAIL",
	"PHONE",
	"LOC",
	"TOOL",
	"NOTE",
	"PRIV",
};

//extern int rtp_init_transport(int cast_type,int protocal,int chn_port);
int rtp_init_transport(int cast_type,int protocal,int chn_port)
{
	int sock=-1;
	if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_UNICAST){
		sock=SOCK_udp_init(chn_port,RTSP_SOCK_TIMEOUT);
	}else if(protocal == RTP_TRANSPORT_TCP && cast_type==RTP_UNICAST){
		VLOG(VLOG_DEBUG,"rtsp over tcp");
	}else if(protocal == RTP_TRANSPORT_UDP && cast_type==RTP_MULTICAST){
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}else{
		VLOG(VLOG_ERROR,"unsupport transport: %s,%s",(protocal == RTP_TRANSPORT_TCP) ? "tcp" : "udp",
			(cast_type==RTP_MULTICAST) ? "multicast" : "unicast");
	}
	return sock;
}


static inline uint16_t decode_int16le(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    unsigned int val;

    val = (c[1] << 8) | c[0];
    return val;
}

static inline int encode_int16le(char *output, int nVal) 
{
    output[0] = nVal;
    nVal >>= 8;
    output[1] = nVal;
    return 2;
}

static inline uint32_t decode_int24le(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    uint32_t val;

    val = (c[2] << 16) | (c[1] << 8) | c[0];
    return val;
}

static inline int encode_int24le(char *output, int nVal) 
{
    output[0] = nVal;
    nVal >>= 8;
    output[1] = nVal;
    nVal >>= 8;
    output[2] = nVal;
    return 3;
}

static inline uint32_t decode_int32le(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    uint32_t val;

    val = (c[3] << 24) | (c[2] << 16) | (c[1] << 8) | c[0];
    return val;
}

static inline int encode_int32le(char *output, int nVal) 
{
    output[0] = nVal;
    nVal >>= 8;
    output[1] = nVal;
    nVal >>= 8;
    output[2] = nVal;
    nVal >>= 8;
    output[3] = nVal;
    return 4;
}

static inline uint16_t decode_int16be(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    unsigned int val;

    val = c[1] | (c[0] << 8);
    return val;
}

static inline int encode_int16be(char *output, int nVal) 
{
    output[1] = nVal;
    nVal >>= 8;
    output[0] = nVal;
    return 2;
}

static inline uint32_t decode_int24be(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    uint32_t val;

    val = (c[2]) | (c[1] << 8) | (c[0] << 16);
    return val;
}

static inline int encode_int24be(char *output, int nVal) 
{
    output[2] = nVal;
    nVal >>= 8;
    output[1] = nVal;
    nVal >>= 8;
    output[0] = nVal;
    return 3;
}

static inline uint32_t decode_int32be(const char *data) 
{
    unsigned char *c = (unsigned char *)data;
    uint32_t val;

    val = (c[3]) | (c[2] << 8) | (c[1] << 16) | (c[0] << 24);
    return val;
}

static inline int encode_int32be(char *output, int nVal) 
{
    output[3] = nVal;
    nVal >>= 8;
    output[2] = nVal;
    nVal >>= 8;
    output[1] = nVal;
    nVal >>= 8;
    output[0] = nVal;
    return 4;
}

static int rtcp_compute_roundtrip_time(Rtcp_t *rtcp,int delay/*[MSB16:seconds,LSB16:fraction second*/)
{
	int rtt=0;
	MillisecondTimer_t t_tmp;
	MilliSecond_t t_lasttime;
	int dms=0;
	dms = ((delay >> 16) & 0xffff)*1000 + (delay & 0xffff)*1000/0xffff;
	if(MilliTimerIsClear(rtcp->rtt_timer) == false){
		MilliTimerStop(rtcp->rtt_timer,t_tmp,t_lasttime);
		rtt = t_lasttime - dms;
	}else{
		rtt = 0;
	}
	VLOG(VLOG_CRIT,"RTT is: %d ms (lasttime:%d delay:%d(%d ms))",rtt,t_lasttime,delay,dms);
	return rtt;
}

static int rtcp_compute_jitter(Rtcp_t *rtcp,uint32_t sr_ts)
{
	int dev=0; // unit:ms
	MillisecondTimer_t t_tmp;
	MilliSecond_t t_lasttime;
	uint32_t old_time;
	if(MilliTimerIsClear(rtcp->jitter_timer) == false){
		MilliTimerStop(rtcp->jitter_timer,t_tmp,t_lasttime);
		//dev = (((sr_ts >> 16) & 0xffff) - ((rtcp->last_sr_ts >> 16) & 0xffff))*1000+
		//	(((sr_ts & 0xffff) - (rtcp->last_sr_ts & 0xffff))<<16)*1000/0xFFFFFFFF;
		old_time = (((sr_ts >> 16) & 0xffff) - ((rtcp->last_sr_ts >> 16) & 0xffff))*1000+
			(((short)(sr_ts & 0xffff) - (short)(rtcp->last_sr_ts & 0xffff)) & 0xffff)*1000/0xFFFF;
		dev = t_lasttime - old_time;
		if(dev < 0) dev = -dev;
		rtcp->jitter += ((double)dev-rtcp->jitter)*1.0/16.0;
	}else{
		rtcp->jitter = 0;
	}
	// restart timer
	MilliTimerStart(rtcp->jitter_timer);
	VLOG(VLOG_CRIT,"jitter is: %d (dev:%d old:%d lastt:%d) ts(%#10x->%#10x)",
		rtcp->jitter,dev,t_lasttime,old_time,rtcp->last_sr_ts,sr_ts);
	return 0;
}

static void rtcp_compute_lost(Rtcp_t *rtcp,int srPs,int srOctect)
{
	int lost_ps;
	lost_ps = rtcp->rtp->packet_cnt - srPs;
	rtcp->rtp->comulative_lost = lost_ps;
	rtcp->rtp->fraction_lost = lost_ps*0xFF/srPs;
	VLOG(VLOG_CRIT,"comulative lost:%d fraction lost:%d lost_octet:%d",
		rtcp->rtp->comulative_lost,rtcp->rtp->fraction_lost,srOctect-rtcp->rtp->octet_cnt);
}

static void rtcp_packet_interleaved_header(char *buffer,int chn,int size)
{
	RtspInterHeader_t *interHeader=(RtspInterHeader_t *)buffer;
	interHeader->magic = RTSP_INTERLEAVED_MAGIC;
	interHeader->length = htons(size);
	interHeader->channel = chn;
}

static int rtcp_sr_request(Rtcp_t *rtcp,char *buf_in,int *out_size)
{
	RtcpPacket_t *p=NULL;
	RtspInterHeader_t *interHeader=NULL;
	rtcp_common_t *common=NULL;
	char *ptr = buf_in;
	MillisecondTimer_t t_now;
	MilliSecond_t t_sec,t_usec;
	if(rtcp->interleaved == true){
		interHeader = (RtspInterHeader_t *)buf_in;
		ptr += sizeof(RtspInterHeader_t);
	}
	common = (rtcp_common_t *)ptr;
	common->version = RTCP_VERSION;
	common->p = 0;
	common->count = 0;
	common->pt = RTCP_PACKET_SR;
	ptr+=sizeof(rtcp_common_t);
	// sender ssrc
	ptr += encode_int32be(ptr,rtcp->src_id[0]);
	// timestamp of sending this packet
	MilliTimerStart(t_now);
	MilliTimerGet(t_now,t_sec,t_usec);
	ptr += encode_int32be(ptr,t_sec);
	ptr += encode_int32be(ptr,0xFFFFFFFF/1000000*t_usec);
	// 
	
	//printf("2 sended ps:%u octet:%u\n",rtcp->rtp->packet_cnt,rtcp->rtp->octet_cnt);
	ptr += encode_int32be(ptr,rtcp->rtp->timestamp);
	ptr += encode_int32be(ptr,rtcp->rtp->packet_cnt);
	ptr += encode_int32be(ptr,rtcp->rtp->octet_cnt);
	
	// compute size
	common->length = ptr-(char *)common;
	common->length = htons((common->length + 3)/4 - 1);
	*out_size = (ptr - buf_in + 3)/4*4;
	
	// if interleaved mode,packet interleaved header
	if(rtcp->interleaved == true){
		interHeader->magic = RTSP_INTERLEAVED_MAGIC;
		interHeader->channel = rtcp->chn_port_c;
		interHeader->length = htons(ptr-(char *)common);
	}

	// start rtt timer
	MilliTimerStart(rtcp->rtt_timer);

	return 0;
}

static int rtcp_rr_request(Rtcp_t *rtcp,char *buf_in,int *out_size)
{
	RtcpPacket_t *p=NULL;
	RtspInterHeader_t *interHeader=NULL;
	rtcp_common_t *common=NULL;
	char *ptr = buf_in;
	int lost_cnt = 0;
	int lost_rate = 0;
	MillisecondTimer_t t_tmp;
	MilliSecond_t delays;
	uint32_t delay_since_last_sr;
	
	if(rtcp->interleaved == true){
		interHeader = (RtspInterHeader_t *)buf_in;
		ptr += sizeof(RtspInterHeader_t);
	}
	common = (rtcp_common_t *)ptr;
	common->version = RTCP_VERSION;
	common->p = 0;
	common->count = 1;
	common->pt = RTCP_PACKET_RR;
	ptr+=sizeof(rtcp_common_t);
	// ssrc
	ptr += encode_int32be(ptr,rtcp->ssrc);
	// source 1
	//id
	ptr += encode_int32be(ptr,rtcp->src_id[0]);
	// lost fraction and comulative lost count until last SR received
	*ptr++ = rtcp->rtp->fraction_lost;
	ptr += encode_int24be(ptr,(rtcp->rtp->comulative_lost == 0) ? 0xffffff : rtcp->rtp->comulative_lost);
	ptr += encode_int16be(ptr,rtcp->rtp->cycle_cnt);
	ptr += encode_int16be(ptr,rtcp->rtp->seq);
	// jitter , computed whiling recving SR
	ptr += encode_int32be(ptr,rtcp->jitter);
	//
	ptr += encode_int32be(ptr,rtcp->last_sr_ts);
	// compute delay since last SR ,32bits, MSB 16b:seconds, LSB16b: fraction second
	if(MilliTimerIsClear(rtcp->rtt_timer)==true)
		delays = 0;
	else
		MilliTimerStop(rtcp->rtt_timer,t_tmp,delays);
	delay_since_last_sr = ((delays/1000) << 16) | (0xFFFF/1000*(delays%1000));
	ptr += encode_int32be(ptr,delay_since_last_sr);
	//printf("delay since lasy SR:%d ms(%#10x)\n",delays,delay_since_last_sr);
	
	// compute size
	common->length = ptr-(char *)common;
	common->length = htons((common->length + 3)/4 - 1);
	*out_size = (ptr - buf_in + 3)/4*4;
	
	// if interleaved mode,packet interleaved header
	if(rtcp->interleaved == true){
		interHeader->magic = RTSP_INTERLEAVED_MAGIC;
		interHeader->channel = rtcp->chn_port_c;
		interHeader->length = htons(ptr-(char *)common);
	}

	return RTSP_RET_OK;
}

static int rtcp_sdes_request(Rtcp_t *rtcp,char *buf_in,int *out_size)
{
	char tmp[256];
	RtcpPacket_t *p=NULL;
	RtspInterHeader_t *interHeader=NULL;
	rtcp_common_t *common=NULL;
	rtcp_sdes_item_t *item=NULL;
	char *ptr = buf_in;
	if(rtcp->interleaved == true){
		interHeader = (RtspInterHeader_t *)buf_in;
		ptr += sizeof(RtspInterHeader_t);
	}
	common = (rtcp_common_t *)ptr;
	common->version = RTCP_VERSION;
	common->p = 0;
	common->count = 1;
	common->pt = RTCP_PACKET_SDES;
	ptr+=sizeof(rtcp_common_t);
	// ssrc
	ptr += encode_int32be(ptr,rtcp->ssrc);
	// cname
	*ptr++ = RTCP_SDES_CNAME;
	sprintf(tmp,"CN_%08x",rtcp->ssrc);
	*ptr++ = strlen(tmp);
	strcpy(ptr,tmp);
	ptr+=strlen(tmp);
	// tools
	*ptr++ = RTCP_SDES_TOOL;
	*ptr++ = strlen(RTCP_DEFAULT_SDES_TOOL);
	strcpy(ptr,RTCP_DEFAULT_SDES_TOOL);
	ptr+=strlen(RTCP_DEFAULT_SDES_TOOL);
	//end
	*ptr++=0;

	// compute size
	common->length = ptr-(char *)common;
	common->length = htons((common->length + 3)/4 - 1);
	*out_size = (ptr - buf_in + 3)/4*4;
	
	// if interleaved mode,packet interleaved header
	if(rtcp->interleaved == true){
		interHeader->magic = RTSP_INTERLEAVED_MAGIC;
		interHeader->channel = rtcp->chn_port_c;
		interHeader->length = htons(ptr-(char *)common);
	}
	
	return 0;
}

static int rtcp_bye_request(Rtcp_t *rtcp,char *buf_in,int *out_size)
{
	RtcpPacket_t *p=NULL;
	RtspInterHeader_t *interHeader=NULL;
	rtcp_common_t *common=NULL;
	rtcp_sdes_item_t *item=NULL;
	char *ptr = buf_in;
	if(rtcp->interleaved == true){
		interHeader = (RtspInterHeader_t *)buf_in;
		ptr += sizeof(RtspInterHeader_t);
	}
	common = (rtcp_common_t *)ptr;
	common->version = RTCP_VERSION;
	common->p = 0;
	common->count = 1;
	common->pt = RTCP_PACKET_BYE;
	ptr+=sizeof(rtcp_common_t);
	// src id
	ptr += encode_int32be(ptr,rtcp->src_id[0]);
	
	// compute size
	common->length = ptr-(char *)common;
	common->length = htons((common->length + 3)/4 - 1);
	*out_size = (ptr - buf_in + 3)/4*4;
	
	// if interleaved mode,packet interleaved header
	if(rtcp->interleaved == true){
		interHeader->magic = RTSP_INTERLEAVED_MAGIC;
		interHeader->channel = rtcp->chn_port_c;
		interHeader->length = htons(ptr-(char *)common);
	}
	
	return 0;
}



static int rtcp_app_request(Rtcp_t *rtcp,char *buf_in,int *out_size)
{
	return 0;
}

static int rtcp_handle_sr(Rtcp_t *rtcp,char *buf)
{
	char *ptr = buf;
	rtcp_common_t *rtcpHeader=(rtcp_common_t *)buf;
	uint32_t ssrc;
	uint32_t t_sec,t_usec,sr_ts;
	uint32_t srPacketCnt=0,srOctecCnt=0,srTimeStamp=0;
	if(rtcpHeader->version != RTCP_VERSION){
		VLOG(VLOG_WARNING,"rtcp version invalid");
	}

	ptr+=sizeof(rtcp_common_t);
	// check ssrc
	ssrc = decode_int32be(ptr);
	if(ssrc != rtcp->src_id[0]){
		VLOG(VLOG_ERROR,"rtcp ssrc:%#08x invalid,expected:%#08x",ssrc,rtcp->src_id[0]);
		return RTSP_RET_FAIL;
	}
	ptr+=4;
	//
	t_sec = decode_int32be(ptr);
	ptr+=4;
	t_usec = decode_int32be(ptr);
	ptr+=4;
	// caculate last_sr_ts
	sr_ts = ((t_sec & 0xffff) << 16) | ((t_usec >> 16) & 0xffff);
	rtcp_compute_jitter(rtcp,sr_ts);
	rtcp->last_sr_ts = sr_ts;
	// start timer
	MilliTimerStart(rtcp->rtt_timer);
	// 
	srTimeStamp = decode_int32be(ptr);
	ptr += 4;
	srPacketCnt = decode_int32be(ptr);
	ptr += 4;
	srOctecCnt = decode_int32be(ptr);
	ptr += 4;

	//compute comulative lost,and fraction lost
	rtcp_compute_lost(rtcp,srPacketCnt,srOctecCnt);

	return RTSP_RET_OK;
}

static int rtcp_handle_rr(Rtcp_t *rtcp,char *buf)
{
	char *ptr = buf;
	rtcp_common_t *rtcpHeader=(rtcp_common_t *)buf;
	uint32_t ssrc,src_id;
	int lost_cnt,max_seq,cycle;
	int delay_since_last_sr=0;
	if(rtcpHeader->version != RTCP_VERSION){
		VLOG(VLOG_WARNING,"rtcp version invalid");
	}

	ptr+=sizeof(rtcp_common_t);
	// get ssrc
	ssrc = decode_int32be(ptr);
	ptr += 4;
	src_id = decode_int32be(ptr);
	ptr += 4;
	if(src_id != rtcp->src_id[0]){
		VLOG(VLOG_ERROR,"rtcp src_id:%#08x invalid,expected:%#08x",src_id,rtcp->src_id[0]);
		return RTSP_RET_FAIL;
	}
	//lost fraction,sequence number,max seq received
	rtcp->rtp->fraction_lost = *ptr++;
	lost_cnt = decode_int24be(ptr);
	ptr += 3;
	cycle = decode_int16be(ptr);
	ptr += 2;	
	max_seq = decode_int24be(ptr);
	ptr += 2;
	// jitter,last SR timestamp, delay since last SR timestamp
	rtcp->jitter = decode_int32be(ptr);
	ptr += 4;	
	rtcp->last_sr_ts = decode_int32be(ptr);
	ptr += 4;
	delay_since_last_sr = decode_int32be(ptr);
	ptr += 4;

	// estimated round-trip timer
	rtcp_compute_roundtrip_time(rtcp,delay_since_last_sr);
	
	return RTSP_RET_OK;
}

static int rtcp_handle_sdes(Rtcp_t *rtcp,char *buf)
{
	return RTSP_RET_OK;
}

static int rtcp_handle_bye(Rtcp_t *rtcp,char *buf)
{
	return RTSP_RET_OK;
}

static int rtcp_handle_app(Rtcp_t *rtcp,char *buf)
{
	return RTSP_RET_OK;
}

static int RTCP_send_packet(Rtcp_t *rtcp,int bBye)
{
	int ret,size;
	RtcpPacketEx_t *ps=&rtcp->packet;
	char *ptr=(char *)ps->buffer;
	int dst_port;

	ps->cnt = 0;
	if(rtcp->role == RTSP_SERVER){
		ret=rtcp_sr_request(rtcp,ptr,&ps->buf_size[ps->cnt]);
		dst_port = rtcp->chn_port_c;
	}else{
		ret=rtcp_rr_request(rtcp,ptr,&ps->buf_size[ps->cnt]);
		dst_port = rtcp->chn_port_s;
	}
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	// update ptr and ps->cnt
	ptr+=ps->buf_size[ps->cnt];
	ps->cnt++;
	//
	ret=rtcp_sdes_request(rtcp,ptr,&ps->buf_size[ps->cnt]);
	if(ret == RTSP_RET_FAIL){
		return RTSP_RET_FAIL;
	}
	// update ptr and ps->cnt
	ptr+=ps->buf_size[ps->cnt];
	ps->cnt++;
	//
	if(bBye == true){
		ret=rtcp_bye_request(rtcp,ptr,&ps->buf_size[ps->cnt]);
		if(ret == RTSP_RET_FAIL){
			return RTSP_RET_FAIL;
		}
		// update ptr and ps->cnt
		ptr+=ps->buf_size[ps->cnt];
		printf("packet size:%d \n",ps->buf_size[ps->cnt]);
		ps->cnt++;
	}
	size = ptr - (char *)ps->buffer;
	//printf("total packet size:%d \n",size);

	// send packets
	if(rtcp->interleaved == true){
		ret=SOCK_send(rtcp->sock,ps->buffer,size);
	}else{
		ret=SOCK_sendto(rtcp->sock,rtcp->peername,dst_port,
			ps->buffer,size);
	}
	if(ret==RTSP_RET_FAIL){
		rtcp->bAlive = false;
		return RTSP_RET_FAIL;
	}
	
	return RTSP_RET_OK;
}

int RTCP_handle_packet(Rtcp_t *rtcp,void *payload,int payload_size)
{
	int ret,size;
	char buf[1024*16],*ptr=NULL,*pend=NULL;
	RtspInterHeader_t *interHeader=NULL;
	rtcp_common_t *rtcpHeader=NULL;

	if(rtcp == NULL){
		return RTSP_RET_FAIL;
	}

	if(rtcp->interleaved == true){
		ptr=(char *)payload+sizeof(RtspInterHeader_t);
		size = payload_size-sizeof(RtspInterHeader_t);
	}else{
		if(rtcp->bAlive == false) return RTSP_RET_FAIL;
		//
		ret=SOCK_recvfrom(rtcp->sock,rtcp->peername,rtcp->chn_port_s,buf,sizeof(buf));
		if(ret == RTSP_RET_FAIL){
			return RTSP_RET_FAIL;
		}
		ptr = buf;
		size = ret;
	}
	pend = ptr+size;
	do{
		rtcpHeader = (rtcp_common_t *)ptr;
		//
		if(rtcpHeader->pt == RTCP_PACKET_SR){
			ret=rtcp_handle_sr(rtcp,ptr);
		}else if(rtcpHeader->pt == RTCP_PACKET_RR){
			ret=rtcp_handle_rr(rtcp,ptr);
		}else if(rtcpHeader->pt == RTCP_PACKET_SDES){
			ret=rtcp_handle_sdes(rtcp,ptr);
		}else if(rtcpHeader->pt == RTCP_PACKET_BYE){
			ret=rtcp_handle_bye(rtcp,ptr);
		}else if(rtcpHeader->pt == RTCP_PACKET_APP){
			ret=rtcp_handle_app(rtcp,ptr);
		}else{
			VLOG(VLOG_ERROR,"unknown rtcp packet type:%d",rtcpHeader->pt);
			ret = RTSP_RET_FAIL;
		}
		//printf("rtcp pakcket type:%d length:%d\n",rtcpHeader->pt,(ntohs(rtcpHeader->length)+1) * 4);
		if(ret == RTSP_RET_FAIL){
			VLOG(VLOG_ERROR,"RTCP BECOME DISABLE");
			rtcp->bAlive = false;
			return RTSP_RET_FAIL;
		}
		//
		ptr += (ntohs(rtcpHeader->length)+1) * 4;
	}while(ptr < pend);
	
	return RTSP_RET_OK;
}

int RTCP_process(Rtcp_t *rtcp)
{
	MillisecondTimer_t t_tmp;
	MilliSecond_t t_lasttime;

	// rtcp not support
	if(rtcp->bAlive == false) return RTSP_RET_OK;
	
	MilliTimerStop(rtcp->trans_timer,t_tmp,t_lasttime);
	if(t_lasttime >= RTCP_MIN_TRANS_FREQ){
		RTCP_send_packet(rtcp,false);
		// restart timer
		MilliTimerStart(rtcp->trans_timer);
	}

	return RTSP_RET_OK;
}

int RTCP_init(Rtcp_t **r,
	int role,	// act as client or server
	uint32_t src_id,
	int protocal,/* udp or tcp */
	int cast_type,/* unicast or multicast */
	int interleaved, /* true or false */
	int rtsp_sock, /* if interleaved , rtp sock use this */
	int chn_port_s,/* rtp port or channel if in interleaved mode */
	int chn_port_c,
	Rtp_t *rtp) 
{
	Rtcp_t *rtcp=NULL;
	RtcpPacketEx_t *ps=NULL;
	// malloc for rtcp
	rtcp = (Rtcp_t *)malloc(sizeof(Rtcp_t));
	if(rtcp == NULL){
		VLOG(VLOG_ERROR,"malloc for rtcp failed");
		return RTSP_RET_FAIL;
	}
	memset(rtcp,0,sizeof(Rtcp_t));
	*r = rtcp;
	// init 
	rtcp->role = role;
	rtcp->ssrc = rand();
	rtcp->src_id[0] = src_id;
	rtcp->chn_port_c = chn_port_c;
	rtcp->chn_port_s = chn_port_s;
	rtcp->interleaved = interleaved;
	rtcp->rtp = rtp;
	rtcp->jitter = 0;
	rtcp->bAlive = true;
	rtcp->last_sr_ts = 0;
	MilliTimerSet(rtcp->trans_timer,0,0);
	MilliTimerSet(rtcp->rtt_timer,0,0);
	//MilliTimerSet(rtcp->jitter_timer,0,0);
	// init packet buffer
	ps = &rtcp->packet;
	ps->cnt = 0;
	//ps->buffer = (void *)malloc(1280);
	ps->buffer = (void *)malloc(sizeof(RtcpPacket_t)*RTCP_MAX_PACKETS);
	if(ps->buffer == NULL){
		VLOG(VLOG_ERROR,"malloc for rtcp packet buffer failed");
		free(rtcp);
		return RTSP_RET_FAIL;
	}
	memset(ps->buffer,0,sizeof(RtcpPacket_t)*RTCP_MAX_PACKETS);
	// init transport
	SOCK_getpeername(rtsp_sock,rtcp->peername);
	VLOG(VLOG_DEBUG,"peer name:%s,port_c:%d port_s:%d",rtcp->peername,chn_port_c,chn_port_s);
	if(interleaved == true)
		rtcp->sock = rtsp_sock;
	else{
		rtcp->sock=rtp_init_transport(cast_type,protocal,
			(role==RTSP_SERVER) ? chn_port_s : chn_port_c);
	}
	if(rtcp->sock == RTSP_RET_FAIL)
		return RTSP_RET_FAIL;
	
	MilliTimerStart(rtcp->trans_timer);

	VLOG(VLOG_CRIT,"rtcp init done,sock:%d port_c:%d port_s:%d",rtcp->sock,rtcp->chn_port_c,rtcp->chn_port_s);
	return RTSP_RET_OK;
}

int RTCP_destroy(Rtcp_t *rtcp)
{
	if(rtcp == NULL){
        VLOG(VLOG_WARNING,"rtcp is null");
		return RTSP_RET_OK;
	}
	// send bye packet
	if(rtcp->bAlive == true){
		RTCP_send_packet(rtcp,true);
	}
	//
	if(rtcp->interleaved == false){
		SOCK_close(rtcp->sock);
	}
	if(rtcp->packet.buffer){
		free(rtcp->packet.buffer);
		rtcp->packet.buffer = NULL;
	}
	free(rtcp);
	VLOG(VLOG_CRIT,"rtcp destroy success");

	return RTSP_RET_OK;
}


