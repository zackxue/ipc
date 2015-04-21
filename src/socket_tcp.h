

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#ifndef SOCKET_TCP_H_
#define SOCKET_TCP_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SOCKET_TCP
{
	int sock; // the file description;

	int (*close)(struct SOCKET_TCP *const tcp);

	int (*connect)(struct SOCKET_TCP *const tcp, const char* serv_addr, in_port_t serv_port);
	int (*listen)(struct SOCKET_TCP *const tcp, in_port_t port, int backlog);

	// interfaces
	int (*send)(struct SOCKET_TCP *const tcp, const void *msg, size_t len, int flags);
	int (*send2)(struct SOCKET_TCP *const tcp, const void *msg, size_t len);
	
	int (*send_timeout)(struct SOCKET_TCP *const tcp, const void *msg, size_t len, time_t timeout_s);

	ssize_t (*recv)(struct SOCKET_TCP *const tcp, void *buf, size_t len, int flags);
	ssize_t (*recv2)(struct SOCKET_TCP *const tcp, void *buf, size_t size);
	
	ssize_t (*peek)(struct SOCKET_TCP *const tcp, void *buf, size_t len);

	int (*getsockname)(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen);
	int (*getpeername)(struct SOCKET_TCP *const tcp, struct sockaddr *name, socklen_t *namelen);

	// option
	int (*set_send_timeout)(struct SOCKET_TCP *const tcp, time_t sec, suseconds_t usec);
	int (*set_recv_timeout)(struct SOCKET_TCP *const tcp, time_t sec, suseconds_t usec);

	int (*set_reuser_addr)(struct SOCKET_TCP *const tcp, bool flag);
	
}stSOCKET_TCP, *lpSOCKET_TCP;

extern lpSOCKET_TCP socket_tcp_r(lpSOCKET_TCP result);
extern lpSOCKET_TCP socket_tcp2_r(int sock, lpSOCKET_TCP result);



#ifdef __cplusplus
};
#endif
#endif //SOCKET_TCP_H_

