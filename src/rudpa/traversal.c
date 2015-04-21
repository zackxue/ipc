/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		traversal.c
 * Describle: some fuction that deal with traverse between client & device(IPC) *		 		 
 * History: 
 * Last modified:	2013-06-04 21:22
 =============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rudpa_debug.h"
#include "traversal.h"
#include "union_protocol.h"
#include "esee.h"

void TraversalInsertReq(Traversal * thiz,unsigned int uiRadom,unsigned int uiIp,unsigned int uiPort,unsigned int uiTTL)
{
	CTraReq * newReq = (CTraReq *)malloc(sizeof(CTraReq));
	if(NULL == newReq)
	{
		_RUDPA_ERROR("Traversal req queue mem malloc failed\n");
		return;
	}
	newReq->pNext = NULL;
	newReq->random = uiRadom;
	newReq->clientIp = uiIp;
	newReq->clientPort = uiPort;
	newReq->TTL = 30;
	pthread_mutex_lock(&thiz->m_Traversal_Reqs_mutex);
	thiz->m_pReqCur->pNext = newReq;
	thiz->m_pReqCur = newReq;
	pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);
}

/*here the lock can optimized*/
int TraversalPopReq(Traversal * thiz,unsigned int uiRadom )
{
	CTraReq * pPre = &thiz->m_Reqs;
	CTraReq * pTraval = thiz->m_Reqs.pNext;
	pthread_mutex_lock(&thiz->m_Traversal_Reqs_mutex);
	while(NULL != pTraval)
	{

		if (uiRadom == pTraval->random)
		{

			_RUDPA_TRACE("TRAVERSAL:pop client ip:%s,port:%d,random:%d\n",\
					inet_ntoa( *(struct in_addr*)&pTraval->clientIp),pTraval->clientPort, pTraval->random);

			if (thiz->m_pReqCur == pTraval)
			{
				thiz->m_pReqCur = pPre;
			}
			CTraReq * pTemp = pTraval;
			pPre->pNext = pTraval->pNext;
			pTraval = pTraval->pNext;
			free(pTemp);
			pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);
			return 0;
		}
		pTraval = pTraval->pNext;
		pPre = pPre->pNext;
	}
	pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);
	return 0;
}

struct sockaddr_in TraversalFindClient(Traversal *thiz,unsigned int uiRadom)
{
	struct sockaddr_in ClientInfo;
	bzero(&ClientInfo,sizeof(ClientInfo));
	ClientInfo.sin_family = AF_INET;
	ClientInfo.sin_port = htons(0);
	ClientInfo.sin_addr.s_addr = inet_addr("0.0.0.0");

	CTraReq * pTraval = thiz->m_Reqs.pNext;

	while(NULL != pTraval)
	{
		if (uiRadom == pTraval->random)
		{
			ClientInfo.sin_port = htons(pTraval->clientPort);
			ClientInfo.sin_addr = *(struct in_addr*)&pTraval->clientIp;
			_RUDPA_DEBUG("Find client:%s,port:%d,random:%d\n",\
					inet_ntoa( *(struct in_addr*)&pTraval->clientIp),pTraval->clientPort, pTraval->random);

			return ClientInfo;
		}
		pTraval = pTraval->pNext;	
	}	
	return ClientInfo;
}


int TraversalEventProc(void* uProtocol,_2Traversal *pData,TraversalStatus status)
{
	UnionProtocol *up = (UnionProtocol*)uProtocol;
	Traversal *thiz = up->up_traversal;
	switch(status)
	{
		case TraS_CONFIRM_HOLE:
			{
				/*get client tarversal req,start confirm&hole client*/ 
				thiz->m_Status = TraS_CONFIRM_HOLE;
				thiz->m_traversal_data = pData;
				TraversalInsertReq(thiz,pData->random,pData->clientIp,pData->clientPort,pData->TTL);
			}
			break;
		case Tras_CHANGE_HOLE:
			{
				/*if the addr which get from client's hole diffrent from the plat offer*/			
				thiz->m_Status = Tras_CHANGE_HOLE;
				thiz->m_traversal_data = pData;				
				pthread_mutex_lock(&thiz->m_Traversal_Reqs_mutex);
				CTraReq * pTraval = thiz->m_Reqs.pNext;				
				while(NULL != pTraval)
				{
					if((pTraval->random == pData->random)
							&& ((pTraval->clientIp != pData->clientIp)||(pTraval->clientPort != pData->clientPort)))
					{
						_RUDPA_ERROR("Client Hole Changed:%d:%d>>>%s:%d\n",
								pTraval->clientIp,pTraval->clientPort,
								TagTable[ESEE_TAG_CLIENTIP].TagTxt,
								pData->clientPort);	
						pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);

						TraversalPopReq(thiz,pData->random);
						TraversalInsertReq(thiz,pData->random,pData->clientIp,pData->clientPort,pData->TTL);
						break;
					}
					pTraval = pTraval->pNext;
				}
				pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);
			}
			break;
		default:
			break;
	}
	return 0;
}

void *TraversalThread(void *pParam)
{
	Traversal *thiz = (Traversal *)pParam;
	UnionProtocol *up = (UnionProtocol*)thiz->m_uProtocol;
	time_t idle_time = time(NULL);
	for(;;)
	{
		usleep(10 * 1000);
		/*if the reqs not NULL ,then will send the hole,which circle the client in the queue*/ 
		CTraReq *pPre  = &thiz->m_Reqs;
		CTraReq *pTra  = thiz->m_Reqs.pNext;
		if(NULL != pTra)
		{
			pthread_mutex_lock(&thiz->m_Traversal_Reqs_mutex);
			while(NULL != pTra)
			{
				struct sockaddr_in client_addr = TraversalFindClient(thiz,pTra->random);
				char random[12] = {0};			
				sprintf(random,"%d",pTra->random);
				SetTagTxt("random",random);
				_RUDPA_TRACE("SHoleClient:%s:%d:%d\n",inet_ntoa(*(struct in_addr*)&client_addr.sin_addr),
						ntohs(client_addr.sin_port), pTra->random);
				EseeSendTo(up->up_esee,SHoleClient,&client_addr);

				pTra->TTL--;
				if(0 == pTra->TTL)
				{
					if (thiz->m_pReqCur == pTra)
					{
						thiz->m_pReqCur = pPre;
					}
					CTraReq * pTemp = pTra;
					pTra  = pTra->pNext;
					pPre->pNext = pTemp->pNext;
					free(pTemp);
					continue;
				}			

				pTra= pTra->pNext;
				pPre = pPre->pNext;		
			}
			pthread_mutex_unlock(&thiz->m_Traversal_Reqs_mutex);
			usleep(TRAVERSAL_TIMEOUT);					
		}
		else
		{	
			/*if null,the code will not run the sleep() below,make it more quickly*/
			if(time(NULL) - idle_time > TRAVERSAL_IDLE_TIME)
			{				
				_RUDPA_DEBUG("The traversal thread is idle\n");
				idle_time = time(NULL);
			}			
		}	
	}
}

/*the constructor of the traversal*/ 
Traversal *CreateNewTraversal(void*uProtocol)
{
	UnionProtocol *up = (UnionProtocol*)uProtocol;
	Traversal *pNew = (Traversal*)malloc(sizeof(Traversal));
	if(NULL == pNew)
	{
		_RUDPA_ERROR("New Traversal :mem malloc failed\n");
		return NULL;
	}

	pNew->m_Status = TraS_IDLE;
	pNew->m_traversal_data = NULL;

	pNew->EventProc = TraversalEventProc;
	pNew->PopReq = TraversalPopReq;
	pNew->FindClient = TraversalFindClient;
	pNew->m_uProtocol = up;
	pthread_mutex_init(&pNew->m_Traversal_Reqs_mutex,NULL);
	up->up_traversal = pNew;

	memset(&pNew->m_Reqs,0,sizeof(pNew->m_Reqs));
	pNew->m_pReqCur = &pNew->m_Reqs;

	pthread_t threadId;
	pthread_create(&threadId, NULL, TraversalThread,pNew);

	return pNew;
}



