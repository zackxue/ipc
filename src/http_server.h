


#include <stdint.h>
#include <stdbool.h>
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
#include "http_util.h"

#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kHTTP_SERV_DEFAULT_NAME "JAWS/1.0"
#define kHTTP_SERV_DEFAULT_USER_AGENT kHTTP_SERV_DEFAULT_NAME" "__DATE__

#define kHTTP_SERV_KEEP_ALIVE_DURATION (60)

typedef struct HTTP_CONTEXT {
	int sock;
	bool *trigger;
	int keep_alive; // keep alive duration, if 0 means close
	// request info
	lpHTTP_HEADER request_header;
	size_t request_content_len;
	void *request_content;
}stHTTP_CONTEXT, *lpHTTP_CONTEXT;

typedef int (*fHTTP_SERV_CGI_HANDLER)(lpHTTP_CONTEXT context);

typedef struct HTTP_SERV_CGI {
	char uri[256];
	fHTTP_SERV_CGI_HANDLER handler;
	struct HTTP_SERV_CGI *next;
}stHTTP_SERV_CGI, *lpHTTP_SERV_CGI;

typedef struct HTTP_SERV_USER {
	char username[32];
	char password[32];
	char auth_basic[128]; // this user base64 auth
	struct HTTP_SERV_USER *next;
}stHTTP_SERV_USER, *lpHTTP_SERV_USER;

typedef struct HTTP_SERV {

	// set the resource directory
	int (*set_resource_dir)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t directory);

	// start / stop the server
	int (*do_loop)(struct HTTP_SERV *const http_serv, bool flag);

	// session loop
	int (*session_loop)(struct HTTP_SERV *const http_serv, bool *session_trigger, int session_sock);

	// user operation
	int (*add_user)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username, HTTP_CSTR_t password);
	int (*del_user)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t username);
	int (*clear_user)(struct HTTP_SERV *const http_serv);

	// cgi operation
	int (*add_cgi)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri, fHTTP_SERV_CGI_HANDLER handler);
	int (*del_cgi)(struct HTTP_SERV *const http_serv, HTTP_CSTR_t uri);
	int (*clear_cgi)(struct HTTP_SERV *const http_serv);
	
}stHTTP_SERV, *lpHTTP_SERV;

extern lpHTTP_SERV HTTP_SERV_init();
extern void HTTP_SERV_destroy(lpHTTP_SERV http_serv);

extern bool HTTP_SERV_probe(HTTP_CSTR_t request_packet, size_t packet_len);

#ifdef __cplusplus
}
#endif
#endif //HTTP_SERVER_H_

