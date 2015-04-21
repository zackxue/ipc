
#ifndef __UPNP_UTIL_H__
#define __UPNP_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

typedef struct UPNP_CONTEXT
{
	char device_type[64];
	char location[64];
	char server_addr[32];
	in_port_t server_port;
}UPNP_CONTEXT_t;

extern void upnp_log(const char* file_name, void* xml_buf, ssize_t xml_size);

extern int upnp_discovery_server(const char* broadcast_addr, in_port_t broadcast_port,
	UPNP_CONTEXT_t* context,char *gw);
extern int upnp_http_get_location(UPNP_CONTEXT_t* context);

#ifdef __cplusplus
};
#endif
#endif //__UPNP_UTIL_H__

