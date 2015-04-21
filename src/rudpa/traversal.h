/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		traversal.h
 * Describle:
 * History: 
 * Last modified:	2013-03-18 14:48
 ============================================================*/
#ifndef TRAVERSAL_H 
#define TRAVERSAL_H 
#include <pthread.h>
#include "rudp_session.h"

#define TRAVERSAL_TIMEOUT 1000*500 //ms 
#define TRAVERSAL_IDLE_TIME 20


typedef enum _enTraversalStatus
{
	TraS_IDLE,
	TraS_REQ,
	TraS_CONFIRM_HOLE,
	Tras_CHANGE_HOLE,
	TraS_RUN,
}TraversalStatus;


typedef struct _tag2Traversal
{
	unsigned int clientIp;
	unsigned int clientPort;
	unsigned int random;
	unsigned int TTL;
}_2Traversal;

typedef struct _tagCTraReq{
	unsigned int clientIp;
	unsigned int clientPort;
	unsigned int random;
	unsigned int TTL;
	struct _tagCTraReq * pNext;
}CTraReq;

typedef struct _tagTraversal
{

	TraversalStatus m_Status;
	_2Traversal *m_traversal_data;
	CTraReq m_Reqs;
	CTraReq *m_pReqCur;
	pthread_mutex_t m_Traversal_Reqs_mutex;

	void *m_uProtocol;


	int (*EventProc)(void*,_2Traversal *,TraversalStatus);
	int (*DataProc)(void* ,SESSIONLISTENER_HANDLE * ,SESSEION_EVENT_t  ,void * ,int );
	int (*PopReq)(struct _tagTraversal *,unsigned int );
	struct sockaddr_in (*FindClient)(struct _tagTraversal *,unsigned int );


}Traversal;

extern Traversal* CreateNewTraversal(void *);


#endif  /*end fo the traversal.h*/ 
