#ifndef __LISTEN_H_
#define __LISTEN_H_

typedef struct _tagLEventMap{
	int		(*proc)(SESSIONLISTENER_HANDLE *,LISTENER_EVENT_t ,void *,int ,void *);
	void *  pUser;
	LISTENER_EVENT_t e;
}LEventMap_t;

typedef struct _linster_t
{
	int 								sock_fd;
	int									quit;
	LEventMap_t							eMap[LEvent_Cnt];
	pthread_mutex_t     mutex;
	struct _linster_t 	*next;
}LISTEN_t;

int framecheck_send(SESSION_t *s,char *data,u_int32_t len,u_int32_t headlen);
int get_socket();
int ListenerEventCall(LISTEN_t * thiz,LISTENER_EVENT_t e,void *pData,int nDataSize);

#endif

