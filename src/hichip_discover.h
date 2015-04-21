

#include "hichip.h"
#include "http_util.h"

#ifndef HICHIP_DISCOVER_H_
#define HICHIP_DISCOVER_H_

extern struct sockaddr_in HICHIP_DISCOVER_multicast_addr();

extern int HICHIP_DISCOVER_sock_create(const char *local_ip);
extern void HICHIP_DISCOVER_sock_release(int sock);

extern int HICHIP_DISCOVER_process_search(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content);
extern int HICHIP_DISCOVER_process_cmd(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content);
extern int HICHIP_DISCOVER_process_gb28181(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content);

#endif //HICHIP_DISCOVER_H_

