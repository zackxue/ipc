
#ifndef __ESEE_CLIENT_H__
#define	__ESEE_CLIENT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>


typedef struct ESEE_IPMAP
{
	char lan[16];
	char upnp[16];
}ESEE_IPMAP_t;

typedef struct ESEE_PORTMAP
{
	in_port_t lan;
	in_port_t upnp;
}ESEE_PORTMAP_t;

typedef struct ESEE_CLIENT_ENV
{
	char dev_ucode[64];
	ESEE_IPMAP_t ip;
	ESEE_PORTMAP_t web_port;
	ESEE_PORTMAP_t data_port;
}ESEE_CLIENT_ENV_t;

typedef int (*ESEE_CLIENT_UPDATE_ENV_CALLBACK)(ESEE_CLIENT_ENV_t* env);

//
// server_domain: the esee server domain name, if NULL use default "www.msndvr.com"
// server_port: the esee server port open for protocol, if 0 use default 60101
// 
extern void ESEE_CLIENT_setup(const char* server_domain, uint16_t server_port,
	const char* sn_code, int ch_cnt, const char* vendor, const char* version);

extern int ESEE_CLIENT_Init(ESEE_CLIENT_UPDATE_ENV_CALLBACK update_env);
extern void ESEE_CLIENT_destroy();

extern int ESEE_CLIENT_update_env(ESEE_CLIENT_ENV_t env);
extern void ESEE_CLIENT_dump_env();

typedef struct ESEE_CLIENT_INFO
{
	char id[32];
	char ip4[32];
	in_port_t heartbeat_port;
	int port_cnt;
	in_port_t port[32]; // as the protocol, port[0] == web port, port[1] == date port
}ESEE_CLIENT_INFO_t;

extern int ESEE_CLIENT_get_info(ESEE_CLIENT_INFO_t* ret_info);

#ifdef __cplusplus
};
#endif


#ifdef _TEST_ESEE
int esee_client_update_env_callback(ESEE_CLIENT_ENV_t* ret_env)
{
	if(ret_env){
		// ip mapping info
		strcpy(ret_env->ip.lan, "192.168.1.45");
		// port mapping info
		// web
		ret_env->web_port.lan = ret_env->web_port.upnp = 10080;
		// data
		ret_env->data_port = ret_env->web_port;
		return 0;
	}
	return -1;
}


int main()
{
//	ESEE_CLIENT_setup("192.168.1.2", 60101, "JAH1250100001512", 1, "JUAN", "1.0.0");
	ESEE_CLIENT_setup(NULL, 60101, "JAH1250100001512", 1, "JUAN", "1.0.0");
	ESEE_CLIENT_Init(esee_client_update_env_callback);
	printf("=== Press 'q' and entern to exit!! ===\r\n");
	while('q' != getchar()){
		ESEE_CLIENT_INFO_t info;
		if(0 == ESEE_CLIENT_get_info(&info)){
			int i = 0;
			printf("id = %s\r\n", info.id);
			printf("ip4 = %s\r\n", info.ip4);
			printf("heartbeat = %u\r\n", info.heartbeat_port);
			printf("port total map: %d\r\n", info.port_cnt);
			for(i = 0; i < info.port_cnt; ++i){
				printf("port(%d) = %u\r\n", i, info.port[i]);
			}
		}
	}
	ESEE_CLIENT_destroy();
	return 0;
}
#endif //_TEST_ESEE
#endif //__ESEE_CLIENT_H__

