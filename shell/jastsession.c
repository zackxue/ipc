#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "jastdef.h"
#include "jastsession.h"

JastSession_t *g_SessionTable=NULL;

int JAST_session_init()
{
	g_SessionTable = (JastSession_t *)calloc(1,sizeof(JastSession_t));
	if(g_SessionTable == NULL){
		printf("ERR: JAST_session_init failed!\n");
		return -1;
	}
	g_SessionTable->data.entries = 0;
	g_SessionTable->next = NULL;
	return 0;
}

JastSession_t *JAST_session_add(char *ip_dst,int sock,void *context)
{
	JastSession_t *p=g_SessionTable;

	JastSession_t *s = (JastSession_t *)calloc(1,sizeof(JastSession_t));
	if(s == NULL){
		printf("ERR: JAST_session_add failed");
		return NULL;
	}
	strcpy(s->data.ip,ip_dst);
	s->data.context = context;
	s->data.fd = sock;
	s->next = NULL;
	//
	while(p->next) p = p->next;
	p->next = s;
	g_SessionTable->data.entries ++;

	JAST_session_dump();
	
	return s;
}

int JAST_session_del(char *ip_dst)
{
	int focus = false;
	JastSession_t *q=g_SessionTable;
	JastSession_t *p=g_SessionTable->next;
	while(p){
		if(strcmp(p->data.ip,ip_dst) == 0){
			focus = true;
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
	g_SessionTable->data.entries --;
	JAST_session_dump();
	return 0;
}

JastSession_t* JAST_session_find(char *ip_dst)
{
	int focus = false;
	JastSession_t *p=g_SessionTable->next;
	while(p){
		if(strcmp(p->data.ip,ip_dst) == 0){
			focus = true;
			break;
		}
		p = p->next;
	}
	if(focus){
		printf("find session:%s sussess!\n",ip_dst);
		return p;
	}else{
		printf("find session:%s failed!\n",ip_dst);
		return NULL;
	}
}

int JAST_session_entries()
{
	return g_SessionTable->data.entries;
}


JastSession_t* JAST_session_get(int index)
{
	int i;
	int focus = false;
	JastSession_t *p=g_SessionTable;
	if(index <=0) return NULL;
	if(index > g_SessionTable->data.entries) return NULL;
	
	for(i=0;i<index;i++) p=p->next;
	return p;
}

int JAST_session_destroy()
{
	JastSession_t *p=g_SessionTable,*q;
	while(p){
		q = p->next;
		free(p);
		p = q;
	}
	return 0;
}

int JAST_session_dump()
{
	int i=0;
	JastSession_t *p=g_SessionTable->next;
	printf("total session entries:%d\n",g_SessionTable->data.entries);
	while(p){
		i++;
		printf("session%d ip:%s sock:%d \n",i,p->data.ip,p->data.fd);
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
		"***********jast session DEBUG *******************\r\n"\
		"*********** 0 quit ******************************\r\n"\
		"*********** 1 insert ****************************\r\n"\
		"*********** 2 delete ****************************\r\n"\
		"*********** 3 print all *************************\r\n"\
		"*********** 4 get data **************************\r\n";
	JastSession_t *s;
	char msg[1024];
	int sock;

	JAST_session_init();
	while(1){
		printf(usage);
		printf("input your command: ");
		scanf("%d",&cmd);
		switch(cmd){
			case 0:
				JAST_session_destroy();
				exit(1);
			case 1:
				printf("input pos and ip & sock: ");
				fflush(stdout);
				scanf("%d %s %d",&pos,msg,&sock);
				JAST_session_add(msg,sock,NULL);
				JAST_session_dump();
				break;
			case 2:
				printf("input delete ip: ");
				scanf("%s",msg);
				JAST_session_del(msg);
				JAST_session_dump();
				break;
			case 3:
				JAST_session_dump();
				break;
			case 4:
				printf("input pos: ");
				scanf("%d",&pos);
				s = JAST_session_get(pos);
				printf("get data:%s %d\n",s->data.ip,s->data.fd);
				break;
			default:
				break;
		}
		printf("\r\n");
	}

	return 0;
}
#endif

