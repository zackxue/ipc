#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "send.h"
#include "check.h"
#include "listen.h"
#include "session.h"

#define FRAME_TIMEOUT  5
#define SUBPKT_TIMEOUT 2
#define CHECK_TIMEOUT  3

struct _check_hdr_t
{
	u_int32_t  pkt_id;
	
}__attribute__((packed));

typedef struct _check_hdr_t CHECK_HDR_t;

#define CHECK_DATA_LEN		sizeof(CHECK_HDR_t)

int check_ack_pkt(SESSION_t *s,char *data,int datalen)
{
	CHECK_HDR_t	*ch;
	u_int32_t 	offset;
	u_int32_t	sub_id;
	
	if (datalen <= CHECK_DATA_LEN)
		return -1;
	
	offset = (datalen - CHECK_DATA_LEN) % sizeof(u_int32_t);
	if (offset != 0)
		return -1;
	
	ch = (CHECK_HDR_t*)data;
	if (ch->pkt_id != s->pkt_id)
		return -1;
	//printf("pkti feed:%u\n",ch->pkt_id);
	for (offset = CHECK_DATA_LEN;offset < datalen;offset += sizeof(u_int32_t))
	{
		sub_id = *(u_int32_t*)(data + offset);
		
		s->ack[sub_id] = 1;
		//printf("session_id:%d,s->ack[%d]:%d\n",s->session_id ,sub_id, s->ack[sub_id]);
	}
	//printf("\n");
	return 0;
}
