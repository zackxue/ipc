

#ifndef __IFCONF_H__
#define __IFCONF_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <ctype.h>

typedef struct ifconf_ipv4_addr
{
	union
	{
		struct in_addr in_addr;
		struct 
		{
			uint8_t s_b[4];
		};
		struct
		{
			uint8_t s_b1, s_b2, s_b3, s_b4; 
		};
		struct
		{
			uint16_t s_w1, s_w2;
		};
		uint32_t s_addr;
	};
}ifconf_ipv4_addr_t;

typedef struct ifconf_hw_addr
{
	union
	{
		struct 
		{
			uint8_t s_b[6];
		};
		struct
		{
			uint8_t s_b1, s_b2, s_b3, s_b4, s_b5, s_b6;
		};
		uint64_t s_addr;
	};
}ifconf_hw_addr_t;

typedef struct ifconf_route
{
	ifconf_ipv4_addr_t dest;
	ifconf_ipv4_addr_t netmask;
	ifconf_ipv4_addr_t gateway;
}ifconf_route_t;

typedef struct ifconf_interface
{
	ifconf_ipv4_addr_t ipaddr;
	ifconf_ipv4_addr_t netmask;
	ifconf_ipv4_addr_t gateway;
	ifconf_ipv4_addr_t broadcast;
	ifconf_hw_addr_t hwaddr;
	uint32_t mtu;
	uint32_t is_up; // no use
}ifconf_interface_t;

typedef struct ifconf_dns
{
	ifconf_ipv4_addr_t preferred;
	ifconf_ipv4_addr_t alternate;
}ifconf_dns_t;

extern char* ifconf_ipv4_ntoa(ifconf_ipv4_addr_t addr);
extern ifconf_ipv4_addr_t ifconf_ipv4_aton(const char* addr);
extern char* ifconf_hw_ntoa(ifconf_hw_addr_t addr);
extern ifconf_hw_addr_t ifconf_hw_aton(const char* addr);

extern int ifconf_add_route(const char* rt_name, ifconf_route_t* if_route);
extern int ifconf_delete_route(const char* rt_name, ifconf_route_t* if_route);
extern int ifconf_set_route(const char* rt_name, ifconf_route_t* if_route);
extern int ifconf_get_route(const char* rt_name, ifconf_route_t* if_route);

extern int ifconf_get_interface(const char if_name[IFNAMSIZ], ifconf_interface_t* ifr);
extern int ifconf_set_interface(const char if_name[IFNAMSIZ], ifconf_interface_t* ifr);

extern int ifconf_get_dns(ifconf_dns_t* ret_dns);
extern int ifconf_set_dns(ifconf_dns_t* dns);

#endif //__IFCONF_H__

