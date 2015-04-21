#include "turn.h"
#include "Aes.h"
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "rudpa_debug.h"
#include "esee.h"
#include "union_protocol.h"

static unsigned char keys[16]={0x2c, 0x56, 0xab, 0x33, 0x57, 0xf4, 0xa1, 0x9c, 0x20, 0xd7, 0xce, 0x53, 0x98, 0xc1,0xaa, 0x2e};

/*
 *struct sockaddr_in GetServerInfo()
 *{
 *    struct sockaddr_in ServerInfo;
 *    memset(&ServerInfo,0,sizeof(ServerInfo));
 *    ServerInfo.sin_family = AF_INET;
 *    ServerInfo.sin_port = htons(10201);
 *    ServerInfo.sin_addr.s_addr = inet_addr("210.21.39.197");[>210.21.39.197<]
 *    return ServerInfo;
 *
 *    struct hostent* addr = gethostbyname("serialsvr.meibu.com");
 *    if (addr == NULL)
 *    {
 *        return ServerInfo;
 *    } 
 *    else
 *    {
 *        struct in_addr inIPAddress;
 *        memcpy((void*)&inIPAddress,addr->h_addr_list[0],sizeof(inIPAddress));
 *        ServerInfo.sin_family = AF_INET;
 *        ServerInfo.sin_port = htons(10201);
 *        ServerInfo.sin_addr = inIPAddress;
 *    }
 *
 *    return ServerInfo;
 *}
 */

void TurnInsertReq(Turn * thiz,unsigned int uiRadom,unsigned int uiIp,unsigned int uiPort,unsigned int uiTTL)
{
	CsReq * newReq = (CsReq *)malloc(sizeof(CsReq));
	if(NULL == newReq)
	{
		_RUDPA_ERROR("Turn req queue:mem malloc failed\n");
		return ;
	}
	newReq->pNext = NULL;
	newReq->uiRadom = uiRadom;
	newReq->uiIp = uiIp;
	newReq->uiPort = uiPort;
	newReq->uiTTL = 30;
	thiz->m_pReqCur->pNext = newReq;
	thiz->m_pReqCur = newReq;
}

void TurnFlushReqs(Turn * thiz)
{
	CsReq * pPre = &thiz->m_Reqs;
	CsReq * pTraval = thiz->m_Reqs.pNext;
	while(NULL != pTraval)
	{
		pTraval->uiTTL--;
		if (0 == pTraval->uiTTL)
		{
			_RUDPA_TRACE("Req %d cleaned!\r\n",pTraval->uiRadom);
			if (thiz->m_pReqCur == pTraval)
			{
				thiz->m_pReqCur = pPre;
			}
			CsReq * pTemp = pTraval;
			pTraval = pTraval->pNext;
			pPre->pNext = pTemp->pNext;
			free(pTemp);
			continue;
		}
		pTraval = pTraval->pNext;
		pPre = pPre->pNext;
	}
}

int TurnPopReq(Turn * thiz,unsigned int uiRadom,unsigned int *uiIp,unsigned int * uiPort)
{
	CsReq * pPre = &thiz->m_Reqs;
	CsReq * pTraval = thiz->m_Reqs.pNext;
	while(NULL != pTraval)
	{
		if (uiRadom == pTraval->uiRadom)
		{
			*uiIp = pTraval->uiIp;
			*uiPort = pTraval->uiPort;
			_RUDPA_DEBUG("ip:%x,port:%u\n",*uiIp,*uiPort);

			if (thiz->m_pReqCur == pTraval)
			{
				thiz->m_pReqCur = pPre;
			}
			CsReq * pTemp = pTraval;
			pPre->pNext = pTraval->pNext;
			pTraval = pTraval->pNext;
			free(pTemp);
			return 0;
		}
		pTraval = pTraval->pNext;
		pPre = pPre->pNext;
	}
	return -1;
}


void TurnReq(Turn *thiz)
{
	unsigned char sIdTemp[16] = {0};
	unsigned char sIdOutTemp[16] = {0};
	memcpy(sIdTemp,thiz->m_sId,16);
	ProtocolTurn Data;
	Data.uiHead = PROTOCOLHEAD;
	Data.uiCmd = DS_Turn_Req;
	Data.uiRadom = rand();
	// xor
	int *nF4 = (int *)sIdTemp;
	*nF4 = *nF4 ^ Data.uiRadom;
	// crypt
	InitAes(16,keys);
	Cipher(sIdTemp,sIdOutTemp);
	memcpy(Data._U.sId,sIdOutTemp,16);
	// Server
	struct sockaddr_in ServerAddr = thiz->turnServer;
	direct_send(thiz->m_l,(char *)&Data,sizeof(Data),&ServerAddr);
}


void *TurnThread(void *pParam)
{
	Turn *thiz = (Turn *)pParam;
	time_t curTime;
	time_t reqFlushTime = time(NULL);

	while (1)
	{
		usleep(10 * 1000);
		switch(thiz->m_Status)
		{
			case TS_IDLE: // start turn_service,but no client req
				{
					//TRACE("Turn:No client req turn!!sleep a while\n");
					sleep(1);
				}
				break;
			case TS_TURNREQ: 
				{
					thiz->b_LoginStatus = false;
					TurnReq(thiz);
					_RUDPA_TRACE("Device start request turn:%d \n",thiz->m_retry+1);
					thiz->m_Status = TS_WAIT_TURNREQ;
					curTime = time(NULL);
				}
				break;
			case TS_WAIT_TURNREQ: // wait for turnserver ack
				{
					if (time(NULL) - curTime >= TURN_TIMEOUT)
					{
						_RUDPA_DEBUG("Turn req: time out \r\n");
						thiz->m_Status = TS_TURNREQ;
						thiz->m_retry++;
					}

					if (true == thiz->b_LoginStatus)
					{ // login success
						_RUDPA_TRACE("Turn req: success\r\n");
						thiz->m_Status = TS_RUN;
					}

					if(3 == thiz->m_retry)
					{
						thiz->m_retry = 0;
						thiz->m_Status = TS_IDLE;
						_RUDPA_DEBUG("Turn req failed,Change status to idle!\n");
					}
				}
				break;
			case TS_RUN: // start turn,but no client req
				{
					_RUDPA_DEBUG("Turn: running\n");
					/*
					 *if(NULL == thiz->m_Reqs.pNext)
					 *    thiz->m_Status = TS_IDLE;
					 */
					sleep(1);
				}
				break;
			default:  
				break;
		}
		if (time(NULL) - reqFlushTime >=1)
		{
			TurnFlushReqs(thiz);
			reqFlushTime = time(NULL);
		}
	}
}


int TurnSetListener(Turn * thiz, SESSIONLISTENER_HANDLE *l)
{
	_RUDPA_ASSERT(thiz,"Input param thiz:%p",thiz);
	thiz->m_l = l;
	return 0;
}

/*turn event proc ,procese the turn event,an wraper for other code to call,*/

int TurnEventProc(struct _tagTurn *thiz,_2Turn *pData,TurnStatus status)
{
	switch(status)
	{
		case TS_TURNREQ:
			{
				strcpy(thiz->m_sId,pData->sId);
				bzero(&thiz->turnServer,sizeof(thiz->turnServer));
				thiz->turnServer.sin_family = AF_INET;
				thiz->turnServer.sin_port = htons(atoi(pData->turnPort));
				thiz->turnServer.sin_addr.s_addr = inet_addr(pData->turnIp);
				thiz->m_Status = TS_TURNREQ;
				thiz->m_retry = 0;
			}
			break;
		default:
			break;
	}
	return 0;
}


int TurnDataProc(void *uProtocol,SESSIONLISTENER_HANDLE *l,SESSEION_EVENT_t e,void *pData,int nDatasize)
{
	UnionProtocol *up = (UnionProtocol *)uProtocol;
	Turn *thiz = up->up_turn;
	Esee *esee = up->up_esee;
	RECV_t * pRecvData = (RECV_t *)pData;
	ProtocolTurn * pPack = (ProtocolTurn *)pRecvData->realbuf;

	if (PROTOCOLHEAD != pPack->uiHead)
	{
		_RUDPA_DEBUG("Not turn pkt!!\n");
		return (int)TE_HEADERROR;
	}

	switch(pPack->uiCmd)
	{
		case SD_Client_Cs:
			{
				unsigned int uiRadom = pPack->uiRadom;
				unsigned int uiIP = pPack->_U.ClientCs.uiIp;
				unsigned int uiPort = pPack->_U.ClientCs.uiPort;
				unsigned int uiTTL = pPack->_U.ClientCs.uiTTL;

				if (0 != uiIP && 0 != uiPort)
				{
					_RUDPA_TRACE("New client req:%08u %08x %08u \r\n",uiRadom,uiIP,uiPort);
					TurnInsertReq(thiz,uiRadom,uiIP,uiPort,uiTTL);
				}
				else
				{
					_2Esee _2esee;
					esee->EventProc(esee, &_2esee,ES_TURNREADY);/*make the device send the readyturn to platform*/
					_RUDPA_TRACE("Req not confirmed:%u %08x %u\r\n",uiRadom,uiIP,uiPort);
				}

				thiz->b_LoginStatus = true;
			}
			break;
		case SD_Invalide_Id:
			break;
		case SD_No_Resouce:
			break;
		case SD_Client_Ready:
			break;
		default:
			return (int)TE_CMDERROR;
	}

	pRecvData->produced = !0;
	return 0;	
}

int TurnPreCreatePack(Turn * thiz,void *pData,unsigned int datasize)
{
	if (TS_IDLE == thiz->m_Status)
	{
		return 0;
	}

	PRE_CREATE_PACK * pac = (PRE_CREATE_PACK *)pData;
	pac->pre_buf_size = sizeof(ProtocolTurn);
	_RUDPA_DEBUG("headlen:%d\n",pac->pre_buf_size);
	return 0;	
}

int TurnTurn(Turn * thiz,void *pData, unsigned int uiDstIp, unsigned int uiDstPort)
{
	if (TS_IDLE == thiz->m_Status || 0 == uiDstIp || 0 == uiDstPort)
	{
		return 0;
	}

	_RUDPA_DEBUG("Turn ip:%s,port:%u\n",inet_ntoa(*(struct in_addr*)&uiDstIp),uiDstPort);

	SEND_PRE_t * pSendPre = (SEND_PRE_t *)pData;
	ProtocolTurn *pTurn = (ProtocolTurn *)pSendPre->Head;
	pTurn->uiHead = PROTOCOLHEAD;
	pTurn->uiCmd = DS_Turn_Data;
	pTurn->uiRadom = rand();
	pTurn->_U.TurnData.uiIp = uiDstIp;
	pTurn->_U.TurnData.uiPort = uiDstPort;
	pSendPre->senddatasize += sizeof(ProtocolTurn);
	pSendPre->taraddress = thiz->turnServer;

	return 0;
}

Turn * CreateNewTurn(void *uProtocol)
{
	Turn * pNew = (Turn *)malloc(sizeof(Turn));
	if(NULL == pNew)
	{
		_RUDPA_ERROR("NewTurn:mem malloc failed\n");
		return NULL;
	}
	/*
	 *[>test turn server is okay!<]
	 *strcpy(pNew->m_sId,"100007149");
	 */

	pNew->m_l = NULL;
	pNew->SetListener = TurnSetListener;
	pNew->EventProc= TurnEventProc; 
	pNew->DataProc = TurnDataProc;
	pNew->PopReq = TurnPopReq;
	pNew->turn = TurnTurn;
	pNew->PreCreatePack = TurnPreCreatePack; 

	UnionProtocol *up = (UnionProtocol*)uProtocol;
	up->up_turn = pNew;
	pNew->m_uProtocol = up;

	pthread_t threadid;
	pthread_create(&threadid,NULL,TurnThread,pNew);
	pNew->m_Status = TS_IDLE;

	memset(&pNew->m_Reqs,0,sizeof(pNew->m_Reqs));
	pNew->m_pReqCur = &pNew->m_Reqs;
	srand((unsigned int)time(NULL));

	return pNew;
}
