

#include "esee_protocol.h"
#include "esee_env.h"
#include "esee_debug.h"

#include "ezxml.h"
#include "generic.h"

static void esee_pack_template_generate(ezxml_t root_node, off_t node_off, int node_cnt, const char** node_names, const char** env_items)
{
	int i = 0;
	int node_index = node_off;
	// foreach node pack need
	for(i = 0; i < node_cnt; ++i){
		const char* env_item_val = ESEE_env_find_item_byname(env_items[i]);
		if(env_item_val){
			ezxml_t cur_node = ezxml_add_child(root_node, node_names[i], node_index++); // including <head> node
			ezxml_set_txt(cur_node, env_item_val);
		}else{
			assert(0);
		}
	}
}

static void esee_pack_identify_generate(ezxml_t root_node, off_t node_off)
{
	const char* nodes[] = { "sn", "channel", "vendor", "version", };
//	const char** items = nodes;
	const char* items[] = { "sn_crypto", "channel", "vendor", "version", };
	esee_pack_template_generate(root_node, node_off, sizeof(nodes) / sizeof(nodes[0]), nodes, items);
}

static void esee_pack_login_generate(ezxml_t root_node, off_t node_off)
{
	const char* nodes[] = { "id", "interip", "interport", "port", "dataport", };
//	char** items = nodes;
	const char* items[] = { "id_crypto", "interip", "interport", "port", "dataport", };
	esee_pack_template_generate(root_node, node_off, sizeof(nodes) / sizeof(nodes[0]), nodes, items);
}

static void esee_pack_heartbeat_generate(ezxml_t root_node, off_t node_off)
{
	const char* nodes[] = { "id", };
	const char** items = nodes;
	esee_pack_template_generate(root_node, node_off, sizeof(nodes) / sizeof(nodes[0]), nodes, items);
}

static char* esee_pack_create(int cmd, void* tick)
{
	ezxml_t esee_node, esee_head_node, esee_other_node;
	char* str_pack  =NULL;
	char str_val[64];

//
// <esee 1.0>
//  <head>
//   <cmd></cmd>
//   ...
//  </head>
//  <sn></sn>
//  ...
// </esee>
//
	esee_node = ezxml_new("esee 1.0");
	//esee_node = ezxml_new("esee");
	//ezxml_set_attr(esee_node, "ver", "1.0");
	{// <esee>
		esee_head_node = ezxml_add_child(esee_node, "head", 0);
		{// <head>
			
			// <cmd />
			esee_other_node = ezxml_add_child(esee_head_node, "cmd", 0);
			sprintf(str_val, "%d", cmd);
			ezxml_set_txt(esee_other_node, str_val);
			// <tick />
			esee_other_node = ezxml_add_child(esee_head_node, "tick", 1);
			ezxml_set_txt(esee_other_node, tick);
			// <pktnum />
			esee_other_node = ezxml_add_child(esee_head_node, "pktnum", 2);
			// <pktno />
			esee_other_node = ezxml_add_child(esee_head_node, "pktno", 3);
			
		}// </head
		switch(cmd)
		{
		case ESEE_CMD_REQUEST_IDENTIFY:
			esee_pack_identify_generate(esee_node, 1);
			break;
		case ESEE_CMD_REQUEST_LOGIN:
			esee_pack_login_generate(esee_node, 1);
			break;
		case ESEE_CMD_REQUEST_HEARTBEAT:
			esee_pack_heartbeat_generate(esee_node, 1);
			break;
		default:
			assert(0);
		}
	}// </esee>
	
	str_pack = ezxml_toxml(esee_node);
	// free the xml structure
	ezxml_free(esee_node);

	// FIXME:
	strcpy(strstr(str_pack, "</esee 1.0>"), "</esee>");
	ESEE_TRACE_REQUEST(str_pack);
	
	return str_pack;
}

static void esee_pack_release(char* pack)
{
	free(pack);
}

static int esee_sock_send(int sock, int cmd)
{
	static uint32_t server_ip = 0;
	if(!server_ip){
		GET_HOST_BYNAME(ESEE_env_find_item_byname("server_domain"), server_ip);
	}
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(ESEE_env_find_item_byname("server_port")));
	server_addr.sin_addr.s_addr = server_ip;
	
	if(server_addr.sin_addr.s_addr > 0){
		int ret = 0;
		char* str_pack = esee_pack_create(cmd, "0");
		ssize_t const snd_size = strlen(str_pack);
		ret = sendto(sock, str_pack, snd_size, 0, (struct sockaddr *)&(server_addr), sizeof(server_addr));
		esee_pack_release(str_pack);
		if(ret == snd_size){
			return 0;
		}
	}
	return -1;
}

ESEE_PROTOCOL_RET_t ESEE_request_identify(int sock)
{
	if(0 == esee_sock_send(sock, ESEE_CMD_REQUEST_IDENTIFY)){
		return EP_SUCCESS;
	}
	return EP_REQUEST_ERROR;
}

ESEE_PROTOCOL_RET_t ESEE_request_login(int sock)
{
	if(0 == esee_sock_send(sock, ESEE_CMD_REQUEST_LOGIN)){
		return EP_SUCCESS;
	}
	return EP_REQUEST_ERROR;
}

ESEE_PROTOCOL_RET_t ESEE_request_heartbreak(int sock)
{
    if(0 == esee_sock_send(sock, ESEE_CMD_REQUEST_HEARTBEAT)){
		return EP_SUCCESS;
	}
	return EP_REQUEST_ERROR;
}

ESEE_PROTOCOL_RET_t ESEE_response_poll(int sock, char* buf, int buf_size)
{
	int ret = recvfrom(sock, buf, buf_size, 0, NULL, NULL);
	if(ret < 0){
		if(ETIMEDOUT == errno){
			return EP_RESPONSE_TIMEOUT;
		}else if(EAGAIN == errno){
			return EP_RESPONSE_RETRY;
		}
		printf("errno = %d %s\r\n", errno, strerror(errno)); 
		return EP_UNKNOWN_ERROR;
	}else if(0 == ret){
		return EP_RESPONSE_RETRY;
	}
	return EP_SUCCESS;
}

