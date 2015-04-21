

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>                 /*socket address struct*/
#include <arpa/inet.h>                  /*host to network convertion*/
#include <sys/socket.h>

// from app
#include "adobe/bufio.h"
#include "adobe/libflv.h"
#include "httpd.h"
#include "http_common.h"
#include "socket_rw.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"
#include "app_debug.h"
#include "cgi.h"

//#define CGI_TRUE (1)
//#define CGI_FALSE (0)

//#define DEBUG_FLV
//#ifdef DEBUG_FLV
//#define APP_TRACE(fmt...) printf(fmt)
//#define FLV_ERROR(fmt...) printf(fmt)
//#else
//#define APP_TRACE(fmt...)
//#define FLV_ERROR(fmt...) printf(fmt)
//#endif


static int flv_send(HTTPD_SESSION_t* _session, void* buf, ssize_t size)
{
	//int ret = fwrite(buf, 1, size, stdout);
	SOCKET_RW_CTX rw_ctx;
	SOCKETRW_rwinit(&rw_ctx, _session->sock, (void *)buf, size, _session->keep_alive);
	SOCKETRW_writen(&rw_ctx);
	if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
	{
		return -1;
	}
	return 0;
}

static int flv_send_http_header(HTTPD_SESSION_t *_session,unsigned int number, unsigned int filesize)
{
#define CONTENT_TYPE "flv-application/octet-stream"
//#define CONTENT_TYPE "application/octet-stream"
//#define CONTENT_TYPE "video/f4v"
	char mime[128] = {""};
	memset(mime, 0, sizeof(mime));
	if(200 == number)
	{
		const char header[] =
			"HTTP/1.0 200 OK\r\n"
			"Content-Length: %u\r\n" // size here
			"Connection: close\r\n"
			"Content-Type: "CONTENT_TYPE"\r\n"
			"\r\n";
		sprintf(mime, header, filesize);
	}
	else if(404 == number)
	{
		const char header[] =
			"HTTP/1.0 404 Not Found\r\n"
			"Connection: close\r\n"
			"Content-Type: "CONTENT_TYPE"\r\n"
			"\r\n";
		sprintf(mime, header);
	}
	return flv_send(_session, mime, strlen(mime)); // this length is very import for some browser, assure it's accurate
}

static int flv_send_flv_header(HTTPD_SESSION_t *_session,
		int width, int height, unsigned long duration,
		unsigned char* _data, int _data_len)
{
	char buf[2048];
	BufIO bufio = {0};

	memset(buf, 0, sizeof(buf));
	memset(&bufio, 0, sizeof(BufIO));

	bufio.buffer = (unsigned char*)buf;
	bufio.buf_ptr = bufio.buffer;
	bufio.buffer_size = sizeof(buf);
	bufio.buf_end = bufio.buffer+sizeof(buf);

	avlib_flv_mkheader(&bufio, width, height, 256*1000, 25, duration, _data, _data_len);
	return flv_send(_session, bufio.buffer, bufio.actlen);
}

static int flv_send_frame(HTTPD_SESSION_t *_session,void* frame, int framesz, int iskeyframe, unsigned long long tsp, void* buf, int bufsz)
{
	BufIO bufio = {0};
	memset(&bufio, 0, sizeof(BufIO));
	bufio.buffer = buf;
	bufio.buf_ptr = bufio.buffer;
	bufio.buffer_size = bufsz;
	bufio.buf_end = bufio.buffer + bufio.buffer_size;

	avlib_flv_write_packet(&bufio, frame, framesz, tsp, iskeyframe ? 1 : 0);
	return flv_send(_session, bufio.buffer, bufio.actlen);
}

int CGI_flv_live_view(HTTPD_SESSION_t* session)
{
	int ret = 0;
	int return_ret = 0;

	//unsigned long long basetsp = 0;
	unsigned char frame[512 * 1024];
	lpSDK_ENC_BUF_ATTR pavenc=NULL;
	int mediabuf_ch;
	int sent_avc = 0;
	const char* const query_string = AVAL_STRDUPA(session->request_line.uri_query_string);
	AVal av_chn = AVC("0");

	http_read_query_string(query_string, "chn", &av_chn);
	mediabuf_ch = atoi(AVAL_STRDUPA(av_chn));

	//mediabuf_ch = MEDIABUF_lookup_byname("720p.264");
	if(mediabuf_ch < 0)
	{
		APP_TRACE("para: chn not valid, old=%d, new=0", mediabuf_ch);
		mediabuf_ch = 0;
	}
	APP_TRACE("connecting to mediabuf %d", mediabuf_ch);
	lpMEDIABUF_USER user = NULL;

	user = MEDIABUF_attach(mediabuf_ch);
	if(!user)
	{
		return_ret = -1;
		goto exit_cgi_flv_main;
	}


	int inspeed = MEDIABUF_in_speed(mediabuf_ch);
	uint64_t base_pts = 0;
	MEDIABUF_sync(user);
	for(;;)
	{
		if(0 == MEDIABUF_out_lock(user))
		{
			void* payload_ptr = NULL;
			ssize_t payload_size = 0;

//			APP_TRACE("MEDIABUF_out_lock ok.");
			ret = MEDIABUF_out(user, &payload_ptr, NULL, &payload_size);
			MEDIABUF_out_unlock(user); // unlock the buf
//			APP_TRACE("MEDIABUF_out ret=%d.", ret);
			if(0 == ret)
			{
				pavenc=(lpSDK_ENC_BUF_ATTR)payload_ptr;
//				APP_TRACE("MEDIABUF_out ok. sent_avc=%d, pavenc->h264.keyframe=%d", sent_avc, pavenc->h264.keyframe);
				if(sent_avc == 0)
				{
//					APP_TRACE("sent_avc == 0.");
					if(pavenc->h264.keyframe == 1)
					{
//						APP_TRACE("iframe, send flv file head.");
						ret = flv_send_http_header(session, 200,  0x7fffffff ); // if live stream, the size is as large as possible
						if(ret != 0)
						{
							return_ret = -1;
							goto exit_cgi_flv_main1;
						}
						ret = flv_send_flv_header(session,
								1280, 720, 0x7fffffff,
								payload_ptr + sizeof(stSDK_ENC_BUF_ATTR), payload_size - sizeof(stSDK_ENC_BUF_ATTR));
						if(ret != 0)
						{
							return_ret = -1;
							goto exit_cgi_flv_main1;
						}
						sent_avc = 1;
					}
					else
					{
//						APP_TRACE("pframe, ignore it.");
						continue;
					}
				}

//				APP_TRACE("send frame.");
				uint32_t cur_pts = 0;

				if(!base_pts)
				{
					base_pts = pavenc->timestamp_us;
				}

				cur_pts = (pavenc->timestamp_us - base_pts + 1000 - 1) / 1000;
				ret = flv_send_frame(session,
						payload_ptr + sizeof(stSDK_ENC_BUF_ATTR),
						payload_size - sizeof(stSDK_ENC_BUF_ATTR),
						pavenc->h264.keyframe,
						cur_pts,
						frame,
						sizeof(frame));

//				APP_TRACE("pts = %d\r\n", cur_pts);
				if(ret < 0)
				{
					APP_TRACE("--------------------send fail , exit !!!");
					return_ret = -1;
					goto exit_cgi_flv_main1;
				}
			}
			else
			{
				//APP_TRACE("x");
				usleep(inspeed / 2);
			}
		}
		else
		{
			APP_TRACE("LOCK mediabuf failed");
		}
	}
exit_cgi_flv_main1:
	MEDIABUF_detach(user);
exit_cgi_flv_main:
	return return_ret;
}

