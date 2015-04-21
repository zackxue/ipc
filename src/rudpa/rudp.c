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
#include <sys/socket.h>

#include "send.h"
#include "check.h"
#include "listen.h"
#include "rudp.h"
#include "session.h"
#include "rudpa_debug.h"

#define UINT32LEN		sizeof(u_int32_t)
#define RECV_TIMEOUT	15

#define MAJOR			1
#define MINOR			0
#define REVISON			0
#define BUILD			0
#define MAIN_VERSION 	(MAJOR << 24 & 0xff000000)
#define SUB_VERSION  	(MINOR << 16 & 0xff0000)
#define REMIND_VERSION 	(REVISON << 8  & 0xff00)
#define BUILD_VERSION  	(BUILD & 0xff )

#define RUDP_VERSION (MAIN_VERSION | SUB_VERSION | REMIND_VERSION | BUILD_VERSION)

static void udp_pkt_header(SESSION_t *s,UDP_PKT_H_t *uph,u_int32_t cmd)
{
	uph->flag 		= 0xff9a1234;
	uph->version	= RUDP_VERSION;
	uph->session_id	= s->session_id;
	uph->tick		= s->tick;
	uph->cmd		= cmd;
}


void receive_data_free(RECV_RDP_t *r)
{	
	if (r->subpkt)
	{
		RCV_PKT_t *sp,*sn;
		sn = r->subpkt;

		while(sn)
		{
			sp = sn->next;
			if (sn->data)
				free(sn->data);
			free(sn);
			sn = sp;
		}
	}
	free(r);

	r = NULL;
}

static RECV_RDP_t *receive_data_new(SESSION_t *s,PKT_t *p)
{
//	static u_int32_t orig_pktid = 0;
	u_int32_t		 subid;
	u_int32_t  		headlen = 0;
	RECV_RDP_t 		*recv;

	if (p->pkt_id && p->pkt_id == s->cur_pktid)/*evolution from orig_pktid*/
	{
		char buf[1024]; ///40 bytes

		SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));

		subid = p->sub_id;
		_RUDPA_DEBUG("\n\n\n receive_data_new; IN?\n\n\n");
		//printf("resend ack-----------------%u[%u]\n",p->pkt_id,p->sub_id);
		*(u_int32_t*)(buf + headlen + UDPH_LEN) = p->pkt_id;
		*(u_int32_t*)(buf + headlen + UDPH_LEN + UINT32LEN) = p->sub_id;
		udp_pkt_header(s,(UDP_PKT_H_t*)buf,CMD_RDPKT_ACK);
		_RUDPA_DEBUG("ack a package here :pkt:%x,subid:%x\n",p->pkt_id,p->sub_id);
		framecheck_send(s,buf,UDPH_LEN + 2*UINT32LEN,headlen);
		return NULL;
	}
	if ((recv = (RECV_RDP_t*)calloc(1,sizeof(RECV_RDP_t))) == NULL)
		return NULL;

	recv->time   = time(NULL);
	recv->pkt_id = p->pkt_id;
	recv->count	 = p->count;
	s->refs |= RECV;
	s->cur_pktid = p->pkt_id;
	return recv;
}

static RCV_PKT_t *sub_pkt_new(PKT_t *p)
{
	RCV_PKT_t *sp;

	if ((sp = (RCV_PKT_t*)calloc(1,sizeof(RCV_PKT_t))) == NULL)
		return NULL;

	sp->subid 	= p->sub_id;
	sp->len		= p->length;

	if ((sp->data = (u_char*)calloc(1,p->length)) == NULL)
	{
		free(sp);
		return NULL;
	}

	memcpy(sp->data,p->data,p->length);

	return sp;
}

int sub_pkt_add(RECV_RDP_t *recv,PKT_t *pkt)
{
	RCV_PKT_t		 		*np;
	RCV_PKT_t        *bp = NULL,*sp;
	static	u_int8_t count = 0;

	if (recv->subpkt == NULL)
	{	
		if ((sp = sub_pkt_new(pkt)) == NULL)
			return -1;

		recv->subpkt = sp;
		recv->recved++;
		_RUDPA_DEBUG("redved:%d,sub_id:%d,pkt_id:%d\n",recv->recved,sp->subid, pkt->pkt_id);
		if(pkt->count == recv->recved)
		{
			return 1;
		}
		return 0;
	}
#if 0	
	if ((np = sub_pkt_new(pkt)) == NULL)

		return -1;
#endif	
	sp = recv->subpkt;

	while(sp) 
	{
		if (pkt->sub_id == sp->subid)
		{
			return 3;
		}

		if (pkt->sub_id < sp->subid)
			break;
		bp = sp;
		sp = sp->next;
	}

	if ((np = sub_pkt_new(pkt)) == NULL)
		return -1;
	//printf("pkt sub id=%u\n",pkt->sub_id);
	if (!bp)
	{
		recv->subpkt = np;
	}
	else if (!sp)
	{
		np->next = sp;
		bp->next = np;
	}
	else
		bp->next = np;

	recv->recved++;

	if (recv->recved == recv->count)
	{
		//printf("last pkt:pktid=%u,subid=%u,count=%u,pktlen=%u\n",pkt->pkt_id,pkt->sub_id,pkt->count,pkt->length);
		return 1;
	}
	count++;
	if (count == 3)
	{
		count = 0;
		return 3;
	}

	return 0;
}

int rdpk_ack(SESSION_t *s)
{
	RCV_PKT_t 	*p;
	u_int32_t 	headlen = 0;
	char		buf[2048] = {0x00};
	int			i;

	if (s->recv == NULL || s->recv->subpkt == NULL)
		return -1;

	SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));

	udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_RDPKT_ACK);
	*(u_int32_t*)(buf + headlen + UDPH_LEN) = s->recv->pkt_id;
	p = s->recv->subpkt;

	for(i = 1;p;)
	{
		*(u_int32_t*)(buf + headlen + UDPH_LEN + i*4) = p->subid;
		p = p->next;
		i++;
	}
	usleep(1000);
	framecheck_send(s,buf,UDPH_LEN + i*4  ,headlen);

	return 0;
}

int rdpk_ack_all(SESSION_t *s)
{
	char buf[2048] = {0x00};
	char cbdata[2048]={0};
	int i = 1;
	int len;
	RECV_RDP_t  *r = s->recv;
	u_int32_t   headlen = 0;
	u_int32_t pktlen=0;
	LD_PACK_DATA_t *lpd;
	if((lpd =(LD_PACK_DATA_t*)calloc(sizeof(LD_PACK_DATA_t),1)) == NULL)
	{
		_RUDPA_ERROR("ld_pack_data calloc error\n");
		return;
	}
	if ((s->refs & RECV) != RECV)
		return 0;
	if (r == NULL)
		return -1;
	SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));
	if (r->subpkt)
	{
		RCV_PKT_t *sp,*sn;
		sn = r->subpkt;
		*(u_int32_t*)(buf + headlen + UDPH_LEN ) = r->pkt_id;

		while(sn)
		{
			sp = sn->next;
			if (sn->data)
			{
				memcpy(cbdata + pktlen,sn->data,sn->len);
				pktlen += sn->len;
				
				free(sn->data);
			}
			*(u_int32_t*)(buf + headlen + UDPH_LEN + i*4) = sn->subid;
			free(sn);
			sn = sp;
			i++;
		}
//		if (pktlen != 0)
//		{
//			ldp.data = cbdata;
//			ldp.packsize = pktlen;
//			SessionEventCall(s,Event_Ld_Pack,&ldp,sizeof(ldp));
//		}

	}
	//printf("ld packet data:pktid=%u pktcount=%u pktlen=%u\n\n",r->pkt_id,pktcount,pktlen);
	len = UDPH_LEN + i*4;
	free(r);

	s->refs &=~RECV;
	s->recv = NULL;

	udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_RDPKT_ACK);
	usleep(1000);
	
	i = framecheck_send(s,buf,len,headlen);
	/*throw the pkt to the top app*/
	if (pktlen != 0)
	{
		lpd->data = cbdata;
		lpd->packsize = pktlen;
		_RUDPA_STUB("ld data:%s\t session ID:%x\n",lpd->data,s->session_id);
		SessionEventCall(s,Event_Ld_Pack,lpd,sizeof(LD_PACK_DATA_t));
	}
#if 0
	struct timeb t1,t2,t3;
	ftime(&t3);
	ftime(&t1);
	hm=(t1.time-t3.time)*1000+(t1.millitm-t3.millitm);
	printf("payload time:%dms\n",hm);
	i = framecheck_send(s,CMD_RDPKT_ACK,buf,len);
	ftime(&t2);
	hm=(t2.time-t1.time)*1000+(t2.millitm-t1.millitm);
	printf("send time:%dms\n",hm);
#endif
	return i;
}

int receive_data_add(SESSION_t *s,UDP_PKT_H_t *uph,u_int32_t datalen)
{
	PKT_t	*pkt;

	if (datalen < UDPH_LEN + sizeof(PKT_t))
		return -1;

	pkt = (PKT_t*)uph->data;

	if (s->recv == NULL)
	{
		_RUDPA_STUB("first pkt:pktid=%u,subid=%u,count=%u,pktlen=%u\n",pkt->pkt_id,pkt->sub_id,pkt->count,pkt->length);
		if ((s->recv = receive_data_new(s,pkt)) == NULL)
			return -1;
	}
	else if (s->recv->pkt_id != pkt->pkt_id )//|| ((pkt->count >1) && (s->recv->recved == pkt->count))
	{
	   	rdpk_ack_all(s);

		if (s->recv)
			receive_data_free(s->recv);
		_RUDPA_DEBUG("next pkt:pktid=%u,subid=%u,count=%u,pktlen=%u\n",pkt->pkt_id,pkt->sub_id,pkt->count,pkt->length);
		if ((s->recv = receive_data_new(s,pkt)) == NULL)
			return -1;
	}
	
	//	printf("sub pkt:pktid=%u,subid=%u,count=%u,pktlen=%u\n",pkt->pkt_id,pkt->sub_id,pkt->count,pkt->length);

	return sub_pkt_add(s->recv,pkt);

}

int rdpkt_recv_all_pkts(SESSION_t *s)
{
	_RUDPA_DEBUG("rdpkt_recv_all_pkts:here we go\n");

	RCV_PKT_t 	*p = s->recv->subpkt;
	int 		ret;

	ret = rdpk_ack_all(s);

	if (s->recv)
			receive_data_free(s->recv);

	FILE 		*fp;
	if ((fp = fopen("./recv.txt","a")) == NULL)
		return -1;


	for(;s->recv && p && p->data;)
	{
		printf("re read\n");
		if (fwrite(p->data,p->len,1,fp) < 0)
			break;

		p = p->next;
	}

	fclose(fp);

	return ret;
}


int rdpkt_data(UDP_PKT_H_t *uph,u_int32_t len)
{
	SESSION_t 			*s;
	int					ret;

	

	if ((s = get_session(uph->session_id)) == NULL) 
		return -2;
	if ( s->timeout)
		return -1;
	_RUDPA_STUB("session ID:%x\n",uph->session_id);

	if ((ret = receive_data_add(s,uph ,len)) == -1)
		return -1;
	_RUDPA_STUB("receive_data_add ret:%d\n",ret);

	if (ret == 1)
		return rdpkt_recv_all_pkts(s);

	if (ret == 3)
		rdpk_ack(s);

	return 0;
	//return rdpk_ack(s);
}

int rdpkt_data_ack(UDP_PKT_H_t *uph,u_int32_t len)
{
	SESSION_t 	*s;

	if ((s = get_session(uph->session_id)) == NULL)
		return -2;

	return check_ack_pkt(s,(char*)uph + UDPH_LEN,len - UDPH_LEN);
	//return check_pkt(s,(char*)uph + UDPH_LEN,len - UDPH_LEN);
}

int error_return(UDP_PKT_H_t *uph)
{
	return 0;
}

int update_session_time(int sid)
{
	SESSION_t *s;

	if ((s = get_session(sid)) == NULL)  
		return -2;

	return 0;
}

int heart_beat(UDP_PKT_H_t *uph)
{
	return update_session_time(uph->session_id);
}

int udp_session_close_res(UDP_PKT_H_t *uph)
{
	SESSION_t 	*s;

	if ((s = get_session(uph->session_id)) == NULL)
		return -2;

	return session_quit(s);
}

//static int udp_session_error(UDP_PKT_H_t *uph,int sock_fd,struct sockaddr_in *clientaddr)
//{
//	int ret;
//	u_int16_t datasize;
//	char buf[1024];
//	u_int32_t headlen = 0;

//	///SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));
//	datasize = headlen + UDPH_LEN + 4;
//	memcpy(buf + headlen,uph,UDPH_LEN - 4);
//	//udp_pkt_header(s,(UDP_PKT_H_t*)buf,CMD_ERROR);
//	*(u_int32_t*)(buf + UDPH_LEN - 4) = CMD_ERROR;
//	*(u_int32_t*)(buf + UDPH_LEN) = ERRNO_SESSION;

//	ret = sendto(sock_fd,(void*)buf,datasize,0,(struct sockaddr*)clientaddr,sizeof(struct sockaddr));

//	return ret;
//}

int udp_pkt_parse(LISTEN_t * thiz,char* data, u_int32_t len,int sock_fd,struct sockaddr_in 	*clientaddr)
{
	UDP_PKT_H_t *uph;
	int			ret = -1;
	SESSION_t   *s;
	char		buf[1024];
	u_int32_t       headlen = 0;

	if (len < UDPH_LEN)
		return ret;

	uph = (UDP_PKT_H_t*)data;

	_RUDPA_STUB(" udph cmd:%d,sessionID:%x\n",uph->cmd,uph->session_id);

	switch (uph->cmd)
	{
		case CMD_CREATE_SESSION:
			if ((s = session_begin(thiz,uph,len,sock_fd, ntohl(clientaddr->sin_addr.s_addr),ntohs(clientaddr->sin_port))) == NULL)
			{
				ret = -2;
				break;
			}

			SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));

			_RUDPA_TRACE("headlen:%d\r\n",headlen);

			udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_SESSION_RES);
			*(u_int32_t*)(buf + headlen + UDPH_LEN) = *(u_int32_t*)uph->data;//set the random in
			if ((ret = framecheck_send(s,buf,UDPH_LEN + sizeof(u_int32_t),headlen)) != 0)
			{
				break;
			}
			_RUDPA_DEBUG("CMD_CREATE_SESSION done!!\n");

			break;

		case CMD_HEARTBEAT:
			ret = heart_beat(uph);
			break;

		case CMD_RDPKT:
			ret = rdpkt_data(uph,len);
			break;

		case CMD_RDPKT_ACK:
			ret = rdpkt_data_ack(uph,len);
			break;

		case CMD_ERROR:
			ret = error_return(uph);
			break;

		case CMD_CLOSE_SESSION:
			ret = _session_close(uph);
			break;

		case CMD_CLOSE_ACK:
			ret = udp_session_close_res(uph);
			break;

		default:break;
	}
#if 0
	if (ret == -2)
		udp_session_error(uph, sock_fd,clientaddr);
#endif
	//printf("recv: ret=%d flag=%u,version=%u,session_id=%u,cmd=%u",ret,uph->flag,uph->version,uph->session_id,uph->cmd);
	//if (uph->cmd == CMD_RDPKT_ACK)
	//	printf(" pktid=%u",*(u_int32_t*)uph->data);

	//printf("\n");
	return ret;

}
#if 1



int udp_pkt_payload(SESSION_t *s,char *buf,u_int32_t cmd,char * data, u_int32_t len)
{
	UDP_PKT_H_t  *uph = (UDP_PKT_H_t*)buf;
#if 0
	u_int32_t offset = 0;

	*(u_int32_t*)buf = 0xff9a1234;
	offset += sizeof(u_int32_t);
	*(u_int32_t*)(buf + offset) = RUDP_VERSION;
	offset += sizeof(u_int32_t);
	*(int*)(buf + offset) = s->session_id;
	offset += sizeof(int) + 12 * sizeof(u_int8_t);
	*(u_int32_t*)(buf + offset) = s->tick;
	offset += sizeof(u_int32_t);
	*(u_int32_t*)(buf + offset) = cmd;
	memcpy(buf + UDPH_LEN,data,len);
#endif
	udp_pkt_header(s,uph,cmd);

	if (len)
		memcpy(uph->data,data,len);

	return UDPH_LEN + len;
}
#endif


int udp_heart_beat(SESSION_t *s)
{
	char buf[1024];
	u_int32_t headlen = 0;

	if ( !s->timeout)
	{
		SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));
		udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_HEARTBEAT);
		return framecheck_send(s,buf,UDPH_LEN,headlen);
	}

	return 0;
}

int udp_session_close_ack(SESSION_t *s)
{
	char buf[1024];
	u_int32_t  headlen = 0;

	if ( !s->timeout)
	{
		SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));
		udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_CLOSE_ACK);
		return framecheck_send(s,buf,UDPH_LEN,headlen);
	}

	return 0;
}

int udp_session_close(SESSION_t *s)
{
	char buf[1024];
	u_int32_t headlen = 0;

	if ( !s->timeout)
	{
		SessionEventCall(s,Event_Pre_Create_Pack,&headlen,sizeof(headlen));
		udp_pkt_header(s,(UDP_PKT_H_t*)(buf + headlen),CMD_CLOSE_SESSION);
		return framecheck_send(s,buf,UDPH_LEN ,headlen);
	}

	return 0;
}


