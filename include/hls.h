
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "httpd.h"

extern int CGI_hls_html(HTTPD_SESSION_t* session);

extern int CGI_hls_live_m3u8(HTTPD_SESSION_t* session);
extern int CGI_hls_live_ts(HTTPD_SESSION_t* session);

extern int CGI_hls_demo_m3u8(HTTPD_SESSION_t* http_session);
extern int CGI_hls_demo_ts(HTTPD_SESSION_t* http_session);


#ifdef __cplusplus
};
#endif

