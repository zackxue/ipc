
#include "generic.h"
#include "sdk/sdk_api.h"
#include "smtp.h"
#include "app_debug.h"
#include "cgi.h"

int CGI_moo(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char moo_header[1024];
	const char moo_content[] =
	{
		"          (__) "CRLF
		"          (oo)  "CRLF
		"    /------\\/  "CRLF
		"   /  |   ||    "CRLF
		" *   /\\---/\\    "CRLF
		"     ~~    ~~   "CRLF
		"....\"Have you mooed today?\"..."CRLF
	};
	HTTP_HEADER_t* http_header = NULL;

	http_header = http_response_header_new(AVAL_STRDUPA(session->request_line.version), 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain");
	http_header->add_tag_int(http_header, "Content-Length", strlen(moo_content));
	http_header->to_text(http_header, moo_header, ARRAY_ITEM(moo_header));
	http_response_header_free(http_header);
	http_header = NULL;

	ret = send(session->sock, moo_header, strlen(moo_header), 0);
	if(strlen(moo_header) == ret){
		ret = send(session->sock, moo_content, strlen(moo_content), 0);
		if(strlen(moo_content) == ret){
			return 0;
		}
	}
	return 0;
}

int CGI_whoami(HTTPD_SESSION_t* session)
{
	int i = 0;
	int ret = 0;
	char response_msg[1024];
	char content_buf[4096];
	HTTP_HEADER_t* http_header = NULL;
	const char* const http_version = AVAL_STRDUPA(session->request_line.version);
	
	HTTP_GENERAL_HEADER_SET_t general_header;
	HTTP_REQUEST_HEADER_SET_t request_header;
	HTTP_ENTITY_HEADER_SET_t entity_header;

	// reponse the request line
	strncpy(content_buf, "<table border=3 "
		"width=\"80%\" "
		"height=\"60\" "
		"align=\"center\" "
		"bordercolor=#336699 >", ARRAY_ITEM(content_buf));

	strncat(content_buf, "<tr><td align=\"center\" colspan=\"2\">", ARRAY_ITEM(content_buf));
	ret = snprintf(content_buf + strlen(content_buf), ARRAY_ITEM(content_buf) - strlen(content_buf),
		"%s %s %s",
		AVAL_STRDUPA(session->request_line.method),
		AVAL_STRDUPA(session->request_line.uri),
		AVAL_STRDUPA(session->request_line.version));
	strncat(content_buf, "</td></tr>", ARRAY_ITEM(content_buf));
	
	if(0 == http_parse_general_header(session->request_buf, &general_header)){
		const char* general_tag[] =
		{
			"Cache-Control", "Connection", "Date", "Pragma", "Triler", "Transfe-Encoding", "Upgrade", "Via", "Warning",
		};
		const char* general_key[] =
		{
			AVAL_STRDUPA(general_header.cache_control),
			AVAL_STRDUPA(general_header.connection),
			AVAL_STRDUPA(general_header.date),
			AVAL_STRDUPA(general_header.pragma),
			AVAL_STRDUPA(general_header.triler),
			AVAL_STRDUPA(general_header.transfe_encoding),
			AVAL_STRDUPA(general_header.upgrade),
			AVAL_STRDUPA(general_header.via),
			AVAL_STRDUPA(general_header.warning),
		};

		for(i = 0; i < ARRAY_ITEM(general_tag); ++i){
			if(strlen(general_key[i]) > 0){
				strncat(content_buf, "<tr>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\" width=\"25%\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, general_tag[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, general_key[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "</tr>", ARRAY_ITEM(content_buf));
			}
		}
	}

	if(0 == http_parse_request_header(session->request_buf, &request_header)){
		const char* request_tag[] =
		{
			"Accept", "Accept-Charset", "Accept-Encoding", "Accept-Language",
			"Authorization","cookie", "Expect", "From", "Host",
			"If-Match", "If-Modified-Since", "If-None-Match", "If-Range", "If -Unmodified-Since",
			"Max-Forwards", "Proxy-Authorization", "Range", "Referer", "TE", "User-Agent",
		};
		const char* request_key[] =
		{
			AVAL_STRDUPA(request_header.accept),
			AVAL_STRDUPA(request_header.accept_charset),
			AVAL_STRDUPA(request_header.accept_encoding),
			AVAL_STRDUPA(request_header.accept_language),
			AVAL_STRDUPA(request_header.authorization),
			AVAL_STRDUPA(request_header.cookie),
			AVAL_STRDUPA(request_header.expect),
			AVAL_STRDUPA(request_header.from),
			AVAL_STRDUPA(request_header.host),
			AVAL_STRDUPA(request_header.if_match),
			AVAL_STRDUPA(request_header.if_modified_since),
			AVAL_STRDUPA(request_header.if_none_match),
			AVAL_STRDUPA(request_header.if_range),
			AVAL_STRDUPA(request_header.if_unmodified_since),
			AVAL_STRDUPA(request_header.max_forwards),
			AVAL_STRDUPA(request_header.proxy_authorization),
			AVAL_STRDUPA(request_header.range),
			AVAL_STRDUPA(request_header.referer),
			AVAL_STRDUPA(request_header.te),
			AVAL_STRDUPA(request_header.user_agent),
		};

		for(i = 0; i < ARRAY_ITEM(request_tag); ++i){
			if(strlen(request_key[i]) > 0){
				strncat(content_buf, "<tr>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\" width=\"25%\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, request_tag[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, request_key[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "</tr>", ARRAY_ITEM(content_buf));
			}
		}
	}

	if(http_parse_entity_header(session->request_buf, &entity_header)){
		const char* entity_tag[] =
		{
			"Allow", "Content-Encoding", "Content-Language", "Content-Length", "Content-Location",
			"Content-MD5", "Content-Range", "Expires", "Last_Modified", "Warning",
		};
		const char* entity_key[] =
		{
			AVAL_STRDUPA(entity_header.allow),
			AVAL_STRDUPA(entity_header.content_encoding),
			AVAL_STRDUPA(entity_header.content_language),
			AVAL_STRDUPA(entity_header.content_length),
			AVAL_STRDUPA(entity_header.content_location),
			AVAL_STRDUPA(entity_header.content_md5),
			AVAL_STRDUPA(entity_header.content_range),
			AVAL_STRDUPA(entity_header.expires),
			AVAL_STRDUPA(entity_header.last_modified),
			AVAL_STRDUPA(entity_header.warning),
		};

		for(i = 0; i < ARRAY_ITEM(entity_tag); ++i){
			if(strlen(entity_key[i]) > 0){
				strncat(content_buf, "<tr>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\" width=\"25%\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, entity_tag[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "<td align=\"center\">", ARRAY_ITEM(content_buf));
				strncat(content_buf, entity_key[i], ARRAY_ITEM(content_buf));
				strncat(content_buf, "</td>", ARRAY_ITEM(content_buf));
				strncat(content_buf, "</tr>", ARRAY_ITEM(content_buf));
			}
		}
	}
	strncat(content_buf, "</table>", ARRAY_ITEM(content_buf));

	http_header = http_response_header_new(http_version, 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/html; charset=UTF-8");
	http_header->add_tag_int(http_header, "Content-Length", strlen(content_buf));
	http_header->to_text(http_header, response_msg, ARRAY_ITEM(response_msg));
	http_response_header_free(http_header);
	http_header = NULL;

	ret = send(session->sock, response_msg, strlen(response_msg), 0);
	if(ret < 0){
		// FIXME:
	}
	APP_TRACE("%s", content_buf);
	
	ret = send(session->sock, content_buf, strlen(content_buf), 0);
	if(ret < 0){
		// FIXME:
	}

	do
	{
		char query_string[4096];
		ret = snprintf(query_string, ARRAY_ITEM(query_string),
			"target=lawishere@yeah.net&subject=Who are you?&content=%s&snapshot=yes&vin=0&size=320x180",
			content_buf);
		//http_fool_style_request("127.0.0.1", 80, "GET", "/email", query_string, "1.1", NULL, 0);
	}while(0);
	return 0;
}

int CGI_shell(HTTPD_SESSION_t* session)
{
	const char* const query_string = AVAL_STRDUPA(session->request_line.uri_query_string);
	char* const shell_cmd = alloca(strlen(query_string));

	if(http_url_decode(query_string, strlen(query_string), shell_cmd, strlen(query_string)) > 0){
		int ret = 0;
		FILE* fid = popen(shell_cmd, "r");
		if(fid > 0){
			int fid_no = fileno(fid);
			fd_set read_fds;
			struct timeval read_timeo;
			
			
			char response_header[1024] = {""};
			char response_content[4096] = {""};
			
			FD_ZERO(&read_fds);
			FD_SET(fid_no, &read_fds);
			read_timeo.tv_sec = session->keep_alive / 2;
			read_timeo.tv_usec = 0;
			
			ret = select(fid_no + 1, &read_fds, NULL, NULL, &read_timeo);
			if(ret <= 0){
				// do something
			}else{
				const char* const http_version = AVAL_STRDUPA(session->request_line.version);
				HTTP_HEADER_t* http_header = NULL;
			
				while(fgets(response_content + strlen(response_content),
					ARRAY_ITEM(response_content) - strlen(response_content), fid) != NULL);

				http_header = http_response_header_new(http_version, 200, NULL);
				http_header->add_tag_text(http_header, "Content-Type", "text/plain");
				http_header->add_tag_int(http_header, "Content-Length", strlen(response_content));
				http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
				http_response_header_free(http_header);
				http_header = NULL;

				// response http header
				ret = send(session->sock, response_header, strlen(response_header), 0);
				if(strlen(response_header) == ret){
					// response content type
					ret = send(session->sock, response_content, strlen(response_content), 0);
				}
			}
			pclose(fid);
			fid = NULL;
			APP_TRACE("Shell \"%s\"", shell_cmd);
			return 0;
		}
	}
	return -1;
}

static int cgi_capture_a_jpeg(int vin, int width, int height, const char* file_name)
{
	FILE* fid = NULL;
	int file_size = -1;
	fid = fopen(file_name, "w+b");
	if(NULL != fid){
		if(0 == sdk_enc->snapshot(vin, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, width, height, fid)){
			GET_FILE_SIZE(file_name, file_size);
		}
		fclose(fid);
	}
	return file_size;
}

int CGI_snapshot(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char jpeg_name[128];
	AVal av_size, av_download;
	int width = 1920, height = 1280;
	bool is_download = false;
	ssize_t file_size = 0;
	
	if(0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string), "size", &av_size)){
		const char* const str_size = AVAL_STRDUPA(av_size);
		if(2 != sscanf(str_size, "%dx%d", &width, &height)){
			width = 640;
			height = 320;
		}
	}
	if(0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string), "download", &av_download)){
		if(AVSTRCASEMATCH(&av_download, "yes") || AVSTRCASEMATCH(&av_download, "1")){
			is_download = true;
		}
	}

	// generate a jpeg file name
	ret = snprintf(jpeg_name, ARRAY_ITEM(jpeg_name), "/tmp/%d%d.jpg", rand(), rand());
	file_size = cgi_capture_a_jpeg(0, width, height, jpeg_name);
	if(file_size > 0){
		const char* const http_version = AVAL_STRDUPA(session->request_line.version);
		HTTP_HEADER_t* http_header = NULL;
		char response_buf[1024];

		http_header = http_response_header_new(http_version, 200, NULL);
		http_header->add_tag_text(http_header, "Content-Type", http_get_file_mime("jpeg"));
		http_header->add_tag_int(http_header, "Content-Length", file_size);
		http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
		http_response_header_free(http_header);
		http_header = NULL;

		ret = send(session->sock, response_buf, strlen(response_buf), 0);
		if(ret > 0){
			FILE* fid = fopen(jpeg_name, "rb");
			if(NULL != fid){
				fseek(fid, 0, SEEK_SET);
				while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
					ret = send(session->sock, response_buf, ret, 0);
					if(ret < 0){
						break;
					}
				}
				fclose(fid);
				fid = NULL;
			}
		}
	}
	unlink(jpeg_name);
	remove(jpeg_name);
	return 0;
}

int CGI_mjpeg(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char response_buf[1024];
	AVal av_size, av_fps;
	int width = 640, height = 360;
	int n_fps = 15, n_f_time = 0;;
	ssize_t file_size = 0;
	
	if(0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string), "size", &av_size)){
		const char* const str_size = AVAL_STRDUPA(av_size);
		if(2 != sscanf(str_size, "%dx%d", &width, &height)){
			width = 640;
			height = 320;
		}
	}
	if(0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string), "fps", &av_fps)){
		n_fps = atoi(AVAL_STRDUPA(av_fps));
		if(n_fps > 30){
			n_fps = 15; // 15 at the most fps
		}
	}
	n_f_time = (1000000 + n_fps - 1) / n_fps;

	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/%s 200 OK"CRLF
		"Content-Type: multipart/x-mixed-replace;boundary=ipcamera"CRLF
		CRLF,
		AVAL_STRDUPA(session->request_line.version));

	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	if(ret > 0){
		while(*session->trigger)
		{
			FILE* fid = NULL;
			char jpeg_name[128];
			ret = snprintf(jpeg_name, ARRAY_ITEM(jpeg_name), "/tmp/%d%d.jpg", rand(), rand());
			file_size = cgi_capture_a_jpeg(0, width, height, jpeg_name);
			if(file_size > 0){
				ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
					"--ipcamera"CRLF
					"Content-Type: image/jpeg"CRLF
					"Content-Length: %d"CRLF
					CRLF, file_size);

				ret = send(session->sock, response_buf, strlen(response_buf), 0);
				if(ret > 0){
					fid = fopen(jpeg_name, "rb");
					if(NULL != fid){
						while((ret = fread(response_buf, 1, ARRAY_ITEM(response_buf), fid)) > 0){
							ret = send(session->sock, response_buf, ret, 0);
							if(ret < 0){
								break;
							}
						}
					}
				}
			}

			unlink(jpeg_name);
			remove(jpeg_name);

			if(ret < 0){
				break;
			}
			usleep(n_f_time); 
		}

		if(ret > 0){
			strcpy(response_buf, "0"CRLF);
			ret = send(session->sock, response_buf, strlen(response_buf), 0);
			return 0;
		}
	}

	return -1;
}


int CGI_mjpeg_html(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char response_buf[1024];
	char response_content[1024];

	strncpy(response_content,
		"<body>"CRLF
		"<img src=\"/mjpeg?size=640x360&fps=15\" />"CRLF
		"</body>", ARRAY_ITEM(response_content));
	
	ret = snprintf(response_buf, ARRAY_ITEM(response_buf),
		"HTTP/%s 200 OK"CRLF
		"Content-Type: text/html"CRLF
		"Content-Length: %d"CRLF
		CRLF
		"%s",
		AVAL_STRDUPA(session->request_line.version),
		strlen(response_content),
		response_content);

	ret = send(session->sock, response_buf, strlen(response_buf), 0);
	return 0;
}

// server
// port
// usermail
// password
// target
// subject
// content
// snapshot : yes / no
// vin :
// size :

int CGI_send_email(HTTPD_SESSION_t* session)
{
#define DEFAULT_USER_EMAIL "dvruser@esee100.com"
#define DEFAULT_USER_PASSWORD "dvrhtml"
#define DEFAULT_SMTP_SERVER "mail.esee100.com"
#define DEAFULT_TARGE_EMAIL "lawishere@yeah.net"

	int ret = 0;
	char* const query_orign = AVAL_STRDUPA(session->request_line.uri_query_string);
	char* const query_string = alloca(strlen(query_orign));
	char usermail[128] = {""};// = "Administrator@esee100.com";
	char password[128] = {""}; // = "GZjuan@tv2211.com";
	char target[128] = {""};
	char eml_subject[128] = {""};
	char eml_content[2048] = {""};
	char smtp_server[128] = {""};
	int smtp_port = 25;
	bool snapshot_use = false;
	int snapshot_vin = 0;
	int snapshot_width = 320, snapshot_height = 180; // 1/4 x 1280 -- 1/4 x 720
	AVal av_connection = AVC("close");
	AVal av_usermail, av_password, av_target, av_subject, av_content, av_server, av_port;
	AVal av_snapshot, av_snapshot_vin, av_snapshot_size;
	SMTP_SESSION_t* smtp = NULL;
	char response_header[1024] = {""};
	char response_content[1024] = {""};
	HTTP_HEADER_t* http_header = NULL;

	// get connection
	http_read_header(session->request_buf, "Connection", &av_connection);

	// decode query string
	http_url_decode(query_orign, strlen(query_orign), query_string, strlen(query_orign));

	// default email
	strcpy(usermail, DEFAULT_USER_EMAIL);
	strcpy(password, DEFAULT_USER_PASSWORD);
	strcpy(target, DEAFULT_TARGE_EMAIL);
	strcpy(smtp_server, DEFAULT_SMTP_SERVER);
	strcpy(eml_subject, "No subject");
	strcpy(eml_content, "");

	if(0 == http_read_query_string(query_string, "usermail", &av_usermail)){
		strncpy(usermail, AVAL_STRDUPA(av_usermail), ARRAY_ITEM(usermail));
		// setup the new smtp server
		if(strrchr(usermail, '@')){
			ret = snprintf(smtp_server, ARRAY_ITEM(smtp_server),
				"smtp.%s", strrchr(usermail, '@') + 1);
		}
	}
	if(0 == http_read_query_string(query_string, "password", &av_password)){
		strncpy(password, AVAL_STRDUPA(av_password), ARRAY_ITEM(password));
	}
	if(0 == http_read_query_string(query_string, "target", &av_target)){
		strncpy(target, AVAL_STRDUPA(av_target), ARRAY_ITEM(target));
	}
	if(0 == http_read_query_string(query_string, "server", &av_server)){
		strncpy(smtp_server, AVAL_STRDUPA(av_server), ARRAY_ITEM(smtp_server));
	}
	if(0 == http_read_query_string(query_string, "subject", &av_subject)){
		strncpy(eml_subject, AVAL_STRDUPA(av_subject), ARRAY_ITEM(eml_subject));
	}
	if(0 == http_read_query_string(query_string, "content", &av_content)){
		strncpy(eml_content, AVAL_STRDUPA(av_content), ARRAY_ITEM(eml_content));
	}
	if(0 == http_read_query_string(query_string, "port", &av_port)){
		smtp_port = atoi(AVAL_STRDUPA(av_port));
	}
	// read snapshot about
	if(0 == http_read_query_string(query_string, "snapshot", &av_snapshot)){
		snapshot_use = AVSTRCASEMATCH(&av_snapshot, "yes") || AVSTRCASEMATCH(&av_snapshot, "1");
	}
	if(0 == http_read_query_string(query_string, "vin", &av_snapshot_vin)){
		snapshot_vin = atoi(AVAL_STRDUPA(av_snapshot_vin));
	}
	if(0 == http_read_query_string(query_string, "size", &av_snapshot_size)){
		const char* const str_size = AVAL_STRDUPA(av_snapshot_size);
		if(2 != sscanf(str_size, "%dx%d", &snapshot_width, &snapshot_height)){
			snapshot_width = 640;
			snapshot_height = 320;
		}
	}

	// make a read response header
	http_header = http_response_header_new(AVAL_STRDUPA(session->request_line.version), 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain");
	http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));

	// start to send email
	smtp = SMTP_login(smtp_server, smtp_port, usermail, password);
	if(smtp){
		int file_size = 0;
		char jpeg_name[128];
		
		ret = snprintf(jpeg_name, ARRAY_ITEM(jpeg_name), "/tmp/%d%d.jpg", rand(), rand());
		ret = snprintf(response_content, ARRAY_ITEM(response_content),
				"[Bingo]"CRLF
				"To '%s'"CRLF
				"Snapshot: %s 'vin=%d,%dx%d'",
				target,
				snapshot_use ? "yes" : "no", snapshot_vin, snapshot_width, snapshot_height);

		if(snapshot_use){
			file_size = cgi_capture_a_jpeg(snapshot_vin, snapshot_width, snapshot_height, jpeg_name);
			if(0 == file_size){
				ret = snprintf(response_content, ARRAY_ITEM(response_content),
					"[Sorry] %s\r\nSnapshot failed @ vin=%d size=%dx%d!"CRLF, target,
					snapshot_vin, snapshot_width, snapshot_height);
			}else{
				char file_name[64];
				ret = snprintf(file_name, ARRAY_ITEM(file_name),
					"vin%d_%dx%d.jpg", snapshot_vin, snapshot_width, snapshot_height);
				if(0 != SMTP_send(smtp, target, eml_subject, eml_content, jpeg_name, file_name)){
					ret = snprintf(response_content, ARRAY_ITEM(response_content),
						"[Sorry] %s"CRLF"%s"CRLF, target, SMTP_strerror(smtp));
				}
			}
		}else{
			if(0 != SMTP_send(smtp, target, eml_subject, eml_content, NULL, NULL)){
				ret = snprintf(response_content, ARRAY_ITEM(response_content),
					"[Sorry] %s"CRLF"%s"CRLF, target, SMTP_strerror(smtp));
			}
		}

		unlink(jpeg_name);
		remove(jpeg_name);

		SMTP_logout(smtp);
		smtp = NULL;
	}else{
		ret = snprintf(response_content, ARRAY_ITEM(response_content),
					"[Sorry] %s"CRLF
					"SMTP Login failed"CRLF
					"sever: %s"CRLF
					"email: %s"CRLF
					"password: %s"CRLF,
			target, smtp_server, usermail, password);
	}

	http_header->add_tag_int(http_header, "Content-Length", strlen(response_content));
	http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
	http_response_header_free(http_header);
	http_header = NULL;
	
	// response theader
	ret = send(session->sock, response_header, strlen(response_header), 0);
	if(strlen(response_header) == ret){
		// response content
		ret = send(session->sock, response_content, strlen(response_content), 0);
		if(strlen(response_content) == ret){
			APP_TRACE("Response\r\n%s%s", response_header, response_content);
			return 0;
		}
	}
	APP_TRACE("Exception error!");
	return -1;
}

int CGI_sdk_reg_rw(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char response_header[1024] = {""};
	char response_content[1024] = {""};
	HTTP_HEADER_t* http_header = NULL;
	AVal av_addr = AVC("0x00000000");
	uint32_t reg_addr = 0x200f0018;
	uint32_t reg_val = 0;

	http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string),"addr", &av_addr);

	// read reg
	sdk_sys->read_reg(reg_addr, &reg_val);
	ret = snprintf(response_content, ARRAY_ITEM(response_content),
		"0x%08X", reg_val);
	
	http_header = http_response_header_new(AVAL_STRDUPA(session->request_line.version), 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain");
	http_header->add_tag_int(http_header, "Content-Length", strlen(response_content));
	http_header->to_text(http_header, response_header, ARRAY_ITEM(response_header));
	http_response_header_free(http_header);
	http_header = NULL;

	ret = send(session->sock, response_header, strlen(response_header), 0);
	ret = send(session->sock, response_content, strlen(response_content), 0);

	return 0;
}

int CGI_today_snapshot(HTTPD_SESSION_t* session)
{
	int ret = 0;
	AVal av_hour = AVC("0");
	char jpeg_path[64] = {""};

	http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string),"hour", &av_hour);
	ret = snprintf(jpeg_path, ARRAY_ITEM(jpeg_path), "/tmp/today/%02d.jpg", atoi(AVAL_STRDUPA(av_hour)));
	HTTPD_response_file(session, jpeg_path);

	return 0;
}

int CGI_focus_measure(HTTPD_SESSION_t *session)
{
	int ret = 0;
	char buf[1024] = {""};
	HTTP_HEADER_t *http_header = NULL;
	AVal av_vin = AVC("0");
	AVal av_algorithm = AVC("auto");
	enSDK_VIN_FOCUS_MEASURE_ALG focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT;
	int focus_measure = 0;
	char str_measure[32] = {""};

	// read the channel
	http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string), "vin", &av_vin);
	// read the algorithm
	if(0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string),"algorithm", &av_algorithm)
		|| 0 == http_read_query_string(AVAL_STRDUPA(session->request_line.uri_query_string),"alg", &av_algorithm)){
		if(AVSTRCASEMATCH(&av_algorithm, "vollath5")){
			focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_VOLLATH5;
		}else{
			focus_alg = kSDK_VIN_FOCUS_MEASURE_ALG_SQUARED_GRADIENT;
		}
	}
	if(NULL != sdk_vin){
		focus_measure = sdk_vin->focus_measure(atoi(AVAL_STRDUPA(av_vin)), focus_alg);
	}
	
	sprintf(str_measure, "%d", focus_measure);
	
	http_header = http_response_header_new(AVAL_STRDUPA(session->request_line.version), 200, NULL);
	http_header->add_tag_text(http_header, "Content-Type", "text/plain");
	http_header->add_tag_int(http_header, "Content-Length", strlen(str_measure));
	http_header->to_text(http_header, buf, ARRAY_ITEM(buf));
	http_response_header_free(http_header);
	http_header = NULL;

	strcat(buf, str_measure);
	ret = send(session->sock, buf, strlen(buf), 0);
	return 0;
}

