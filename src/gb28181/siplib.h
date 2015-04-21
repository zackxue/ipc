/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	siplib.h
 * Describle:
 * History: 
 * Last modified: 2013-06-28 20:39
 =============================================================*/

#ifndef __SIPLIB_H__
#define __SIPLIB_H__

#include "sipparse.h"
#ifdef __cplusplus
extern "C"{
#endif

enum SIP_MSG_TYPE
{
	MSG_AUTH,
	MSG_RE_AUTH,
	MSG_HEARTBEAT,
	MSG_200,
	MSG_CATALOG,
	MSG_INVITE_200,
	MSG_DEV_CTRL,
	MSG_DEV_INFO,
	MSG_DEV_STATUS,
	MSG_ALARM,
};

typedef struct _SipEnv
{
	char name[32];
	char value[64];
	struct _SipEnv *next;
}sSipEnv,*pSipEnv;

enum AuthStatus
{
	AUTH_S_REG,
	AUTH_S_401,
	AUTH_S_REG_AGAIN,
	AUTH_S_200,
	AUTH_S_SUCCESS,
};

enum SipDevCtrlCmd
{
	/*ptz */
	SIP_DEV_CTRL_RIGHT = 0x01,
	SIP_DEV_CTRL_LEFT = 0x02,
	SIP_DEV_CTRL_DOWN = 0x04,
	SIP_DEV_CTRL_UP = 0x08,
	SIP_DEV_CTRL_ZOOM_IN = 0x10,
	SIP_DEV_CTRL_ZOOM_OUT = 0x20,
	SIP_DEV_CTRL_PTZ_STOP = 0x00,
	/*record,alarm,etc*/
	SIP_DEV_CTRL_RECORD_START = 0x80,
	SIP_DEV_CTRL_RECORD_STOP ,
	SIP_DEV_CTRL_GUARD_SET,
	SIP_DEV_CTRL_GUARD_RESET,
	SIP_DEV_CTRL_ALARM_RESET,
	SIP_DEV_CTRL_TELEBOOT,
};

typedef struct _SipSession
{
	int sock;
	int auth_status;
	int login_out;//0 logout or NZ login,
	int online;   //0 offline or NZ online,
	int alarm_status; //alarm: 0 offduty 1 onduty  else ALARM
	char auth_key[512];
	int heartbeat_time;
	char heartbeat_callid[128];//diff the 200 is whose
	int run_msg_loop;   //0 stop msg_loop,NZ start msg_loop
	pSipEnv env;
	sSipHead send_siphead;//store the siphead info
	sSipHead recv_siphead;
	char *xml_str;//point to the recvdata's xml section

	int trigger;

	char recvbuf[2048];
	char sendbuf[2048];
	struct sockaddr_in server;

// method of SipSession
	int (*auth)(struct _SipSession* s,int login);
	int (*pack_msg)(struct _SipSession* s,int MsgType,void *data);
	int (*ptz_cmd_parse)(struct _SipSession* s,char *cmd);
	int (*get_subcmd)(struct _SipSession* s,char *xml_str,char *sub_cmd,char *cmd);
	char *(*get_env)(pSipEnv env,char *name);

	char *(*get_tagvalue)(char *xml_str, char*name);
	char *(*get_systime)(void);
	int (*destroy)(struct _SipSession* s);

//dev ctrl ,alarm,record etc event calback
	int (*ptz_ctrl)(int cmd,char speed);
	int (*dev_ctrl)(int cmd,char *arg);
}sSipSession,*pSipSession;
//
extern int SIP_ENV_insert(pSipEnv env,char *name,char *value);
extern int SIP_ENV_edit(pSipEnv env,char *name,char *value);

extern int SIP_ENV_destroy(pSipEnv env);
extern int SIP_ENV_print(pSipEnv env);
char *xml_get_tagvalue(char *xml_str,char *name);



//
extern int sip_init_session(pSipSession s);




#ifdef __cplusplus
}
#endif

#endif //end of the siplib.h
