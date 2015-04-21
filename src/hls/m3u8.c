
#include "generic.h"
#include "httpd.h"
#include "hls.h"

int CGI_hls_html(HTTPD_SESSION_t* session)
{
	const char* hls_html = "<video src=\"/example/20120504110730.ts\" >This is an HLS demo </video>";
	http_fool_style_response(session->sock, "1.1", 200, hls_html, strlen(hls_html));
	return 0;
}

static int media_sequence = 1;
