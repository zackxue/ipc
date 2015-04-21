
#ifndef __ESEE_PROTOCOL_H__
#define __ESEE_PROTOCOL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

typedef enum ESEE_PROTOCOL_RET
{
	EP_UNKNOWN_ERROR = -1,
	EP_SUCCESS = 0,
	EP_REQUEST_ERROR,
	EP_RESPONSE_TIMEOUT,
	EP_RESPONSE_RETRY,
	EP_RESPONSE_ERROR,
}ESEE_PROTOCOL_RET_t;

#define ESEE_CMD_REQUEST_IDENTIFY (10000)
#define ESEE_CMD_REQUEST_LOGIN (10001)
#define ESEE_CMD_REQUEST_HEARTBEAT (10002)

#define ESEE_CMD_RESPONSE_IDENTIFY (11000)
#define ESEE_CMD_RESPONSE_LOGIN (11001)
#define ESEE_CMD_RESPONSE_HEARTBEAT (11002)

#define ESEE_CMD_RESPONSE_ERROR (11100)


extern ESEE_PROTOCOL_RET_t ESEE_request_identify(int sock);
extern ESEE_PROTOCOL_RET_t ESEE_request_login(int sock);
extern ESEE_PROTOCOL_RET_t ESEE_request_heartbreak(int sock);

extern ESEE_PROTOCOL_RET_t ESEE_response_poll(int sock, char* buf, int buf_size);

#ifdef __cplusplus
};
#endif
#endif //__ESEE_PROTOCOL_H__

