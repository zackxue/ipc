
/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, JUAN, Co, All Rights Reserved.
*
*	Project Name: HTTP utilies
*	File Name:
*
*	Writed by Frank Law at 2013 - 06 - 03 JUAN,Guangzhou,Guangdong,China
*
*	ChangeLog:
*		Add interfaces at 2013-06-24 Frank Law
*
*/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef HTTP_UTIL_H_
#define HTTP_UTIL_H_
#ifdef __cplusplus
extern "C" {
#endif

#define kCRLF "\r\n"
#define kQUERY_DELIMIT "&"

#define kHTTP_UTIL_MAGIC (('H'<<24)|('T'<<16)|('T'<<8)|('P'<<0))

typedef char* HTTP_STR_t;
typedef const char* HTTP_CSTR_t;

typedef struct HTTP_TAG {
	HTTP_STR_t name;
	HTTP_STR_t val;
	struct HTTP_TAG *next;
}stHTTP_TAG, *lpHTTP_TAG;

typedef struct HTTP_QUERY_PARA_LIST {
	lpHTTP_TAG paras;
	
	HTTP_CSTR_t (*read)(struct HTTP_QUERY_PARA_LIST *const list, HTTP_CSTR_t name);

	int (*add)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name, HTTP_STR_t val);
	int (*del)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t name);
	
	void (*dump)(struct HTTP_QUERY_PARA_LIST *const thiz);
	ssize_t (*to_text)(struct HTTP_QUERY_PARA_LIST *const thiz, HTTP_STR_t text, size_t text_size);
	
	void (*free)(struct HTTP_QUERY_PARA_LIST *const thiz);
}stHTTP_QUERY_PARA_LIST, *lpHTTP_QUERY_PARA_LIST;

extern lpHTTP_QUERY_PARA_LIST HTTP_UTIL_parse_query_as_para(HTTP_STR_t query_text, size_t header_len);

typedef struct HTTP_HEADER {
	uint32_t magic; // == "HTTP"
	bool request_flag; // if true, this is a request header, else is a response header
	HTTP_STR_t protocol; // HTTP, RTSP ...1.0, 1.1...
	HTTP_STR_t version;
	union {
		// if request_flag is true
		// 5.1 Request-Line = Method SP Request-URI SP HTTP-VERSION CRLF
		// 5.1.2 Request-URI = "*"|absoluteURI|abs_path|authority
		struct {
			HTTP_STR_t method;
			HTTP_STR_t uri;
			HTTP_STR_t query;
		};
		// if request_flag = false
		struct {
			int status_code;
			HTTP_STR_t reason_phrase;
		};
	};
	lpHTTP_TAG tags;
	
	// add & delete the tag
	int (*add_tag_text)(struct HTTP_HEADER *const thiz, HTTP_CSTR_t tag_name, HTTP_CSTR_t tag_val, bool over_write);
	int (*add_tag_int)(struct HTTP_HEADER *const thiz, HTTP_CSTR_t tag_name, int tag_val, bool over_write);
	int (*del_tag)(struct HTTP_HEADER *const thiz, HTTP_CSTR_t tag_name);
	// add & delete tag in special
	int (*add_tag_server)(struct HTTP_HEADER *const thiz, HTTP_CSTR_t server_name);
	int (*del_tag_server)(struct HTTP_HEADER *const thiz);
	int (*add_tag_date)(struct HTTP_HEADER *const thiz, time_t t);
	int (*del_tag_date)(struct HTTP_HEADER *const thiz);
	
	HTTP_CSTR_t (*read_tag)(struct HTTP_HEADER *const thiz, HTTP_CSTR_t tag_name);
	
	void (*dump)(struct HTTP_HEADER *const thiz);
	ssize_t (*to_text)(struct HTTP_HEADER *const thiz, HTTP_STR_t text, size_t text_size);

	// free the memory allocation of this header, this header would be invalid once free
	void (*free)(struct HTTP_HEADER *const thiz);
}stHTTP_HEADER, *lpHTTP_HEADER;

// filter the double slash in url
extern void HTTP_UTIL_url_filter(HTTP_STR_t url);

// uri encode / decode
extern ssize_t HTTP_UTIL_url_encode(HTTP_CSTR_t in_text, size_t in_size, HTTP_STR_t out_text, size_t out_size);
extern ssize_t HTTP_UTIL_url_decode(HTTP_CSTR_t in_text, size_t in_size, HTTP_STR_t out_text, size_t out_size);

// check whether there is an http header in text
extern ssize_t HTTP_UTIL_check_header(HTTP_STR_t text, size_t stack_len);

// http response reason phrase
extern HTTP_CSTR_t HTTP_UTIL_reason_phrase(int status_code);

// http file mime
extern HTTP_CSTR_t HTTP_UTIL_file_mime(HTTP_CSTR_t file_ext);

// http date header value
extern HTTP_CSTR_t HTTP_UTIL_date_header(HTTP_STR_t buf, size_t buf_len, time_t t);

// new a request / response header of http
extern lpHTTP_HEADER HTTP_UTIL_new_request_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, HTTP_CSTR_t method, HTTP_CSTR_t uri, HTTP_CSTR_t query);
extern lpHTTP_HEADER HTTP_UTIL_new_response_header(HTTP_CSTR_t protocol, HTTP_CSTR_t version, int status_code, HTTP_CSTR_t reason_phrase);

// parse a request / response header of http from text
extern lpHTTP_HEADER HTTP_UTIL_parse_request_header(HTTP_STR_t header_text, size_t header_len);
extern lpHTTP_HEADER HTTP_UTIL_parse_response_header(HTTP_STR_t header_text, size_t header_len);

// receive a request / response header of http from socket
extern lpHTTP_HEADER HTTP_UTIL_recv_request_header(int sock);
extern lpHTTP_HEADER HTTP_UTIL_recv_response_header(int sock);

#ifdef __cplusplus
};
#endif
#endif //HTTP_UTIL_H_

#ifdef HTTP_UTIL_TEST

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

int main()
{
	do{
		int ret = 0;
		
		char snd_buf[1024] = {""};

		lpHTTP_HEADER snd_header = NULL;
		lpHTTP_HEADER rcv_header = NULL;

		struct sockaddr_in pear_addr;
		struct hostent *host_ent = NULL;

		int sock; 
		char pear_name[] = "www.baidu.com";
		int pear_port = 80;

		host_ent = gethostbyname(pear_name);
		if(!host_ent){
			perror("gethostbyname");
		}

		memset(&pear_addr, 0, sizeof(pear_addr));
		pear_addr.sin_family = AF_INET;
		pear_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
		pear_addr.sin_addr.s_addr = ((struct in_addr *)(host_ent->h_addr))->s_addr;
		pear_addr.sin_port = htons(pear_port);

		sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock < 0){
			perror("socket");
		}

		if(connect(sock, (struct sockaddr*)&pear_addr, sizeof(pear_addr)) < 0){
			perror("connect");
		}

		snd_header = HTTP_UTIL_new_request_header(NULL, "1.1", "get", "/", NULL);
		snd_header->add_tag_int(snd_header, "Content-Length", 0, false);
		snd_header->to_text(snd_header, snd_buf, sizeof(snd_buf));

		ret = send(sock, snd_buf, strlen(snd_buf), 0);
		if(ret < 0){
			perror("send");
		}
		
		rcv_header = HTTP_UTIL_recv_response_header(sock);
		if(NULL != rcv_header){
			rcv_header->dump(rcv_header);
		}

		snd_header->free(snd_header);
		rcv_header->free(rcv_header);

		close(sock);
		
		char *request_text = 
			"GET /aksdjkfskdfklasldfjaskdfjl?aa=cc&badfsd=ddddf&dd=waeraw&cc=werwe HTTP/1.1\r\n"
			"Date: Mon, 03 Jun 2013 11:12:10 GMT\r\n"
			"Server: BWS/1.0\r\n"
			"Content-Length: 10407\r\n"
			"Content-Type: text/html;charset=utf-8\r\n"
			"Cache-Control: private\r\n"
			"Set-Cookie: BDSVRTM=19; path=/\r\n"
			"Set-Cookie: H_PS_PSSID=2359_1788_2548_2249; path=/; domain=.baidu.com\r\n"
			"Set-Cookie: BAIDUID=5D615A222F3E07F155AABCC9C6047F9A:FG=1; expires=Mon, 03-Jun-4 3 11:12:10 GMT; path=/; domain=.baidu.com \r\n"
			"Expires: Mon, 03 Jun 2013 11:12:10 GMT\r\n"
			"P3P: CP=\" OTI DSP COR IVA OUR IND COM \"\r\n"
			"Connection: Keep-Alive\r\n";

		rcv_header = HTTP_UTIL_parse_request_header(request_text, (size_t)strlen(request_text));
		rcv_header->dump(rcv_header);

		lpHTTP_QUERY_PARA_LIST query = HTTP_UTIL_parse_query_as_para(rcv_header->query, strlen(rcv_header->query));
		query->dump(query);
		printf("read \"badfsd=%s\"\r\n", query->read(query, "badfsd"));
		query->free(query);
		
		rcv_header->free(rcv_header);
		
	}while(1);

	return 0;
}
#endif


