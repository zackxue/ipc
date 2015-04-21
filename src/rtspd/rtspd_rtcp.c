/*
 * rtspd_rtcp.c
 *
 *  Created on: 2012-6-18
 *      Author: root
 */
#include "rtspd_debug.h"
#include "rtspd_rtcp.h"
#include "rtcp_from_spec.h"


static double dTimeNow() {
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    return (double) (timeNow.tv_sec + timeNow.tv_usec/1000000.0);
}

static void rtcp_print_header(void* val)
{
	rtcp_t* const pRR= (rtcp_t*)val;
	rtcp_t* psdes;
	int i = 0;

	//        .---------------------------------------------------------------.
	//        |0                   10                  20                  30 |
	//        |0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|
	//        |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
	//        |V=2|P|X|  CC   |M|     PT      |       sequence number         |
	//        |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
	//        |                           timestamp                           |
	//        |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
	//        |           synchronization source (SSRC) identifier            |
	//        |-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
	//        |            contributing source (CSRC) identifiers             |
	//        |                             ....                              |
	//        '---------------------------------------------------------------'
	RTSPD_TRACE("RTCP Header");
	RTSPD_TRACE(".---------------------------------------------------------------.");
	RTSPD_TRACE("|0                   10                  20                  30 |");
	RTSPD_TRACE("|0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|");
	RTSPD_TRACE("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	RTSPD_TRACE("|V=%u|P=%u|  RC=%u  |     PT=%u    |         length=%u          |",
			pRR->common.version,
			pRR->common.p,
			pRR->common.count,
			pRR->common.pt,
			htons(pRR->common.length));
		
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");
	RTSPD_TRACE("|     SSRC:0x%x(%u)",htonl(pRR->rr.ssrc),htonl(pRR->rr.ssrc));
/**/
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-Source(1)+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");
	RTSPD_TRACE("|     SSRCid:0x%x(%u)",htonl(pRR->rr.src[0].ssrc),htonl(pRR->rr.src[0].ssrc));
	RTSPD_TRACE("|     fraction lost:%d",pRR->rr.src[0].fraction);
	RTSPD_TRACE("|     packetlost:0x%x(%d)",htonl(pRR->rr.src[0].lost),htonl(pRR->rr.src[0].lost));
	RTSPD_TRACE("|     highestrecv:0x%x(%d)",htonl(pRR->rr.src[0].last_seq),htonl(pRR->rr.src[0].last_seq));
	RTSPD_TRACE("|     jitter:0x%x(%d)",htonl(pRR->rr.src[0].jitter),htonl(pRR->rr.src[0].jitter));
	RTSPD_TRACE("|     lsr:0x%x(%d)",htonl(pRR->rr.src[0].lsr),htonl(pRR->rr.src[0].lsr));
	RTSPD_TRACE("|     dlsr:0x%x(%d)",htonl(pRR->rr.src[0].dlsr),htonl(pRR->rr.src[0].dlsr));

	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");

	psdes = (rtcp_t*)(val + (htons(pRR->common.length)+1)*4);
	RTSPD_TRACE("+-+-+-+-+-+-+-+-+-+-+source description-+-+-+-+-+-+-+-+-+-+-+-+-+");
	RTSPD_TRACE("|V=%u|P=%u|  RC=%u  |     PT=%u    |         length=%u          |",
			psdes->common.version,
			psdes->common.p,
			psdes->common.count,
			psdes->common.pt,
			htons(psdes->common.length));	
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");	
	RTSPD_TRACE("|     SSRC:0x%x(%u)",htonl(psdes->sdes.ssrc),htonl(psdes->sdes.ssrc));
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");
	RTSPD_TRACE("|     cname:%u",psdes->sdes.item[0].type);
	RTSPD_TRACE("|     length:%u",psdes->sdes.item[0].length);
	RTSPD_TRACE("|     text:%s",psdes->sdes.item[0].data);
	RTSPD_TRACE("'---------------------------------------------------------------'");
#if 0
	for(i=0; i < sizeof(SSRCSource_t);i++){
		printf("%02x ",*((unsigned char*)psdes+i));
		if((i+1)%8 == 0)printf("\n");
	}
#endif	
}
static unsigned int rtcp_get_ssrc(RtspdSession_t* session)
{
	return session->session_id ^ 0xa0a00505;
}

static char *rtcp_write_sdes(char *b, uint32_t ssrc, int argc,rtcp_sdes_type_t type[], char *value[],int length[])
{
	rtcp_sdes_t *s = (rtcp_sdes_t *)b;
	rtcp_sdes_item_t *rsp;
	int i;
	int len;
	int pad;
	/* SSRC header */
	s->ssrc = ssrc;
	rsp = &s->item[0];
	/* SDES items */
	for (i = 0; i < argc; i++) {
		rsp->type = type[i];
		len = length[i];
		if (len > RTP_MAX_SDES) {
			/* invalid length, may want to take other action */
			len = RTP_MAX_SDES;
		}
		rsp->length = len;
		memcpy(rsp->data, value[i], len);
		rsp = (rtcp_sdes_item_t *)&rsp->data[len];
	}
	/* terminate with end marker and pad to next 4-octet boundary */
	len = ((char *) rsp) - b;
	pad = 4 - (len & 0x3);
	b = (char *) rsp;
	while(pad--)*b++ = RTCP_SDES_END;
	
	return b;
}

static void rtcp_send_report(RtspdSession_t* session)
{
	ssize_t payload_sz;
	uint8_t* const interleave = (uint8_t*)session->response_buf;
	interleave[0] = 0x24;
	interleave[1] = 1;
	interleave[2] = payload_sz / 0x100;
	interleave[3] = payload_sz % 0x100;

	rtcp_t* psr = (rtcp_t*)(session->response_buf+4);

	psr->common.version = 2;
	psr->common.p = 0;
	psr->common.count = 0;
	psr->common.pt = RTCP_SR;
	uint16_t srlen = 1+5;
	psr->common.length = htons(srlen);
	psr->sr.ssrc = rtcp_get_ssrc(session);
	// Insert the NTP and RTP timestamps for the 'wallclock time':
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);
	psr->sr.ntp_sec = htonl(timeNow.tv_sec + 0x83AA7E80);
	// NTP timestamp most-significant word (1970 epoch -> 1900 epoch)
	double fractionalPart = (timeNow.tv_usec/15625.0)*0x04000000; // 2^32/10^6
	psr->sr.ntp_frac = htonl((unsigned)(fractionalPart+0.5));
	// NTP timestamp least-significant word
	psr->sr.rtp_ts = htonl(session->rtp_timestamp); // RTP ts
	// Insert the packet and byte counts:
	psr->sr.psent = htonl(session->rtp_packetcount);
	psr->sr.osent = htonl(session->rtp_octetcount);

	//sdes
	rtcp_t* psdes = (rtcp_t*)(session->response_buf + (srlen +1)* 4 + 4);
	psdes->common.version = 2;
	psdes->common.p = 0;
	psdes->common.count = 1;
	psdes->common.pt = RTCP_SDES;
	psdes->common.length = htons(6);
	rtcp_sdes_type_t type[1] = {RTCP_SDES_CNAME};
	//char *value[1] ={"hhhhhhhhhhhhhh"};
	char *value[1] ={"ipcameraofjuan"};
	int length[1] = {15};
	char* bufoffset = rtcp_write_sdes(&psdes->sdes, rtcp_get_ssrc(session), 1, type, value,length);

	payload_sz = (ssize_t)(bufoffset - session->response_buf);
	session->response_sz = payload_sz;
	interleave[2] = payload_sz / 0x100;
	interleave[3] = payload_sz % 0x100;

}

static void rtcp_on_expire(RtspdSession_t* session) 
{
	// Note: fTotSessionBW is kbits per second
	double rtcpBW = 0.05*session->total_session_bw*1024/8; // -> bytes per second

	//RTSPD_TRACE("OnExpire(%d)\n", session->avg_rtcp_size);

	OnExpire(session, // event
		   session->members, // members
		   session->senders, // senders
		   rtcpBW, // rtcp_bw
		   1 , // we_sent
		   &session->avg_rtcp_size, // ave_rtcp_size
		   &session->initial, // initial
		   dTimeNow(), // tc
		   &session->prev_report_time, // tp
		   &session->pmembers// pmembers
		   );
}
static void rtcp_on_receive(RtspdSession_t* session) 
{
	int type_of_packet= PACKET_UNKNOWN_TYPE; 
	int total_packet_size = session->request_sz; 
	unsigned report_sender_ssrc;
	char* pbuf_offset;

	//RTSPD_TRACE("RTSP interleaved frame");
	//RTSPD_TRACE("magic:0x%x",session->request_buf[0]);
	//RTSPD_TRACE("channel:0x%x",session->request_buf[1]);
	//RTSPD_TRACE("rtcpbodylen:%d bytes",*((unsigned char*)(session->request_buf+3)) | (*((unsigned char*)(session->request_buf+2)) << 8));

	pbuf_offset = session->request_buf +4;
	//print_rtcp_header(pbuf_offset);

	rtcp_t* const preport= (rtcp_t*)pbuf_offset;
	switch(preport->common.pt){
		case RTCP_SR:
		{

			RTSPD_TRACE("RTCP receive  senderreport!");
			break;
		}

		case RTCP_RR:
		{
			RTSPD_TRACE("RTCP receive  receiverreport!");

			report_sender_ssrc = preport->rr.ssrc;
			type_of_packet = PACKET_RTCP_REPORT;
			break;
		}
		case RTCP_BYE:
		{
			RTSPD_TRACE("RTCP receive  byereport!");
			type_of_packet = PACKET_BYE;
			break;
		}
	}

	session->type_of_packet= type_of_packet;
	session->last_received_size= total_packet_size;
	session->last_received_ssrc= report_sender_ssrc;

	OnReceive(session, // p
	    session, // e
	    &session->members, // members
	    &session->pmembers, // pmembers
	    &session->senders, // senders
	    &session->avg_rtcp_size, // avg_rtcp_size
	    &session->prev_report_time, // tp
	    dTimeNow(), // tc
	    session->next_report_time);
}


////////// Implementation of routines imported by the "rtcp_from_spec" C code

void Schedule(double nextTime, event e) {
	RtspdSession_t* session = (RtspdSession_t*)e;
	if (session == NULL) return;
	
	session->next_report_time = nextTime;
}

void Reschedule(double nextTime, event e) {
	RtspdSession_t* session = (RtspdSession_t*)e;
	if (session == NULL) return;
	
	session->next_report_time = nextTime;
}

void SendRTCPReport(event e) {
	RtspdSession_t* session = (RtspdSession_t*)e;
	if (session == NULL) return;

	RTSPD_TRACE("Send a rtcp packet!");
	rtcp_send_report(session);
}

void SendBYEPacket(event e) {

}

int TypeOfEvent(event e) {
	RtspdSession_t* session = (RtspdSession_t*)e;
	if (session == NULL) return EVENT_UNKNOWN;

	return session->type_of_event;
}

int SentPacketSize(event e) {
	return 0;
}
int PacketType(packet p) {
	RtspdSession_t* session = (RtspdSession_t*)p;
	if (session == NULL) return PACKET_UNKNOWN_TYPE;

	return session->type_of_packet;
}

int ReceivedPacketSize(packet p) {
	RtspdSession_t* session = (RtspdSession_t*)p;
	if (session == NULL) return PACKET_UNKNOWN_TYPE;

	return session->last_received_size;	
}

int NewMember(packet p) {
	RtspdSession_t* session = (RtspdSession_t*)p;
	if (session == NULL) return 0;

	return ++session->members;	
}

int NewSender(packet p) {
  return 0; // we don't yet recognize senders other than ourselves #####
}

void AddMember(packet p) {
  // Do nothing; all of the real work was done when NewMember() was called
}

void AddSender(packet p) {
  // we don't yet recognize senders other than ourselves #####
}

void RemoveMember(packet p) {
	RtspdSession_t* session = (RtspdSession_t*)p;
	if (session == NULL) return ;

	session->members--;	
}

void RemoveSender(packet p) {
  // we don't yet recognize senders other than ourselves #####
}

double drand30() {
  unsigned tmp = random()&0x3FFFFFFF; // a random 30-bit integer
  return tmp/(double)(1024*1024*1024);
}
void RTCP_init(RtspdSession_t* session)
{
	session->type_of_event = EVENT_REPORT;
	session->initial = 1;
	session->pmembers = 2;
	session->members = 2;
	session->senders = 1;
	session->rtp_octetcount = 0;
	session->total_session_bw = 500;
	session->prev_report_time= session->next_report_time= dTimeNow();
}

int RTCP_handle(RtspdSession_t* session)
{
	rtcp_on_receive(session);
	//session->response_sz = 0;
	return 0;
}

void RTCP_schedule(RtspdSession_t* session)
{
	double secondsToDelay = session->next_report_time - dTimeNow();

	//printf("             (%f->%f)\n", secondsToDelay, session->next_report_time);
	
	if (secondsToDelay <= 0){
		rtcp_on_expire(session);
		secondsToDelay = 0;
	}
}

