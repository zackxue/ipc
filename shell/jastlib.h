#ifndef __JASTLIB_H__
#define __JASTLIB_H__

#ifdef __cplusplus
extern "C" {
#endif	

#include "jastdef.h"
#include "authentication.h"


#define JAST_DISCOVERY_CPORT	(25666)
#define JAST_DISCOVERY_SPORT	(25665)
#define JAST_CPORT_START		(25700)
#define JAST_SPORT_START		(26667)

#define JAST_DEFAULT_REALM		"jast 1.0"
#define JAST_DEFAULT_USER		"admin"
#define JAST_DEFAULT_PWD		"12345"
#define JAST_DEFAULT_PAYLOAD_SIZE	(1024*4)
#define JAST_VERSION				"JAST/1.0"
#define JAST_MULTICAST_ADDR			"239.10.11.12"

#define JAST_HEARTBREAK_TIME	(10*1000)	// UNIT:MS
#define JAST_SOCK_TIMEOUT		(5*1000)	// UNIT:MS

typedef enum
{
	JAST_METHOD_HANDSHAKE = 0,
	JAST_METHOD_HEARTBREAK,
	JAST_METHOD_STDIN,
	JAST_METHOD_STDOUT,
	JAST_METHOD_INVOKE,
	JAST_METHOD_BYE,
	JAST_METHOD_CNT
}JastMethod_t;

// response status code
#define JAST_RSC_OK						200
#define JAST_RSC_UNAUTHORIZED			401
#define JAST_RSC_METHOD_NOTALLOWED		405
#define JAST_RSC_INTERNEL_SERVER_ERR	500

// response status string
#define JAST_RSS_OK						"OK"
#define JAST_RSS_UNAUTHORIZED			"Unauthorized"
#define JAST_RSS_METHOD_NOTALLOWED		"Method Not Allowed"
#define JAST_RSS_INTERNEL_SERVER_ERR	"Internal Server Error"


#define JAST_ROLE_SERVER (0)
#define JAST_ROLE_CLIENT (1)

typedef struct _jastdevice
{
	char ip[20];
	int port;
}JastDevice_t;

typedef struct __jast
{
	int role;
	int seq;
	int trigger;

	// transport
	char ip_me[20];
	char ip_dst[20];
	//unsigned short search_port_me;
	//unsigned short search_port_dst;
	unsigned short port_me;
	unsigned short port_dst;
	//int search_sock;
	int sock;
	
	Authentication_t *auth;	
	int bLogin;		// login success or not
	char user[32];
	char pwd[32];

	//hearbreak timer
	MillisecondTimer_t hb_timer;
	//
	int remotelog;
	
	// payload
	char payload[JAST_DEFAULT_PAYLOAD_SIZE];
	int payload_size;
}Jast_t;

Jast_t *JAST_server_init(char *dst_ip,int port_mine);
Jast_t *JAST_client_init(char *dst_ip,int dst_port,int port_mine);
int JAST_destroy(Jast_t *j);
//
int JAST_request_discovery(int sock,JastDevice_t *devlist);
int JAST_handle_discovery(int sock,char *dst_ip,int dst_port,char *mine_ip,int mine_port);
//
int JAST_handle_handshake(Jast_t *j);
int JAST_handle_bye(Jast_t *j);
int JAST_handle_stdout(Jast_t *j);
int JAST_handle_stdin(Jast_t *j);
int JAST_handle_invoke(Jast_t *j);
//
int JAST_send_heartbreak(Jast_t *j);
int JAST_send_stdout(Jast_t *j,char *msg,int len);
//
int JAST_request_handshake(Jast_t *j);
int JAST_request_bye(Jast_t *j);
int JAST_request_stdout(Jast_t *j);
int JAST_request_stdin(Jast_t *j,char *input);
int JAST_request_invoke(Jast_t *j,char *command);
//
int JAST_parse_request(Jast_t *j,char *ip,int port);
int JAST_read_message(Jast_t *j,char *ip,int *port);
int JAST_parse_response(Jast_t *j,int *code);


#ifdef __cplusplus
}
#endif
#endif

