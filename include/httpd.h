
#ifndef __HTTPD_H__
#define __HTTPD_H__


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

#include "http_common.h"
#include "spook/spook.h"
#include "aval.h"


#define HTTPD_SESSION_REQUEST_BUF_SZ (8 * 1024)
#define HTTPD_SESSION_REQUEST_LINE_SZ (8)


typedef struct HTTPD_SESSION
{
	char request_buf[HTTPD_SESSION_REQUEST_BUF_SZ];
	int request_sz;
	
	int sock;
	bool* trigger;
	
	int keep_alive;
//	int method;
//	AVal path_info;
//	AVal path_ext;

	int auth_counter;

	// request line
	HTTP_REQUEST_LINE_t request_line;
}HTTPD_SESSION_t;

extern int HTTPD_init(const char* file_folder);

extern SPOOK_SESSION_PROBE_t HTTPD_probe(const void* msg, ssize_t msg_sz);
extern SPOOK_SESSION_LOOP_t HTTPD_loop(bool* trigger, int sock, time_t* read_pts);

typedef int (*CGI_HANDLER)(HTTPD_SESSION_t *session);

extern int HTTPD_add_cgi(const char* name, CGI_HANDLER handler);
extern int HTTPD_remove_cgi(const char* name);

extern int HTTPD_response_file(HTTPD_SESSION_t* session, const char* file_name);

extern int HTTPD_auth_add(const char* username, char* password, bool override);
extern int HTTPD_auth_del(const char* username);
extern void HTTPD_auth_clear();

extern bool HTTPD_auth_access(const char* auth);

extern void HTTPD_auth_dump();

extern int HTTPD_auth_user_cnt();

#endif //__HTTPD_H__

