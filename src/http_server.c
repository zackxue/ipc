
#include "http_server.h"
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <pthread.h>
#include "generic.h"
#include "socket_tcp.h"
#include "app_debug.h"

typedef struct HTTP_SERV_ATTR {
	char resource_dir[64]; // the http server resource root directory

	lpHTTP_SERV_CGI cgi_list; // cgi list of this server
	lpHTTP_SERV_USER user_list; // user list of this server;

	// the socket of this server
	int sock;

	// server running status
	bool trigger;
	pthread_t tid;
	
}stHTTP_SERV_ATTR, *lpHTTP_SERV_ATTR;


// set the resource directory
static int http_serv_set_resource_dir(struct HTTP_SERV *const http_serv, HTTP_CSTR_t directory)
{
	lpHTTP_SERV_ATTR attr = (lpHTTP_SERV_ATTR)(http_serv + 1);
	if(strlen(directory) < sizeof(attr->resource_dir)){
		strcpy(attr->resource_dir, directory);
		// FIXME: remember to remove the ending slash
		return 0;
	}
	return -1;
}

static void *http_serv_listener(void *arg)
{
	lpHTTP_SERV const http_serv = (lpHTTP_SERV)arg;
	lpHTTP_SERV_ATTR attr = (lpHTTP_SERV_ATTR)(http_serv + 1);

	// FIXME:
	// establish lister socket

	// listener loop
	// probe -> check -> create thread -> loop
	http_serv->session_loop(http_serv, &attr->trigger, attr->sock);

	// close socket
	close(attr->sock);
	attr->sock = -1;

	
	pthread_exit(NULL);
}

// start / stop the server
static int http_serv_do_loop(struct HTTP_SERV *const http_serv, bool flag)
{
	return -1; // FIXME:
	int ret = -1;
	lpHTTP_SERV_ATTR attr = (lpHTTP_SERV_ATTR)(http_serv + 1);

	if(!attr->tid && flag){
		// start
		attr->trigger = true;
		ret = pthread_create(&attr->tid, NULL, http_serv_listener, (void*)http_serv);
		return ret;
	}else if(attr->tid && flag){
		// stop
		attr->trigger = false;
		pthread_join(attr->tid, NULL);
		attr->tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}

// user operation
static int http_serv_add_user(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username, HTTP_CSTR_t password)
{
	// FIXME:
	return -1;
}

static int http_serv_del_user(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username)
{
	// FIXME:
	return -1;
}

static int http_serv_clear_user(struct HTTP_SERV *const http_serv)
{
	// FIXME:
	return -1;
}

// cgi operation
static int http_serv_add_cgi(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri, fHTTP_SERV_CGI_HANDLER handler)
{
	// FIXME:
	return -1;
}

static int http_serv_del_cgi(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri)
{
	// FIXME:
	return -1;
}

static int http_serv_clear_cgi(struct HTTP_SERV *const http_serv)
{
	// FIXME:
	return -1;
}

static int http_serv_response_content(lpHTTP_CONTEXT context, int status_code, HTTP_CSTR_t reason_phrase,
	HTTP_CSTR_t content_type, const void *content_buf, size_t content_len)
{
	int const buf_len = 4096 + content_len;
	char *response_buf = alloca(4096 + content_len);
	size_t response_len = 0;
	lpHTTP_HEADER response_header = HTTP_UTIL_new_response_header(context->request_header->protocol, context->request_header->version,
		status_code, reason_phrase);
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);
	
	if(NULL != response_header){
		response_header->add_tag_server(response_header, kHTTP_SERV_DEFAULT_NAME);
		response_header->add_tag_date(response_header, 0);
		response_header->add_tag_int(response_header, "Content-Length", content_len, true);
		// add content
		if(content_len > 0 && NULL != content_type){
			response_header->add_tag_text(response_header, "Content-Type", (HTTP_STR_t)content_type, true);
		}
		response_len = response_header->to_text(response_header, response_buf, buf_len);
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;
		// catch the content
		if(content_len > 0){
			memcpy(response_buf + response_len, content_buf, content_len);
		}
		//printf("%s\r\n", response_buf);
		if(tcp->send(tcp, response_buf, response_len, 0) == response_len){
			return 0;
		}
	}
	return -1;
}

static int http_serv_response_401_unauthorized(lpHTTP_CONTEXT context)
{
	lpHTTP_HEADER response_header = NULL;
	char response_buf[1024] = {""};
	size_t response_len = 0;
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);
	
	response_header = HTTP_UTIL_new_response_header(context->request_header->protocol,
		context->request_header->version, 401, NULL);
	response_header->add_tag_server(response_header, kHTTP_SERV_DEFAULT_NAME);		
	response_header->add_tag_date(response_header, 0);
	response_header->add_tag_text(response_header, "Connection", "close", true);
	response_header->add_tag_text(response_header, "WWW-Authenticate", "MD5 realm=\"IP Camera\"", true);
	response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
	response_header->dump(response_header);
	response_header->free(response_header);
	response_header = NULL;

	// clear keep alive
	context->keep_alive = 0;
	
	if(tcp->send(tcp, response_buf, response_len, 0) == response_len){
		return 0;
	}
	return -1;
}

static int http_serv_response_404_not_found(lpHTTP_CONTEXT context)
{
	HTTP_CSTR_t content_404 = 
		"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>404 Not Found</title>\r\n"
		"</head><body>\r\n"
		"<h1>Not Found</h1>\r\n"
		"<p>The requested URL requested was not found on this server.</p>\r\n"
		"</body></html>\r\n";
	return http_serv_response_content(context, 404, NULL, "text/html", content_404, strlen(content_404));
}

static const char *resource_dir = "./web/";

static HTTP_CSTR_t http_serv_where_path(HTTP_STR_t uri_path, HTTP_STR_t file_path, size_t file_path_len)
{
	snprintf(file_path, file_path_len, "%s%s", resource_dir, uri_path);
	APP_ASSERT(strlen(file_path) < file_path_len, "file path size is not enough");
	return file_path;
}

static bool http_serv_test_dir(HTTP_STR_t dir_path)
{
	DIR *dir = opendir(dir_path);
	APP_TRACE("open dir %s", dir_path);
	if(!dir){
		return false;
	}
	closedir(dir);
	return true;
}

static bool http_serv_test_file(HTTP_STR_t file_path)
{
	return (0 == access(file_path, F_OK)) ? true : false;
}

static int http_serv_response_file(lpHTTP_CONTEXT context,
	HTTP_CSTR_t file_path, HTTP_CSTR_t file_mime, HTTP_CSTR_t encoding)
{
	int ret = -1;
	char response_buf[4096] = {""};
	size_t response_len = 0;
	FILE* fid = NULL;
	HTTP_CSTR_t http_if_modified_since = context->request_header->read_tag(context->request_header, "If-Modified-Since");
	HTTP_CSTR_t http_connection = context->request_header->read_tag(context->request_header, "Connection");
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);
	
	fid = fopen(file_path, "rb");
	if(NULL == fid){
		return http_serv_response_content(context, 404, NULL, NULL, NULL, 0);
	}else{
		// get file state
		char http_date_header[64] = {""};
		struct stat file_stat;
		fstat(fileno(fid), &file_stat);
		lpHTTP_HEADER response_header = NULL;
		
		HTTP_UTIL_date_header(http_date_header, sizeof(http_date_header), file_stat.st_mtime);		
		if(NULL != http_if_modified_since
			&& STR_CASE_THE_SAME(http_if_modified_since, http_date_header)){
			// needless to send this file because it has exist in browser's cache
			fclose(fid);			
			//APP_TRACE("File modified if-modified-since=\"%s\" file=\"%s\"", http_date_header, http_if_modified_since);
			return http_serv_response_content(context, 304, NULL, file_mime, NULL, 0);
		}

		// make the response packet
		response_header = HTTP_UTIL_new_response_header(context->request_header->protocol,
			context->request_header->version, 200, NULL);
		response_header->add_tag_server(response_header, kHTTP_SERV_DEFAULT_NAME);		
		response_header->add_tag_date(response_header, 0);
		response_header->add_tag_text(response_header, "Content-Type", file_mime, true);
		response_header->add_tag_text(response_header, "Last-Modified", HTTP_UTIL_date_header(http_date_header, sizeof(http_date_header), file_stat.st_mtime), true);
		if(NULL != http_connection){
			response_header->add_tag_text(response_header, "Connection", http_connection, true);
		}
		if(NULL != encoding){
			response_header->add_tag_text(response_header, "Content-Encoding", encoding, true);
		}
		response_header->add_tag_int(response_header, "Content-Length", file_stat.st_size, true);
		response_len = response_header->to_text(response_header, response_buf, sizeof(response_buf));
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;
			
		// response the http header
		ret = tcp->send2(tcp, response_buf, strlen(response_buf));
		if(strlen(response_buf) == ret){
			// continue to resopnse the file data
			ssize_t read_size = 0;
			char file_buf[1024] = {""};

			while((read_size = fread(file_buf, 1, sizeof(file_buf), fid)) > 0){
				ret = tcp->send2(tcp, file_buf, read_size);
				if(ret < 0){
					fclose(fid);
					return -1;
				}
			}
		}else{
			fclose(fid);
			return -1;
		}
		fclose(fid);
	}
	return 0;
}

static int http_serv_response_file_auth(lpHTTP_SERV const http_serv, lpHTTP_CONTEXT context,
	HTTP_CSTR_t file_path, HTTP_CSTR_t file_mime, HTTP_CSTR_t encoding)
{
	lpHTTP_SERV_ATTR attr = (lpHTTP_SERV_ATTR)(http_serv + 1);	
	HTTP_CSTR_t http_authorization = context->request_header->read_tag(context->request_header, "Authorization");
	
	// if user user autherized 
	if(NULL != attr->user_list){
		if(NULL == http_authorization){
			return http_serv_response_401_unauthorized(context);
		}else{
			APP_TRACE("Authorization: %s", http_authorization);
			if(0 != strcmp(http_authorization, "Basic YWRtaW46YWRtaW4=")){
				return http_serv_response_401_unauthorized(context);
			}
		}
	}
	return http_serv_response_file(context, file_path, file_mime, encoding);
}


static int http_serv_process_response(lpHTTP_SERV const http_serv, lpHTTP_CONTEXT context)
{
	lpHTTP_SERV_ATTR const attr = (lpHTTP_SERV_ATTR)(http_serv + 1);
	char abs_path[128] = {""}, abs_path_gz[128] = {""};
	size_t abs_path_len = 0;
	HTTP_CSTR_t file_mime = NULL;
	HTTP_CSTR_t accept_encoding = context->request_header->read_tag(context->request_header, "Accept-Encoding");
	bool accept_gzip = strstr(accept_encoding, "gzip");
//	stSOCKET_TCP sock_tcp;
//	lpSOCKET_TCP tcp = socket_tcp2_r(context->sock, &sock_tcp);
	
	// get the abs file with gzip
	abs_path_len = snprintf(abs_path, sizeof(abs_path), "%s%s", attr->resource_dir, context->request_header->uri);

	APP_TRACE("ABS path = %s", abs_path);

	if(0 == strcmp("/favicon.ico", context->request_header->uri)){
		// FIXME: no icon
		return http_serv_response_file(context, abs_path, file_mime, "gzip");
	}else{
		// test directory
		if('/' == abs_path[abs_path_len - 1]){
			if(http_serv_test_dir(abs_path)){
				// try its default file
				abs_path_len = snprintf(abs_path + abs_path_len, sizeof(abs_path) - abs_path_len,
					"%s", "index.html");
				APP_ASSERT(abs_path_len < sizeof(abs_path), "WEB abs path size no enough");
			}
		}
		
		// test file
		file_mime = HTTP_UTIL_file_mime(strrchr(abs_path, '.'));
		snprintf(abs_path_gz, sizeof(abs_path_gz), "%s.gz", abs_path);
		
		if(accept_gzip && http_serv_test_file(abs_path_gz)){
			// relevant file which gzip found
			return http_serv_response_file_auth(http_serv, context, abs_path_gz, file_mime, "gzip");

		}else if(http_serv_test_file(abs_path)){
			// relevant file found
			return http_serv_response_file_auth(http_serv, context, abs_path, file_mime, NULL);
		
		}else if(0){
			// check response cgi
			
			
		}else{
			return http_serv_response_404_not_found(context);
		}
	}
	return -1;
}

bool HTTP_SERV_probe(HTTP_CSTR_t request_packet, size_t packet_len)
{
	if(0 == strncasecmp(request_packet, "GET", 3)
		|| 0 == strncasecmp(request_packet, "POST", 4)){
		// check the method of http
		if(1){
			// check the protocol name
			return true;
		}
	}
	return false;
}

static void http_serv_context_init_r(bool *trigger, int sock, lpHTTP_CONTEXT context)
{
	context->trigger = trigger;
	context->sock = sock;
	context->keep_alive = kHTTP_SERV_KEEP_ALIVE_DURATION;
	// request header
	context->request_header = NULL; // never receive a header
	context->request_content_len = 0; // no content
	context->request_content = NULL; // no content
}

static int http_serv_session_loop(struct HTTP_SERV *const http_serv, bool *session_trigger, int session_sock)
{
	int ret;
	stHTTP_CONTEXT context;
	stSOCKET_TCP sock_tcp;
	lpSOCKET_TCP tcp = 	socket_tcp2_r(session_sock, &sock_tcp);
	
	http_serv_context_init_r(session_trigger, session_sock, &context);

	while(*context.trigger && context.keep_alive > 0){
		fd_set read_fds;
		struct timeval select_timeo;

		// poll wait timeout
		select_timeo.tv_sec = context.keep_alive;
		select_timeo.tv_usec = 0;
		
		// set socket timeout
		ret = tcp->set_send_timeout(tcp, context.keep_alive * 2, 0);
		if(ret < 0){
			// FIXME:
		}
		ret = tcp->set_recv_timeout(tcp, context.keep_alive, 0);
		if(ret < 0){
			// FIXME:
		}

		// ready for select
		FD_ZERO(&read_fds);
		FD_SET(context.sock, &read_fds);
		
		ret = select(context.sock + 1, &read_fds, NULL, NULL, &select_timeo);
		if (ret < 0){
			// an error
			break;
		}else if(0 == ret){
			APP_TRACE("Keep alive timeout!");
			break;
		}else{
			if(FD_ISSET(context.sock, &read_fds))	{
				HTTP_CSTR_t http_connection = NULL;
				HTTP_CSTR_t http_keep_alive = NULL;
				HTTP_CSTR_t http_content_len = NULL;

				// receive request header
				context.request_header = HTTP_UTIL_recv_request_header(context.sock);
				if(NULL == context.request_header){
					// an error when receive the head
					break;
				}

				// request header dump
				//context.request_header->dump(context.request_header);
					
				// get keep alive duration
				http_connection = context.request_header->read_tag(context.request_header, "Connection");
				if(NULL != http_connection && 0 == strcasecmp("Keep-Alive", http_connection)){
					http_keep_alive = context.request_header->read_tag(context.request_header, "Keep-Alive");
					if(NULL != http_keep_alive){
						context.keep_alive = atoi(http_keep_alive);
					}else{
						context.keep_alive = kHTTP_SERV_KEEP_ALIVE_DURATION;
					}
				}else{
					context.keep_alive = 0;
				}
				//APP_TRACE("HTTP keep alive: %d", context.keep_alive);

				// get content length
				http_content_len = context.request_header->read_tag(context.request_header, "Content-Length");
				if(NULL != http_content_len){
					// receive the content of http request
					context.request_content_len = atoi(http_content_len);
					context.request_content = calloc(context.request_content_len + 1, 1);
					ret = tcp->recv2(tcp, context.request_content, context.request_content_len);
					APP_ASSERT(ret == context.request_content_len, "HTTP recv content error");
				}

				if(http_serv_process_response(http_serv, &context) < 0){
					// break the loop
					context.keep_alive = 0;
				}
				
				// free the context memory
				if(0 != context.request_content_len){
					free(context.request_content);
					context.request_content = NULL;
					context.request_content_len = 0;
				}
				context.request_header->free(context.request_header);
				context.request_header = NULL;
	
			}
		}
	}

	usleep(200000);
	*context.trigger = false;
	return 0;
}


lpHTTP_SERV HTTP_SERV_init()
{
	lpHTTP_SERV http_serv = calloc(sizeof(stHTTP_SERV) + sizeof(stHTTP_SERV_ATTR), 1);
	lpHTTP_SERV_ATTR attr = (lpHTTP_SERV_ATTR)(http_serv + 1);

	// attributes init
	attr->sock = -1;
	attr->trigger = false;
	attr->tid = (pthread_t)NULL;
	attr->user_list = NULL;
	attr->cgi_list = NULL;
	strcpy(attr->resource_dir, "./web"); // defaut web directory

	// interfaces
	http_serv->set_resource_dir = http_serv_set_resource_dir;
	http_serv->do_loop = http_serv_do_loop;
	http_serv->session_loop = http_serv_session_loop;

	// user operations
	http_serv->add_user = http_serv_add_user;
	http_serv->del_user = http_serv_del_user;
	http_serv->clear_user = http_serv_clear_user;

	// cgi operations
	http_serv->add_cgi = http_serv_add_cgi;
	http_serv->del_cgi = http_serv_del_cgi;
	http_serv->clear_cgi = http_serv_clear_cgi;

	return http_serv;
}

void HTTP_SERV_destroy(lpHTTP_SERV http_serv)
{
}


