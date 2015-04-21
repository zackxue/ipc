
#include "rtsp_parser.h"

AVal RTSPPARSER_create(const char* str, ssize_t str_sz)
{
	AVal parser;
	parser.av_val = str;
	parser.av_len = str_sz;
	return parser;
}

AVal RTSPPARSER_get_command(AVal parser)
{
	char* ptr = NULL;
	AVal cmd = AVV(NULL, 0);
	ptr = strchr(parser.av_val, ' ');
	if(ptr){
		cmd = RTSPPARSER_create(parser.av_val, ptr - parser.av_val);
		return cmd;
	}
	return cmd;
}

AVal RTSPPARSER_read_option(AVal parser, const char* option)
{
	char* ptr = NULL;
	char option_hdr[64];
	AVal av_opt = AVV(NULL, 0);
	sprintf(option_hdr, "%s: ", option);
	ptr = strstr(parser.av_val, option_hdr);
	if(ptr){
		char* ptr_end = NULL;
		ptr_end = strstr(ptr, "\r\n");
		if(ptr_end){
			av_opt.av_val = ptr + strlen(option_hdr);
			av_opt.av_len = ptr_end - av_opt.av_val;
		}
	}
	return av_opt;
}


int RTSPD_parse_url(const char* url, char* ret_addr, uint16_t* ret_port, char* ret_trace)
{
	char* const url_addr = strdupa(url);
	char* const url_trace = strdupa(url);;
	uint32_t ip[4];

	if(ret_addr && ret_port){
		sscanf(dirname(url_addr), "rtsp://%u.%u.%u.%u:%u",
			ip + 0, ip + 1, ip + 2, ip + 3, ret_port);
		sprintf(ret_addr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	}
	if(ret_trace){
		strcpy(ret_trace, basename(url_trace));
	}
	return 0;
}

int RTSPPARSER_read_int(const char* msg, const char* tag)
{
	char* ptr = msg;
	char tag_hdr[32];
	sprintf(tag_hdr, "%s: ", tag);
	ptr = strstr(msg, tag_hdr);
	if(ptr){
		ptr += strlen(tag_hdr);
		return atoi(ptr);
	}
	return 0;
}

int RTSPD_parse_request(const char* request, char* ret_command, char* ret_url)
{
	char command[64] = {""};
	char url[128] = {""};
	char reserved[1024] = {""};
	sscanf(request, "%s %s RTSP/1.0%s", command, url, reserved);
	if(ret_command){
		strcpy(ret_command, command);
	}
	if(ret_url){
		strcpy(ret_url, url);
	}
	return 0;
}

