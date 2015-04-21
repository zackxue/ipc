#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/queue.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <pthread.h>
#include <sys/stat.h>

#include <sys/socket.h>

#include "send.h"
#include "listen.h"
#include "check.h"
#include "rudp.h"
#include "session.h"
#include "rudpa_debug.h"

#define FRAME_TIMEOUT	5
#define SUB_TIMEOUT		2

#define DATA_OFFSET (DEFAULT_PKT_LEN + PKTINFO_HEAD_LEN + UDPH_LEN)

#define FILE_PATH		"./CIF_12fps_128kbps.h264"

static u_int32_t packet_id;

u_int32_t get_now_usec()
{
	struct timeval now;
	
	gettimeofday(&now, NULL); 
	return  now.tv_sec * 1000 + now.tv_usec / 1000;
}

#define MAJOR			1
#define MINOR			0
#define REVISON			0
#define BUILD			0
#define MAIN_VERSION 	(MAJOR << 24 & 0xff000000)
#define SUB_VERSION  	(MINOR << 16 & 0xff0000)
#define REMIND_VERSION 	(REVISON << 8  & 0xff00)
#define BUILD_VERSION  	(BUILD & 0xff )

#define RUDP_VERSION (MAIN_VERSION | SUB_VERSION | REMIND_VERSION | BUILD_VERSION)

u_int32_t gen_pkt_id()
{
	return packet_id++;
}

static void udp_pkt_header(SESSION_t *s,char *data,u_int32_t cmd)
{
	UDP_PKT_H_t 		*udata = (UDP_PKT_H_t *)data;
	udata->flag 		= 0xff9a1234;
	udata->version	= RUDP_VERSION;
	udata->session_id	= s->session_id;
	udata->tick		= s->tick++;
	udata->cmd		= cmd;
}

static u_int16_t sub_pkt_count(u_int32_t datalen,u_int32_t *lastlen)
{
	u_int16_t count;
	
	count = datalen / DEFAULT_PKT_LEN;
	if ( (*lastlen = (datalen % DEFAULT_PKT_LEN)) != 0)
		count++;
	
	return count;
}

static u_int8_t get_send_stauts(SESSION_t *s ,u_int16_t *sendpack,u_int16_t pktcount)
{
	int 	 i;
	u_int8_t ret = 0;
	
	(*sendpack) = 0;
	
	for (i = 0;i < pktcount;i++)
	{
		if (s->ack[i] == 0)
			ret = 1;
		else
			(*sendpack)++;
	}
	
	return ret;
}

#if 0
static void _recv(SESSION_t *s)
{
	struct 				timeval tv; 
	char   				buffer[2048] = {0x00};
	fd_set 				rset;
	int				n;
	socklen_t			len = sizeof(struct sockaddr);
	struct sockaddr_in 	clientaddr;
	

	tv.tv_sec =0;
	tv.tv_usec=20000;
	
	FD_ZERO(&rset);
	
	FD_SET(s->fd,&rset);

	n = select(s->fd+1,&rset,NULL,NULL,&tv);
	
	if(n < 1)
	  return;
		
	if (s->fd != -1 && FD_ISSET(s->fd,&rset))
	{
		memset(&clientaddr,0,len);
		
		if ((n = recvfrom(s->fd,buffer,2048,0,(struct sockaddr*)&clientaddr,&len)) < 0)
		    return;
		
		udp_pkt_parse(buffer, n,s->fd,&clientaddr);
	}
}
#endif
static int send_go(SESSION_t *s,u_int16_t pkt_count,u_int32_t headlen)
{
	int sock;
	int i;
	int n;
	struct sockaddr_in saddr;
	SEND_PRE_t sp;
	
	//if ((sock = get_socket(s)) == -1)
	//		return -1;
	sock = s->fd;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(s->port);
	saddr.sin_addr.s_addr = htonl(s->ip);
	//connect(sock, (struct sockaddr*)&saddr, sizeof(saddr));
	for (i = 0;i < pkt_count;i++)
	{		
		if (s->ack[i])
			continue;		
		sp.data = s->data + i*(DATA_OFFSET + headlen) + headlen;
		sp.Head = s->data + i*(DATA_OFFSET + headlen);
		sp.senddatasize = ((PKT_DATA_t*)(s->data + i*(DATA_OFFSET + headlen) + headlen + UDPH_LEN))->length
						+ headlen + PKTINFO_HEAD_LEN + UDPH_LEN;
		sp.taraddress   = saddr;

		SessionEventCall(s,Event_Send_Pre,&sp,sizeof(sp));
		/*
		 *TRACE("dst ip:%s:port%d\n",inet_ntoa(sp.taraddress.sin_addr),ntohs(sp.taraddress.sin_port));
		 */
		
		n = sendto(sock,sp.Head,sp.senddatasize,0,(struct sockaddr*)&sp.taraddress,sizeof(saddr));
		if (n < 0)
			return -1;
	}
	
	return 0;
}

static ERROR_CODE_t session_send_data(SESSION_t *s,u_int16_t pktcount,u_int32_t headlen)
{
	int  	  send = 1;
	u_int32_t now;
	u_int32_t begin;
	u_int32_t last = 0;
	u_int16_t sendpack;
	u_int16_t resend = 0;
	SEND_TIMEOUT_t st;
	ERROR_CODE_t   ret = SUCCESS;
	
	begin 	= get_now_usec();
	for(;send;)
	{
		now = get_now_usec();
		if (now - last > s->sub_pkt_interval)
		{		
			send_go(s,pktcount,headlen);			
			last = get_now_usec();
		}
		//大包超时
		if (now - begin > s->pkt_interval)
		{
			//if ((send = get_send_stauts(s,&sendpack,pktcount)) == 0)
		//	  break;
			
			st.resendcount 	= resend;
			st.sendpack		= sendpack;
			st.totalpcak		= pktcount;
			st.bcontinue		= 0;
			SessionEventCall(s,Event_Send_TimeOut,&st,sizeof(st));
			
			if (st.bcontinue)
			{
				resend ++;
				
				begin = get_now_usec();
			}
			else
			{				  
				return ETIMEDOUT2;
			}
		}
		
		//_recv(s);
		usleep(5000);		
		send = get_send_stauts(s,&sendpack,pktcount);		
	}
	
	return ret;
}

ERROR_CODE_t send_data(SESSION_HANDLE *session,char *data,u_int32_t datalen)
{
	SESSION_t 	*s = (SESSION_t*)session;
	u_int32_t  	dlen,datasize;
	u_int32_t	headlen = 0;
	u_int16_t	pktcount;
	u_int32_t 	lastlen;
	u_int32_t	pktid;
	int 		i;
	PKT_DATA_t	*pdata;
	ERROR_CODE_t ret;
	
	if (s->timeout)
		return ESENDFAILED;
	if (s->sendflag)
		return ES_SYN_WAIT;
	if (0 != SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen)))
		return EPRESEND;
	_RUDPA_DEBUG("pre_create_pack headlen:%d\n",headlen);
	
	pktcount = sub_pkt_count(datalen,&lastlen);
	
	pktid = gen_pkt_id();
	
	for (i = 0;i < pktcount ;i++)
	{
		if (i == pktcount -1 && lastlen)
		{
			datasize = lastlen;
		}
		else
			datasize = DEFAULT_PKT_LEN;
		
		dlen = datasize;
		
		//sub_pkt_alen = dlen + PKTINFO_HEAD_LEN + sizeof(UDP_PKT_H_t);
		
		udp_pkt_header(s,s->data + i*(DATA_OFFSET + headlen) + headlen,CMD_RDPKT);
		
		pdata = (PKT_DATA_t*)(s->data + i*(DATA_OFFSET + headlen) + headlen + UDPH_LEN);
		pdata->count  = pktcount;
		pdata->pkt_id = pktid;
		pdata->sub_id = i;
		pdata->length = dlen;
		memcpy(s->data + i*(DATA_OFFSET + headlen) + UDPH_LEN + PKTINFO_HEAD_LEN + headlen,data + i*DEFAULT_PKT_LEN,datasize);
	}

	s->sendflag	= 1;
	memset(s->ack,0,pktcount);
	s->pkt_id   = pktid;
	_RUDPA_DEBUG("test the ack\n");
	ret = session_send_data(s,pktcount,headlen);
	_RUDPA_DEBUG("test the ack ret:%d\n",ret);
	//send_go(s,pktcount,headlen);
	s->sendflag = 0;
	
	return ret;
}

/*send the data in the child thread, then will not block the main thread*/

static void* aside_send_callback(void *data)
{
	
	AsideSend_t * thiz = (AsideSend_t*)data;	

	_RUDPA_DEBUG("session addr:aside>>%p\n",thiz->session);
	send_data(thiz->session,thiz->data,strlen(thiz->data));
	_RUDPA_DEBUG("session id:%x\n",((SESSION_t*)thiz->session)->session_id);
	_RUDPA_DEBUG("send data okay,plz check the tcpdump whether get the pkt\n");
	free(thiz);
	thiz = NULL;
	return NULL;
}
void aside_send(void *session,void *data)
{
	AsideSend_t * asideData = (AsideSend_t*)malloc(sizeof(AsideSend_t));
	if(NULL ==asideData)
	{
		_RUDPA_ERROR("asideData malloc error\n");
		return;
	}
	asideData->session= session;
	asideData->data = (char*)data;
	_RUDPA_DEBUG("session id:%x\n",((SESSION_t*)session)->session_id);

	pthread_t th;
	if (pthread_create(&th,NULL,aside_send_callback,asideData) != 0)
	return ;				
	pthread_detach(th);
}

int SessionEventCall(SESSION_t *s,SESSEION_EVENT_t e,void *pData,int nDatasize)
{
	
	if (NULL == s)
	{
		return -1;
	}
	//_RUDPA_DEBUG("session :%p session id:%x,event:%d\n",s, s->session_id,e);
	if (e > Event_Cnt)
	{
		return -2;
	}
	if (NULL == s->eMap[e].proc)
	{
		_RUDPA_DEBUG("s->eMap[%d].proc:NULL\n",e);
		return -3;
	}
	if(NULL == s->eMap[e].pUser)
	{
		_RUDPA_DEBUG("s->eMap[%d].pUser:NULL\n",e);
		return -4;
	}
	return s->eMap[e].proc((SESSION_HANDLE *)s,e,pData,nDatasize,s->eMap[e].pUser);
}



