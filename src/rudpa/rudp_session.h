#ifndef __RUDP_SESESSION_H_
#define __RUDP_SESESSION_H_
#include <netinet/in.h>
#include <stdbool.h>


typedef void SESSION_HANDLE;
typedef void SESSIONLISTENER_HANDLE;

typedef enum _enListenerEvent
{
	LEvent_New_Session,
	LEvent_Recv,

	LEvent_Cnt,
}LISTENER_EVENT_t;

typedef enum _session_event
{
    Event_Send_TimeOut,
    Event_Send_Pre,
    Event_Ld_Pack,
    Event_Pre_Create_Pack,
    Event_Close_Session,
    Event_Finished_Session,

	Event_Cnt,
}SESSEION_EVENT_t;

typedef enum _errorcode
{
	SUCCESS,
	EOUTOFRANGE,
	ECONNECTED,
	ECREATESOCKET,
	ECREATETHREAD,
	EBIND,
	EINVALIDPARAM,
	EOUTOFMEMORY,
	ESENDFAILED,
	ENOMEMORY,
	EPRESEND,
	ERECV,
	ESYNFAILED,
	ETIMEDOUT2,
	ES_SYN,
	ES_SYN_WAIT,
	ESESSIONDOWN,
}ERROR_CODE_t;

// Event_Send_TimeOut
typedef struct _send_timeout
{
   unsigned int   totalpcak;
   unsigned int   sendpack;
   unsigned int   resendcount;
   char           bcontinue;
}SEND_TIMEOUT_t;

// Event_Send_Pre
typedef struct _send_pre
{
	void      			*Head;
	void      			*data;
    struct sockaddr_in  taraddress;
	unsigned int 		senddatasize;
}SEND_PRE_t;

// Event_Ld_Pack
typedef struct _ld_pack_data
{
void       	*data;
	unsigned int   	packsize;
}LD_PACK_DATA_t;

// Event_Pre_Create_Pack
typedef struct _pre_create_pack
{
    unsigned int pre_buf_size;
}PRE_CREATE_PACK;

typedef struct _new_session
{
    SESSION_HANDLE *session;
    int		   radom;
}NEW_SESSION_t;

//Event_Finished_Session
typedef struct _finished_session
{
    int finished;
}FINISHED_SESSION_t;

typedef struct _recv
{
	char *            realbuf;
	unsigned int      bufsize;
	struct sockaddr * from;
	int               fromlen;
	char *            subbuff;
	unsigned int      recvlen;
	unsigned char     produced;
}RECV_t;


typedef int (*EVENTPROC)(SESSION_HANDLE *s,SESSEION_EVENT_t e,void *data,int datasize,void *userdata);
typedef int (*LEVENTPROC)(SESSIONLISTENER_HANDLE *l,SESSEION_EVENT_t e,void *data,int datasize,void *userdata);

SESSIONLISTENER_HANDLE  *create_session_listener(char *ipaddress,unsigned short port,EVENTPROC proc,void *userdata);

ERROR_CODE_t            set_listener_event_proc(SESSIONLISTENER_HANDLE *listener,LISTENER_EVENT_t e,LEVENTPROC proc,void *userdata);

ERROR_CODE_t			direct_send(SESSIONLISTENER_HANDLE *listener, char *data,unsigned int datasize,struct sockaddr_in *addr);

ERROR_CODE_t			set_event_proc(SESSION_HANDLE *session, SESSEION_EVENT_t e,EVENTPROC proc,void *userdata);

void					session_timeout(SESSION_HANDLE *session, unsigned int timeout);

void					session_send_timeout(SESSION_HANDLE *session, unsigned int timeout);

ERROR_CODE_t			send_data(SESSION_HANDLE *session, char *data,unsigned int datasize);

void					session_sub_pack_timeout(SESSION_HANDLE *session, unsigned int timeout);

void					session_sub_pack_send_timeout(SESSION_HANDLE *session, unsigned int timeout);

void					session_close(SESSION_HANDLE *session);

void 					session_linster_close(SESSIONLISTENER_HANDLE *listener);

#endif
