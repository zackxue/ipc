
#ifndef __DDNS_UTIL_H__
#define __DDNS_UTIL_H__

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

extern int DDNS_get_host(const char* domain, char* host);

#endif //__DDNS_UTIL_H__

