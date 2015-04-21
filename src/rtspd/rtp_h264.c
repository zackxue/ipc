
#include "rtp_h264.h"
#include "rtspd_debug.h"

#define RTP_H264_PT (96)
typedef struct RtpFixedHeader
{
	// 1st. byte
	uint8_t csrc_count : 4; // CC on table expect 0
	uint8_t extension : 1; // X on table expect 0, see RTP_OP below
	uint8_t padding : 1; // P on table expect 0
	uint8_t version : 2; // V on table expect 2
	// 2nd. byte
	uint8_t payload_type : 7; // PT on table expect 94 for h264 stream
	uint8_t marker : 1; // M on table expect 1
	// 3rd. 4th. bytes
	uint16_t sequence_number;
	// 5-8th. bytes
	uint32_t timestamp;
	uint32_t ssrc;
	// ignore the csrc id si crsc_count always 0
	//uint32_t csrc[16];
} RtpFixedHeader_t;

ssize_t rtph264_mk_rtp_header(uint32_t maker, uint16_t sn, uint32_t pts, uint32_t ssrc, void* ret_val)
{
	if(ret_val){
		RtpFixedHeader_t* const rtp_hdr = (RtpFixedHeader_t*)ret_val;
		ssize_t rtp_hdr_sz = sizeof(RtpFixedHeader_t);
		memset(rtp_hdr, 0, rtp_hdr_sz);
		//        1st. byte
		rtp_hdr->version = 2;
		rtp_hdr->padding = 0;
		rtp_hdr->extension = 0;
		rtp_hdr->csrc_count = 0;
		//        2nd. bytes
		rtp_hdr->marker = maker ? 1 : 0; //        frame border maker
		rtp_hdr->payload_type = RTP_H264_PT; //        h264 recommand
		//        3:4 bytes
		rtp_hdr->sequence_number = htons(sn); //        an increasing number
		//        5:8 bytes
		rtp_hdr->timestamp = htonl(pts); //        the timestamp
		//        9:12 bytes
		rtp_hdr->ssrc = htonl(ssrc); //        with a random number
		//RTSPD_TRACE("%d\r\n", sizeof(header));
		return rtp_hdr_sz;
	}
	return -1;
}

void rtph264_print_rtp_header(void* val)
{
	RtpFixedHeader_t* const rtp_hdr = (RtpFixedHeader_t*)val;
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

	RTSPD_TRACE("RTP Header");
	RTSPD_TRACE(".---------------------------------------------------------------.");
	RTSPD_TRACE("|0                   10                  20                  30 |");
	RTSPD_TRACE("|0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|");
	RTSPD_TRACE("| V |P|X|   CC  |M|     PT      |       sequence number         |");
	RTSPD_TRACE("|%3u|%1u|%1u|%7u|%1u|%13u|%31u|",
			rtp_hdr->version,
			rtp_hdr->padding,
			rtp_hdr->extension,
			rtp_hdr->csrc_count,
			rtp_hdr->marker ? 1 : 0,
			rtp_hdr->payload_type,
			htons(rtp_hdr->sequence_number));
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");
	RTSPD_TRACE("|%63u|", htonl(rtp_hdr->timestamp));
	RTSPD_TRACE("|-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|");
	RTSPD_TRACE("|%63u|", htonl(rtp_hdr->ssrc));
	for(i = 0; i < rtp_hdr->csrc_count; ++i){
		RTSPD_TRACE("|===============================================================|");
//       		RTSPD_TRACE("|%62d|", rtp_hdr->csrc[i]);
	}
	RTSPD_TRACE("'---------------------------------------------------------------'");
	RTSPD_TRACE(".");
}

ssize_t rtph264_mk_avp_interleave(ssize_t payload_sz, void* ret_val)
{
	if(ret_val){
		uint8_t* const interleave = (uint8_t*)ret_val;
		interleave[0] = 0x24;
		interleave[1] = 0;
		interleave[2] = payload_sz / 0x100;
		interleave[3] = payload_sz % 0x100;
		return 4;
	}
	return -1;
}

ssize_t rtph264_mk_nalu_header(uint8_t nal_ref_idc, uint8_t type, void* ret_val)
{
	//
	//        .---------------.
	//        |0 1 2 3 4 5 6 7|
	//        |-+-+-+-+-+-+-+-|
	//        |F|NRI|  Type   |
	//        '---------------'
	//       
	//         F: 1 bit
	//           forbidden_zero_bit.  A value of 0 indicates that the NAL unit type
	//       	      octet and payload should not contain bit errors or other syntax
	//       	      violations.  A value of 1 indicates that the NAL unit type octet
	//       	      and payload may contain bit errors or other syntax violations.
	//       
	//       	      MANEs SHOULD set the F bit to indicate detected bit errors in the
	//       	      NAL unit.  The H.264 specification requires that the F bit is
	//       	      equal to 0.  When the F bit is set, the decoder is advised that
	//       	      bit errors or any other syntax violations may be present in the
	//       	      payload or in the NAL unit type octet.  The simplest decoder
	//       	      reaction to a NAL unit in which the F bit is equal to 1 is to
	//       	      discard such a NAL unit and to conceal the lost data in the
	//       	      discarded NAL unit.
	//       
	//         NRI: 2 bits
	//       	      nal_ref_idc.  The semantics of value 00 and a non-zero value
	//       	      remain unchanged from the H.264 specification.  In other words, a
	//       	      value of 00 indicates that the content of the NAL unit is not used
	//       	      to reconstruct reference pictures for inter picture prediction.
	//       	      Such NAL units can be discarded without risking the integrity of
	//       	      the reference pictures.  Values greater than 00 indicate that the
	//       	      decoding of the NAL unit is required to maintain the integrity of
	//       	      the reference pictures.
	//       
	//       	      In addition to the specification above, according to this RTP
	//       	      payload specification, values of NRI greater than 00 indicate the
	//       	      relative transport priority, as determined by the encoder.  MANEs
	//
	
	if(ret_val){
		uint8_t* const nalu = (uint8_t*)ret_val;
		*nalu &= ~0x80; // forbidden zero bit
		*nalu |= ((nal_ref_idc & 0x03) << 5);
		*nalu |= (type & 0x1f);
		return 1;
	}
	return -1;
}

