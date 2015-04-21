
#ifndef __JSOCKET_H__
#define __JSOCKET_H__

#include <assert.h>
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

extern ssize_t jsock_send(int fd, const void* msg, size_t len);

#endif //__JSOCKET_H__

