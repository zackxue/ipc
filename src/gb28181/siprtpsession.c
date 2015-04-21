#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "siprtpsession.h"
#include "gb28181debug.h"

SipRtpSession_t *g_SipRtpSessionTable=NULL;

int SIPRTP_session_init()
{
	g_SipRtpSessionTable = (SipRtpSession_t *)calloc(1,sizeof(SipRtpSession_t));
	if(g_SipRtpSessionTable == NULL){
		printf("ERR: SIPRTP_session_init failed!\n");
		return -1;
	}
	g_SipRtpSessionTable->data.entries = 0;
	g_SipRtpSessionTable->next = NULL;
	return 0;
}

SipRtpSession_t *SIPRTP_session_add(char *id,int port,void *context,void *data)
{
	SipRtpSession_t *p=g_SipRtpSessionTable;

	SipRtpSession_t *s = (SipRtpSession_t *)calloc(1,sizeof(SipRtpSession_t));
	if(s == NULL){
		printf("ERR: SIPRTP_session_add failed");
		return NULL;
	}
	strcpy(s->data.id,id);
	s->data.port = port;
	s->data.data = data;
	s->data.flag = FALSE;
	s->data.context = context;
	s->next = NULL;
	//
	while(p->next) p = p->next;
	p->next = s;
	g_SipRtpSessionTable->data.entries++;

	SIPRTP_session_dump();
	
	return s;
}

int SIPRTP_session_set_context(SipRtpSession_t *s,void *context)
{
	s->data.context = context;
	s->data.flag = TRUE;
	return 0;
}

int SIPRTP_session_toggle(SipRtpSession_t *s,int flag)
{
	s->data.flag = flag;
	return 0;
}


int SIPRTP_session_del(char *id)
{
	int focus = FALSE;
	SipRtpSession_t *q=g_SipRtpSessionTable;
	SipRtpSession_t *p=g_SipRtpSessionTable->next;
	while(p){
		if(strcmp(p->data.id,id) == 0){
			focus = TRUE;
			break;
		}
		q = p;
		p = p->next;
	}
	if(focus){
		q->next = p->next;
		free(p);
		p = NULL;
	}
	g_SipRtpSessionTable->data.entries--;
	SIPRTP_session_dump();
	return 0;
}

SipRtpSession_t* SIPRTP_session_find(char *id)
{
	int focus = FALSE;
	SipRtpSession_t *p=g_SipRtpSessionTable->next;
	while(p){
		if(strcmp(p->data.id,id) == 0){
			focus = TRUE;
			break;
		}
		p = p->next;
	}
	if(focus){
		printf("find session:%s sussess!\n",id);
		return p;
	}else{
		printf("find session:%s failed!\n",id);
		return NULL;
	}
}

int SIPRTP_session_entries()
{
	return g_SipRtpSessionTable->data.entries;
}


SipRtpSession_t* SIPRTP_session_get(int index)
{
	int i;
	SipRtpSession_t *p=g_SipRtpSessionTable;
	if(index <=0) return NULL;
	if(index > g_SipRtpSessionTable->data.entries) return NULL;
	
	for(i=0;i<index;i++) p=p->next;
	return p;
}

int SIPRTP_session_destroy()
{
	SipRtpSession_t *p=g_SipRtpSessionTable,*q;
	while(p){
		q = p->next;
		free(p);
		p = q;
	}
	return 0;
}

int SIPRTP_session_dump()
{
	int i=0;
	SipRtpSession_t *p=g_SipRtpSessionTable->next;
	printf("total session entries:%d\n",g_SipRtpSessionTable->data.entries);
	while(p){
		i++;
		printf("session%d ip:%s \n",i,p->data.id);
		p = p->next;
	}
	return 0;
}

#ifdef __TEST
int main(int argc,char *argv[])
{
	int cmd = 0;
	int pos = 0;
	const char *usage=
		"*************************************************\r\n"\
		"***********sip rtp session DEBUG ****************\r\n"\
		"*********** 0 quit ******************************\r\n"\
		"*********** 1 insert ****************************\r\n"\
		"*********** 2 delete ****************************\r\n"\
		"*********** 3 print all *************************\r\n"\
		"*********** 4 get data **************************\r\n";
	SipRtpSession_t *s;
	char msg[1024];
	int sock;

	SIPRTP_session_init();
	while(1){
		printf(usage);
		printf("input your command: ");
		scanf("%d",&cmd);
		switch(cmd){
			case 0:
				SIPRTP_session_destroy();
				exit(1);
			case 1:
				printf("input pos and id: ");
				fflush(stdout);
				scanf("%d %s",&pos,msg);
				SIPRTP_session_add(msg,NULL);
				SIPRTP_session_dump();
				break;
			case 2:
				printf("input delete id: ");
				scanf("%s",msg);
				SIPRTP_session_del(msg);
				SIPRTP_session_dump();
				break;
			case 3:
				SIPRTP_session_dump();
				break;
			case 4:
				printf("input pos: ");
				scanf("%d",&pos);
				s = SIPRTP_session_get(pos);
				printf("get data:%s\n",s->data.id);
				break;
			default:
				break;
		}
		printf("\r\n");
	}

	return 0;
}
#endif

