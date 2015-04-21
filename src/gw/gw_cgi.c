/*
 * gw_cgi.c
 *
 *  Created on: 2012-10-19
 *      Author: root
 */
#include <string.h>

#include <sysconf.h>

#include "socket_rw.h"

#include "httpd.h"
#include "httpd_debug.h"
#include "cgi_load.h"
#include "http_common.h"
#include "unistruct.h"
#include "unistruct_gfun.h"

typedef enum
{
	GW_CGI_CTX_AUTH_STATUS_INIT,
	GW_CGI_CTX_AUTH_STATUS_SUCCESS,
	GW_CGI_CTX_AUTH_STATUS_FAILED,
	GW_CGI_CTX_AUTH_STATUS_CNT
}GW_CGI_CTX_AUTH_STATUS;

typedef struct
{
	SYSCONF_t* conf;
	SYSSTATE_t* setup_info;
	GW_CGI_CTX_AUTH_STATUS auth_status;
	char* conf_type;
	char* setup_type;
	union
	{
		struct
		{
			char* user;
			char* password;
		}conf;
	}tmp;
}GW_CGI_CTX;

CompareInfo_t g_compare_info;


static int _check_auth(GW_CGI_CTX* _ctx)
{
//	HTTPD_TRACE("_ctx->tmp.auth.user=%s, _ctx->tmp.auth.password=%s", _ctx->tmp.auth.user, _ctx->tmp.auth.password);
	if(_ctx->tmp.conf.user != NULL && _ctx->tmp.conf.password != NULL)
	{
		if(strcmp(_ctx->tmp.conf.user, "admin") == 0 && strcmp(_ctx->tmp.conf.password, "") == 0)
		{
			_ctx->auth_status = GW_CGI_CTX_AUTH_STATUS_SUCCESS;
		}
		else
		{
			_ctx->auth_status = GW_CGI_CTX_AUTH_STATUS_FAILED;
		}
	}
	else
	{
		_ctx->auth_status = GW_CGI_CTX_AUTH_STATUS_INIT;
	}
//	HTTPD_TRACE("_ctx->auth_status=%d", _ctx->auth_status);
	return 0;
}



static int _handle_state_attr(UniStructAttr* _action_attr_in, UniStructAttr* _action_attr_out, GW_CGI_CTX* _ctx, char* _key)
{
//	HTTPD_TRACE("_action_attr_in->path=%s", _action_attr_in->path);
	char value[128] = {0};
	if(strcmp(_action_attr_in->path , "/juan/setup/@type") == 0)
	{
		if(_ctx->setup_type == NULL)
		{
			_ctx->setup_type = _action_attr_in->value;
			strcpy(value, _action_attr_in->value);
		}
	}
	else if(strcmp(_action_attr_in->path , "/juan/setup/@user") == 0)
	{
		if(_ctx->tmp.conf.user == NULL)
		{
			_ctx->tmp.conf.user = _action_attr_in->value;
			_check_auth(_ctx);
		}
	}
	else if(strcmp(_action_attr_in->path , "/juan/setup/@password") == 0)
	{
		if(_ctx->tmp.conf.password == NULL)
		{
			_ctx->tmp.conf.password = _action_attr_in->value;
			_check_auth(_ctx);
		}
	}
	else
	{
		if(_ctx->auth_status == GW_CGI_CTX_AUTH_STATUS_SUCCESS)
		{
			if(strcmp(_ctx->setup_type, "write") == 0)
			{
				char* left = _key;
				char* right = strstr(_key, "/@");;
				char key[128];
				int ret;
				if(left != NULL && right != NULL)
				{
					memset(key, 0, sizeof(key));
					strncpy(key, left, right - left);
					strcat(key, right + 1);
					ret = SYSSTATE_UNISTRUCT_set(_ctx->setup_info, key, _action_attr_in->value);
					if(ret != 0)
					{
						strcpy(value, "w,no attr");
					}
				}
				else
				{
					strcpy(value, "w,unknown path");
				}
			}
			else
			{
				char* left = _key;
				char* right = strstr(_key, "/@");;
				char key[128];
				int ret;
				if(left != NULL && right != NULL)
				{
					memset(key, 0, sizeof(key));
					strncpy(key, left, right - left);
					strcat(key, right + 1);
					ret = SYSSTATE_UNISTRUCT_get(_ctx->setup_info, key, value);
					if(ret != 0)
					{
						strcpy(value, "r,no attr");
					}
				}
				else
				{
					strcpy(value, "r,unknown path");
				}
			}
		}
		else
		{
			strcpy(value, "no auth");
		}
	}
	UniStruct_modify_attr2(_action_attr_out, value);

	return 0;
}


static int _handle_conf_attr(UniStructAttr* _action_attr_in, UniStructAttr* _action_attr_out, GW_CGI_CTX* _ctx, char* _key)
{
//	HTTPD_TRACE("_action_attr_in->path=%s", _action_attr_in->path);
	char value[128] = {0};
	if(strcmp(_action_attr_in->path , "/juan/conf/@type") == 0)
	{
		if(_ctx->conf_type == NULL)
		{
			_ctx->conf_type = _action_attr_in->value;
			strcpy(value, _action_attr_in->value);
		}
	}
	else if(strcmp(_action_attr_in->path , "/juan/conf/@user") == 0)
	{
		if(_ctx->tmp.conf.user == NULL)
		{
			_ctx->tmp.conf.user = _action_attr_in->value;
			_check_auth(_ctx);
		}
	}
	else if(strcmp(_action_attr_in->path , "/juan/conf/@password") == 0)
	{
		if(_ctx->tmp.conf.password == NULL)
		{
			_ctx->tmp.conf.password = _action_attr_in->value;
			_check_auth(_ctx);
		}
	}
	else
	{
		if(_ctx->auth_status == GW_CGI_CTX_AUTH_STATUS_SUCCESS)
		{
			if(strcmp(_ctx->conf_type, "write") == 0 || strcmp(_ctx->conf_type, "set") == 0 )
			{
				char* left = _key;
				char* right = strstr(_key, "/@");;
				char key[128];
				int ret;
				if(left != NULL && right != NULL)
				{
					memset(key, 0, sizeof(key));
					strncpy(key, left, right - left);
					strcat(key, right + 1);
					ret = SYSCONF_UNISTRUCT_set(_ctx->conf, key, _action_attr_in->value);
					if(ret != 0)
					{
						strcpy(value, "w,no attr");
					}
				}
				else
				{
					strcpy(value, "w,unknown path");
				}
			}
			else
			{
				char* left = _key;
				char* right = strstr(_key, "/@");;
				char key[128];
				int ret;
				if(left != NULL && right != NULL)
				{
					memset(key, 0, sizeof(key));
					strncpy(key, left, right - left);
					strcat(key, right + 1);
					ret = SYSCONF_UNISTRUCT_get(_ctx->conf, key, value);
					if(ret != 0)
					{
						strcpy(value, "r,no attr");
					}
				}
				else
				{
					strcpy(value, "r,unknown path");
				}
			}
		}
		else
		{
			strcpy(value, "no auth");
		}
	}
	UniStruct_modify_attr2(_action_attr_out, value);

	return 0;
}

static int _proc_attr(UniStructAttr* _action_attr_in, UniStructAttr* _action_attr_out, GW_CGI_CTX* _ctx)
{
	//int ret;
	char key[128];
	if(sscanf(_action_attr_in->path, "/juan/conf/%s", key) == 1)
	{
		_handle_conf_attr(_action_attr_in, _action_attr_out, _ctx, key);
	}
	else if(sscanf(_action_attr_in->path, "/juan/setup/%s", key) == 1)
	{
		_handle_state_attr(_action_attr_in, _action_attr_out, _ctx, key);
	}
	else
	{
	}


	return 0;
}

static int _proc_attrs(UniStructNode* _action_node_in, UniStructNode* _action_node_out, GW_CGI_CTX* _ctx)
{
	int i;
	int return_ret = 0;
	//int ret;

	memset(&_ctx->tmp, 0, sizeof(_ctx->tmp));

	UniStructAttr* attr_in = NULL;
	UniStructAttr* attr_out = NULL;

	int attr_count = UniStruct_get_attr_count(_action_node_in);
//	HTTPD_TRACE("_action_node_in->path=%s, attr_count=%d", _action_node_in->path, attr_count);

	if(attr_count > 0)
	{
		for(i = 0; i < attr_count; i++)
		{
			attr_in = UniStruct_get_attr_by_index(_action_node_in, i);
			HTTPD_ASSERT(attr_in != NULL, "_action_node_in->path=%s, i=%d", _action_node_in->path, i);

//			HTTPD_TRACE("attr_in->path=%s, attr_in->name=%s", attr_in->path, attr_in->name);
			attr_out = UniStruct_append_attr(_action_node_out, attr_in->name, "");
//			_key(attr_in);
			_proc_attr(attr_in, attr_out, _ctx);
		}
	}
	return return_ret;
}

static int _proc_node(UniStructNode* _action_node_in, UniStructNode* _action_node_out, GW_CGI_CTX* _ctx)
{
	int i;
//	int j;
	int return_ret = 0;

	UniStructNode* node_in = NULL;
	UniStructNode* node_out = NULL;

	int children_count = UniStruct_get_child_count(_action_node_in);
//	HTTPD_TRACE("children_count=%d", children_count);

	if(children_count > 0)
	{
		for(i = 0; i < children_count; i++)
		{
			node_in = UniStruct_get_child_by_index(_action_node_in, i);
			HTTPD_ASSERT(node_in != NULL, "_action_node_in->path=%s, i=%d", _action_node_in->path, i);

			node_out = UniStruct_append_child(_action_node_out, node_in->name, "");
			_proc_attrs(node_in, node_out, _ctx);
			_proc_node(node_in, node_out, _ctx);
		}
	}
//	HTTPD_TRACE("return_ret=%d", return_ret);
	return return_ret;
}

int cgi_gw_main(HTTPD_SESSION_t* _session)
{
	HTTPD_TRACE("%s start", __FUNCTION__);
	int return_ret = 0;

//	HTTP_dump_session(_session);
//	SOCKET_RW_CTX rw_ctx;
	GW_CGI_CTX gw_ctx;
	memset(&gw_ctx, 0, sizeof(GW_CGI_CTX));

	UniStructRoot* root_out = NULL;
	UniStructDoc* doc_out = NULL;
	UniStructRoot* root_in = NULL;
	UniStructDoc* doc_in = NULL;

	doc_out = UniStruct_init_doc("juan", "");
	root_out = UniStruct_get_root(doc_out);
	UniStruct_append_attr(root_out, "ver", "1.0");
	UniStruct_append_attr(root_out, "seq", "0");

#define FORMAT_XML (0)
#define FORMAT_JQUERY_XML (1)
#define MAX_XML_LENGTH (1024*10)
	int format = FORMAT_XML;
	char jsoncallback[64] = {0};
	char raw_xml[MAX_XML_LENGTH] = {0};
	char xml[MAX_XML_LENGTH] = {0};


	_URL_PARAM_t params[MAX_PARAM_IN_URL];
	memset(&params, 0, sizeof(_URL_PARAM_t)*MAX_PARAM_IN_URL);
	int param_count = CGI_parse_url(_session->request_line.uri_query_string, params);
	HTTPD_TRACE("nParam_count=%d", param_count);
	_URL_PARAM_t* param_f = CGI_find_param(params, MAX_PARAM_IN_URL, "f");
	_URL_PARAM_t* param_xml = CGI_find_param(params, MAX_PARAM_IN_URL, "xml");
	_URL_PARAM_t* param_jsoncallback = CGI_find_param(params, MAX_PARAM_IN_URL, "jsoncallback");
	if(param_f != NULL)
	{
		AVal av_f = AVC("j");
		if(AVCASEMATCH(&param_f->value, &av_f))
		{
			format = FORMAT_JQUERY_XML;
		}
	}
	if(param_jsoncallback != NULL)
	{
		AV2STR(param_jsoncallback->value, jsoncallback);
	}
	if(param_xml != NULL)
	{
		AV2STR(param_xml->value, raw_xml);
//		HTTPD_TRACE("raw_xml=%s", raw_xml);
//		CGI_url_decode(raw_xml, strlen(raw_xml), xml, MAX_XML_LENGTH);
		http_url_decode(raw_xml, strlen(raw_xml), xml, MAX_XML_LENGTH);
//		HTTPD_TRACE("input xml=%s", xml);
	}
	else
	{
		return -1;
	}

	doc_in=UniStruct_from_xml_string(xml);

	if(doc_in != NULL)
	{
		root_in = UniStruct_get_root(doc_in);


		
		//gw_ctx.conf = SYSCONF_dup();

		SYSCONF_t* sysconf_tmp=SYSCONF_dup();
		gw_ctx.conf = calloc(sizeof(SYSCONF_t),1);
		memcpy(gw_ctx.conf, sysconf_tmp, sizeof(SYSCONF_t));
		
		gw_ctx.setup_info = calloc(sizeof(SYSSTATE_t),1);
		memset(&g_compare_info, 0, sizeof(CompareInfo_t));
		
		int proc_ret;
		proc_ret = _proc_node(root_in, root_out, &gw_ctx);
		if(gw_ctx.conf_type != NULL){
			if(strcasecmp(gw_ctx.conf_type, "write") == 0)
			{
				memcpy(sysconf_tmp, gw_ctx.conf, sizeof(SYSCONF_t));
				SYSCONF_save(gw_ctx.conf);
			}
		}
		if(gw_ctx.setup_info){
			free(gw_ctx.setup_info);
			gw_ctx.setup_info = NULL;
		}
		if(gw_ctx.conf){
			free(gw_ctx.conf);
			gw_ctx.conf = NULL;
		}
//		HTTPD_TRACE("gw_ctx.auth_status=%d", gw_ctx.auth_status);

		switch(format)
		{
		case FORMAT_JQUERY_XML:
		{
			UniStruct_to_xml_string(root_out, xml);
			char* p_xml = xml;
			char* p_tmp_xml = raw_xml;
			while(*p_xml != 0)
			{
				if(*p_xml == '\"')
				{
					*p_tmp_xml++ = '\\';
				}
				*p_tmp_xml++ = *p_xml++;
			}
			*p_tmp_xml++ = 0;
			sprintf(xml, "%s({\"xml\":\"%s\"})", jsoncallback, raw_xml);
		}
			break;
		case FORMAT_XML:
		default:
			UniStruct_to_xml_string(root_out, xml);
			break;
		}

//		int nRet = -1;
		if (0 < strlen(xml)) {
			const char* ack_template = "HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html; charset=UTF-8\r\n"
					"Content-length: %d\r\n"
					"Connection: keep-alive\r\n"
					"\r\n"
					"%s";
			char ack_buf[1024*10]={0,};
			int ack_size=sprintf(ack_buf,ack_template,strlen(xml),xml);

			SOCKET_RW_CTX rw_ctx;
			SOCKETRW_rwinit(&rw_ctx, _session->sock, (void *)ack_buf, ack_size, _session->keep_alive);
			SOCKETRW_writen(&rw_ctx);
			if(rw_ctx.result != SOCKET_RW_RESULT_SUCCESS)
			{
				printf("send error\r\n");
			}
		}


		UniStruct_destory(doc_in);
		proc_module_reboot();
	}


exit_cgi_gw_main21:
	UniStruct_destory(doc_out);

exit_cgi_gw_main2:
	HTTPD_TRACE("%s end, ret=%d", __FUNCTION__, return_ret);
	return return_ret;
}
