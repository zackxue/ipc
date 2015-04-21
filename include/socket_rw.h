
#ifndef __SOCKET_RW_H__
#define __SOCKET_RW_H__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>


typedef enum
{
	SOCKET_RW_RESULT_UNKNOWN,
	SOCKET_RW_RESULT_SUCCESS,
	SOCKET_RW_RESULT_CLIENT_DISCONNECT,
	SOCKET_RW_RESULT_ERROR,
	SOCKET_RW_RESULT_TIMEOUT,
}SOCKET_RW_RESULT;

typedef struct
{
	int socketfd;
	char* buf;
	int request_len;
	int timeout_sec;

	SOCKET_RW_RESULT result;
	int errno_cpy;
	int actual_len;
}SOCKET_RW_CTX;


extern void SOCKETRW_rwinit(SOCKET_RW_CTX* _ctx, int _socketfd, void* _buf, int _request_len, int _timeout_sec);
extern void SOCKETRW_read(SOCKET_RW_CTX* _ctx);
extern void SOCKETRW_readn(SOCKET_RW_CTX* _ctx);
extern void SOCKETRW_write(SOCKET_RW_CTX* _ctx);
extern void SOCKETRW_writen(SOCKET_RW_CTX* _ctx);

//extern int SOCKET_readn(int sock, void* buf, ssize_t sz, int timeout_s);
//extern int SOCKET_read(int sock, void* buf, ssize_t sz, int timeout_s);
//extern int SOCKET_writen(int sock, void* buf, ssize_t sz, int timeout_s);

#endif //__SOCKET_RW_H__

