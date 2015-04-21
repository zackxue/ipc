/*============================================================
 * Author: Wang tsmyfau@gmail.com
 * Filename:		esee.h
 * Describle:
 * History: 
 * Last modified:	2013-03-18 14:48
 ============================================================*/
#ifndef ESEE_H
#define ESEE_H

#include "rudp_session.h"
#include "esee_protocol.h"


#define USE_OUTSIDE_ESEEPLAT 1 


#define ESEE_TURN_AUTH_TIMEOUT 5
#define ESEE_HEARTBEAT_TIMEOUT 30
#define ESEE_READYTURN_TIMEOUT 5

#define ESEE_TURN_AUTH_RE_CNT 5 
#define ESEE_HEARTBEAT_RE_CNT 5
#define ESEE_TURNREADY_RE_CNT 5


typedef enum _ErrorEsee{
	EE_SUCCESS,
	EE_HEADERROR,
	EE_CMDERROR,
}ErrorEsee;


typedef enum _EseeStatus
{
	ES_IDLE,
	ES_TURN_AUTH,
	ES_TURN_AUTH_ACK,
	ES_HEARTBEAT,
	ES_HEARTBEAT_ACK,
	ES_TURNREADY,
	ES_TURNREADY_ACK,	
	ES_CONFIRM_TRAVERSAL,	
}EseeStatus;

typedef struct _tag2Esee
{	
	/*left for other moudle to pass data to eventproc*/ 
	char *blah;
}_2Esee;



typedef struct _Esee{	
	SESSIONLISTENER_HANDLE *m_l;
	bool b_turnAuth;
	bool b_turn_ready;
	bool b_heartbeat;
	bool b_hole;
	int m_TurnAuthReCnt;
	int m_HeartbeatReCnt;
	int m_ReadyTurnReCnt;
	int m_HoleReCnt;
	int m_EseePlatInfoValid;/*when it NZ,the esee thread can send msg to plat,or can't*/
	char m_externIp[24];
	char m_externPort[8];/*store the externIp/Port,if changed the turn_auth will do again*/
	EseeStatus m_Status;
	struct sockaddr_in esee_server;
	struct sockaddr_in client_addr;
	void *m_uProtocol;

	int (*EventProc)(struct _Esee *thiz, _2Esee *pData,EseeStatus status);
	int (*SetListener)(struct _Esee *,SESSIONLISTENER_HANDLE *);
	int (*DataProc)(void* ,SESSIONLISTENER_HANDLE * ,SESSEION_EVENT_t  ,void * ,int );	
}Esee;


extern Esee* CreateNewEsee(void*);
extern int EseeSendTo(Esee * ,EseeCmd  ,struct sockaddr_in * );



#endif  /*end of  esee.h*/ 
