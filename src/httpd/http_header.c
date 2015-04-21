
#include "generic.h"
#include "httpd_def.h"
#include "httpd_debug.h"
#include "httpd.h"



static struct HTTP_HEADER_TAG* header_tag_new(const char* name, const char* value)
{
	struct HTTP_HEADER_TAG* tag = calloc(sizeof(struct HTTP_HEADER_TAG), 1);
	tag->name = strdup(name);
	tag->value = strdup(value);
	tag->next = NULL;
	return tag;
}

static int header_add_tag_text(HTTP_HEADER_t* const header, const char* tag_name, const char* tag_val)
{
	if(header){
		if(tag_name && strlen(tag_name) && tag_val && strlen(tag_val) > 0){
			if(NULL == header->tags){
				// add the first tag
				header->tags = header_tag_new(tag_name, tag_val);
				return 0;
			}else{
				struct HTTP_HEADER_TAG* tag = header->tags;
				// for each tag
				for(;;){
					if(STR_CASE_THE_SAME(tag->name, tag_name)){
						// over write the ready tag
						free(tag->value);
						tag->value = strdup(tag_val);
						return 0;
					}
					if(NULL == tag->next){
						tag->next = header_tag_new(tag_name, tag_val);
						return 0;
					}
					tag = tag->next;
				}
			}
			
		}
	}
	return -1;
}

static int header_add_tag_int(HTTP_HEADER_t* const header, const char* tag_name, int const tag_val)
{
	char str_val[64] = {""};
	sprintf(str_val, "%d", tag_val);
	return header->add_tag_text(header, tag_name, str_val);
}

static int header_del_tag(HTTP_HEADER_t* const header, const char* tag_name)
{
	return -1;
}

static int header_to_text(HTTP_HEADER_t* const header, char* text, ssize_t text_size)
{
	if(text && text_size > 0){
		int ret;
		struct HTTP_HEADER_TAG* tag = header->tags;
		// setup response line
		ret = snprintf(text, text_size,
			"HTTP/%s %d %s"CRLF,
			header->verson, header->status_code, http_get_reason_phrase(header->status_code));
		// adding tags
		while(NULL != tag){
			ret = snprintf(text + strlen(text), text_size - strlen(text),
				"%s:%s"CRLF,
				tag->name, tag->value);
			tag = tag->next;
		}
		// append the CRLF
		strncat(text, CRLF, strlen(CRLF));
		// finish
//		http_header_dump(header);
		return 0;
	}
	return -1;
}

static int header_dump(HTTP_HEADER_t* const header)
{
	struct HTTP_HEADER_TAG* tag = header->tags;
	const char* reason_phrase = http_get_reason_phrase(header->status_code);
	printf("%s"CRLF"{"CRLF, __func__);
	printf("\tHTTP/%s %d %s"CRLF,
		header->verson, header->status_code, reason_phrase ? reason_phrase : "Unkwoned");
	while(NULL != tag){
		printf("\t%s: %s"CRLF, tag->name, tag->value);
		tag = tag->next;
	}
	printf("}"CRLF);
	return 0;
}

HTTP_HEADER_t* http_response_header_new(const char* version, int status_code, const char* custom_reason)
{
	HTTP_HEADER_t* header = calloc(sizeof(HTTP_HEADER_t), 1);
	header->verson = strdup(version);
	// response part
	header->status_code = status_code;
	// request part
	header->method = NULL;
	header->uri_host = NULL;
	header->uri_query_string = NULL;
	// the tag
	header->tags = NULL;
	// add some basic header

	// interfaces
	header->add_tag_text = header_add_tag_text;
	header->add_tag_int = header_add_tag_int;
	header->del_tag = header_del_tag;
	header->to_text = header_to_text;
	header->dump = header_dump;
	
	//header->add_tag_text(header, "Server", HTTPD_SERVER_NAME);
	header->add_tag_text(header, "Server", "HiIpcam");
	return header;
}

void http_response_header_free(HTTP_HEADER_t* header)
{
	if(header){
		struct HTTP_HEADER_TAG* tag = header->tags;
		// release tags
		while(NULL != tag){
			struct HTTP_HEADER_TAG* const next_tag = tag->next;
			free(tag);
			tag = next_tag; // very important
		}
		// free items
		free(header->verson);
		free(header->method);
		free(header->uri_host);
		free(header->uri_query_string);
		// free header
		free(header);
	}
}

int http_fool_style_response(int sock, const char* http_version, int status_code,
	const void* content, ssize_t content_len)
{
	int ret = 0;
	const char* reason_phrase = http_get_reason_phrase(status_code);
	HTTP_HEADER_t* response_header = NULL;
	char response_buf[1024] = {""};

	response_header = http_response_header_new(http_version, status_code, reason_phrase ? reason_phrase : "Unknown");
	response_header->add_tag_text(response_header, "Content-Type", "text/html; charset=UTF-8");
	if(content && content_len > 0){
		response_header->add_tag_int(response_header, "Content-length", content_len);
	}
	response_header->to_text(response_header, response_buf, ARRAY_ITEM(response_buf));
	http_response_header_free(response_header);
	response_header = NULL;

	// response header
	ret = send(sock, response_buf, strlen(response_buf), 0);
	if(strlen(response_buf) == ret){
		// send header success
		// then send content
		if(content && content_len > 0){
			ret = send(sock, content, strlen(content), 0);
			if(strlen(content) == ret){
				HTTPD_TRACE("Send error %s", strerror(errno));
				return -1;
			}
		}
		return 0;
	}
	if(ret < 0){
		return -1;
	}
	return http_fool_style_response(sock, http_version, 417, content, content_len);
}

