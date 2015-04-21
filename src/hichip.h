
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "httpd.h"

#ifndef HICHIP_H_
#define HICHIP_H_

#define HICHIP_MULTICAST_PORT (8002)
#define HICHIP_MULTICAST_NET_SEGMENT "224.0.0.0"
#define HICHIP_MULTICAST_IPADDR "239.255.255.250"

typedef const char * (*fHICHIP_DEVICE_ID)(void);
typedef const char * (*fHICHIP_DEVICE_MODEL)(void);
typedef const char * (*fHICHIP_DEVICE_NAME)(void);

typedef const char * (*fHICHIP_ETHER_LAN)(void); // for nvr
typedef const char * (*fHICHIP_ETHER_VLAN)(void); // for dnvr
typedef int (*fHICHIP_GB28181_CONF)(const void*);

typedef struct HICHIP_CONF_FUNC {
	// device info
	fHICHIP_DEVICE_ID device_id;
	fHICHIP_DEVICE_MODEL device_model;
	fHICHIP_DEVICE_NAME device_name;
	// network info
	fHICHIP_ETHER_LAN ether_lan;
	fHICHIP_ETHER_VLAN ether_vlan;
	fHICHIP_GB28181_CONF gb28181_conf;
}stHICHIP_CONF_FUNC, *lpHICHIP_CONF_FUNC;

extern int HICHIP_init(stHICHIP_CONF_FUNC conf_func);
extern void HICHIP_destroy();

extern int HICHIP_http_cgi(HTTPD_SESSION_t* session);




#endif //HICHIP_H_

