/*
 * rtspd_rtp.c
 *
 *  Created on: 2012-6-18
 *      Author: root
 */

#include "rtspd_rtp.h"
#include "rtspd_debug.h"

#define RTP_SINK (1448)
#define ALIGIN_PTR_OFFSET(value) do{(value) = ((((uint32_t)value) / 4 + 1) * 4);}while(0)

static ssize_t rtp_mk_tcp_interleave(int ch, ssize_t payload_sz, void* ret_val)
{
	if(ret_val){
		uint8_t* const interleave = (uint8_t*)ret_val;
		interleave[0] = 0x24;
		interleave[1] = 0;
		interleave[2] = payload_sz / 0x100;
		interleave[3] = payload_sz % 0x100;
		return 4;
	}
	return 0;
}

static ssize_t rtp_packet_nalu(uint32_t over_tcp, int tcp_ch, 
	uint8_t maker, uint32_t rtp_seq, uint32_t timestamp, uint32_t ssrc,
	uint8_t fu_indicator, uint8_t fu_header,
	void* rtp_buf,
	uint8_t nalu_header, void* nalu_data, ssize_t nalu_sz)
{
	void* interleave_offset = rtp_buf;
	void* rtp_header_offset = rtp_buf;
	void* nalu_offset = NULL;
	void* payload_offset = NULL;

	ssize_t interleave_sz = 0;
	ssize_t rtp_hdr_sz = 0;

	unsigned char* fu_offset = NULL;
	int fu_sz = 0;
	
	if(over_tcp){
		// interlevaed tcp only
		interleave_offset = rtp_buf;
		interleave_sz = rtp_mk_tcp_interleave(tcp_ch, 0, interleave_offset); // ready make interleave
	}else{
		// no interleaved when udp
		interleave_offset = NULL;
		interleave_sz = 0;
	}
	// rtp header
	rtp_header_offset += interleave_sz; // rtp header is the head when no interleaved
	rtp_hdr_sz = rtph264_mk_rtp_header(maker, rtp_seq, timestamp, ssrc, rtp_header_offset);
	// fu header
	fu_offset = rtp_header_offset + rtp_hdr_sz;
	if(fu_indicator && fu_header){
		fu_sz = 2;
		fu_offset[0] = fu_indicator;
		fu_offset[1] = fu_header;
	}else{
		fu_sz = 1;
		fu_offset[0] = nalu_header;
	}
	// nalu data
	nalu_offset = fu_offset + fu_sz;
	memcpy(nalu_offset, nalu_data, nalu_sz);
	// re-confirm the rtp interleaved
	rtp_mk_tcp_interleave(0, rtp_hdr_sz + fu_sz +  nalu_sz, interleave_offset);

	return interleave_sz + rtp_hdr_sz + fu_sz + nalu_sz;
}


//int RTP_packet_nalu(RtspdSession_t* session, H264_FILE_BUFFER* hfb)
int RTP_packet_nalu(RtspdSession_t* session, void* nalu_ptr, ssize_t nalu_sz)
{
//	int ssrc = rand() % 4096;
	int const ssrc = session->session_id ^ 0xa0a00505;

	float framerate=25;
	unsigned int timestamp_increse=(unsigned int)(90000.0 / framerate);

	uint8_t nalu_header = *((uint8_t*)nalu_ptr + 4);
	ssize_t nalu_data = nalu_ptr + 5;
	ssize_t nalu_pack_sz;

	int send_sz = 0;
	int const payload_type = nalu_header & 0x1f;
	int const maker = ((1 == payload_type) || (5 == payload_type)) ? true : false;
	uint32_t const overtcp = session->transport == RTP_OVER_TCP;

	// offset to payload data area
	nalu_sz -= 5;

	// reset the segment count
	session->rtp_entries = 0;
	
	if(nalu_sz < RTP_SINK){
		void* const send_buf = session->response_buf;
		void* const nalu_pack_buf = nalu_data;
		
		nalu_pack_sz = nalu_sz;
		send_sz = rtp_packet_nalu(overtcp, 0, maker, session->rtp_seq++,
			session->rtp_timestamp, ssrc, 0, 0, send_buf, nalu_header, nalu_pack_buf, nalu_pack_sz);
		// mark down the rtp packet
		session->rtp_entry[session->rtp_entries].ptr = send_buf;
		session->rtp_entry[session->rtp_entries].len = send_sz;
		session->rtp_entries++;
	}else{
		int i = 0;
		int nalu_pack_cnt = nalu_sz / RTP_SINK;
		int nalu_pack_left = nalu_sz % RTP_SINK;
		
		void* send_buf = session->response_buf;
		void* nalu_pack_buf = nalu_data;
		
		nalu_pack_sz = nalu_sz;
		if(0 == nalu_pack_left){
			nalu_pack_cnt -= 1;
			nalu_pack_left = RTP_SINK;
		}

		nalu_pack_sz = RTP_SINK;
		for(i = 0; i < nalu_pack_cnt; ++i){	
			// rtp interleaved
			send_sz = rtp_packet_nalu(overtcp, 0, false, session->rtp_seq++,
				session->rtp_timestamp, ssrc, (nalu_header&0xe0)|0x1c, (nalu_header&0x1f)|(0==i?0x80:0x00),
				send_buf, nalu_header , nalu_pack_buf, nalu_pack_sz);

			session->rtp_entry[session->rtp_entries].ptr = send_buf;
			session->rtp_entry[session->rtp_entries].len = send_sz;
			session->rtp_entries++;

			nalu_pack_buf += nalu_pack_sz;
			send_buf += send_sz;
			ALIGIN_PTR_OFFSET(send_buf); // aligned legal offset
		}
		nalu_pack_sz = nalu_pack_left;
		// rtp interleaved
		send_sz = rtp_packet_nalu(overtcp, 0, maker, session->rtp_seq++,
			session->rtp_timestamp, ssrc, (nalu_header&0xe0)|0x1c, (nalu_header&0x1f)|0x40,
			send_buf, nalu_header , nalu_pack_buf, nalu_pack_sz);
		session->rtp_entry[session->rtp_entries].ptr = send_buf;
		session->rtp_entry[session->rtp_entries].len = send_sz;
		session->rtp_entries++;

		nalu_pack_buf += nalu_pack_sz;
		send_buf += send_sz; // aligned 4 bytes
		ALIGIN_PTR_OFFSET(send_buf); // aligned legal offset
		assert(nalu_pack_buf - nalu_data == nalu_sz);
	}
	
	if(maker){
		session->rtp_timestamp+= timestamp_increse;
	}
//	usleep(1000000 * timestamp_increse / 90000 + 10000);
	//for rtcp
	session->rtp_packetcount = session->rtp_seq;
	session->rtp_octetcount += nalu_pack_sz;
	
	return session->rtp_entries;
}

