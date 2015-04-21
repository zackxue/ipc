

#ifndef __RTSP_PARSER_H__
#define __RTSP_PARSER_H__

#include <stdint.h>
#include <libgen.h>
#include "aval.h"


extern AVal RTSPPARSER_create(const char* str, ssize_t str_sz);

extern AVal RTSPPARSER_get_command(AVal parser);
extern AVal RTSPPARSER_read_option(AVal parser, const char* option);
extern int RTSPPARSER_read_int(const char* msg, const char* tag);

extern int RTSPD_parse_url(const char* url, char* ret_addr, uint16_t* ret_port, char* ret_trace);
extern int RTSPD_parse_request(const char* request, char* ret_command, char* ret_url);

#endif //__RTSP_PARSER_H__

