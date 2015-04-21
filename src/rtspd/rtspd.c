

#include "spook/rtspd.h"


#include "rtspd_debug.h"

#include "rtsp_parser.h"
#include "rtp_h264.h"
#include "socket_rw.h"

#include "rtspd_rtsp.h"
#include "rtspd_rtcp.h"

#include "sdk/sdk_api.h"
#include "mediabuf/mediabuf.h"

#define RTSPD_READ_TIMEOUT (10)
#define RTSPD_WRITE_TIMEOUT (10)

static void rtspd_print_session(RtspdSession_t* session)
{
	printf("RTSPD Session\r\n");
	printf("Socket Fd: %d\r\n", session->sock);
	printf("Task Thread ID: %u\r\n", (uint32_t)session->sub_task_tid);
	if(session->request_sz > 0){
		printf("Request Buffer:\r\n%s\r\n", session->request_buf);
	}
	if(session->response_sz > 0){
		printf("Response Buffer:\r\n%s\r\n", session->response_buf);
	}
	printf("Session ID: %u\r\n", session->session_id);

	printf("Client Address: %s\r\n", session->client_ip);
	printf("Server URL: %s\r\n", session->server_url);
	printf("Server Address: %s\r\n", session->server_ip);
	printf("Server Port: %u\r\n", session->server_port);
	printf("Server Trace: %s\r\n", session->server_trace);

}

static RtspdSession_t* rtspd_create_session(uint32_t* trigger, int sock,  const char* req_msg, ssize_t req_msg_sz)
{
	struct sockaddr_in addr_in;
	socklen_t addr_len = sizeof(addr_in);
	struct timeval session_tv;

	// create a data area
	RtspdSession_t* const session = calloc(sizeof(RtspdSession_t), 1);

	// init elements
	session->sock = sock;
	session->trigger = trigger;
	session->sub_task_tid = (pthread_t)NULL;

//	session->request_sz = req_msg_sz;
//	memcpy(session->request_buf, req_msg, session->request_sz);
//	session->response_sz = 0;

	session->request_sz = 0;
	session->response_sz = 0;

	// rtp info
	session->session_id = 0;
	gettimeofday(&session_tv, NULL);
	session->session_id = session_tv.tv_sec ^ session_tv.tv_usec;
	// get client address
	memset(&addr_in, 0, addr_len);
	getpeername(session->sock, (struct sockaddr *)&addr_in, &addr_len);
	strcpy(session->client_ip, inet_ntoa(addr_in.sin_addr));
	// get server ip and port
	RTSPD_parse_request(req_msg, NULL, session->server_url);
	RTSPD_parse_url(session->server_url, session->server_ip, &session->server_port, session->server_trace);

	rtspd_print_session(session);

	return session;
}

static void rtspd_free_session(RtspdSession_t* session)
{
	free(session);
}

int is_rtsp_req_complete(char* _buf, int _buf_len)
{
	if(_buf_len > 4
			&& _buf[_buf_len - 4] == '\r'
			&& _buf[_buf_len - 3] == '\n'
			&& _buf[_buf_len - 2] == '\r'
			&& _buf[_buf_len - 1] == '\n')
	{
		return 1;
	}
	return 0;
}

static void udp_send2(RtspdSession_t* session, void* buf, ssize_t buf_sz, const char* client_ip, int client_port)
{
	struct sockaddr_in stAddrIn;
	memset(&(stAddrIn), 0, sizeof(struct sockaddr_in));
	stAddrIn.sin_family = AF_INET;
	stAddrIn.sin_port = htons(client_port);
	stAddrIn.sin_addr.s_addr = inet_addr(client_ip);
	sendto(session->server_udp_sock0, buf, buf_sz, 0, (struct sockaddr *)&(stAddrIn), sizeof(struct sockaddr_in));

}

static char* _rtspd_server_map_path(char* _url, char* _file_path)
{
	RTSPD_TRACE("_url=%s", _url);

	_file_path[0] = 0;

	char* p = strrchr(_url, '/');
	char* folder_name = "./h264_raw/";

	if(p != NULL)
	{
		sprintf(_file_path, "%s%s", folder_name, p);
	}

	RTSPD_TRACE("_file_path=%s", _file_path);
	return _file_path;
}

SPOOK_SESSION_PROBE_t RTSPD_probe(const void* msg, ssize_t msg_sz)
{
	int ret = 0;
	AVal av_options = AVC("OPTIONS");
	AVal av_describe = AVC("DESCRIBE");
	AVal av_set_parameter = AVC("SET_PARAMETER");
	AVal av_get_parameter = AVC("GET_PARAMETER");
	//
	AVal av_parser = RTSPPARSER_create((const char*)msg, msg_sz);
	AVal av_cmd = RTSPPARSER_get_command(av_parser);
	
	if(AVMATCH(&av_cmd, &av_options)
		|| AVMATCH(&av_cmd, &av_describe)
		|| AVMATCH(&av_cmd, &av_set_parameter)
		|| AVMATCH(&av_cmd, &av_get_parameter)){
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t RTSPD_loop(uint32_t* trigger, int sock, time_t* read_pts, const void* msg, ssize_t msg_sz)
{
	RtspdSession_t* session = NULL;

	int i;

	int ret = 0;
	
	int first_request = 0;

	int rtsp_req_complete = 0;
	int rtsp_req_handle_ret = 0;

	int rtcp_req_handle_ret = 0;

	unsigned char* ptr = NULL;
	int len = 0;
	unsigned char buf[2048];

	SOCKET_RW_CTX rw_ctx;
//	H264_FILE_BUFFER* hfb = NULL;

	struct timeval timeout;
	fd_set read_set;
	fd_set write_set;
	int select_ret;

	int rtcp_head_len = 0;
	int rtcp_body_len = 0;
	char file_name[64];

	void* payload_ptr = NULL;
	ssize_t payload_sz = 0;

	int nalu_size;
	int mediabuf_ch = 0;
	MediaBufUser_t* user = NULL;

	session = rtspd_create_session(trigger, sock, msg, msg_sz);

	mediabuf_ch = MEDIABUF_lookup_byname(session->server_trace);

	RTSPD_TRACE("connecting to mediabuf %d", mediabuf_ch);

	if(mediabuf_ch < 0){
		return SPOOK_LOOP_FAILURE;
	}

	user = MEDIABUF_user_attach(mediabuf_ch);
	if(!user){
		return SPOOK_LOOP_FAILURE;
	}
	
	
	//rtcp init
	RTCP_init(session);
	while(*session->trigger)
	{
		timeout.tv_sec = 0;
		timeout.tv_usec = 50*1000;
		FD_ZERO(&read_set);
		FD_SET(session->sock, &read_set);
		if(session->server_udp_sock1 > 0)
		{
//			RTSPD_TRACE("server_udp_sock1=%d", session->server_udp_sock1);
			FD_SET(session->server_udp_sock1, &read_set);
		}
		if(first_request != 0)
		{
#define MAX(a, b) ((a) > (b) ? (a) : (b))
			select_ret = select(MAX(session->sock, session->server_udp_sock1)+ 1, &read_set, NULL, NULL, &timeout);
		}
		else
		{
			select_ret = 1;
		}

		if (select_ret < 0)
		{
			*session->trigger = 0;
//			printf("read select error stop\n");
			break;
		}
		else if(select_ret == 0)
		{
//			printf("read select timeout...\n");
		}
		else
		{
			if(first_request == 0 || FD_ISSET(session->sock, &read_set))
			{
				//处理上传
				if(first_request == 0)
				{
					ptr = (unsigned char*)msg;
					len = msg_sz;
					first_request = 1;
				}
				else
				{
					SOCKETRW_rwinit(&rw_ctx, session->sock, buf, sizeof(buf), RTSPD_READ_TIMEOUT);
					SOCKETRW_read(&rw_ctx);
					if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
					{
						*session->trigger = 0;
						break;
					}

					ptr = buf;
					len = rw_ctx.actual_len;
				}

				for(i = 0; i < len; i++)
				{

					//rtcp over tcp
					if(ptr[i] == '$') //rtcp start
					{
						session->request_sz = 0;
						session->request_buf[session->request_sz] = ptr[i];
						session->request_sz++;
						rtcp_head_len = 3;
						continue;
					}

					if(rtcp_head_len > 0) //rtcp head
					{
						session->request_buf[session->request_sz] = ptr[i];
						session->request_sz++;
						rtcp_head_len--;

						if(rtcp_head_len == 0)
						{
							rtcp_body_len = *((unsigned char*)(session->request_buf+3)) | (*((unsigned char*)(session->request_buf+2)) << 8);
						}
						continue;
					}

					if(rtcp_body_len > 0) //rtcp body
					{
						session->request_buf[session->request_sz] = ptr[i];
						session->request_sz++;
						rtcp_body_len--;
						if(rtcp_body_len == 0)
						{
							RTSPD_TRACE("found tcp rtcp len=%d p[0]=%02x p[1]=%02x p[2]=%02x p[3]=%02x",
									rtcp_body_len, session->request_buf[0], session->request_buf[1], session->request_buf[2], session->request_buf[3]);
							rtcp_req_handle_ret = RTCP_handle(session);
							if(rtcp_req_handle_ret != 0)
							{
								*session->trigger = 0;
								break;
							}
							if(session->response_sz > 0)
							{
								SOCKETRW_rwinit(&rw_ctx, session->sock, session->response_buf, session->response_sz, RTSPD_WRITE_TIMEOUT);
								SOCKETRW_writen(&rw_ctx);
								if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
								{
									*session->trigger = 0;
									break;
								}
							}

							session->response_sz = 0;
							session->request_sz = 0;
						}
						continue;
					}


					//rtsp
					session->request_buf[session->request_sz] = ptr[i];
					session->request_sz++;

					rtsp_req_complete = is_rtsp_req_complete(session->request_buf, session->request_sz);
					if(rtsp_req_complete == 1)
					{
						session->request_buf[session->request_sz] = 0;
						rtsp_req_handle_ret = RTSP_handle(session);
						if(rtsp_req_handle_ret != 0)
						{
							*session->trigger = 0;
							break;

						}
						if(session->response_sz > 0)
						{
							SOCKETRW_rwinit(&rw_ctx, session->sock, session->response_buf, session->response_sz, RTSPD_WRITE_TIMEOUT);
							SOCKETRW_writen(&rw_ctx);
							if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
							{
								*session->trigger = 0;
								break;
							}
						}
						session->response_sz = 0;
						session->request_sz = 0;
					}
				}
			}

			if(FD_ISSET(session->server_udp_sock1, &read_set))
			{
				session->request_sz = recvfrom(session->server_udp_sock1, session->request_buf, sizeof(session->request_buf), 0, NULL, NULL);
				if(session->request_sz > 0)
				{
					RTSPD_TRACE("found udp rtcp len=%d p[0]=%02x p[1]=%02x p[2]=%02x p[3]=%02x",
							rtcp_body_len, session->request_buf[0], session->request_buf[1], session->request_buf[2], session->request_buf[3]);
					rtcp_req_handle_ret = RTCP_handle(session);
					if(rtcp_req_handle_ret != 0)
					{
						*session->trigger = 0;
						break;
					}
					if(session->response_sz > 0)
					{
						//todo udp send backup
					}

					session->response_sz = 0;
					session->request_sz = 0;
				}
			}
		}

		if(session->playing == RTSPD_PLAYING)
		{
			RTCP_schedule(session);
			if(session->response_sz > 0)
			{
				SOCKETRW_rwinit(&rw_ctx, session->sock, session->response_buf, session->response_sz, RTSPD_WRITE_TIMEOUT);
				SOCKETRW_writen(&rw_ctx);
				if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
				{
					*session->trigger = 0;
					break;
				}
				session->response_sz = 0;
			}
						
			if(0 == MEDIABUF_out_lock(user)){
				ret = MEDIABUF_out(user, &payload_ptr, NULL, &payload_sz);
				if(0 == ret){
					RTP_packet_nalu(session, payload_ptr + sizeof(SDK_AVENC_BUF_ATTR_t), payload_sz - sizeof(SDK_AVENC_BUF_ATTR_t));
					if(session->transport == RTP_OVER_TCP)
					{
						timeout.tv_sec = 0;
						timeout.tv_usec = 10*1000;
						FD_ZERO(&write_set);
						FD_SET(session->sock, &write_set);
						select_ret = select(session->sock + 1, NULL, &write_set, NULL, &timeout);
						if (select_ret < 0)
						{
							*session->trigger = 0;
							printf("write select error stop\n");
						}
						else if(select_ret == 0)
						{
							printf("write select timeout...\n");
						}
						else
						{
							for(i = 0; i < session->rtp_entries; ++i)
							{
								SOCKETRW_rwinit(&rw_ctx, session->sock, session->rtp_entry[i].ptr, session->rtp_entry[i].len, RTSPD_WRITE_TIMEOUT);
								SOCKETRW_writen(&rw_ctx);
								if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
								{
									*session->trigger = 0;
									break;
								}
							}
						}
					}
					else if(session->transport == RTP_UDP)
					{
						RTSPD_ASSERT(session->server_udp_sock0 != 0);
						RTSPD_ASSERT(session->server_udp_sock1 != 0);

						for(i = 0; i < session->rtp_entries; i++)
						{
							udp_send2(session, session->rtp_entry[i].ptr, session->rtp_entry[i].len,
									session->client_ip, session->client_udp_port0);
						}
					}
				}
				MEDIABUF_out_unlock(user); // unlock the buf
				if(!ret){
					usleep(MEDIABUF_in_speed(user->pool_ch));
				}
			}
		}
	}

	MEDIABUF_user_detach(user);
	user = NULL;

	close(session->sock);
	if(session->transport == RTP_UDP)
	{
		close(session->server_udp_sock0);
		close(session->server_udp_sock1);
	}
	rtspd_free_session(session);

	return SPOOK_LOOP_SUCCESS;
}


