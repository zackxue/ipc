#ifndef __OWSP_IMP_H__
#define __OWSP_IMP_H__
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "owsp_def.h"
//#include "conf.h"


typedef struct {
    int  Sock;      //Socket hHandle
    //int  Stat;      //Flag to Sign Context Working Status
    int  PackCnt;   //Package Send Count
    int  Chnl;      //Channel Number
    int *pNum;      //Current Connections
} OWSP_CONX;

typedef struct {
    unsigned int  iChnl;    //Channel ID
    unsigned int  iFrmCnt;  //Frame Order Counter
    unsigned int  iType;    //Frame Type "I" "P" "B"
    unsigned int  tStmp;    //Time Stamp in ms
} OWSP_FRMI;

#define OWSP_RESPONSE_LEN (sizeof(OwspPacketHeader)+sizeof(TLV_HEADER)+512)
#define OWSP_DBG printf

#define OWSP_HUASHIDA_P2P_COMPANY_IDENTITY       "84389BBE7DF3F074"
#define OWSP_JIUANGUANGDIAN_P2P_COMPANY_IDENTITY "3E652CE333436ACD"
#define OWSP_TONGLI_P2P_COMPANY_IDENTITY         "6AFD88D85450BD22"
#define OWSP_DEFAULT_IDENTITY                    OWSP_TONGLI_P2P_COMPANY_IDENTITY

int  OWSP_Sock_InitParam(int Socket);
int  OWSP_SendPacket(OWSP_CONX *pConX,   //Package & Send Packet
    int nType, char Buff[], char pPack[], int nSize);
int  OWSP_Pack_Create(int nType, int iPackNum, char Buff[], char pPack[], int nSize);
int  OWSP_VFrm_Create(OWSP_FRMI * pFrmI, int iPackNum, char Buff[], char pPack[], int nSize);

int  OWSP_Mob_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Dvs_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Ver_Info_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Usr_Login_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Chn_Switch_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Ptz_Ctrl_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int  OWSP_Chn_Suspend_ReqProc(OWSP_CONX *pConX, char Buff[], int Size);
int OWSP_SendStreamDataFormat(OWSP_CONX _owsp, TLV_V_StreamDataFormat _StrmInfo);


#endif //#ifndef __OWSP_IMP_H__
