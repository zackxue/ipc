
#ifndef __RTP_H264_H__
#define __RTP_H264_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

extern ssize_t rtph264_mk_rtp_header(uint32_t maker, uint16_t sn, uint32_t pts, uint32_t ssrc, void* ret_header);
extern void rtph264_print_rtp_header(void* header);

extern ssize_t rtph264_mk_avp_interleave(ssize_t payload_sz, void* ret_header);
extern ssize_t rtph264_mk_nalu_header(uint8_t nal_ref_idc, uint8_t type, void* ret_val);

#endif //__RTP_H264_H__

