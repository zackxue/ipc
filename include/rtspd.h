
#ifndef __RTSPD_H__
#define __RTSPD_H__

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

#include "spook/spook.h"

#define RTSPD_SERVER_NAME "RTSPD Media Server"
#define RTSPD_SERVER_VERSION "v1.0.0"

#define RTSPD_RTP_ENTRY_MAX_CNT (200)
typedef struct RtpEntry
{
	void* ptr;
	int len;
}RtpEntry_t;

#define RTSPD_SESSION_REQUEST_BUF_SZ (2 * 1024)
#define RTSPD_SESSION_RESPONSE_BUF_SZ (256 * 1024)
typedef struct RtspdSession
{
	// handler
	int sock;
	uint32_t* trigger;
	pthread_t sub_task_tid;

	// operating
	char request_buf[RTSPD_SESSION_REQUEST_BUF_SZ];
	ssize_t request_sz;

	char response_buf[RTSPD_SESSION_RESPONSE_BUF_SZ];
	ssize_t response_sz;

	RtpEntry_t rtp_entry[RTSPD_RTP_ENTRY_MAX_CNT];
	int rtp_entries;

	// rtp info
	uint32_t session_id;

	char server_url[128];
	char server_ip[32];
	uint16_t server_port;
	char server_trace[32];
	int server_udp_port0;
	int server_udp_port1;
	int server_udp_sock0;
	int server_udp_sock1;

	char client_ip[32];
	int client_udp_port0;
	int client_udp_port1;

#define RTSPD_STOP (0)
#define RTSPD_PLAYING (1)
#define RTSPD_PAUSE (2)
	int playing;

#define RTP_OVER_TCP (0)
#define RTP_UDP (1)
	int transport;

	// rtp attribute
	uint32_t rtp_seq;
	uint32_t rtp_timestamp;
	//rtcp attribute	
	uint32_t rtp_packetcount;
	uint32_t rtp_octetcount;
	uint32_t total_session_bw;
	int members;
	int pmembers;     
	int senders;
	double avg_rtcp_size;
	int initial;
	double prev_report_time; //tp
	double next_report_time;  //tn
	int last_received_size;
	uint32_t last_received_ssrc;
	int type_of_packet;
	int type_of_event;
}RtspdSession_t;

extern SPOOK_SESSION_PROBE_t RTSPD_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t RTSPD_loop(uint32_t* trigger, int sock, time_t* read_pts, const void* msg, ssize_t msg_sz);

#endif //__RTSPD_H__

