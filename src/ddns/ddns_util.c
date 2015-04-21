
#include "ddns_util.h"
#include "generic.h"

int DDNS_get_host(const char* domain, char* host)
{
	struct in_addr addr;
	GET_HOST_BYNAME(domain, addr.s_addr);
	strcpy(host, inet_ntoa(addr));
	return 0;
}




