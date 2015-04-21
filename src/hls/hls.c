/*
 * ts.c
 *
 *  Created on: 2012-3-15
 *      Author: root
 */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "sdk/sdk_api.h"
#include "media_buf.h"


#include "hls_segmentor.h"
#include "hls_debug.h"
#include "mpegts.h"
#include "hls.h"

#include "generic.h"
#include "jsocket.h"

#define HLS_HTTP_M3U8_MIME "application/vnd.apple.mpegurl"
#define HLS_HTTP_MPEG2TS_MIME "video/MP2T"

#define HLS_M3U8_MEDIA_DURATION (3)
#define HLS_M3U8_MEDIA_COUNT (32)


#define HLS_SESSION_BACKLOG (5)
typedef struct HLS_SESSION
{
	uint32_t session_id; // session identify

	uint32_t m3u8_counter; // count how many times m3u8 keep-alive
	pid_t m3u8_pid;
	pthread_t m3u8_tid;
	
	lpMEDIABUF_USER buf_user;
	int buf_speed;
	pthread_mutex_t buf_mutex;
	
	uint32_t media_sequence;
	uint32_t media_ready;
	uint64_t media_ti_us;
}HLS_SESSION_t;

void hlsd_session_reclaim(HLS_SESSION_t* hls_session)
{
	
	hls_session->media_sequence = 0;
	hls_session->media_ready = 0;
	hls_session->media_ti_us = 0;

	pthread_mutex_destroy(&hls_session->buf_mutex);
	hls_session->buf_speed = 0;
	if(hls_session->buf_user){
		MEDIABUF_detach(hls_session->buf_user);
		hls_session->buf_user = NULL;
	}
	hls_session->m3u8_tid = 0;
	hls_session->m3u8_pid = 0;
	hls_session->m3u8_counter = 0;
	hls_session->session_id = 0;

	HLS_TRACE("Reclaim an HLS session");
}

typedef struct HLS_SERVER
{
	int n_session;
	HLS_SESSION_t session[HLS_SESSION_BACKLOG];
}HLS_SERVER_t;
static HLS_SERVER_t _hls_server;
static HLS_SERVER_t* _p_hls_server = NULL;

static HLS_SERVER_t* hlsd_session_lookup(int const session_id)
{
	int i = 0;
	HLS_ASSERT(_p_hls_server, "HLS has not been built!");
	for(i = 0; i < HLS_SESSION_BACKLOG; ++i){
		const HLS_SESSION_t* hls_session = _p_hls_server->session + i;
		if(session_id == hls_session->session_id){
			// session has been existed, keep it alive, come on!
			return hls_session;
		}
	}
	return NULL;
}

static HLS_SERVER_t* hlsd_session_new(int const session_id)
{
	int i = 0;
	HLS_ASSERT(_p_hls_server, "HLS has not been built!");
	for(i = 0; i < HLS_SESSION_BACKLOG; ++i){
		HLS_SESSION_t* const hls_session = _p_hls_server->session + i;
		if(0 == hls_session->session_id){
			// new a session
			HLS_TRACE("New an HLS session @ %d", i);
			hlsd_session_reclaim(hls_session);
			hls_session->session_id = session_id;
			return hls_session;
		}
	}
	return NULL;
}


int HLSD_init()
{
	int i = 0;
	if(!_p_hls_server){
		STRUCT_ZERO(_hls_server);
		_p_hls_server = &_hls_server;
		return 0;
	}
	return -1;
}

void HLSD_destroy()
{
	if(_p_hls_server){
		// FIXME:

	}
}

static int hlsd_get_session_id(HTTPD_SESSION_t* http_session)
{
	AVal av_sessionid = AVC("0");
	const char* const str_query_string = AVAL_STRDUPA(http_session->request_line.uri_query_string);

	if(0 == http_read_query_string(str_query_string, "session", &av_sessionid)){
		return atoi(AVAL_STRDUPA(av_sessionid));
	}
	return -1;
}

void hlsd_mark_pid(HLS_SESSION_t* hls_session)
{
	hls_session->m3u8_pid = getpid();
	hls_session->m3u8_tid = pthread_self();
}

/*
<html>
<script>
function get_session_id()
{
	var ret = parseInt(Math.random() * 65536);
	alert(ret);
	return ret;
}
function gen_video()
{
	var str = "<video width=\"100%\" height=\"100%\" src=\"/hls/live.m3u8?session=";
	str += get_session_id();
	str += "\" poster=\"/snapshot?size=320x240\" controls=\"controls\" autoplay=\"autoplay\" preload=\"auto\" loop=\"loop\">This is an HLS demo</video>";
//	str += "\" poster=\"/snapshot?size=320x240\" loop=\"loop\" >This is an HLS demo</video>";
	document.write(str);
}
</script>
<body>
<!--video src="/m3u8?session="<script>document.write(get_session_id())</script> >This is an HLS demo </video-->
<script>gen_video()</script>
</body>
</html>
*/

int CGI_hls_live_m3u8(HTTPD_SESSION_t* http_session)
{
	int i = 0;
	int ret = 0;
	int n32_sessionid = hlsd_get_session_id(http_session);
	HLS_SESSION_t* hls_session = NULL;
	
	// look up this session id and check whether it has been ready
	hls_session = hlsd_session_lookup(n32_sessionid);
	if(!hls_session){
		AVal av_useragent = AVC("unknown");
		char* str_useragent = NULL;
		int buf_id = 1;
		
		http_read_header(http_session->request_buf, "User-Agent", &av_useragent);
		str_useragent = AVAL_STRDUPA(av_useragent);

		// bind to different buf id
		if(strstr(str_useragent, "AppleCoreMedia")){
			if(strstr(str_useragent, "iPad")){
				buf_id = MEDIABUF_lookup_byname("360p.264");
			}else if(strstr(str_useragent, "iPhone")){
				buf_id = MEDIABUF_lookup_byname("qvga.264");
			}

		}
		
		// this session is a stranger to HLS server
		// setup a new session is a smart job
		HLS_TRACE("Setup a new HLS session id is %d", n32_sessionid);
		HLS_TRACE("User-Agent: %s", str_useragent);
		hls_session = hlsd_session_new(n32_sessionid);

		// this is a new session
		// you need to assign a new mediabuf user for getting media data and get its relatively attributes
		hls_session->buf_user = MEDIABUF_attach(buf_id);
		HLS_ASSERT(hls_session->buf_user, "Assigned a user for HLS is failed!");
		hls_session->buf_speed = MEDIABUF_in_speed(buf_id);
		pthread_mutex_init(&hls_session->buf_mutex, 0);

		// mark down the pid / tid
		hlsd_mark_pid(hls_session);

		hls_session->m3u8_counter = 0;
		hls_session->media_sequence = 0;		
		hls_session->media_ti_us = 0;
		
	}else{
		// update the m3u8 session pid / tid
		hlsd_mark_pid(hls_session);
	
		++hls_session->m3u8_counter;
		HLS_TRACE("Keep alive the session %d me3u8 %d", n32_sessionid,
			hls_session->m3u8_counter);
	}

	// read the 1st. media, hot key active
	hls_session->media_ready = (1<<0); 

	// response the http header
	do
	{
		char response_header[1024];
		char response_content[4096];
		const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
		HTTP_HEADER_t* http_header = NULL;
		AVal av_connection = AVC("Keep-Alive");
		
		ret = snprintf(response_content, ARRAY_ITEM(response_content),
			"#EXTM3U"CRLF
			//"#EXT-X-DISCONTINUITY"CRLF
			"#EXT-X-ALLOW-CACHE:NO"CRLF
			"#EXT-X-TARGETDURATION:%d"CRLF // target duration
			"#EXT-X-MEDIA-SEQUENCE:%d"CRLF,
			HLS_M3U8_MEDIA_DURATION + 1,
			hls_session->media_sequence);

		// increase this value from next m3u8
		hls_session->media_sequence += HLS_M3U8_MEDIA_COUNT;

		for(i = 0; i < HLS_M3U8_MEDIA_COUNT; ++i){
			void* const response_content_off = response_content + strlen(response_content);
			ssize_t const response_content_size = ARRAY_ITEM(response_content) - strlen(response_content);
			
			ret = snprintf(response_content_off, response_content_size,
				"#EXTINF:%d, law is here"CRLF // duration
				"/hls/live.ts?session=%u&sequence=%d"CRLF,
				HLS_M3U8_MEDIA_DURATION,
				n32_sessionid, i);
		}
		// with end list
		strncat(response_content, "#EXT-X-ENDLIST ", ARRAY_ITEM(response_content));
		
		// make and http response header
		http_header = http_response_header_new(http_version, 200, NULL);
		http_read_header(http_session->request_buf, "Connection", &av_connection);
		http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));
		http_header->add_tag_text(http_header, "Content-Type", HLS_HTTP_M3U8_MIME); // very important
		http_header->add_tag_int(http_header, "Content-Length", strlen(response_content));
		http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
		http_header->dump(http_header);
		http_response_header_free(http_header);
		http_header = NULL;

		// send out the http header
		ret = jsock_send(http_session->sock, response_header, strlen(response_header));
		HLS_ASSERT(strlen(response_header) == ret, "TS response %d %s", ret, strerror(errno));

		HLS_TRACE("M3U8 counter=%d\r\n%s", hls_session->m3u8_counter, response_content);

		// send out the http content
		ret = jsock_send(http_session->sock, response_content, strlen(response_content));
		HLS_ASSERT(strlen(response_content) == ret, "TS response %d %s", ret, strerror(errno));

	}while(0);

	return 0;
}

int CGI_hls_live_ts(HTTPD_SESSION_t* http_session)
{
	int i = 0;
	int ret = 0;
	AVal av_sequence = AVC("0");
	int n32_sequence = 0;
	char response_buf[1024] = {""};
	HTTP_HEADER_t* http_header = NULL;
	const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
	HLS_SESSION_t* hls_session = NULL;
	int n32_sessionid = hlsd_get_session_id(http_session);

	http_read_query_string(AVAL_STRDUPA(http_session->request_line.uri_query_string),
		"sequence", &av_sequence);
	n32_sequence = atoi(AVAL_STRDUPA(av_sequence));

	
	// lookup and check whether this session has been active
	hls_session = hlsd_session_lookup(n32_sessionid);
	if(hls_session){
		//CTX ctx;
		int wait_timeo = HLS_M3U8_MEDIA_DURATION;
		//ctx.packets_count = 0;
		//ctx.frame_rate = 15;
		HLS_SEGMENTOR_t hls_segmentor;

		HLS_TRACE("Request ts session %d sequence %d m3u8 %d", n32_sessionid, n32_sequence,
			hls_session->m3u8_counter);
		
		//
		while(wait_timeo > 0 && hls_session->media_ready < (1<<n32_sequence)){
			HLS_TRACE("Wait sequence %d %08x", n32_sequence, (hls_session->media_ready));
			wait_timeo--;
			sleep(1);
		}

		hls_session->media_ready = 1<<n32_sequence;

		// write the ts pat/pmt
		//write_ts_head(&ctx);
		HLS_segmentor_init(&hls_segmentor);
		pthread_mutex_lock(&hls_session->buf_mutex);
		while(1){
			bool read_success = false;

			// start to make an live ts file
			if(0 == MEDIABUF_out_lock(hls_session->buf_user)){
				lpSDK_ENC_BUF_ATTR frame_attr = NULL;
				void* frame_ptr = NULL;
				ssize_t frame_size = 0;
				// out a new frame from media buffer
				if(0 == MEDIABUF_out(hls_session->buf_user, &frame_attr, NULL, &frame_size)){
					// frame data and size
					frame_ptr = frame_attr + 1;
					frame_size = frame_size - sizeof(stSDK_ENC_BUF_ATTR);

					//write_packet(frame_ptr, frame_size, frame_attr->timestamp_us, &ctx);
					HLS_segmentor_append_video(&hls_segmentor, frame_ptr, frame_size, frame_attr->h264.keyframe, frame_attr->timestamp_us);

					// establish the base timestamp uint.us
					if(0 == hls_session->media_ti_us){
						hls_session->media_ti_us = frame_attr->timestamp_us;
					}
					
					if(frame_attr->timestamp_us - hls_session->media_ti_us > (HLS_M3U8_MEDIA_DURATION * 1000000 - 1)){
						int const next_sequence = (n32_sequence + 1) % HLS_M3U8_MEDIA_COUNT;
						// read
						hls_session->media_ready = (1 << next_sequence);
						hls_session->media_ti_us = frame_attr->timestamp_us;

						HLS_TRACE("Finished this gop packet count = %d", hls_segmentor.n_trans_packet);
					}

					read_success = true;
				}
				MEDIABUF_out_unlock(hls_session->buf_user);
			}
			
			// you need to change into next sequence
			// coz this sequence is to the end
			if((1<<n32_sequence) != hls_session->media_ready){
				break;
			}

			if(true != read_success){
				usleep(hls_session->buf_speed);
			}
		}
		pthread_mutex_unlock(&hls_session->buf_mutex);

		do
		{
			//int const ts_size = ctx.packets_count * 188;
			int const ts_size = hls_segmentor.n_trans_packet * 188;
			AVal av_connection = AVC("Keep-Alive");
			
			http_header = http_response_header_new(http_version, 200, NULL);
			http_header->add_tag_text(http_header, "Content-Type", HLS_HTTP_MPEG2TS_MIME);
			http_header->add_tag_int(http_header, "Content-Length", ts_size);
			http_read_header(http_session->request_buf, "Connection", &av_connection);
			http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));
			http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
			//http_header->dump(http_header);
			http_response_header_free(http_header);
			http_header = NULL;

			// response the http header
			ret = jsock_send(http_session->sock, response_buf, strlen(response_buf));
			if(ret < 0){
				hlsd_session_reclaim(hls_session);
			}

			// response the http content which is mpeg2ts
			ret = jsock_send(http_session->sock, hls_segmentor.trans_packet, ts_size);
			if(ret < 0){
				hlsd_session_reclaim(hls_session);
			}
			
		}while(0);
	}

	return 0;
		
}

#include "hls_demo.c"


