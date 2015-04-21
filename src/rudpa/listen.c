#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/file.h>

#include "send.h"
#include "check.h"
#include "listen.h"
#include "rudp.h"
#include "session.h"
#include "rudpa_debug.h"

#define 	SERVER_PORT	10201
#define 	DATA_PORT	9400
#define 	BUF_SIZE	4096

#define 	socketlen 	sizeof(struct sockaddr)


static LISTEN_t *linsters;


static inline int _linster_lock(LISTEN_t *l)
{
    int ret;
	
	while((ret = pthread_mutex_lock(&l->mutex)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _linster_unlock(LISTEN_t *l)
{
    int ret;
	
	while((ret = pthread_mutex_unlock(&l->mutex)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static int linster_insert(LISTEN_t *l)
{
		LISTEN_t *ol;
		
		if (!linsters)
		{
				linsters = l;
				return 0;
		}
		
		ol = linsters;
		
		while(ol->next)
			ol = ol->next;
			
		ol->next = l;
			
		return 0;
}

static void linster_destroy(LISTEN_t *l)
{
		LISTEN_t *b,*s;
	
		b = s = linsters;
		while(s)
		{
			if (s == l)
				break;
			
			b =	s;
			s = s->next;
		}
		if (b == s)
		{
			linsters = b->next;
			free(s);			
			return;
		}
		
		b->next = s->next;
		free(s);
}

LISTEN_t *get_linster(int sockfd)
{
	 LISTEN_t *l = NULL;
	 


	 l = linsters;
	 while(l)
	 {
	    if (_linster_lock(l) != 0)
	      return NULL;
	 if (l->sock_fd == sockfd)
	 { 	
	    _linster_unlock(l);
	    break;
	 }
	    l = l->next;
	  _linster_unlock(l);
	 
	 }
	 return l;
}


int sock_create(char *ip,u_int16_t port)
{
	struct sockaddr_in servaddr;
	int sock_fd;
	int 	r,on;
	int 	flags;
	int	n; 
	socklen_t	optlen;
	
	if ((sock_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
		return -1;
	
	on = 1;
	
	if ((r = setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))) == -1)
		goto err;
	
	optlen = sizeof(n);
	if (getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &n, &optlen) < 0)
	    printf("SO_RCVBUF getsockopt error");
	n = 4*n;
	printf("send buf=%d\n",n);
	if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &n, optlen)< 0)
	    printf("set recv buf err\n");
	
	if ((flags = fcntl(sock_fd,F_GETFL,0)) < 0)
		goto err;
	
	if (fcntl(sock_fd,F_SETFL,flags | O_NONBLOCK) < 0)
		goto err;

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	if (port == 0)
		port = SERVER_PORT;
	servaddr.sin_port=htons(0);//use the port which linux offer dynamic
	inet_pton(AF_INET,ip?ip:"0.0.0.0",&servaddr.sin_addr);
#if 0	
	sip.s_addr = htonl(gp->ip);
	snprintf(ip,16,"%s",inet_ntoa(*(struct in_addr*)&sip));
	inet_pton(AF_INET,ip[0]?ip:"0.0.0.0",&servaddr.sin_addr);
#endif

	if ((r = bind(sock_fd,(struct sockaddr*)&servaddr,sizeof(servaddr))) == -1)
		goto err;
	
	return sock_fd;
err:
	close(sock_fd);
	return -1;
	
}

int framecheck_send(SESSION_t *s,char *data,u_int32_t len,u_int32_t headlen)
{
	int 				n;
	LISTEN_t 			*l;
	SEND_PRE_t 			sp;
	struct sockaddr_in 	clientaddr;
	
	memset(&clientaddr,0,sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_port=htons(s->port);
	clientaddr.sin_addr.s_addr = htonl(s->ip);
	
	sp.data = data + headlen;
	sp.Head = data;
	sp.senddatasize = len;
	sp.taraddress   = clientaddr;

	if ((l = get_linster(s->fd)) == NULL || l->sock_fd == -1)
			return -1;
	
	SessionEventCall(s,Event_Send_Pre,&sp,sizeof(sp));
	
	if ((n = sendto(l->sock_fd,(void*)sp.Head,sp.senddatasize,0,(struct sockaddr*)&sp.taraddress,socketlen)) < 0)
		return -1;

	return 0;
}

static void *session_listen(void *proc)
{
	struct 				timeval tv; 
	char   				buffer[BUF_SIZE] = {0x00};
	fd_set 				rset;
	int					n;
	socklen_t			len = sizeof(struct sockaddr);
	LISTEN_t			*l = (LISTEN_t*)proc;
	struct sockaddr_in 	clientaddr;
	
	for(;;)
	{
		tv.tv_sec =2;
		tv.tv_usec=0;
		
		FD_ZERO(&rset);
		
		FD_SET(l->sock_fd,&rset);
		
		if (l->quit)
			break;
		n = select(l->sock_fd+1,&rset,NULL,NULL,&tv);
		
		if(n == 0)
		  continue;
		else if (n == -1)
		{
		    if (errno == EINTR || errno == EAGAIN)
				continue;
			
			break;
		}
		
		if (l->sock_fd != -1 && FD_ISSET(l->sock_fd,&rset))
		{
			memset(&clientaddr,0,len);
			memset(buffer,0,BUF_SIZE);

			if ((n = recvfrom(l->sock_fd,buffer,BUF_SIZE,0,(struct sockaddr*)&clientaddr,&len)) < 0)
				continue;

			RECV_t recv;
			recv.realbuf = buffer;
			recv.subbuff = buffer;
			recv.recvlen = n;
			recv.bufsize = BUF_SIZE;
			recv.from = (struct sockaddr *)&clientaddr;
			recv.fromlen = len;
			recv.produced = 0;		

			ListenerEventCall(l,LEvent_Recv,&recv,sizeof(recv));

			if (!recv.produced)
			{
				udp_pkt_parse(l,recv.subbuff,recv.recvlen,l->sock_fd,&clientaddr);
			}
		}
	}
	
	if (_linster_lock(l) != 0)
		return NULL;
	
	linster_destroy(l);
	_linster_unlock(l);		

	return NULL;
}

ERROR_CODE_t set_listener_event_proc(SESSIONLISTENER_HANDLE *listener,LISTENER_EVENT_t e,LEVENTPROC proc,void *userdata)
{
	if (NULL == listener)
	{
		return EINVALIDPARAM;
	}
	LISTEN_t * l = (LISTEN_t *)listener;
	l->eMap[e].proc = proc;
	l->eMap[e].e = e;
	l->eMap[e].pUser = userdata;

	return SUCCESS;
}

SESSIONLISTENER_HANDLE *create_session_listener(char *sip,unsigned short port,int  (*proc)(SESSION_HANDLE *,SESSEION_EVENT_t ,void *,int ,void *),void *pser)
{
	pthread_t		th;
	int sockfd;
	LISTEN_t *l;
	
	if ((sockfd = sock_create(sip,port)) == -1)
		return NULL;
	
	if (session_init() != 0)
		return NULL;
#if 0	
	if( check_init() != 0)
		return NULL;
#endif


	if ( (l = (LISTEN_t*)calloc(1,sizeof(LISTEN_t))) == NULL)
			return NULL;
	l->quit = 0;
	l->sock_fd = sockfd;
	l->eMap[LEvent_New_Session].e = LEvent_New_Session;
	l->eMap[LEvent_New_Session].proc = proc;
	l->eMap[LEvent_New_Session].pUser = pser;
	
	DBG("***here is create_session_listener***\n\n");
	DBG("proc id:%p\n",proc);
	
	pthread_mutex_init(&l->mutex,NULL);
	if (pthread_create(&th,NULL,session_listen,(void*)l) != 0)
	{
		free(l);
		l = NULL;
		return NULL;
	}
	
	pthread_detach(th);
	
	//l->th = th;
	linster_insert(l);
	return (void*)l;
}

int get_socket(SESSION_t *s)
{
	LISTEN_t *l;
	if ( (l = get_linster(s->fd)) ==NULL)
		 return -1;
		 
	return l->sock_fd;
}

int ListenerEventCall(LISTEN_t * thiz,LISTENER_EVENT_t e,void *pData,int nDataSize)
{
	if (NULL == thiz)
	{
		return -1;
	}
	if (e >= LEvent_Cnt)
	{
		return -2;
	}
	if (NULL == thiz->eMap[e].proc)
	{
		return -3;
	}
	DBG("***here is ListenerEventCall***\n\n");
	DBG("listen_t addr:%p,sesseion event:%d\n", thiz,e);
	DBG("proc id:%p\n",thiz->eMap[e].proc);
	return thiz->eMap[e].proc(thiz,e,pData,nDataSize,thiz->eMap[e].pUser);
}

void session_linster_close(SESSIONLISTENER_HANDLE *listener)
{
	LISTEN_t *l = (LISTEN_t*)listener;
	
	if (_linster_lock(l) != 0)
		return;
	
	l->quit = 1;
	close(l->sock_fd);
	_linster_unlock(l);

}


ERROR_CODE_t direct_send(SESSIONLISTENER_HANDLE *listener, char *data,unsigned int datasize,struct sockaddr_in *addr)
{
    int 			n;
    LISTEN_t 			*l = (LISTEN_t*)listener;
    
    if ( l->sock_fd == -1)
	return ESENDFAILED;
			
    if ((n = sendto(l->sock_fd,(void*)data,datasize,0,(struct sockaddr*)addr,socketlen)) < 0)
	return ESENDFAILED;

    return SUCCESS;
}
