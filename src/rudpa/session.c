#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <sys/timeb.h>
#include <time.h>

#include <pthread.h>
#include <sys/stat.h>

#include "send.h"
#include "check.h"
#include "listen.h"
#include "rudp.h"
#include "session.h"
#include "rudpa_debug.h"


#define STIMEOUT	30
#define RECV_TIMEOUT	15

static int sessionid;

static SESSION_t *sessionl;

void session_clean(SESSION_t *s)
{
	s->timeout = 1;
	
	
	if (s->recv)
		receive_data_free(s->recv);
	
	free(s);
}

SESSION_t *get_session(int sid)
{
	SESSION_t *s;
	
	s = sessionl;
	while(s)
	{
		if (s->session_id == sid)
		{
			if (s->timeout)
				return NULL;
			
			s->time	= time(NULL);
			
			return s;
		}
		
		s = s->next;
	}
	
	return s;
}

SESSION_t *query_old_session(u_int32_t radom)
{
    SESSION_t *s;
	
	s = sessionl;
	while(s)
	{
		if (s->radom == radom)
		      return s;
		
		s = s->next;
	}
	
	return s;
}

SESSION_t *create_session(int sid)
{
	SESSION_t *s,*p;

	if (!sessionl)
	{
		if ((s = (SESSION_t*)calloc(1,sizeof(SESSION_t))) == NULL)
			return NULL;
		
		sessionid 	= rand();
		s->session_id	= sessionid;
		s->time		= time(NULL);
		s->cur_pktid = 0;
		sessionl 	= s;
#if 0	
		session_create_send_thread(s);
	
		pthread_cond_init(&s->cond,NULL);
		pthread_mutex_init(&s->mutex,NULL);
		s->quit  = 2;
#endif		
		return s;
	}
	
	if (sid == 0xffffffff)
	{
		if ((s = (SESSION_t*)calloc(1,sizeof(SESSION_t))) == NULL)
			return NULL;
		
		sessionid++;
		s->session_id		= sessionid;
		s->time					= time(NULL);
		p = sessionl;
		
		while(p->next)
				p = p->next;
		p->next = s;
#if 0
		session_create_send_thread(s);
		pthread_cond_init(&s->cond,NULL);
		(&s->mutex,NULL);
		s->quit  = 2;
#endif
	
		return s;
	}

	
	return get_session(sid);
}

SESSION_t *session_begin(LISTEN_t * thiz,UDP_PKT_H_t *uph,u_int32_t len,int sock_fd,u_int32_t ip,u_int16_t port)
{
	SESSION_t *s;
	u_int32_t radom;
	_RUDPA_DEBUG("session_begin>>in   \n");	
	
	
	if (len != UDPH_LEN + sizeof(u_int32_t))
		return NULL;
	
	radom = *(u_int32_t*)uph->data;
	if ((s = query_old_session((radom))) == NULL)
	{
	    if ((s = create_session(uph->session_id)) == NULL)
			return NULL;
		

		NEW_SESSION_t ns;
		ns.session = s;
		ns.radom   = radom;
		
		ListenerEventCall(thiz,LEvent_New_Session,(void *)&ns,sizeof(ns));
		_RUDPA_DEBUG("session:%x\r\n",s);

		_RUDPA_TRACE("\033[31m new session id=====0x%x\r\n\033[0m",s->session_id);
	}
	
	s->fd	= sock_fd;
	s->ip 	= ip;
	s->port	= port;
	s->tick	= uph->tick;
	s->radom = radom;
	_RUDPA_DEBUG("session_begin>>out\n");	
	return s;
}


int session_quit(SESSION_t *s)
{
	if (_send_lock(s) != 0)
		return -1;
		s->timeout = 1;
	SessionEventCall(s,Event_Close_Session,NULL,0);
	_send_cond_singal(s);
	_send_unlock(s);
	
	return 0;
}

static int session_close_ack(SESSION_t *s)
{
    return udp_session_close_ack(s);
}

int _session_close(UDP_PKT_H_t *uph)
{
	SESSION_t 	*s = sessionl;

//	_RUDPA_TRACE("session dump here ID:%x\n",s->session_id);
/*what the fuck r u doing , if the sessionl is NULL,the code goes wrong*/
	_RUDPA_TRACE("session dump here ID:%x\n",uph->session_id);
	
	while(s)
	{
		if (s->session_id == uph->session_id)
		{
			//s->timeout = 1;
			session_close_ack(s);
			session_quit(s);
			return 0;
		}
		s = s->next;
	}
	return -2;
}


static int check_recv_pkt_timeout(SESSION_t *s,time_t t)
{
	if (s->recv && s->timeout && s->recv->time + RECV_TIMEOUT < t)
	{
		receive_data_free(s->recv);
		s->recv = NULL;
		return 1;
	}
	return 0;
}

int heart_beat_send(SESSION_t *s,time_t t)
{

	if ( !s->timeout)
		return udp_heart_beat(s);
	
	return 0;
}

static void session_pkt_timeout_check(SESSION_t *s,time_t t)
{
#if 0
//TODO
	if (s->dinfo)
		check_send_pkt_timeout(s,t);
#endif	
	if (s->recv)
		check_recv_pkt_timeout(s,t);

}

static void *session_timeout_thread(void *args)
{
	SESSION_t *s,*b;
	time_t	  t;
	struct 	timeval 	tv; 
	int			n;
	FINISHED_SESSION_t	fs;
	
	for (;;)
	{
		tv.tv_sec  = 2;
		tv.tv_usec = 0;

		n = select(0,NULL,NULL,NULL,&tv);
		t = time(NULL);
		if (n == -1)
		{
			if (errno == EINTR || errno == EAGAIN)
				continue;
			
			break;
		}
		
		b = s = sessionl;
		while(s)
		{
			
			session_pkt_timeout_check(s,t);
			if (s->timeout && !s->recv)
			{
				//TRACE("here we go session free: \n",);
				fs.finished = 0;
				if (0 == SessionEventCall(s,Event_Finished_Session,&fs,sizeof(fs)))
				{
				    if (fs.finished)
				    {
						if (b == s)
							sessionl = s->next;
						else
							b->next = s->next;
						TRACE("session free: 0x%x, \n",s->session_id);
						free(s);		
						s = NULL;
				    }
				    
				    continue;
				}
				
				if (b == s)
					sessionl = s->next;
				else
					b->next = s->next;
				
				TRACE("session free: 0x%x, \n",s->session_id);
				free(s);		
				s = NULL;
				continue;
			}
			
			if (s->time + STIMEOUT < t || s->close > 2)
			{ 
				if (s->timeout == 0)
				{
					session_quit(s);
					printf("session ready close:%u\n",s->session_id);
					
				}
			}
			else if (t - s->hbt > 10)
			{
			    heart_beat_send(s,t);
			    s->hbt = t;
			}
			
			if (s->close && s->timeout == 0)
			{
			    udp_session_close(s);
			    s->close++;
			}
			
			b = s;
			s = s->next;
		}
	}
	perror("session timeout thread exit!");
	return NULL;
}

int session_init()
{
	pthread_t th;
	
	if (pthread_create(&th,NULL,session_timeout_thread,NULL) != 0)
		return -1;
	
	pthread_detach(th);
	
	return 0;
}

ERROR_CODE_t set_event_proc(SESSION_HANDLE *session, SESSEION_EVENT_t e,EVENTPROC proc,void *priv)
{
	if(NULL == session || e >= Event_Cnt)
	{
		return EINVALIDPARAM;
	}
	SESSION_t *s = (SESSION_t*)session;

	s->eMap[e].e = e;
	s->eMap[e].proc = proc;
	s->eMap[e].pUser = priv;
	
	return SUCCESS;
}

void session_send_timeout(SESSION_HANDLE *session, u_int32_t timeout_interval)
{
	SESSION_t *s = (SESSION_t*)session;
	
	s->pkt_interval = timeout_interval;
}

void session_sub_pack_timeout(SESSION_HANDLE *session, u_int32_t timeout_interval)
{
	SESSION_t *s = (SESSION_t*)session;
	
	s->sub_interval = timeout_interval;
}

#if 0
ERROR_CODE_t send_data(SESSION_HANDLE session, char *data,unsigned int dsize)
{
	SESSION_t *s = (SESSION_t*)session;
	
}
#endif

void session_sub_pack_send_timeout(SESSION_HANDLE *session, u_int32_t timeout_interval)
{
	SESSION_t *s = (SESSION_t*)session;
	
	s->sub_pkt_interval = timeout_interval;
}

void session_close(SESSION_HANDLE *session)
{
	SESSION_t *s = (SESSION_t*)session;
	
	udp_session_close(s);
	s->close = 1;
}
