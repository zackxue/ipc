/*
 * rtspd_rtp.h
 *
 *  Created on: 2012-6-18
 *      Author: root
 */

#ifndef RTSPD_RTP_H_
#define RTSPD_RTP_H_

#include "spook/rtspd.h"

//extern int RTP_packet_nalu(RtspdSession_t* session, H264_FILE_BUFFER* hfb);
extern int RTP_packet_nalu(RtspdSession_t* session, void* nalu_ptr, ssize_t nalu_sz);

#endif /* RTSPD_RTP_H_ */
