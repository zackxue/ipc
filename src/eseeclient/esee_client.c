

#include "esee_client.h"
#include "esee_env.h"
#include "esee_protocol.h"
#include "esee_crypto.h"
#include "esee_debug.h"

#include "ezxml.h"
#include "generic.h"

typedef struct ESEE_SESSION
{
	bool fsm_trigger;
	pthread_t fsm_tid;
	// callback function
	ESEE_CLIENT_UPDATE_ENV_CALLBACK upgdate_env;
}ESEE_SESSION_t;
static ESEE_SESSION_t* _esee_session = NULL;

#define UNAT_PORT_KEEPALIVE (20)

// uint: second
#define _DEPRESS_TESTING
#ifdef _DEPRESS_TESTING 
#define FSM_IDEL_POLLING (1)
#define FSM_IDENTIFY_RETRY (1)
#define FSM_IDENTIFY_TIMEOUT (2) 
#define FSM_LOGIN_RETRY (1)
#define FSM_LOGIN_TIMEOUT (2)
#define FSM_HEARTBEAT_RETRY (2)
#define FSM_HEARTBEAT_TIMEOUT (2)
#define FSM_HEARTBEAT_INTERVAL (3)
#else
#define FSM_IDEL_POLLING (300) // 5min
#define FSM_IDENTIFY_RETRY (3)
#define FSM_IDENTIFY_TIMEOUT (UNAT_PORT_KEEPALIVE) 
#define FSM_LOGIN_RETRY (3)
#define FSM_LOGIN_TIMEOUT (UNAT_PORT_KEEPALIVE)
#define FSM_HEARTBEAT_RETRY (5)
#define FSM_HEARTBEAT_TIMEOUT (UNAT_PORT_KEEPALIVE)
#define FSM_HEARTBEAT_INTERVAL (120) // 2min
#endif

static void esee_client_update_env()
{
	ESEE_CLIENT_ENV_t info;
	if(!_esee_session->upgdate_env){
		return;
	}
	if(0 == _esee_session->upgdate_env(&info)){
		ESEE_CLIENT_update_env(info);
	}	
}

static void* esee_fsm(void* arg)
{
	int ret = 0;
	ESEE_PROTOCOL_RET_t ep_ret;
    char buf[2048];

	int locked_step = 0;
	int idel_polling = FSM_IDEL_POLLING;
	int identify_retry = FSM_IDENTIFY_RETRY;
	int identify_timeout = FSM_IDENTIFY_TIMEOUT;
	int login_retry = FSM_LOGIN_RETRY;
	int login_timeout = FSM_LOGIN_TIMEOUT;
	int heartbeat_retry = FSM_HEARTBEAT_RETRY;
	int heartbeat_timeout = FSM_HEARTBEAT_TIMEOUT;
	int heartbeat_interval = FSM_HEARTBEAT_INTERVAL;
	int sock = 0;

	enum ESEE_FSM
	{
		EFSM_IDLE = 0,
		EFSM_LOCKED,
		EFSM_STARTING,
		EFSM_QUITING,
		EFSM_IDENTIFY_REQUEST,
		EFSM_IDENTIFY_RESPONSE,
		EFSM_LOGIN_REQUEST,
		EFSM_LOGIN_RESPONSE,
		EFSM_HEARTBEAT_REQUEST,
		EFSM_HEARTBEAT_RESPONSE,
		EFSM_HEARTBEAT_PREPARE,
		
	}esee_fsm_current = EFSM_STARTING;

    while(_esee_session->fsm_trigger)
	{
        switch(esee_fsm_current)
		{
		case EFSM_LOCKED:
			{
				if( 0 == locked_step++ % 60){
					ESEE_TRACE("esee client %s is locked errcmd=%s ecode=%s errinfo=\"%s\"",
						ESEE_env_find_item_byname("sn"),
						ESEE_env_find_item_byname("errcmd"),
						ESEE_env_find_item_byname("ecode"),
						ESEE_env_find_item_byname("errinfo"));
				}
			}
			break;
			
		case EFSM_IDLE:
			{
				ESEE_TRACE("esee client idle %ds", idel_polling);
				if(0 == idel_polling--){
					idel_polling = FSM_IDEL_POLLING;
					esee_fsm_current = EFSM_STARTING; // idle -> starting
				}
			}
			break;
			
		case EFSM_STARTING:
			{
				if(sock){
					close(sock);
					sock = 0;
				}else{
//					struct sockaddr_in local_addr;
//					struct timeval tvout;
					// create a new udp socket
					sock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
					assert(sock);
//					// bind
//					memset(&local_addr, 0, sizeof(local_addr));
//					local_addr.sin_family = AF_INET;
//					local_addr.sin_port = htons(0);
//					local_addr.sin_addr.s_addr = INADDR_ANY;
//					ret = bind(sock,(struct sockaddr*)&local_addr, sizeof(local_addr));
//					assert(0 == ret);
//					// set timeout
//					tvout.tv_sec = 1;
//					tvout.tv_usec = 0;
//					//setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tvout, sizeof(tvout));
//					ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tvout, sizeof(tvout));
//					assert(0 == ret);
					// set nonblock
					ret = fcntl(sock, F_SETFL, O_NONBLOCK);
					assert(0 == ret);

					// check whether have got the esee id
					if(!strlen(ESEE_env_find_item_byname("id"))){
						esee_fsm_current = EFSM_IDENTIFY_REQUEST; // starting -> identify request
					}else{
						esee_fsm_current = EFSM_LOGIN_REQUEST; // start -> login request
					}
				}
			}
			break;

		case EFSM_QUITING:
			{
				esee_fsm_current = EFSM_IDLE; // quiting -> idle
			}
			break;
			
        case EFSM_IDENTIFY_REQUEST:
			{
				ep_ret = ESEE_request_identify(sock);
				if(EP_SUCCESS == ep_ret){
					esee_fsm_current = EFSM_IDENTIFY_RESPONSE;
				}else{
					// some error occur
					esee_fsm_current = EFSM_QUITING;
				}
        	}
            break;

		case EFSM_LOGIN_REQUEST:
            {
				// update the esee environment
				esee_client_update_env();
				//
				ep_ret = ESEE_request_login(sock);
				if(EP_SUCCESS == ep_ret){
					esee_fsm_current = EFSM_LOGIN_RESPONSE;
				}else{
					// some error occur
					esee_fsm_current = EFSM_QUITING;
				}
            }
            break;

		case EFSM_HEARTBEAT_REQUEST:
			{
				ep_ret = ESEE_request_heartbreak(sock);
				if(EP_SUCCESS == ep_ret){
					esee_fsm_current = EFSM_HEARTBEAT_RESPONSE; // heartbeat request -> response
				}else{
					// some error occur
					esee_fsm_current = EFSM_QUITING;
				}
        	}
			break;

		case EFSM_IDENTIFY_RESPONSE:
		case EFSM_LOGIN_RESPONSE:
		case EFSM_HEARTBEAT_RESPONSE:
			{
				ep_ret = ESEE_response_poll(sock, buf, sizeof(buf));
				if(EP_RESPONSE_ERROR == ep_ret){
					ESEE_TRACE("response error!");
					esee_fsm_current = EFSM_IDLE;
				}else if(EP_RESPONSE_TIMEOUT == ep_ret || EP_RESPONSE_RETRY == ep_ret){
					if(EFSM_IDENTIFY_RESPONSE == esee_fsm_current){
						ESEE_TRACE("identify timeout %d!", identify_timeout);
						if(0 == identify_timeout--){
							// retry identify again
							identify_timeout = FSM_IDENTIFY_TIMEOUT;
							esee_fsm_current = EFSM_IDENTIFY_REQUEST;							
							ESEE_TRACE("identify timeout and retry %d", identify_retry);
							if(0 == identify_retry--){
								// retry identify again
								identify_retry = FSM_IDENTIFY_RETRY;
								esee_fsm_current = EFSM_QUITING; // quit
							}
						}
					}else if(EFSM_LOGIN_RESPONSE == esee_fsm_current){
						ESEE_TRACE("login timeout %d!", login_timeout);
						if(0 == login_timeout--){
							// retry login again
							login_timeout = FSM_LOGIN_TIMEOUT;
							esee_fsm_current = EFSM_LOGIN_REQUEST;							
							ESEE_TRACE("login timeout and retry %d", login_retry);
							if(0 == login_retry--){
								// retry identify again
								login_retry = FSM_LOGIN_RETRY;
								esee_fsm_current = EFSM_QUITING; // quit
							}
						}
					}else if(EFSM_HEARTBEAT_RESPONSE == esee_fsm_current){
						ESEE_TRACE("heartbeat timeout %d!", heartbeat_timeout);
						if(0 == heartbeat_timeout--){
							// retry heartbeat again
							heartbeat_timeout = FSM_HEARTBEAT_TIMEOUT;
							esee_fsm_current = EFSM_HEARTBEAT_REQUEST;							
							ESEE_TRACE("heartbeat timeout and retry %d", heartbeat_retry);
							if(0 == heartbeat_retry--){
								// retry login again
								heartbeat_retry = FSM_HEARTBEAT_RETRY;
								esee_fsm_current = EFSM_LOGIN_REQUEST; // back to login
							}
						}
					}
				}else if(EP_SUCCESS == ep_ret){
					ezxml_t const esee_node = ezxml_parse_str(buf, strlen(buf));
					ezxml_t const esee_head_node = ezxml_child(esee_node, "head");
					ezxml_t const esee_head_cmd_node = ezxml_child(esee_head_node, "cmd");
					uint32_t const const cmd_code = atoi(ezxml_txt(esee_head_cmd_node));
					ESEE_TRACE_RESPONSE("%s", ezxml_toxml(esee_node));
					
					if(ESEE_CMD_RESPONSE_ERROR == cmd_code){
//
// example
//
// <esee 1.0="">
//	<head>
//	 <cmd>11000</cmd>
//	 <tick>0</tick>
//	</head>
//	<id>100001747</id>
// </esee>
//
						ezxml_t const esee_errcmd_node = ezxml_child(esee_node, "errcmd");
						ezxml_t const esee_ecode_node = ezxml_child(esee_node, "ecode");
						ezxml_t const esee_errinfo_node = ezxml_child(esee_node, "errinfo");
						//
						ESEE_env_set_item_byname("errcmd", ezxml_txt(esee_errcmd_node));
						ESEE_env_set_item_byname("ecode", ezxml_txt(esee_ecode_node));
						ESEE_env_set_item_byname("errinfo", ezxml_txt(esee_errinfo_node));
						//
						esee_fsm_current = EFSM_LOCKED;
					}else{
						if(EFSM_IDENTIFY_RESPONSE == esee_fsm_current && ESEE_CMD_RESPONSE_IDENTIFY == cmd_code){
//
// example
//
// <esee 1.0="">
//	<head>
//	 <cmd>11100</cmd>
//	 <tick>0</tick>
//	</head>
//	<errcmd>10000</errcmd>
//	<ecode>0</ecode>
// </esee>
//

							char id_crypto[128];
							ezxml_t esee_id_node = ezxml_child(esee_node, "id");
							char* str_id = strdupa(ezxml_txt(esee_id_node));
							assert(strlen(str_id) > 0);
							ESEE_env_set_item_byname("id", str_id);
							ESEE_encrypt_id(str_id, id_crypto);
							ESEE_env_set_item_byname("id_crypto", id_crypto);
							//////////////////////////////
							ESEE_env_dump();
							esee_fsm_current = EFSM_LOGIN_REQUEST; // identify response -> login request
							
						}else if(EFSM_LOGIN_RESPONSE == esee_fsm_current && ESEE_CMD_RESPONSE_LOGIN == cmd_code){
//
// example
//
// <esee 1.0="">
//	<head>
//	 <cmd>11001</cmd>
//	 <tick>0</tick>
//	</head>
//	<id>100001747</id>
//	<pwd>(null)</pwd>
//	<exterip>192.168.1.41</exterip>
//	<port>10080</port>
// </esee>
//
							//ezxml_t const esee_id_node = ezxml_child(esee_node, "id");
							ezxml_t const esee_exterip_node = ezxml_child(esee_node, "exterip");
							// update the upnp ip
							ESEE_env_set_item_byname("exterip", ezxml_txt(esee_exterip_node));
							ESEE_env_dump();
							esee_fsm_current = EFSM_HEARTBEAT_REQUEST; // login response -> heartbeat request
							
						}else if(EFSM_HEARTBEAT_RESPONSE == esee_fsm_current && ESEE_CMD_RESPONSE_HEARTBEAT == cmd_code){
//
// example
//
// <esee 1.0="">
//  <head>
//  <cmd>11002</cmd>
//  <tick>0</tick>
//  </head>
//  <id>100001747</id>
//  <status>1</status>
//  <exterip></exterip>
// </esee>
//

							ezxml_t const esee_exterip_node = ezxml_child(esee_node, "exterip");
							const char* const heartbeat_ip = ezxml_txt(esee_exterip_node);
							ezxml_t const esee_exterport_node = ezxml_child(esee_node, "exterport");
							const char* const heartbeat_port = ezxml_txt(esee_exterport_node);
							//
							if(!STR_THE_SAME(heartbeat_port, ESEE_env_find_item_byname("exterport"))){
								ESEE_env_set_item_byname("exterport", heartbeat_port);
								ESEE_TRACE("HEARTBEAT port -> %s changed", heartbeat_port);
							}
							//
							if(!STR_THE_SAME(heartbeat_ip, ESEE_env_find_item_byname("exterip"))){
								// the WAN ip changed
								// need to re-login again
								ESEE_TRACE("WANIP has been changed -> %s esee re-login", heartbeat_ip);
								esee_fsm_current = EFSM_LOGIN_REQUEST; // -> login request
							}else{
								esee_fsm_current = EFSM_HEARTBEAT_PREPARE; // wait to next heartbeat
							}
						}else{
							char log[1024];
							sprintf(log, "echo \"FSM=%d CMD=%d\r\n%s\" >> /tmp/esee_log.txt",
								esee_fsm_current, cmd_code, buf);
							system(log);
							
							
							ESEE_TRACE("FSM=%d CMD=%d\r\n%s", esee_fsm_current, cmd_code, buf);
							// FIXME: FSM=9 CMD=0
							//assert(0);
							esee_fsm_current = EFSM_IDLE; // loop to send heart beat
						}
					}
					ezxml_free(esee_node); // dont forget this!!
				}
			}
			break;
			

		case EFSM_HEARTBEAT_PREPARE:
			{
				//if(1){
				if(heartbeat_interval % 10 == 0){
					ESEE_TRACE("esee %ds left to next heartbeat", heartbeat_interval);
				}
				if(0 == heartbeat_interval--){
					// send a new heartbeat
					ESEE_TRACE("esee heartbeat preparing");
					esee_fsm_current = EFSM_HEARTBEAT_REQUEST; // loop to send heart beat
					heartbeat_interval = FSM_HEARTBEAT_INTERVAL;
				}
			}
			break;

		default:
			break;
        }

		sleep(1);
    }

	if(sock){
		close(sock);
		sock = 0;
	}
    pthread_exit(NULL);
}

static void esee_fsm_start()
{
	if(!_esee_session->fsm_tid){
		int ret = 0;
		_esee_session->fsm_trigger = true;
		ret = pthread_create(&_esee_session->fsm_tid, NULL, esee_fsm, NULL);
		assert(0 == ret);
	}
}

static void esee_fsm_quit()
{
	if(_esee_session->fsm_tid){
		_esee_session->fsm_trigger = false;
		pthread_join(_esee_session->fsm_tid, NULL);
		_esee_session->fsm_tid = (pthread_t)NULL;
	}
}

int ESEE_CLIENT_Init(ESEE_CLIENT_UPDATE_ENV_CALLBACK upgdate_env)
{
	if(!_esee_session){
		// alloc
		_esee_session = calloc(sizeof(ESEE_SESSION_t), 1);
		// init
		_esee_session->fsm_trigger = false;
		_esee_session->fsm_tid = (pthread_t)NULL;
		_esee_session->upgdate_env = upgdate_env;
		esee_fsm_start();
	}
	return -1;
}

void ESEE_CLIENT_destroy()
{
	if(_esee_session){
		esee_fsm_quit();
		// free
		free(_esee_session);
		_esee_session = NULL;
		// release esee env
		ESEE_env_clear_item_all();
	}
}

#define ESEE_SERVER_DEFAULT_DOMAIN "www.msndvr.com"
#define ESEE_SERVER_DEFAULT_PORT (60101)

void ESEE_CLIENT_setup(const char* server_domain, uint16_t server_port,
	const char* sn_code, int ch_cnt, const char* vendor, const char* version)
{
	char sn_crypto[128];
	char str_para[32];
	// avoid invalid addresss
	if(!server_domain){
		server_domain = ESEE_SERVER_DEFAULT_DOMAIN;
	}
	if(!server_port){
		server_port = ESEE_SERVER_DEFAULT_PORT;
	}
	// encrypt ucode for protocol
	ESEE_encrypt_ucode((void*)sn_code, (void*)sn_crypto);
	// install base para
	ESEE_env_set_item_byname("server_domain", server_domain);
	sprintf(str_para, "%d", server_port);
	ESEE_env_set_item_byname("server_port", str_para);
	ESEE_env_set_item_byname("sn", sn_code);
	ESEE_env_set_item_byname("sn_crypto", sn_crypto);
	sprintf(str_para, "%d", ch_cnt);
	ESEE_env_set_item_byname("channel", str_para);
	ESEE_env_set_item_byname("vendor", vendor);
	ESEE_env_set_item_byname("version", version);
}

int ESEE_CLIENT_update_env(ESEE_CLIENT_ENV_t env)
{
	char str_para[32];
	ESEE_env_set_item_byname("interip", env.ip.lan);
	ESEE_env_set_item_byname("exterip", env.ip.upnp);
	// web port
	sprintf(str_para, "%d", env.web_port.lan);
	ESEE_env_set_item_byname("interport", str_para); // lan
	sprintf(str_para, "%d", env.web_port.upnp);
	ESEE_env_set_item_byname("port", str_para); 
	// client port
//	sprintf(str_para, "%d", env.data_port.lan);
//	ESEE_env_set_item_byname("dataport", str_para);
	sprintf(str_para, "%d", env.data_port.upnp);
	ESEE_env_set_item_byname("dataport", str_para);
	// phone port
	ESEE_env_set_item_byname("phoneport", "");
	//
	ESEE_env_set_item_byname("url", "");
	ESEE_env_set_item_byname("status", "0");
	return 0;
}

void ESEE_CLIENT_dump_env()
{
	ESEE_env_dump();
}

int ESEE_CLIENT_get_info(ESEE_CLIENT_INFO_t* ret_info)
{
	if(ret_info && strlen(ESEE_env_find_item_byname("exterport")) > 0){
		strcpy(ret_info->id, ESEE_env_find_item_byname("id"));
		strcpy(ret_info->ip4, ESEE_env_find_item_byname("exterip"));
		ret_info->heartbeat_port = atoi(ESEE_env_find_item_byname("exterport"));
		ret_info->port_cnt = 2;
		ARRAY_ZERO(ret_info->port);
		ret_info->port[0] = atoi(ESEE_env_find_item_byname("port"));
		ret_info->port[1] = atoi(ESEE_env_find_item_byname("dataport"));
		return 0;
	}else{
		ESEE_TRACE("Client not ready!");
	}
	return -1;
}



