#ifndef HICHIP_HTTP_CGI_H_
#define HICHIP_HTTP_CGI_H_

#include "httpd.h"

void HICHIP_http_init();
void HICHIP_http_destroy();
int HICHIP_http_cgi(HTTPD_SESSION_t* session);


#endif

