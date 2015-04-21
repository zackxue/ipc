#ifndef _UPNP_H_
#define _UPNP_H_

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>


//upnp status
 //
enum{
 UPNP_STATE_INIT,
 UPNP_STATE_UPNP_DISCOVERY,

 UPNP_STATE_GET_LOCATION,
 UPNP_STATE_PARSE_LOCATION,
 
 UPNP_STATE_REQUEST_XML,
 UPNP_STATE_PARSE_XML,
 
 UPNP_STATE_REQUEST_WAN_IP,
 UPNP_STATE_PARSE_RESPONSE_WAN_IP,

 UPNP_SATE_REQUEST_ROUTER_STATUS,
 UPNP_STATE_PARSE_RESPONSE_ROUTER_STATUS,
 UPNP_STATE_REQUEST_ADD_PORT_MAP,
 UPNP_STATE_PARSE_RESPONSE_ADD_PORT_MAP,

 UPNP_STATE_REQUEST_QUERY_PORT,
 UPNP_STATE_PARSE_RESPONSE_QUERY_PORT,
 
 UPNP_STATE_WAIT_NEXT_TIME,
 UPNP_STATE_END
};

enum{
	PROTOCAL_TCP,
	PROTOCAL_UDP,
	PROTOCAL_TCP_UDP
};

#define EXPORT_MIN	5500
#define EXPORT_MAX	50000

#define UPNP_MAX_BUF_SIZE	(5*1024)
#define UPNP_READ_TIMEOUT	(3)
#define UPNP_UPDATE_PERIOD	(3)

typedef struct __upnp_port_map{
    uint16_t exPort;
    uint16_t inPort;
	uint16_t protocal;
    uint16_t isMapped;
}stPortMap;

#define MAX_PORT_MAP	(10)
typedef struct _upnp_para{
	char ip_me[20];
	char ip_gw[20];
	char ip_wan[20];
	uint16_t num;	
	stPortMap port_maps[MAX_PORT_MAP];
}UPNP_PARA_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////FUNCTION 

extern void UPNP_start(void *para);
extern void UPNP_stop();
extern int UPNP_external_port(uint16_t in_port,uint16_t protocal /* 0 for tcp , 1 for udp */);
extern char *UPNP_wan_ip(char *ip_str);
extern int UPNP_done();
extern int UPNP_started();
#endif
