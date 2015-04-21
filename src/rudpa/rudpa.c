/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	rudpa.c
 * Describle:
 * History: 
 * Last modified: 2013-03-19 16:09
 =============================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/stat.h>

#include <ucode.h>


#include "rudpa_debug.h"
#include "rudp_session.h"
#include "turn.h"
#include "send.h"
#include "Aes.h"
#include "base64.h"
#include "esee.h"
#include "esee_protocol.h"
#include "union_protocol.h"
#include "traversal.h"
#include "rudpa_soup.h"

/*if defined this Macro ,the soup need the usr_auth before  the client req the stream*/
//#define SOUP_USR_AUTH 1 



#define IPC_USR "admin"
#define IPC_PSW ""
#define DEFAULT_STREAM "720p.264"

char  IPC_MAC[32];
char *ESEE_PLAT_IP = "192.168.2.2";
int ESEE_PLAT_PORT = 60101;

#define FRAME_TIMEOUT	5
#define SUB_TIMEOUT		2

#define FILE_PATH		"./CIF_12fps_128kbps.h264"

typedef struct _tagRudpa{
	SESSION_HANDLE *s;
	unsigned int    m_uiSessionStatus;
	unsigned int    m_uiSessionCanBeFree;
	unsigned int 	m_DataThreadStart_If;/*flag of data thread whether create?*/
	unsigned int    m_uiIp;
	unsigned int    m_uiPort;
	Turn *			m_pTurn;
	Soup *m_pSoup;
	char* m_stream;
	uint32_t m_SoupAuthOk;
}Rudpa;


u_int32_t get_now_usec1()
{
	struct timeval now;

	gettimeofday(&now, NULL); 
	return  now.tv_sec * 1000 + now.tv_usec / 1000;
}

struct test_nalu_header
{
	u_int32_t flag;
	u_int32_t size;    //数据大小
	u_int32_t isider; //1－是i-frame
};

typedef struct test_nalu_header NALU_HEADER_t;

Rudpa * CreateNewRudpa()
{
	Rudpa * pRet = (Rudpa *)malloc(sizeof(Rudpa));
	pRet->s = NULL;
	pRet->m_uiSessionStatus = 0;
	return pRet;
}

#include "sdk/sdk_api.h"
#include "media_buf.h"

static void *test_send_data(void *args)
{

	ERROR_CODE_t err;
	Rudpa * thiz = (Rudpa *)args;
	SESSION_t *session = thiz->s;

	
	void *data_buf = (void*)malloc(500*1024);
	if(NULL == data_buf)
	{
		_RUDPA_ERROR("malloc 264 frame buf failed\n");
	}							

	_RUDPA_STUB("Start send_data for session:%d\r\n",session->session_id);
	int mediabuf_inspeed = 0;
	_RUDPA_STUB("client want stream:%s\n",thiz->m_stream);
	int mediabuf_id = MEDIABUF_lookup_byname(thiz->m_stream);
	lpMEDIABUF_USER mediabuf_user = NULL;
	TRACE("mediabuf_id:%d\r\n",mediabuf_id);
	if(mediabuf_id >= 0)
	{
		mediabuf_inspeed = MEDIABUF_in_speed(mediabuf_id);
		TRACE("mediabuf_inspeed:%d\r\n",mediabuf_inspeed);
		if(mediabuf_inspeed > 0)
		{
			mediabuf_user = MEDIABUF_attach(mediabuf_id);
			TRACE("mediabuf_user:%p\r\n",mediabuf_user);
			if(mediabuf_user)
			{
				// all is ready
				for(;;)
				{
					bool out_success = false;
					if(1 != thiz->m_uiSessionStatus)
						break;
					if(0 == MEDIABUF_out_lock(mediabuf_user))
					{
						const lpSDK_ENC_BUF_ATTR attr = NULL;
						ssize_t out_size = 0;	
						if(0 == MEDIABUF_out(mediabuf_user, (void**)&attr, NULL, &out_size))
						{
							const void* raw_ptr = (void*)(attr + 1);
							size_t const raw_size = attr->data_sz;							
												
							ssize_t size = attr->data_sz;
							memcpy(data_buf+sizeof(SoupFrameHead), raw_ptr,raw_size);
							
							/*here pack the soup head*/

							thiz->m_pSoup->PackHead(data_buf,attr);
							size += sizeof(SoupFrameHead);

							err = SUCCESS;
							
							const void* payload = data_buf;

							out_success = true;
							if (0 == thiz->m_uiIp)
							{
								err = send_data(session, (char *)payload, size);
								_RUDPA_DEBUG("session_ID:%d\n",((SESSION_t*)session)->session_id);
							}
							else
							{
								if (attr->h264.keyframe)
								{
									err = send_data(session, (char *)payload, size);
								}
							}

							if(SUCCESS != err)
							{
								//	printf("send failed :%d \r\n",err);
							}							
						}
						MEDIABUF_out_unlock(mediabuf_user);

						if(!out_success){
							usleep(mediabuf_inspeed);
						}
					}
				}
				MEDIABUF_detach(mediabuf_user);
				_RUDPA_TRACE("detach will through\n");
			}
		}
	}
	free(data_buf);
	data_buf = NULL;
	thiz->m_uiSessionCanBeFree = 1;
	thiz->m_DataThreadStart_If = 0;
	printf("Session thread terminate\r\n");
	return NULL;
}

void ld_packet_proc(void *data,int packsize,void *priv)
{
	char *pSoupPkt;
	SoupData *soup_data = (SoupData*)calloc(sizeof(SoupData),1);
	Rudpa * thiz;
	SESSION_t *session;
	Soup *soup;
	if((thiz = priv) == NULL)	
		return;	
	if((session = thiz->s) == NULL)
		return;
	if((soup = thiz->m_pSoup) == NULL)
		return;
	_RUDPA_DEBUG("ld_packet_proc :get all the handle,can work now\n");

	LD_PACK_DATA_t 	*lpd = (LD_PACK_DATA_t*)data;

	//lpd->data,lpd->packsize
	soup->DataProc(lpd->data,lpd->packsize,soup_data);
	_RUDPA_DEBUG("soup_cmd:%d\n",soup_data->soup_cmd);
	switch(soup_data->soup_cmd)
	{
		case SoupCmdPtz:
			{
				pSoupPkt = PackSoupPkt(soup_data);
				_RUDPA_TRACE("SOUP>>>cmd:%d,%s\n",SoupCmdPtz,pSoupPkt);
				aside_send(thiz,pSoupPkt);
			}
			break;
		case SoupCmdSeekStream:
			{
				/*how to get the specify vin streaminfo,later ask law*/
				pSoupPkt = PackSoupPkt(soup_data);
				_RUDPA_STUB("SOUP>>>cmd:%d,%s\n",SoupCmdSeekStream,pSoupPkt);
				aside_send(thiz,pSoupPkt);
			}
			break;
		case SoupCmdReqStream:
			{		
					
				if(0 == strcmp("start",soup_data->streamreq_opt) 
				#if defined(SOUP_USR_AUTH)
				&& 0 != thiz->m_SoupAuthOk
				#endif
				)//
				{				      
					/*!=0 ,this is turn ,so select the lower stream,else send what they want*/
					if(0 != thiz->m_uiIp)
					{
						thiz->m_stream  = GetStreamName(1);
					}
					else
					{
						thiz->m_stream = GetStreamName(soup_data->streamreq_stream_No);
					}			
					if(0 == thiz->m_uiSessionStatus)
					{	
						_RUDPA_TRACE("new thread to send data,session ID:0x%x\n",session->session_id);				
						thiz->m_uiSessionStatus = 1;
						thiz->m_uiSessionCanBeFree = 0;
						pthread_t th;
						if (pthread_create(&th,NULL,test_send_data,thiz) != 0)
							return ;				
						pthread_detach(th);
						thiz->m_DataThreadStart_If = 1;
												
					}
				}
				else if(0 == strcmp("stop",soup_data->streamreq_opt)
				#if defined(SOUP_USR_AUTH)
				 && 0 != thiz->m_SoupAuthOk
				 #endif
				) 
				{
					/*stop send stream*/
					if(1 == thiz->m_uiSessionStatus)
					{
						_RUDPA_STUB("stop send stream session ID:0x%x\n",session->session_id);
						thiz->m_uiSessionStatus = 0;
					}
				}
			}
			break;
		case SoupCmdAuth:
			{
				_RUDPA_DEBUG("soup usr auth ::in\n");
				_RUDPA_DEBUG("usr:%s,psw:%s\n",soup_data->auth_usr,soup_data->auth_psw); 
				if(0 == strcmp(IPC_USR,soup_data->auth_usr) 
				    && 0 == strcmp(IPC_PSW, soup_data->auth_psw))
				{
					thiz->m_SoupAuthOk = !0;
					_RUDPA_STUB("here a bug?session addr:%x\n",session);
					_RUDPA_TRACE(" IPC USR_AUTH sucess,sessionID:0x%x!\n",session->session_id);
				}
				else
				{
					soup_data->soup_error = ES_PASSWD;
				}
				pSoupPkt = PackSoupPkt(soup_data);
				_RUDPA_DEBUG("SOUP>>>>cmd:%d,%s\n",SoupCmdPtz,pSoupPkt);			
				aside_send(thiz->s,pSoupPkt);
				_RUDPA_DEBUG("soup usr auth ::out\n");
			}
			break;
		case SoupCmdDevinfo:
			{
				_RUDPA_TRACE("max_cam_ch\n");
				soup_data->sd_camcnt = 1;
				pSoupPkt = PackSoupPkt(soup_data);
				aside_send(thiz->s,pSoupPkt);
			}
		default:
			break;
	}
	free(soup_data);
	soup_data = NULL;
	return ;
}

int event_proc(SESSION_HANDLE *s,SESSEION_EVENT_t e,void *data,int datasize,void  *priv)
{
	SESSION_t *session = (SESSION_t*)s;
	Rudpa * thiz = (Rudpa *)priv;
//	_RUDPA_DEBUG("sessionid :%x, event:%d\n",session->session_id,e);
	SEND_TIMEOUT_t 	*st;
	SEND_PRE_t		*sp;
	PRE_CREATE_PACK *pcp;
	switch (e)
	{
		case Event_Send_TimeOut:
			st = (SEND_TIMEOUT_t*)data;
			if(st->resendcount > 2)
				break;
			st->bcontinue = 1;
			_RUDPA_DEBUG("resend one package\n");
			break;

		case Event_Send_Pre:
			sp = (SEND_PRE_t*)data;
			break;

		case Event_Ld_Pack:			
			ld_packet_proc(data,datasize,thiz);	
			break;

		case Event_Pre_Create_Pack:
			pcp = (PRE_CREATE_PACK*)data;
			pcp->pre_buf_size = 0;
			break;

		case Event_Close_Session:	
			_RUDPA_TRACE(" \033[32msession 0x%x closed \n\033[0m",session->session_id);
		       /*stop send stream*/
//		       thiz->m_uiSessionStatus = 0;			  
			if(1 == thiz->m_uiSessionStatus )
			{
				thiz->m_uiSessionStatus = 0;
			}
			break;
		case Event_Finished_Session:
			if((thiz->m_DataThreadStart_If && thiz->m_uiSessionCanBeFree)
				||((0 == thiz->m_DataThreadStart_If) && session->timeout && !session->recv))
			{
				
				// Set the flag to release session
				FINISHED_SESSION_t * finish = (FINISHED_SESSION_t *)data;
				finish->finished = 1;
				// release the Rudpa object
				free(thiz);/*include the datathreadcrete flag*/
				thiz = NULL;
				printf("\033[31m Fineshed Session\n\033[0m");
			}
			break;
		default:
			break;
	}
	return 0;
}



int RecvProc(SESSIONLISTENER_HANDLE *l,SESSEION_EVENT_t e,void *data,int datasize,void *userdata)
{
	UnionProtocol *up = (UnionProtocol*)userdata;
	RECV_t *recv =(RECV_t*)data;
	_RUDPA_DEBUG("up:%p\n",up);

	_RUDPA_DEBUG("esee:%p,DataProc:%p\n",up->up_esee,up->up_esee->DataProc);
	_RUDPA_DEBUG("turn->DataProc :%p\n",up->up_turn->DataProc);

	Esee *esee = up->up_esee;	
	esee->DataProc( up,l,e,data,datasize);

	if (!recv->produced)
	{
		up->up_turn->DataProc(up,l,e,data,datasize);
	}

	return 0;
}

int SendPre(SESSION_HANDLE *s,SESSEION_EVENT_t e,void *data,int datasize,void  *priv)
{
	_RUDPA_DEBUG("Send pre>>>in\n");
	Rudpa * pRudpa = (Rudpa *)priv;
	pRudpa->m_pTurn->turn(pRudpa->m_pTurn,data,pRudpa->m_uiIp,pRudpa->m_uiPort);
	_RUDPA_DEBUG("Send pre>>>out\n");
	return 0;
}

int PreCreatePack(SESSION_HANDLE *s,SESSEION_EVENT_t e,void *data,int datasize,void  *priv)
{

	_RUDPA_DEBUG("PreCreatePack>>in\n");	

	Rudpa * pRudpa = (Rudpa *)priv;
	if (0 == pRudpa->m_uiIp || 0 == pRudpa->m_uiPort)
	{
		_RUDPA_DEBUG("No need to add ProtocolTurn head\n");
		_RUDPA_DEBUG("PreCreatePack>>out\n");		
		return 0;
	}
	pRudpa->m_pTurn->PreCreatePack(pRudpa->m_pTurn,data,datasize);	
	_RUDPA_DEBUG("PreCreatePack>>out\n");
	return 0;
}


int new_session_proc(SESSIONLISTENER_HANDLE *l,SESSEION_EVENT_t e,void *data,int datasize,void *userdata)
{
	_RUDPA_DEBUG("new_session_proc>>in\n");	

	UnionProtocol *up = (UnionProtocol*)userdata;
	Turn * turn = up->up_turn;
	Traversal *traversal = up->up_traversal;
	Soup *soup = up->up_soup;
	NEW_SESSION_t *pParam = (NEW_SESSION_t *)data;
	_RUDPA_TRACE("session addr:%p\n",pParam->session);

	Rudpa *newRudpa = CreateNewRudpa();
	newRudpa->m_uiSessionStatus = 0;
	newRudpa->s = pParam->session;
	newRudpa->m_uiSessionCanBeFree = 0;
	newRudpa->m_DataThreadStart_If =  0;
	newRudpa->m_uiIp = 0;
	newRudpa->m_uiPort = 0;
	newRudpa->m_SoupAuthOk = 0;
	newRudpa->m_stream = DEFAULT_STREAM;

	_RUDPA_DEBUG("ensure the rudpa:%p\t session:%p\n",newRudpa,newRudpa->s);

	newRudpa->m_pTurn = turn;
	turn->PopReq(turn,pParam->radom,&newRudpa->m_uiIp,&newRudpa->m_uiPort);/*here ip&port be zero ,ture?*/
	_RUDPA_DEBUG("After Turn Pop:%d %d %d\r\n",pParam->radom,newRudpa->m_uiIp,newRudpa->m_uiPort);

	/*pop the specify radom's clientinfo in traversal reqs,
	 *so the device not send hole to client any more*/
	traversal->PopReq(traversal,pParam->radom);

	newRudpa->m_pSoup = soup;

	set_event_proc(pParam->session,Event_Send_TimeOut,event_proc,newRudpa);
	set_event_proc(pParam->session,Event_Send_Pre,SendPre,newRudpa);
	set_event_proc(pParam->session,Event_Ld_Pack,event_proc,newRudpa);
	set_event_proc(pParam->session,Event_Pre_Create_Pack,PreCreatePack,newRudpa);
	set_event_proc(pParam->session,Event_Close_Session, event_proc,newRudpa);
	set_event_proc(pParam->session,Event_Finished_Session,event_proc,newRudpa);

	session_send_timeout(pParam->session,1000);
	session_sub_pack_send_timeout(pParam->session,500);
	
	_RUDPA_DEBUG("new_session_proc>>out\n");	

	return 0;
}

int RUDPA_init()
{

	_RUDPA_TRACE("Rudpa start\r\n");
	
	SESSIONLISTENER_HANDLE  *sh;
	char sn[32] = "JA";	
	if(!UCODE_check(UCODE_SN_MTD, -1)){
		UCODE_read(UCODE_SN_MTD, -1, &sn[2]);
	}
	strncpy(IPC_MAC,sn,32);
	printf("ESEE DEVICE ID:%s\r\n",IPC_MAC);
	if(0 == IPC_MAC[2] && 0 ==IPC_MAC[3])
	{
		_RUDPA_ERROR("NULL IPC ID,gonna kill RUDPA\n");
		return 0;
	}
	
	UnionProtocol  *uProtocol = (UnionProtocol*)calloc(sizeof(UnionProtocol),1);
	sh = create_session_listener(NULL,10201, new_session_proc, uProtocol);
	set_listener_event_proc(sh,LEvent_Recv,RecvProc,uProtocol);

	Esee *esee = CreateNewEsee(uProtocol);
	esee->SetListener(esee,sh);
	Traversal *traversal = 	CreateNewTraversal(uProtocol);
	Turn * turn = CreateNewTurn(uProtocol);
	turn->SetListener(turn,sh);	
	Soup *soup = CreateNewSoup(uProtocol);

	

	
		 

	// Just for test now,it will be called in the future while the  
	// esee server notifies that there is a new client going to 
	// connect this device .


	if (sh == NULL){
		return -1;
	}

	return 0;
}


void RUDPA_destroy()
{
	return;
}

