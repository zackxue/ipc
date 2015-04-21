
#ifndef __HTTP_COMMON_H__
#define __HTTP_COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "aval.h"

typedef struct HTTP_REQUEST_LINE
{
	AVal method;
	AVal uri;
	AVal uri_host;
	AVal uri_hostname;
	AVal uri_suffix;
	AVal uri_suffix_extname;
	AVal uri_query_string;
	AVal version;
}HTTP_REQUEST_LINE_t;

typedef struct HTTP_GENERAL_HEADER_SET
{
	AVal cache_control; // Cache-Control
	AVal connection; // Connection
	AVal date; // Date
	AVal pragma; // Pragma
	AVal triler; // Triler
	AVal transfe_encoding; // Transfe-Encoding
	AVal upgrade; // Upgrade
	AVal via; // Via
	AVal warning; // Warning
}HTTP_GENERAL_HEADER_SET_t;

typedef struct HTTP_REQUEST_HEADER_SET
{
	AVal accept;
	AVal accept_charset; // Accept-Charset
	AVal accept_encoding; // Accept-Encoding
	AVal accept_language; // Accept-Language
	AVal authorization; // Authorization
	AVal cookie; // cookie
	AVal expect; // Expect
	AVal from; // From
	AVal host; // Host
	AVal if_match; // If-Match
	AVal if_modified_since; // If-Modified-Since
	AVal if_none_match; // If-None-Match
	AVal if_range; // If-Range
	AVal if_unmodified_since; // If -Unmodified-Since
	AVal max_forwards; // Max-Forwards
	AVal proxy_authorization; // Proxy-Authorization
	AVal range; // Range
	AVal referer; // Referer
	AVal te; // TE
	AVal user_agent; // User-Agent
}HTTP_REQUEST_HEADER_SET_t;

typedef struct HTTP_ENTITY_HEADER_SET
{
	AVal allow; // Allow
	AVal content_encoding; // Content-Encoding
	AVal content_language; // Content-Language
	AVal content_length; // Content-Length
	AVal content_location; // Content-Location
	AVal content_md5; // Content-MD5
	AVal content_range; // Content-Range
	AVal expires; // Expires
	AVal last_modified; // Last_Modified
	AVal warning; // Warning
}HTTP_ENTITY_HEADER_SET_t;

struct HTTP_HEADER_TAG
{
	char* name;
	char* value;
	struct _HTTP_HEADER_TAG* next;
};

typedef struct _HTTP_HEADER
{
	char* verson;
	// response
	int status_code;
	// request
	char* method;
	char* uri_host;
	char* uri_query_string;
	struct _HTTP_HEADER_TAG* tags;

	// interfaces
	int (*add_tag_text)(struct _HTTP_HEADER* const header, const char* tag_name, const char* tag_val);
	int (*add_tag_int)(struct _HTTP_HEADER* const header, const char* tag_name, int const tag_val);

	int (*add_server)(struct _HTTP_HEADER* const header); // FIXME: add default server name
	int (*add_date)(struct _HTTP_HEADER* const header); // FIXME:
	
	int (*del_tag)(struct _HTTP_HEADER* const header, const char* tag_name);
	int (*to_text)(struct _HTTP_HEADER* const header, char* text, ssize_t text_size);
	int (*dump)(struct _HTTP_HEADER* const header);
	
}HTTP_HEADER_t;

#define CRLF "\r\n"

extern const char* http_get_file_mime(const char* extname);
extern const char* http_get_reason_phrase(int status_code);

extern int http_read_header(const char* header, char* tag, AVal* ret_val);

extern void http_uri_slash_filter(char* uri_rw);

extern int http_parse_request_line(char* request_msg, HTTP_REQUEST_LINE_t* ret_request_line);;
extern int http_parse_general_header(const char* request_msg, HTTP_GENERAL_HEADER_SET_t* ret_header);
extern int http_parse_request_header(const char* request_msg, HTTP_REQUEST_HEADER_SET_t* ret_header);
extern int http_parse_entity_header(const char* request_msg, HTTP_ENTITY_HEADER_SET_t* ret_header);

extern int http_read_query_string(const char* query_string, const char* key, AVal* ret_val);

extern int http_url_encode(const char* in_str, ssize_t const in_size, char* out_str, ssize_t const out_size);
extern int http_url_decode(const char* in_str, ssize_t const in_size, char* out_str, ssize_t const out_size);

// http response header implenament
extern HTTP_HEADER_t* http_response_header_new(const char* version, int status_code, const char* custom_reason);
extern void http_response_header_free(HTTP_HEADER_t* header);
extern int http_fool_style_response(int sock, const char* http_version, int status_code,
	const void* content, ssize_t content_len);

extern HTTP_HEADER_t* http_request_header_new(const char* version, const char* method,
	const char* uri_host, const char* uri_query_string);
extern void http_request_header_free(HTTP_HEADER_t* header);
extern int http_request_header_add_tag_string(HTTP_HEADER_t* const header, const char* tag_name, const char* tag_val);
extern int http_request_header_add_tag_int(HTTP_HEADER_t* const header, const char* tag_name, int const tag_val);
extern int http_request_header_del_tag(HTTP_HEADER_t* const header, const char* tag_name);
extern int http_request_header_to_text(HTTP_HEADER_t* const header, char* text, ssize_t text_size);
extern void http_request_header_dump(HTTP_HEADER_t* const header);
extern int http_fool_style_request(const char* request_ip, uint16_t request_port,
	const char* method, const char* uri_host, const char* uri_query_string, const char* http_version, const char* content, ssize_t content_len);

#ifdef __cplusplus
};
#endif
#endif //__HTTP_COMMON_H__

