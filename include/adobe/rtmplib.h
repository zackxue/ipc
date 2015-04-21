#ifndef __RTMP_LIB_H__
#define __RTMP_LIB_H__

#include "rtmpdef.h"
#include "amf.h"
//#include "rtmpstream.h"

#define RTMP_MAX_CHANNEL		(63)
#define RTMP_SOCK_TIMEOUT		(5)

#define RTMP_LIB_VERSION	0x00000100	/* 1.0 */

#define RTMP_CONTROL_STREAM_ID		(0)

#define RTMP_FEATURE_HTTP	0x01
#define RTMP_FEATURE_ENC	0x02
#define RTMP_FEATURE_SSL	0x04
#define RTMP_FEATURE_MFP	0x08	/* not yet supported */
#define RTMP_FEATURE_WRITE	0x10	/* publish, not play */
#define RTMP_FEATURE_HTTP2	0x20	/* server-side rtmpt */

#define RTMP_PROTOCOL_UNDEFINED	-1
#define RTMP_PROTOCOL_RTMP      0
#define RTMP_PROTOCOL_RTMPE     RTMP_FEATURE_ENC
#define RTMP_PROTOCOL_RTMPT     RTMP_FEATURE_HTTP
#define RTMP_PROTOCOL_RTMPS     RTMP_FEATURE_SSL
#define RTMP_PROTOCOL_RTMPTE    (RTMP_FEATURE_HTTP|RTMP_FEATURE_ENC)
#define RTMP_PROTOCOL_RTMPTS    (RTMP_FEATURE_HTTP|RTMP_FEATURE_SSL)
#define RTMP_PROTOCOL_RTMFP     RTMP_FEATURE_MFP

#define RTMP_DEFAULT_CHUNKSIZE	(128)
#define RTMP_DEFAULT_PORT			(1935)

/*      RTMP_PACKET_TYPE_...                0x00 */
#define RTMP_PACKET_TYPE_CHUNK_SIZE         0x01
/*      RTMP_PACKET_TYPE_...                0x02 */
#define RTMP_PACKET_TYPE_BYTES_READ_REPORT  0x03
#define RTMP_PACKET_TYPE_CONTROL            0x04
#define RTMP_PACKET_TYPE_SERVER_BW          0x05
#define RTMP_PACKET_TYPE_CLIENT_BW          0x06
/*      RTMP_PACKET_TYPE_...                0x07 */
#define RTMP_PACKET_TYPE_AUDIO              0x08
#define RTMP_PACKET_TYPE_VIDEO              0x09
/*      RTMP_PACKET_TYPE_...                0x0A */
/*      RTMP_PACKET_TYPE_...                0x0B */
/*      RTMP_PACKET_TYPE_...                0x0C */
/*      RTMP_PACKET_TYPE_...                0x0D */
/*      RTMP_PACKET_TYPE_...                0x0E */
#define RTMP_PACKET_TYPE_FLEX_STREAM_SEND   0x0F
#define RTMP_PACKET_TYPE_FLEX_SHARED_OBJECT 0x10
#define RTMP_PACKET_TYPE_FLEX_MESSAGE       0x11
#define RTMP_PACKET_TYPE_INFO               0x12
#define RTMP_PACKET_TYPE_SHARED_OBJECT      0x13
#define RTMP_PACKET_TYPE_INVOKE             0x14
/*      RTMP_PACKET_TYPE_...                0x15 */
#define RTMP_PACKET_TYPE_FLASH_VIDEO        0x16


#define RTMP_VERSION_CODE			(0x03)
#define RTMP_SIG_SIZE 				(1536)
#define RTMP_LARGE_HEADER_SIZE 	(12)
#define RTMP_MAX_HEADER_SIZE 		(18)

#define RTMP_PACKET_SIZE_LARGE    0
#define RTMP_PACKET_SIZE_MEDIUM   1
#define RTMP_PACKET_SIZE_SMALL    2
#define RTMP_PACKET_SIZE_MINIMUM  3



typedef struct _rtmp_packet {
    int m_headerType;
    int m_packetType;
    int m_hasAbsTimestamp;	/* timestamp absolute or relative? */
    int m_nChannel;
    uint32_t m_nTimeStamp;	/* timestamp */
    uint32_t m_nInfoField2;	/* last 4 bytes in a long header */
    uint32_t m_nBodySize;
    uint32_t m_nBytesRead;
    char *m_body;
}RtmpPacket_t;

typedef struct _rtmp {
    int sock;	
	int m_bLinkOk;
    int m_inChunkSize;
    int m_outChunkSize;

    AVal m_playPath;
    int m_bPlayStart;
	int m_bAudio;	// with or without audio
	int m_bVideo;
    
	//RtmpStream_t m_stream;
    int m_stream_id;		/* returned in _result from createStream */

    RtmpPacket_t *m_vecChannelsIn[RTMP_MAX_CHANNEL];
    RtmpPacket_t *m_vecChannelsOut[RTMP_MAX_CHANNEL];
    int m_channelTimestamp[RTMP_MAX_CHANNEL];	/* abs timestamp of last packet */

    double m_fAudioCodecs;	/* audioCodecs for the connect packet */
    double m_fVideoCodecs;	/* videoCodecs for the connect packet */
    double m_fEncoding;		/* AMF0 or AMF3 */

    double m_fDuration;		/* duration of stream in seconds */

	uint32_t m_nTotalSend;
	uint32_t m_nTotalAck;
}Rtmp_t;

int RTMP_init(Rtmp_t *r,int fd);
int RTMP_destroy(Rtmp_t *r);

int RTMP_read_packet(Rtmp_t *r,RtmpPacket_t *packet);
int RTMP_write_packet(Rtmp_t *r,RtmpPacket_t *packet);
int RTMP_parse_packet(Rtmp_t *r,RtmpPacket_t *packet);

int RTMP_send_avc(Rtmp_t * _r, char* _data, int _data_len);
int RTMP_send_frame(Rtmp_t* _r, void* frame, int framesz, int iskeyframe, unsigned int tsp);
int RTMP_send_g711(Rtmp_t* _r, void* frame, int framesz,unsigned int tsp,unsigned char conf);

#endif
