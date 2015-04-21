/*============================================================
 * Author:	Wang tsmyfau@gmail.com
 * Filename:		sipparse.c
 * Describle:
 * History: 
 * Last modified:	2013-06-28 20:23
=============================================================*/
#include <stdio.h>
#include <string.h>
#include "sipparse.h"
//"REGISTER sip:34020000002000000001@3402000000 SIP/2.0\r\n"
int sip_parse_ReqLine(pReqLine buf,char *sip_str)
{
	if(!sip_str)return -1;
	sscanf(sip_str,"%s%*[^:]:%[^@]@%s%*[^/]/%[^\r\n]",
			buf->cmd,buf->id,buf->realm,buf->version);
	return 0;
}

//SIP/2.0 200 OK
int sip_parse_EchoLine(pEchoLine buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *EchoLine_str = strstr(sip_str,"SIP");
	if(EchoLine_str)
		sscanf(sip_str,"%[^/]/%s%*[^0-9]%s%*[^a-zA-z0-9]%[^\r\n]",
				buf->protocol,buf->version,buf->msg_code,buf->msg);
	return 0;
}
//"Via: SIP/2.0/UDP 192.168.3.135:5060;rport=5060;branch=z9hG4bK229342462;received=192.168.3.75";
//"Via: SIP/2.0/UDP 192.168.2.12:60000;branch=z9hG4bK229342462\r\n"
int sip_parse_Via(pVia buf,char *sip_str)
{
	if(!sip_str)return -1;
	char *via_str = strstr(sip_str,"Via");
	if(via_str)
		sscanf(via_str,"%*[^/]/%[^/]/%s%*[^a-zA-Z0-9]%[^:]:%[^;]",
				buf->version,buf->sock_type,buf->src_ip,buf->src_port);

	char *rport = strstr(via_str,"rport=");
	if(rport)
		sscanf(rport,"rport=%[^;\r\n]",buf->rport);
	char *received = strstr(via_str,"received=");
	if(received)
		sscanf(received,"received=%[^;\r\n]",buf->received);
	char *branch = strstr(via_str,"branch=");
	if(branch)
		sscanf(branch,"branch=%[^;\r\n]",buf->branch);
	return 0;
}
//"From: <sip:34020000001180000002@3402000000>;tag=1601583377\r\n"
//"To: <sip:34020000001180000002@3402000000>\r\n"
int sip_parse_From(pFromTo buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *from_str = strstr(sip_str,"From");
	if(from_str)
		sscanf(from_str,"%*[^<]%*[^:]:%[^@]@%[^>]%*[^=]=%[^\r\n]",
				buf->id,buf->realm,buf->tag);
	return 0;
}

int sip_parse_To(pFromTo buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *to_str = strstr(sip_str,"To");
	if(to_str)
		sscanf(to_str,"%*[^<]%*[^:]:%[^@]@%[^>]%*[^=]=%[^\r\n]",
				buf->id,buf->realm,buf->tag);
	return 0;
}
//"Call-ID: 604961831\r\n"
//"CSeq: 3 REGISTER\r\n"
//"Max-Forwards: 70\r\n"
//"User-Agent: Embedded Net DVR/DVS\r\n"
//"Expires: 0\r\n"
//"Content-Length: 0\r\n";
int sip_parse_CallID(pCallID buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *callID_str = strstr(sip_str,"Call-ID");
	if(callID_str)  
		sscanf(callID_str,"%*[^:]%*[^a-zA-Z0-9]%[^\r\n]",
				buf->call_id);
	return 0;
}

int sip_parse_CSeq(pCSeq buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *CSeq_str = strstr(sip_str,"CSeq");
	if(CSeq_str) 
		sscanf(CSeq_str,"%*[^0-9]%s%*[^a-zA-Z0-9]%[^\r\n]",
				buf->CSeq,buf->cmd);
	return 0;
}
int sip_parse_MaxFwd(pMaxFwd buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *MaxFwd_str = strstr(sip_str,"Max-Forwards");
	if(MaxFwd_str)
		sscanf(MaxFwd_str,"%*[^0-9]%[^\r\n]",
				buf->max_forwards);
	return 0;
}
int sip_parse_UserAgent(pUserAgent buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *UserAgent_str = strstr(sip_str,"UserAgent");
	if(UserAgent_str)
		sscanf(UserAgent_str,"%*[^:]%*[^a-zA-Z0-9]%[^\r\n]",
				buf->user_agent);
	return 0;
}
int sip_parse_Expires(pExpires buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *Expires_str = strstr(sip_str,"Expires");
	if(Expires_str) 
		sscanf(Expires_str,"%*[^0-9]%[^\r\n]",
				buf->expires);
	return 0;
}
int sip_parse_ContentLen(pContentLen buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *ContentLen_str = strstr(sip_str,"Content-Length");
	if(ContentLen_str)
		sscanf(ContentLen_str,"%*[^0-9]%[^\r\n]",
				buf->content_len);
	return 0;
}
int sip_parse_ContentType(pContentType buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *ContentType_str = strstr(sip_str,"Content-Type");
	if(ContentType_str)
		sscanf(ContentType_str,"%*[^ ]%*[^a-zA-Z0-9]%[^\r\n]",
				buf->content_type);
	return 0;
}
//"Contact: <sip:34020000001180000002@192.168.2.12:60000>\r\n"
int sip_parse_Contact(pContact buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *Contact_str  = strstr(sip_str,"Contact");
	if(Contact_str)
		sscanf(Contact_str,"%*[^<]%*[^:]:%[^@]@%[^:]:%[^>]",
				buf->id,buf->ip,buf->port);
	return 0;
}

int sip_parse_WWW_Auth(pWWW_Auth buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *WWW_Auth_str = strstr(sip_str,"WWW-Authenticate");
	if(WWW_Auth_str)
		sscanf(WWW_Auth_str,"%*[^\"]\"%[^\"]\"%*[^\"]\"%[^\"]",buf->realm,buf->nonce);
	return 0;
}
//Date: 2013-06-17T16:49:57.409
int sip_parse_Date(pDate buf,char *sip_str)
{
	if(!sip_str) return -1;
	char *Date_str = strstr(sip_str,"Date");
	if(Date_str)
		sscanf(Date_str,"%*[^0-9]%d-%d-%dT%d:%d:%d.%d",&buf->YYYY,&buf->MM,&buf->DD,&buf->hh,&buf->mm,&buf->ss,&buf->mmm);
		return 0;
}

//Subject: 34020000001320000001:34020000001320000001,34020000004000000001:132420
int sip_parse_Subject(pSubject subject,char *sip_str)
{
	if(!sip_str)return -1;
	char *Subject_str = strstr(sip_str,"Subject");
	if(Subject_str)
		sscanf(Subject_str,"%*[^:]%*[^0-9]%[^:]:%[^,],%[^:]:%[^\r\n]",subject->send_sn,
															  subject->send_id,															 
															  subject->recv_id,
															  subject->recv_sn);
	return 0;		
}

int sip_parse_siphead(pSipHead sip_head,char *sip_str)
{
	if(!sip_str)return -1;
	memset(sip_head,0,sizeof(*sip_head));
	if(0 == strncmp(sip_str,"SIP",strlen("SIP"))){
		sip_parse_EchoLine(&sip_head->sh_EchoLine,sip_str);
	}else{
		sip_parse_ReqLine(&sip_head->sh_ReqLine,sip_str);
		}
	sip_parse_Via(&sip_head->sh_Via,sip_str);
	sip_parse_From(&sip_head->sh_From,sip_str);
	sip_parse_To(&sip_head->sh_To,sip_str);
	sip_parse_CallID(&sip_head->sh_CallID,sip_str);
	sip_parse_CSeq(&sip_head->sh_CSeq,sip_str);
	sip_parse_Contact(&sip_head->sh_Contact,sip_str);
	sip_parse_MaxFwd(&sip_head->sh_MaxFwd,sip_str);
	sip_parse_UserAgent(&sip_head->sh_UserAgent,sip_str);
	sip_parse_Expires(&sip_head->sh_Expires,sip_str);
	sip_parse_ContentLen(&sip_head->sh_ContentLen,sip_str);
	sip_parse_ContentType(&sip_head->sh_ContentType,sip_str);
	sip_parse_WWW_Auth(&sip_head->sh_WWW_Auth,sip_str);
	sip_parse_Date(&sip_head->sh_Date,sip_str);
	sip_parse_Subject(&sip_head->sh_Subject,sip_str);
	return 0;//well done the parse
}

/* a test for sip Register*/


int PrintAuth(pSipHead sip_head)
{
	char zero_area[16] = {0};
	if(0 != memcmp(&sip_head->sh_ReqLine,zero_area,16))
		printf("reqline info:cmd:%s,addr:%s@%s,version:%s\n",
				sip_head->sh_ReqLine.cmd,
				sip_head->sh_ReqLine.id,
				sip_head->sh_ReqLine.realm,
				sip_head->sh_ReqLine.version);
	if(0 != memcmp(&sip_head->sh_EchoLine,zero_area,16))
		printf("Echo info:%s/%s %s:%s\n",
				sip_head->sh_EchoLine.protocol,
				sip_head->sh_EchoLine.version,
				sip_head->sh_EchoLine.msg_code,
				sip_head->sh_EchoLine.msg);
	if(0 != memcmp(&sip_head->sh_Via,zero_area,16))
		printf("via info:%s/%s,addr:%s:%s,branch:%s\n",
				sip_head->sh_Via.version,
				sip_head->sh_Via.sock_type,
				sip_head->sh_Via.src_ip,
				sip_head->sh_Via.src_port,
				sip_head->sh_Via.branch);
	if(0 != memcmp(&sip_head->sh_From,zero_area,16))
		printf("From info:%s@%s,tag:%s\n",
				sip_head->sh_From.id,
				sip_head->sh_From.realm,
				sip_head->sh_From.tag);
	if(0 != memcmp(&sip_head->sh_To,zero_area,16))
		printf("To info:%s@%s,tag:%s\n",
				sip_head->sh_To.id,
				sip_head->sh_To.realm,
				sip_head->sh_To.tag);
	if(0 != memcmp(&sip_head->sh_CallID,zero_area,16))
		printf("callid:%s\n",sip_head->sh_CallID.call_id);

	if(0 != memcmp(&sip_head->sh_CSeq,zero_area,16))
		printf("Cseq:%s,%s\n",sip_head->sh_CSeq.CSeq,sip_head->sh_CSeq.cmd);

	if(0 != memcmp(&sip_head->sh_Contact,zero_area,16))
		printf("contact:%s@%s:%s\n",sip_head->sh_Contact.id,
				sip_head->sh_Contact.ip,
				sip_head->sh_Contact.port);
	if(0 != memcmp(&sip_head->sh_Contact,zero_area,16))
		printf("UserAgent:%s\n",sip_head->sh_UserAgent.user_agent);
	if(0 != memcmp(&sip_head->sh_MaxFwd,zero_area,16))
		printf("max-fwd:%s\n",sip_head->sh_MaxFwd.max_forwards);
	if(0 != memcmp(&sip_head->sh_Expires,zero_area,16))
		printf("expires:%s\n",sip_head->sh_Expires.expires);
	if(0 != memcmp(&sip_head->sh_ContentLen,zero_area,16))
		printf("content-len:%s\n",sip_head->sh_ContentLen.content_len);
	if(0 != memcmp(&sip_head->sh_WWW_Auth,zero_area,16))
		printf("www:%s by %s\n",sip_head->sh_WWW_Auth.realm,sip_head->sh_WWW_Auth.nonce);
	return 0;
}
