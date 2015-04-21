
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "generic.h"
#include "jsocket.h"
#include "media_buf.h"
#include "sdk/sdk_api.h"
#include "httpd_debug.h"
#include "http_common.h"
#include "httpd.h"


#define DEFAULT_KEEP_ALIVE_DURATION (60)
#define HTTP_MAX_CGI_ENTRY (128)

struct HTTP_CGI_ENTRY
{
	char* name;
	CGI_HANDLER handler;
};

struct HTTP_SERVER
{
	char* file_folder;
	
	uint32_t n_cgi_entry;
	struct HTTP_CGI_ENTRY cgi_entry[HTTP_MAX_CGI_ENTRY];
};

static struct HTTP_SERVER* _http_server = NULL;


int HTTPD_add_cgi(const char* name, CGI_HANDLER handler)
{
	int i = 0;
	for(i = 0; i < (typeof(i))ARRAY_ITEM(_http_server->cgi_entry); ++i){
		if(NULL == _http_server->cgi_entry[i].name){
			_http_server->cgi_entry[i].handler = handler;
			_http_server->cgi_entry[i].name = strdup(name);
			++_http_server->n_cgi_entry;
			HTTPD_TRACE("Add a new cgi \"%s\"", _http_server->cgi_entry[i].name);
			return 0;
		}
	}
	return -1;
}

int HTTPD_remove_cgi(const char* name)
{
	// FIXME: to implement
	return -1;
}

void HTTPD_dump(HTTPD_SESSION_t* session)
{
	const char* str_method = AVAL_STRDUPA(session->request_line.method);
	const char* str_path = AVAL_STRDUPA(session->request_line.uri);
	const char* str_uri_host = AVAL_STRDUPA(session->request_line.uri_host);
	const char* str_uri_hostname = AVAL_STRDUPA(session->request_line.uri_hostname);
	const char* str_uri_suffix = AVAL_STRDUPA(session->request_line.uri_suffix);
	const char* str_uri_suffix_ext = AVAL_STRDUPA(session->request_line.uri_suffix_extname);
	const char* str_query_string = AVAL_STRDUPA(session->request_line.uri_query_string);
	const char* str_http_version = AVAL_STRDUPA(session->request_line.version);

	HTTPD_TRACE("Request parse:\r\n"
		"Method: \"%s\"\r\n"
		"URI: \"%s\"\r\n"
		"URI host: \"%s\"\r\n"
		"URI hostname: \"%s\"\r\n"
		"URI suffix: \"%s\"\r\n"
		"URI suffix ext: \"%s\"\r\n"
		"URI Query string: \"%s\"\r\n"
		"HTTP version: \"%s\"\r\n",
		str_method,
		str_path, str_uri_host, str_uri_hostname, str_uri_suffix, str_uri_suffix_ext,
		str_query_string,
		str_http_version);
}

static char* date_header(char* buf, int buf_len, time_t t)
{
	strftime(buf, buf_len, "%a, %e %b %Y %T GMT", gmtime(&t));
	return buf;
}

static int httpd_response(HTTPD_SESSION_t* session, int status_code, const char* content)
{
	const char* http_version = AVAL_STRDUPA(session->request_line.version);
	return http_fool_style_response(session->sock, http_version, status_code,
		content, strlen(content));
}

static int httpd_response_304_not_modified(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 304, "");
}

/*
static int httpd_response_400_bad_request(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 400,
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>400 Bad Request</title>\r\n"
		"</head><body>\r\n"
		"<h1>Bad Request</h1>\r\n"
		"<p>Bad Request.</p>\r\n"
		"</body></html>\r\n");
}

static int httpd_response_403_forbidden(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 403,
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>403 Forbidden</title>\r\n"
		"</head><body>\r\n"
		"<h1>Forbidden</h1>\r\n"
		"<p>You don't have permission to access the path requested"
		"on this server.</p>\r\n"
		"</body></html>\r\n");
}
*/

static int httpd_response_404_not_found(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 404,
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>404 Not Found</title>\r\n"
		"</head><body>\r\n"
		"<h1>Not Found</h1>\r\n"
		"<p>The requested URL requested was not found on this server.</p>\r\n"
		"</body></html>\r\n");
}

/*
static int httpd_response_501_not_implemented(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 501, "501 Not Implemented");
}

static int httpd_response_503_service_unavailable(HTTPD_SESSION_t* session)
{
	return httpd_response(session, 503,
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>503 Service Unavailable</title>\r\n"
		"</head><body>\r\n"
		"<h1>Not Found</h1>\r\n"
		"<p>Service Temporarily Overloaded.</p>\r\n"
		"</body></html>\r\n");
}
*/

static int httpd_try_file(const char* name)
{
	if(CHECK_FILE_EXIST(name)){
		return 0;
	}
	return -1;
}

static ssize_t httpd_read_and_send_file(int sock, FILE* fid)
{
	int ret = 0;
	ssize_t read_sum = 0;
	ssize_t read_size = 0;
	char send_buf[1024] = {""};

	while((read_size = fread(send_buf, 1, sizeof(send_buf), fid)) > 0){
		ret = jsock_send(sock, send_buf, read_size);
		if(ret < 0){
			return -1;
		}
		read_sum += read_size;
	}

	return read_sum;
}

static int httpd_response_file(HTTPD_SESSION_t* session, const char* file_name, bool gzip)
{
	int ret;
	const char* const file_extname = AVAL_STRDUPA(session->request_line.uri_suffix_extname);
	FILE* fp = NULL;
	
	fp = fopen(file_name, "rb");
	if(fp == NULL){
		HTTPD_TRACE("Request file \"%s\" not found!", file_name);
		return httpd_response_404_not_found(session);
	}else{
		// get file state
		struct stat file_stat;
		fstat(fileno(fp), &file_stat);
		AVal av_modified_since;
		AVal av_connection = AVC("close");
		char date_buf[64];
		char last_modified_buf[64];
		char send_buf[1024];
		HTTP_HEADER_t* http_header = NULL;
		const char* const http_version = AVAL_STRDUPA(session->request_line.version);

		// check file modified since date
		if(0 == http_read_header(session->request_buf, "If-Modified-Since", &av_modified_since)){
			const char* const str_modified_since = AVAL_STRDUPA(av_modified_since);
			const char* const str_file_modified = date_header(date_buf, ARRAY_ITEM(date_buf), file_stat.st_mtime);

			if(STR_CASE_THE_SAME(str_modified_since, str_file_modified)){
				// needless to send this file because it has exist in browser's cache
				fclose(fp);
				return httpd_response_304_not_modified(session);
			}
			HTTPD_TRACE("File modified if-modified-since=\"%s\" file=\"%s\"", str_modified_since, str_file_modified);
		}
		http_read_header(session->request_buf, "Connection", &av_connection);

		// make the target
		http_header = http_response_header_new(http_version, 200, NULL);
		http_header->add_tag_text(http_header, "Content-Type", http_get_file_mime(file_extname));
		http_header->add_tag_text(http_header, "Date", date_header(date_buf, ARRAY_ITEM(date_buf), time(NULL)));
		http_header->add_tag_text(http_header, "Last-Modified", date_header(last_modified_buf, ARRAY_ITEM(last_modified_buf), file_stat.st_mtime));
		http_header->add_tag_text(http_header, "Connection", AVAL_STRDUPA(av_connection));
		if(gzip){
			http_header->add_tag_text(http_header, "Content-Encoding", "gzip");
		}
		http_header->add_tag_int(http_header, "Content-Length", file_stat.st_size);
		http_header->to_text(http_header, send_buf, ARRAY_ITEM(send_buf));
		//http_header->dump(http_header);
		http_response_header_free(http_header);
		http_header = NULL;
			
		// response the http header
		ret = jsock_send(session->sock, send_buf, strlen(send_buf));
		if(strlen(send_buf) == ret){
			// continue to resopnse the file data
			ret = httpd_read_and_send_file(session->sock, fp);
			if(ret < 0){
				HTTPD_TRACE("Response file \"%s\" failed! %s", file_name, strerror(errno));
				fclose(fp);
				return -1;
			}
		}else{
			HTTPD_TRACE("Response header \"%s\" failed! %s", send_buf, strerror(errno));
			fclose(fp);
			return -1;
		}
		fclose(fp);
	}
	return 0;
}

static int httpd_response_401_unauthorized(HTTPD_SESSION_t* session)
{
	int ret = 0;
	char response_buf[1024] = {""};
	const char* const http_version = AVAL_STRDUPA(session->request_line.version);
	HTTP_HEADER_t* http_header = NULL;

	http_header = http_response_header_new(http_version, 401, NULL);
	http_header->add_tag_text(http_header, "WWW-Authenticate", "Basic realm=\"TP-LINK Wireless N Router WR840N\"");
	http_header->to_text(http_header, response_buf, ARRAY_ITEM(response_buf));
//	http_header->dump(http_header);
	http_response_header_free(http_header);
	http_header = NULL;

	ret = jsock_send(session->sock, response_buf, strlen(response_buf));
	if(strlen(response_buf) == ret){
		return 0;
	}

	return -1;
}

int HTTPD_response_file(HTTPD_SESSION_t* session, const char* file_name)
{
	return httpd_response_file(session, file_name, false);
}

static int httpd_try_cgi(const char* name, CGI_HANDLER* ret_handler)
{
	int i = 0;
	for(i = 0; i < (typeof(i))ARRAY_ITEM(_http_server->cgi_entry); ++i){
		struct HTTP_CGI_ENTRY* const cgi_entry = _http_server->cgi_entry + i;
		if(cgi_entry->name && STR_CASE_THE_SAME(name, cgi_entry->name)){
			*ret_handler = cgi_entry->handler;
			return 0;
		}
	}
	return -1;
}

static int httpd_response_cgi(HTTPD_SESSION_t* session, CGI_HANDLER handler)
{
	return handler(session);
}


int HTTPD_init(const char* file_folder)
{
	if(!_http_server){
		_http_server = calloc(sizeof(struct HTTP_SERVER), 1);
		// init the parameters
		if(file_folder){
			_http_server->file_folder = strdup(file_folder);
		}
		
		_http_server->n_cgi_entry = 0;
		return 0;
	}
	return -1;
}

void HTTPD_destroy()
{
	if(_http_server){
		free(_http_server->file_folder);
		_http_server->file_folder = NULL;
		
		// FIXME: remember to remove the cgi
		free(_http_server);
		_http_server = NULL;
	}
}

static ssize_t request_check_completed(const char* request_buf)
{
	// caution:
	// the request_buf must be with an end mark '\0'
	const char* str_ptr = strstr(request_buf, "\r\n\r\n");
	if(NULL != str_ptr){
		return str_ptr - request_buf + strlen("\r\n\r\n");
	}
	return 0;
}

static const char* httpd_where_file(HTTPD_SESSION_t* session, char* file_path, ssize_t file_path_size, const char* folder, const char* with_suffix)
{
	const char* const uri_suffix = AVAL_STRDUPA(session->request_line.uri_suffix);
	struct stat stat_buf;
	
	// get the absolute file path
	strncpy(file_path, folder, file_path_size);
	if('/' == file_path[strlen(file_path) - 1]){
		// truncated the end of '/'
		file_path[strlen(file_path) - 1] = '\0';
	}
	strncat(file_path, uri_suffix, file_path_size);

	// check the file dir / file
	STRUCT_ZERO(stat_buf);
	if(0 == lstat(file_path, &stat_buf)){
		if(S_ISDIR(stat_buf.st_mode)){
			// check the ended '/'
			if('/' != file_path[strlen(file_path) - 1]){
				strncat(file_path, "/", file_path_size);
				//HTTPD_TRACE("Completed path %s", file_path);
			}
		}
	}

	// check whether need a default file request
	if('/' == file_path[strlen(file_path) - 1]){
		AVal const av_index_html = AVC("/index.html");
		AVal const av_index_html_extname = AVC("html");
		const char* const str_index_html = AVAL_STRDUPA(av_index_html);
		// truncated the end of '/'
		file_path[strlen(file_path) - 1] = '\0';
		strncat(file_path, str_index_html, file_path_size);
		// update the uri suffix paramenter
		session->request_line.uri_suffix = av_index_html;
		session->request_line.uri_suffix_extname = av_index_html_extname;
	}

	// maybe with a suffix
	if(with_suffix){
		strncat(file_path, with_suffix, file_path_size);
	}
	return file_path;
}	

static bool httpd_auth(HTTPD_SESSION_t* session)
{
	AVal av_auth = AVC("");
	if(0 == HTTPD_auth_user_cnt()){
		// no user to authorize
		HTTPD_TRACE("no user to authorize\n");
		return true;
	}else if(0 == http_read_header(session->request_buf, "Authorization", &av_auth)){
		const char* const str_auth = AVAL_STRDUPA(av_auth);
		// check whether header tag "authorization" existed
		return HTTPD_auth_access(str_auth);
	}
	return false;
}

static int httpd_handle(HTTPD_SESSION_t* session)
{
	int ret = 0;
	const char* const uri_suffix = AVAL_STRDUPA(session->request_line.uri_suffix);
	char where_gzip_file[128];
	char where_file[128];
	CGI_HANDLER cgi_handler = NULL;
	bool gzip_file = false;

//	HTTPD_TRACE("Looking for file \"%s\"", where_file);

	if((gzip_file = true, httpd_where_file(session, where_gzip_file, ARRAY_ITEM(where_file), _http_server->file_folder, ".gz"), 0 == httpd_try_file(where_gzip_file))
			|| (gzip_file = false, httpd_where_file(session, where_file, ARRAY_ITEM(where_file), _http_server->file_folder, NULL), 0 == httpd_try_file(where_file))){
		// all the file need to authorize
		if(STR_CASE_THE_SAME(basename(where_gzip_file), "index.html.gz")
				|| STR_CASE_THE_SAME(basename(where_file), "index.html")){
			if(true != httpd_auth(session)){
				HTTPD_TRACE("HTTP authorize failed!");
				return httpd_response_401_unauthorized(session);
			}
		}

		// http requesting a file
		HTTPD_TRACE("Response file \"%s\"", where_file);
		ret = httpd_response_file(session, where_file, gzip_file);
		
	}else if(0 == httpd_try_cgi(uri_suffix, &cgi_handler)){
		// http request a cgi
		ret = httpd_response_cgi(session, cgi_handler);
		
	}else{
		// 404 not found
		HTTPD_TRACE("Request \"%s\" not found!", uri_suffix);
		ret = httpd_response_404_not_found(session);
	}
	return ret;
}

static void httpd_session_init(int sock, bool* trigger, HTTPD_SESSION_t* session)
{
	memset(session, 0, sizeof(HTTPD_SESSION_t));
	session->trigger = trigger;
	session->sock = sock;
	session->keep_alive = 60;
}

SPOOK_SESSION_PROBE_t HTTPD_probe(const void* msg, ssize_t msg_sz)
{
	if(0 == strncasecmp(msg, "GET", 3)){
		return SPOOK_PROBE_MATCH;
	}else if(0 == strncasecmp(msg, "POST", 4)){
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t HTTPD_loop(bool* trigger, int sock, time_t* read_pts)
{
	signal(SIGPIPE, SIG_IGN);
	int ret;
	HTTPD_SESSION_t* session = alloca(sizeof(HTTPD_SESSION_t));

	httpd_session_init(sock, trigger, session);

	while(*session->trigger)
	{
		fd_set read_set;
		struct timeval select_timeo = { .tv_sec = session->keep_alive, 	.tv_usec = 0, };
		struct timeval recv_timeo = { .tv_sec = session->keep_alive, 	.tv_usec = 0, };
		struct timeval send_timeo = { 	.tv_sec = session->keep_alive * 2, 	.tv_usec = 0, };
		
		// set socket timeout
		ret = setsockopt(session->sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeo, sizeof(send_timeo));
		if(ret < 0){
			// FIXME:
		}
		ret = setsockopt(session->sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeo, sizeof(recv_timeo));
		if(ret < 0){
			// FIXME:
		}

		// ready for select
		FD_ZERO(&read_set);
		FD_SET(session->sock, &read_set);
		
		ret = select(session->sock + 1, &read_set, NULL, NULL, &select_timeo);
		if (ret < 0){
			break;
		}else if(0 == ret){
			HTTPD_TRACE("Keep alive timeout!");
			break;
		}else{
			if(FD_ISSET(session->sock, &read_set))	{
				ssize_t peek_buf_seen;	
				// peek the message first and check whether this a completed http request
				memset(session->request_buf, 0, ARRAY_SIZE(session->request_buf));
				peek_buf_seen = recv(session->sock, session->request_buf, ARRAY_SIZE(session->request_buf), MSG_PEEK);
				if(0 == peek_buf_seen){
					HTTPD_TRACE("Peek select socket close!");
					break;
				}
				
				// remember to put the string end mark '\0'
				if(peek_buf_seen == ARRAY_SIZE(session->request_buf)){
					// the peek_buf is full use
					session->request_buf[peek_buf_seen - 1] = '\0';
				}else{
					session->request_buf[peek_buf_seen] = '\0';
				}

				

				// check the request completed
				//printf("CXC request buf :%s\n", session->request_buf);
				ret = request_check_completed(session->request_buf);
				if(0 == ret){
					HTTPD_TRACE("Request packet is not completed!");
					// retry
					usleep(200000);
					continue;
				}
				
				//////////////////////////////////////////////////////////////////////////
				//http_parse_request_line(session->request_buf, &session->request_line);

				//////////////////////////////////////////////////////////////////////////
				
				// truely receive an analyse
				session->request_sz = recv(session->sock, session->request_buf, ret, 0);
				if(session->request_sz < 0){
					HTTPD_TRACE("An error occurs!");
					// do something
				}

				// parse the http request
				http_parse_request_line(session->request_buf, &session->request_line);
				// ready some attributes for inquery
				// read connection
				do
				{
					AVal av_connection;
					if(0 == http_read_header(session->request_buf, "Connection", &av_connection)){
						session->keep_alive = AVSTRCASEMATCH(&av_connection, "keep-alive") ? DEFAULT_KEEP_ALIVE_DURATION : 0;
						if(session->keep_alive){
							AVal av_keepalive;
							if(0 == http_read_header(session->request_buf, "Keep-Alive", &av_keepalive)){
								session->keep_alive = atoi(AVAL_STRDUPA(av_keepalive));
							}
						}
					}else{
						// default not use keep alive
						session->keep_alive = 0;
					}
				}while(0);
				
				if(0 != httpd_handle(session)){
					// do something
				}

				if(0 == session->keep_alive){
					break;
				}

				usleep(200000);				
			}
		}
	}

	*session->trigger = false;
	return SPOOK_LOOP_SUCCESS;
}

