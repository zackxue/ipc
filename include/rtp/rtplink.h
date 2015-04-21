#ifndef __RTP_LINK_H__
#define __RTP_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <malloc.h>

#include "rtplib.h"

#ifndef RET_OK
#define RET_OK 	(0)
#endif
#ifndef RET_FAIL
#define RET_FAIL (-1)
#endif

#define DLINK_POS_TOP	(-1)// insert as newest one,always pos is n
#define DLINK_POS_BOT	(-2)// insert as oldest one, always pos is 1
#define DLINK_INSERT_BY_SEQ	(-3)// insert as oldest one, always pos is 1


typedef struct _ElemType
{
	union
	{
		RtpFrameInfo_t frame;
		struct{
			int entries;
			int max_seq;
			//char type;// 'V' or 'A'
			//void *last_node;//last node pointer
		};
	};
}DLElem_t;

typedef struct _DLNode
{
	DLElem_t  data;
	struct _DLNode *prev;
	struct _DLNode *next;
}DLNode_t;//28bytes

typedef DLNode_t DLink_t;

int DLINK_init(DLink_t **DL);
int DLINK_destroy(DLink_t *DL);
int DLINK_print(DLink_t *DL);
int DLINK_bprint(DLink_t *DL);/* inverse output*/
int DLINK_insert(DLink_t *DL,int i,DLElem_t *e);
int DLINK_delete(DLink_t *DL,int i);
int DLINK_pull(DLink_t *DL,int i,DLElem_t *e);

//
int DLINK_pull_by_seq(DLink_t *DL,DLElem_t *elem);
int DLINK_pull_by_seq_and_del(DLink_t *DL,DLElem_t *elem);
int DLINK_pull_and_del(DLink_t *DL,int i,DLElem_t *elem);
//
int DLINK_check_lost_seq(DLink_t *DL);

#ifdef __cplusplus
}
#endif

#endif

