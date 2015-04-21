#ifndef __JAST_SOCK_H__
#define __JAST_SOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#define UDP_SOCK_BUF_SIZE	(128*1024)

#ifndef RET_OK
#define RET_OK	(0)
#endif
#ifndef RET_FAIL
#define RET_FAIL	(-1)
#endif

//#define QT_PLATFORM

#if (defined(_WIN32) || defined(_WIN64)) && !defined(QT_PLATFORM)
	//#include <winsock.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <windef.h>
	#include <WS2TCPIP.H>
	#include <errno.h>

	typedef SOCKET	SOCK_t;
	typedef int SOCKLEN_t; 
	typedef SOCKADDR SOCKADDR_t;
	typedef SOCKADDR_IN SOCKADDR_IN_t;
	typedef const char SOCKOPTARG_t;

	#define SOCK_close(s) \
		do{\
			closesocket(s);\
			WSACleanup();\
		}while(0)
	#define SOCK_ERR		WSAGetLastError()
	#define SOCK_EAGAIN		WSAEWOULDBLOCK
	#define SOCK_ETIMEOUT	WSAETIMEDOUT
	#define SOCK_EINTR		WSAEINTR

#else

	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <ifaddrs.h>
	#include <fcntl.h>
	#include <errno.h>

	typedef int	SOCK_t;
	typedef socklen_t SOCKLEN_t; 
	typedef struct sockaddr SOCKADDR_t;
	typedef struct sockaddr_in SOCKADDR_IN_t;
	typedef const void SOCKOPTARG_t;

	#define SOCK_close(s)	close(s)
	#define SOCK_ERR		errno

	#define SOCK_EAGAIN		EAGAIN
	#define SOCK_ETIMEOUT	ETIMEDOUT
	#define SOCK_EINTR		EINTR

#endif


#define DEFAULT_SOCK_TIMEOUT	(5)

SOCK_t SOCK_tcp_listen(int listen_port);
SOCK_t SOCK_tcp_connect(char *ip,int port,int rwtimeout); 
int SOCK_tcp_init(SOCK_t fd,int rwtimeout);
SOCK_t SOCK_udp_init(int bind_port,int rwtimeout);
int SOCK_recv(SOCK_t sock,char *buf,int iBufSize,int flag);// recv one time
int SOCK_recv2(SOCK_t sock,char *buf,int iToReadSize,int flag);// recv until reach the the size to be read or error occur
int SOCK_send(SOCK_t sock,char *buf,int size);
int SOCK_recvfrom(SOCK_t sock,char *ip,int *port,char *buf,int size);
int SOCK_sendto(SOCK_t sock,char *ip,int port,char *buf,int size); 
int SOCK_getpeername(SOCK_t sock,char *peer);
int SOCK_getsockname(SOCK_t sock,char *ip);
int SOCK_gethostname(char *ip);
int SOCK_isreservedip(char *szIp);
int SOCK_set_broadcast(int sock);

#ifdef __cplusplus
}
#endif

#endif

