#ifndef __SEND_H_
#define __SEND_H_

#include <netinet/in.h>
#include "rudp_session.h"

#define DEFAULT_PKT_LEN	1320
#define PKTINFO_HEAD_LEN sizeof(PKT_DATA_t)


typedef struct _pkt_data_t
{	
	u_int32_t   pkt_id;
	u_int32_t   sub_id;
	u_int32_t   count;
	u_int32_t   length;
	u_char		data[0];
}PKT_DATA_t;

typedef struct _udp_pkt_h_t
{
	u_int32_t	flag;
	u_int32_t	version;
	int			session_id;
	u_int8_t	reserve[12];
	u_int32_t   tick;
	u_int32_t	cmd;
	char        data[0];
}UDP_PKT_H_t;
#define UDPH_LEN		sizeof(UDP_PKT_H_t)

struct _sub_pkt_t
{
	u_int32_t  	send_time;
	u_int32_t  	ack;
	UDP_PKT_H_t *udata;
	
}__attribute__((packed));

typedef struct _sub_pkt_t SUB_PKT_t;



typedef struct _received_t
{
	u_int8_t  acked;
	u_int32_t last;
}RECEIVED_t;

typedef struct _send_t
{
	u_int32_t		begin;
	u_int32_t		pkt_id;
	u_int16_t		pkt_count;
	
	SUB_PKT_t	 	**pkts;
}SEND_t;


#define CMD_CREATE_SESSION	0x00000001
#define CMD_SESSION_RES		0x00000002
#define CMD_HEARTBEAT		0x00000003
#define CMD_RDPKT		0x00000004
#define CMD_RDPKT_ACK		0x00000005
#define CMD_ERROR		0x00000006
#define CMD_CLOSE_SESSION	0x00000007
#define CMD_CLOSE_ACK		0x00000008

#define ERRNO_SESSION		0x00000001

struct _pkt_t
{
        u_int32_t   pkt_id;
        u_int32_t   sub_id;
        u_int32_t   count;
        u_int32_t   length;
        char        data[0];
}__attribute__((packed));

typedef struct _pkt_t PKT_t;

typedef struct _recv_pkt_t
{
	u_int32_t   subid;
	u_int32_t   len;
	u_char              *data;
	struct _recv_pkt_t   *next;
}RCV_PKT_t;


typedef struct _receive_t
{
	u_int16_t   recved;
	u_int32_t 	pkt_id;
	u_int32_t	count;
	time_t      time;
	RCV_PKT_t	*subpkt;
	//DATA_PKT_t		  *pkt;
	//struct _receive_t *next;		
}RECV_RDP_t;




#define RECV	0x01
#define SEND	0x02

#define PKTCOUNT	 100
#define SENDBUF	(500*1024)

typedef struct _tagEventMap{
	int		(*proc)(SESSION_HANDLE *,SESSEION_EVENT_t ,void *,int ,void *);
	void *  pUser;
	SESSEION_EVENT_t e;
}EventMap_t;


typedef struct session_t
{
	int		fd;
	int 		session_id;
	u_int32_t	radom;
	u_int8_t	sendflag;
	u_int32_t	time;
	u_int8_t	close;
	u_int8_t         timeout;
	u_int32_t           tick;
	u_int32_t           interval;
	u_int32_t           pkt_interval;
	u_int32_t           sub_interval;
	u_int32_t           sub_pkt_interval;
	u_int32_t           ip;
	u_int16_t           port;
	u_int8_t            refs;
	u_int8_t            quit;
	time_t		    hbt;
	pthread_mutex_t     mutex;
	pthread_cond_t      cond;
	u_int32_t			pkt_id;
	u_int8_t			ack[PKTCOUNT];
	char				data[SENDBUF];
	SEND_t              *send;
	RECV_RDP_t	   *recv;
	EventMap_t   eMap[Event_Cnt];
	u_int32_t cur_pktid;/*store the current pktid,for a session*/
	struct session_t 	*next;
}SESSION_t;

/*send the data in a child thread,then the main thread will not blocked*/
typedef struct _AsideSend_t
{
	SESSION_HANDLE* session;/*session handle*/
	char * data;/*the data ready to send*/
}AsideSend_t;

// static inline int _send_lock(SESSION_t *s);
// static inline int _send_unlock(SESSION_t *s);
// static inline int _send_cond_singal(SESSION_t *s);
// static inline int _send_cond_broadcast(SESSION_t *s);
// static inline int _send_cond_wait(SESSION_t *s);
// static inline int _send_cond_timewait(SESSION_t *s,struct timespec *to);

static inline int _send_cond_timewait(SESSION_t *s,struct timespec *to)
{
    int ret;
	
	while((ret = pthread_cond_timedwait(&s->cond,&s->mutex,to)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _send_cond_wait(SESSION_t *s)
{
    int ret;
	
	while((ret = pthread_cond_wait(&s->cond,&s->mutex)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _send_cond_broadcast(SESSION_t *s)
{
    int ret;
	
	while((ret = pthread_cond_broadcast(&s->cond)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _send_cond_singal(SESSION_t *s)
{
    int ret;
	
	while((ret = pthread_cond_signal(&s->cond)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _send_unlock(SESSION_t *s)
{
    int ret;
	
	while((ret = pthread_mutex_unlock(&s->mutex)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}

static inline int _send_lock(SESSION_t *s)
{
    int ret;
	
	while((ret = pthread_mutex_lock(&s->mutex)) != 0 && (errno == EINTR || errno == EAGAIN))
		sleep(1);
	
	return ret;
}


int SessionEventCall(SESSION_t *s,SESSEION_EVENT_t e,void *pData,int nDatasize);
u_int32_t get_now_usec();
int get_socket(SESSION_t *s);
void send_free(SESSION_t *s);
int send_begin();
void send_end();
int send_one_frame(SESSION_t *s);
int send_sub_block_data(SESSION_t *s,u_int32_t pktid,u_int32_t subid,u_int32_t count,u_char *data,u_int32_t datalen);
void aside_send(void *handle,void *data);

#endif
