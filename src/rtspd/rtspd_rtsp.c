
#include <sys/socket.h>
#include <netinet/in.h>

#include "spook/rtspd.h"
#include "rtspd_debug.h"
#include "rtsp_parser.h"


#define DATE_HEADER_MAX_SZ (64)
static ssize_t date_header(char ret_date[DATE_HEADER_MAX_SZ], time_t tt)
{
	if(ret_date){
		char* const date = (char*)ret_date;
		if(!tt){
			time(&tt);
		}
		strftime(date, DATE_HEADER_MAX_SZ, "%a, %b %d %Y %H:%M:%S GMT", gmtime(&tt));
		return strlen(date);
	}
	return -1;
}

static uint32_t rtsp_request_check(const char* request, ssize_t request_sz)
{
	if(request_sz > 4){
		char* ptr = strstr(request, "\r\n\r\n");
		if(ptr){
			if(ptr - request + strlen("\r\n\r\n") == request_sz){
				RTSPD_TRACE("request completed");
				return true;
			}
		}
	}
	return false;
}

#define ALLOWED_COMMAND_NAMES "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, GET_PARAMETER"
int rtsp_response_options(RtspdSession_t* session)
{
	int cseq = 0;
	char date_str[DATE_HEADER_MAX_SZ];
	cseq = RTSPPARSER_read_int(session->request_buf, "CSeq");
	date_header(date_str, 0);
	sprintf(session->response_buf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Date: %s\r\n"
		"Public: %s\r\n\r\n",
		cseq,
		date_str,
		ALLOWED_COMMAND_NAMES);
	session->response_sz = strlen(session->response_buf);
	return 0;
}

static int rtspd_handle_rtcp(RtspdSession_t* session)
{
	return 0;
}

static int rtsp_handle_describe(RtspdSession_t* session)
{
	char sdq_buf[1024];
	int cseq;

	cseq = RTSPPARSER_read_int(session->request_buf, "CSeq");

	sprintf(sdq_buf,
		"v=0\r\n"
		"o= %u %d IN IP4 %s\r\n"
		"s=H.264 Video, streamed by the %s\r\n"
		"i=%s\r\n"
		"t=0 0\r\n"
		"a=tool:%s %s %s %s\r\n"
		"a=type:broadcast\r\n"
		"a=control:*\r\n"
		"a=range:npt=0-\r\n"
		"a=x-qt-text-nam:H.264 Video, streamed by the %s\r\n"
		"a=x-qt-text-inf:%s\r\n"
		"m=video 0 RTP/AVP 96\r\n"
		"c=IN IP4 0.0.0.0\r\n"
		"b=AS:500\r\n"
		"a=rtpmap:96 H264/90000;packetization-mode=1\r\n"
		"a=fmtp:96\r\n"
		"%s",
		session->session_id, 1, session->server_ip, // o= <session id> o= <version> o= <address>
		RTSPD_SERVER_NAME, // s=
		session->server_trace, // i=
		RTSPD_SERVER_NAME, RTSPD_SERVER_VERSION, __DATE__, __TIME__, // a= <libname> <libversion>
		RTSPD_SERVER_NAME, // a=x-qt-text-nam: <name>
		session->server_trace, // a=x-qt-text-inf: <info>
		""); // miscellaneous session SDP lines (if any)

	sprintf(session->response_buf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Content-Base: rtsp://%s:%u/%s/\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s",
		cseq,
		session->server_ip, session->server_port, session->server_trace,
		strlen(sdq_buf),
		sdq_buf);
	session->response_sz = strlen(session->response_buf);
	return 0;
}

static int rtsp_handle_setup(RtspdSession_t* session)
{
	int cseq;
	char transport[128];
	char date_str[DATE_HEADER_MAX_SZ];
	
	AVal av_parser = RTSPPARSER_create((char*)session->request_buf, session->request_sz);
	AVal av_transport = RTSPPARSER_read_option(av_parser, "Transport");

	//check tcp/udp
	AVal av_transport_value_tcp = AVC("RTP/AVP/TCP");
	AVal av_transport_value = { 0 };

	av_transport_value.av_val = strstr(session->request_buf, "Transport: ");
	if(av_transport_value.av_val != NULL){
		av_transport_value.av_val += strlen("Transport: ");
		av_transport_value.av_len = strchr(av_transport_value.av_val, ';') - av_transport_value.av_val;
	}
	if(AVMATCH(&av_transport_value, &av_transport_value_tcp)){
		session->transport = RTP_OVER_TCP;
	}else{
		session->transport = RTP_UDP;
	}

	if(session->transport == RTP_UDP){
		char* p = strstr(av_transport_value.av_val, "client_port");
		if(!p){
			return -1;
		}
		if(2 != sscanf(p, "client_port=%d-%d;", &session->client_udp_port0, &session->client_udp_port1)){
			return -1;
		}
	}

	RTSPD_TRACE("Setup: tcp/udp=%d, port0=%d, port1=%d",
			session->transport, session->client_udp_port0, session->client_udp_port1);

	{
		static int server_udp_port0 = 60000;
		static int server_udp_port1 = 60001;


		int ret = 0;

		int nSock0 = socket(AF_INET, SOCK_DGRAM, 0);
		struct sockaddr_in server_addr0;
		server_addr0.sin_family = AF_INET;
		server_addr0.sin_port = htons(server_udp_port0);
		server_addr0.sin_addr.s_addr = INADDR_ANY;
		memset(&(server_addr0.sin_zero), 0,  sizeof(server_addr0.sin_zero));
		ret = bind(nSock0, (struct sockaddr *)&server_addr0, sizeof(struct sockaddr));
		assert(ret <= 0);
		session->server_udp_port0 = server_udp_port0;
		session->server_udp_sock0 = nSock0;
		server_udp_port0 += 2;

		int nSock1 = socket(AF_INET, SOCK_DGRAM, 0);
		struct sockaddr_in server_addr1;
		server_addr1.sin_family = AF_INET;
		server_addr1.sin_port = htons(server_udp_port1);
		server_addr1.sin_addr.s_addr = INADDR_ANY;
		memset(&(server_addr1.sin_zero), 0,  sizeof(server_addr1.sin_zero));
		ret = bind(nSock1, (struct sockaddr *)&server_addr1, sizeof(struct sockaddr));
		assert(!(ret < 0));
		session->server_udp_port1 = server_udp_port1;
		session->server_udp_sock1 = nSock1;
		server_udp_port1 += 2;


		RTSPD_TRACE("server port0=%d, port1=%d, sock0=%d, sock1=%d",
				session->server_udp_port0, session->server_udp_port1,
				session->server_udp_sock0, session->server_udp_sock1);
	}

	//Transport: RTP/AVP/UDP;unicast;client_port=32000-32001;server_port=61004-61005
	cseq = RTSPPARSER_read_int(session->request_buf, "CSeq");
	date_header(date_str, 0);
	
	memset(transport, 0, sizeof(transport));
	strncpy(transport, av_transport.av_val, av_transport.av_len);
	sprintf(transport + strlen(transport), ";server_port=%d-%d", session->server_udp_port0, session->server_udp_port1);
	
	sprintf((char*)session->response_buf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Date: %s\r\n"
		"Transport: %s\r\n"
		"Session: %X\r\n\r\n",
		cseq,
		date_str,
		transport,
		session->session_id);
	session->response_sz = strlen((char*)session->response_buf);
	return 0;
}

static int rtsp_handle_play(RtspdSession_t* session)
{
	int cseq;
	char date_str[DATE_HEADER_MAX_SZ];
	uint64_t tc = (uint64_t)time(NULL);
	// covertion utc to 90000 hz
	tc *= 90000;
	tc /= 10000000;
	session->rtp_seq = rand() % 4096;
	session->rtp_timestamp= (uint32_t)tc;

	cseq= RTSPPARSER_read_int(session->request_buf, "CSeq");
	date_header(date_str, 0);
	sprintf((char*)session->response_buf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Date: %s\r\n"
		"Range: npt=0.000-\r\n"
		"Session: %X\r\n"
		"RTP-Info: url=%s;seq=%u;rtptime=%u\r\n\r\n",
		cseq,
		date_str,
		session->session_id,
		session->server_url,
		session->rtp_seq,
		session->rtp_timestamp);
	session->response_sz = strlen((char*)session->response_buf);
	return 0;
}

static int rtsp_handle_teardown(RtspdSession_t* session)
{
	return 0;
}

static int rtsp_handle_get_parameter(RtspdSession_t* session)
{
	int cseq;
	char date_str[DATE_HEADER_MAX_SZ];

	cseq= RTSPPARSER_read_int(session->request_buf, "CSeq");
	date_header(date_str, 0);
	sprintf((char*)session->response_buf,
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %d\r\n"
		"Date: %s\r\n"
		"Session: %X\r\n"
		"\r\n",
		cseq,
		date_str,
		session->session_id);
	session->response_sz = strlen((char*)session->response_buf);
	return 0;
}

int RTSP_handle(RtspdSession_t* session)
{
	int ret = 0;
	AVal av_options = AVC("OPTIONS");
	AVal av_describe = AVC("DESCRIBE");
	AVal av_setup = AVC("SETUP");
	AVal av_play = AVC("PLAY");
	AVal av_teardown = AVC("TEARDOWN");
	AVal av_get_parameter = AVC("GET_PARAMETER");
	//
	AVal av_parser = RTSPPARSER_create((const char*)session->request_buf, session->request_sz);
	AVal av_cmd = RTSPPARSER_get_command(av_parser);

	RTSPD_TRACE_REQUEST("%s", session->request_buf);
	if(AVMATCH(&av_cmd, &av_options)){
		// OPTIONS
		ret = rtsp_response_options(session);
	}else if(AVMATCH(&av_cmd, &av_describe)){
		// DESCRIBE
		ret = rtsp_handle_describe(session);
	}else if(AVMATCH(&av_cmd, &av_setup)){
		// SETUP
		ret = rtsp_handle_setup(session);
	}else if(AVMATCH(&av_cmd, &av_play)){
		// PLAY
		ret = rtsp_handle_play(session);
		session->playing = RTSPD_PLAYING;
	}else if(AVMATCH(&av_cmd, &av_teardown)){
		// TEARDOWN
		ret = rtsp_handle_teardown(session);
		ret = -1;
	}else if(AVMATCH(&av_cmd, &av_get_parameter)){
		ret = rtsp_handle_get_parameter(session);
	}else{
		//unknown
		RTSPD_TRACE("req unknown=%s", session->request_buf);
		ret = -1;
	}
	RTSPD_TRACE_RESPONSE("%s", session->response_buf);
	return ret;
}

