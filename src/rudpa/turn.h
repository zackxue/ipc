#ifndef __TURN_PROTOCOL_HEAD_FILE__
#define __TURN_PROTOCOL_HEAD_FILE__
#include "rudp_session.h"
#include "esee.h"

#define PROTOCOLHEAD      (0x5455524E)
#define TURN_TIMEOUT 3

typedef enum _enTurnStatus{
	TS_IDLE,
	TS_TURNREQ,
	TS_WAIT_TURNREQ,
	TS_RUN,
}TurnStatus;

typedef enum _enTurnCmd{
	DS_Turn_Req = 0x1001,
	DS_Turn_Data,
	DS_Idle,

	SD_Client_Cs = 0x2001,
	SD_Invalide_Id,
	SD_No_Resouce,
	SD_Client_Ready,
}TrunCmd;

typedef enum _enTurnError{
	TE_SUCCESS,
	TE_HEADERROR,
	TE_CMDERROR,
}TurnError;

typedef struct _tagProtocolTurn{
	unsigned int uiHead;
	unsigned int uiCmd;
	unsigned int uiRadom;
	union {
		char sId[16];
		struct _tagClientCs{
			unsigned int uiIp;
			unsigned int uiPort;
			unsigned int uiTTL;
		}ClientCs;

		struct _tagTurnData{
			unsigned int uiIp;
			unsigned int uiPort;
		}TurnData;
		
		char cReverse[20];
	}_U;
}ProtocolTurn;

typedef struct _tag2Turn
{
	char *turnIp;
	char *turnPort;
	char *clientIp;
	char *clientPort;
	char *sId;	
}_2Turn;

typedef struct _tagCsReq{
	unsigned int uiRadom;
	unsigned int uiIp;
	unsigned int uiPort;
	unsigned int uiTTL;
	struct _tagCsReq * pNext;
}CsReq;

typedef struct _tagTurn{
	TurnStatus m_Status;
	SESSIONLISTENER_HANDLE *m_l;
	char m_sId[16];
	bool  b_LoginStatus;
	int m_retry;
	CsReq m_Reqs;
	CsReq *m_pReqCur;
	struct sockaddr_in turnServer;

	void *m_uProtocol;
	
	int (*SetListener)(struct _tagTurn *,SESSIONLISTENER_HANDLE *);
	int (*EventProc)(struct _tagTurn *,_2Turn *,TurnStatus);
	int (*DataProc)(void *,SESSIONLISTENER_HANDLE *,SESSEION_EVENT_t,void *,int);
	int (*PopReq)(struct _tagTurn *,unsigned int,unsigned int *,unsigned int *);
	int (*turn)(struct _tagTurn *, void *, unsigned int, unsigned int);
	int (*PreCreatePack)(struct _tagTurn *, void *, unsigned int);
}Turn;

Turn * CreateNewTurn(void*);

/*
 *struct sockaddr_in GetServerInfo();
 */

#endif
