/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:	sipparse.h
 * Describle:
 * History: 
 * Last modified: 2013-06-28 20:23
=============================================================*/
#ifndef __SIPPARSE_H__
#define __SIPPARSE_H__

/*the basic section in sip protcol*/
typedef struct _ReqLine
{
	char cmd[16];
	char id[32];
	char realm[128];
	char version[32];
}sReqLine,*pReqLine;

typedef struct _EchoLine
{
	char protocol[16];
	char version[16];
	char msg_code[16];
	char msg[64];
}sEchoLine,*pEchoLine;

typedef struct _Via
{
	char version[16];
	char sock_type[16];
	char src_ip[32];
	char src_port[16];
	char received[32];
	char rport[16];
	char branch[128];
}sVia,*pVia;

typedef struct _FromTo
{
	char id[128];
	char realm[128];
	char tag[128];
}sFromTo,*pFromTo;

typedef struct _CallID
{
	char call_id[128];
}sCallID,*pCallID;

typedef struct _CSeq
{
	char CSeq[16];
	char cmd[32];
}sCSeq,*pCSeq;

typedef struct _Contact
{
	char id[32];
	char ip[32];
	char port[16];
}sContact,*pContact;

typedef struct _MaxFwd
{
	char max_forwards[16];
}sMaxFwd,*pMaxFwd;

typedef struct _UserAgent
{
	char user_agent[128];
}sUserAgent,*pUserAgent;

typedef struct _ContentLen
{
	char content_len[16];
}sContentLen,*pContentLen;

typedef struct _ContentType
{
	char content_type[32];
}sContentType,*pContentType;

typedef struct _Expires
{
	char expires[16];
}sExpires,*pExpires;

typedef struct _WWW_Auth
{
	char realm[128];
	char nonce[128];
}sWWW_Auth,*pWWW_Auth;

typedef struct _Authorization
{
	char username[32];//often the id
	char realm[128];
	char nonce[128];
	char uri[128];
	char response[128];
	char algorithm[16];
}sAuthorization,*pAuthorization;

typedef struct _Date
{
	int YYYY;
	int MM;
	int DD;
	int hh;
	int mm;
	int ss;
	int mmm;
}sDate,*pDate;

typedef struct _Subject
{
	char send_sn[32];
	char send_id[32];
	char recv_id[32];
	char recv_sn[32];
}sSubject,*pSubject;

typedef struct _SipHead
{
	sReqLine  sh_ReqLine;
	sEchoLine sh_EchoLine;
	sVia      sh_Via;
	sFromTo   sh_From;
	sFromTo   sh_To;
	sCallID   sh_CallID;
	sCSeq     sh_CSeq;
	sContact  sh_Contact;
	sMaxFwd   sh_MaxFwd;
	sUserAgent sh_UserAgent;
	sContentLen sh_ContentLen;
	sContentType sh_ContentType;
	sExpires  sh_Expires;
	sWWW_Auth sh_WWW_Auth;
	sAuthorization sh_Authorization;
	sDate     sh_Date;
	sSubject sh_Subject;
	/*.......*/

}sSipHead,*pSipHead;

#ifdef __cplusplus
extern "C"{
#endif

int sip_parse_siphead(pSipHead auth,char *sip_str);
int PrintAuth(pSipHead auth);

#ifdef __cplusplus
}
#endif

#endif //endof the sipparse.h
