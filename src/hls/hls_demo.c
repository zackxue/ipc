
int CGI_hls_demo_m3u8(HTTPD_SESSION_t* http_session)
{
	int i = 0;
	int ret = 0;
	int n32_sessionid = hlsd_get_session_id(http_session);
	HLS_SESSION_t* hls_session = NULL;
	
	hls_session = hlsd_session_lookup(n32_sessionid);
	if(!hls_session){
		// this session is very strange to HLS server
		// setup a new session is a smart job
		HLS_TRACE("New a session id %d", n32_sessionid);
		hls_session = hlsd_session_new(n32_sessionid);
	}

	// response the http header
	do
	{
		char resopnse_header[1024];
		char resopnse_content[4096];
		
		AVal av_useragent = AVC("unknown");
		AVal av_connection = AVC("close");
		const char* str_useragent = NULL;
		const char* str_connection = NULL;
		
		const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
		HTTP_HEADER_t* http_header = NULL;

		int const hls_segment_count = 10;
		int const hls_segment_duration = 10;

		http_read_header(http_session->request_buf, "Connection", &av_connection);
		http_read_header(http_session->request_buf, "User-Agent", &av_useragent);
		str_useragent = AVAL_STRDUPA(av_useragent);
		str_connection = AVAL_STRDUPA(av_connection);
		
		ret = snprintf(resopnse_content, ARRAY_ITEM(resopnse_content),
			"#EXTM3U"CRLF
			"#EXT-X-ALLOW-CACHE:NO"CRLF
			"#EXT-X-TARGETDURATION:%d"CRLF // target duration
			"#EXT-X-MEDIA-SEQUENCE:%d"CRLF,
			hls_segment_count * hls_segment_duration,
			time(NULL));

		for(i = 0; i < hls_segment_count; ++i){
			ret = snprintf(resopnse_content + strlen(resopnse_content),ARRAY_ITEM(resopnse_content) - strlen(resopnse_content),
				"#EXTINF:%d,"CRLF // duration
				"/hls/demo.ts?session=%u&sequence=%d"CRLF,
				hls_segment_duration,
				n32_sessionid, i);
		}

		HLS_TRACE("Make m3u8 content\r\n%s", resopnse_content);

		// make and http response header
		http_header = http_response_header_new(http_version, 200, NULL);
		http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));
		http_header->add_tag_text(http_header, "Content-Type", "application/vnd.apple.mpegurl"); // very important
		http_header->add_tag_int(http_header, "Content-Length", strlen(resopnse_content));
		http_header->to_text(http_header, resopnse_header, ARRAY_ITEM(resopnse_header));
		http_header->dump(http_header);
		http_response_header_free(http_header);
		http_header = NULL;

		ret = send(http_session->sock, resopnse_header, strlen(resopnse_header), 0);
		if(ret < 0){
			HLS_TRACE("ret = %d %s", ret, strerror(errno));
			exit(1);
		}
		
		ret = send(http_session->sock, resopnse_content, strlen(resopnse_content), 0);
		if(ret < 0){
			HLS_TRACE("ret = %d %s", ret, strerror(errno));
			exit(1);
		}

	}while(0);

	return 0;
}

int CGI_hls_demo_ts(HTTPD_SESSION_t* http_session)
{
	int i = 0;
	int ret = 0;
	char response_buf[1024];
	AVal av_sequence = AVC("0");
	AVal av_connection = AVC("Close");
	int n32_sequence = 0;
	HLS_SESSION_t* hls_session = NULL;
	int n32_sessionid = hlsd_get_session_id(http_session);

	http_read_header(http_session->request_buf, "Connection", &av_connection);
	http_read_query_string(AVAL_STRDUPA(http_session->request_line.uri_query_string),
		"sequence", &av_sequence);
	n32_sequence = atoi(AVAL_STRDUPA(av_sequence));

	hls_session = hlsd_session_lookup(n32_sessionid);
	if(hls_session){
		char file_name[64];
		HTTP_HEADER_t* http_header = NULL;
		const char* const http_version = AVAL_STRDUPA(http_session->request_line.version);
		ssize_t file_size = 0;
		
		sprintf(file_name, "%s/hls/%d.ts", getenv("WEBDIR"), n32_sequence % 3);
		GET_FILE_SIZE(file_name, file_size);
		
		http_header = http_response_header_new(http_version, 200, NULL);
	//	http_header->add_tag_text(http_header, "Transfer-Encoding", "chunked");
		http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));
		http_header->add_tag_text(http_header, "Content-Type", "video/MP2T");
		http_header->add_tag_int(http_header, "Content-Length", file_size); //ctx.packets_count * sizeof(MPEGTS_PACKET));
		http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
//		http_header->dump(http_header);
		http_response_header_free(http_header);
		http_header = NULL;

		// send out the ts segment file
		ret = send(http_session->sock, response_buf, strlen(response_buf), 0);
		if(ret < 0){
			HLS_TRACE("do something");
		}

		HLS_TRACE("Keep alive sequence %d file = \"%s\" size = %d", n32_sequence, file_name, file_size);
		FILE* fid = fopen(file_name, "rb");
		while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
			ret = send(http_session->sock, response_buf, ret, 0);
			if(ret < 0){
				HLS_TRACE("do something");
			}
		}
		fclose(fid);

		return 0;
	}

	return 0;
		
}



