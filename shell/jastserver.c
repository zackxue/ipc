#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "jastdef.h"
#include "sock.h"
#include "vlog.h"
#include "jastlib.h"
#include "jastsession.h"
#include "jastserver.h"

static SOCK_t server_fd=-1;
static int m_toggle=true;
static unsigned short g_JastServerPort = JAST_SPORT_START;


void* JASTS_SEARCH_proc(void *para)
{
	int ret;
	fd_set read_set;
	struct timeval timeout;
	char dst_ip[20],mine_ip[20];
	int dst_port,mine_port;
	char buff[1024];
	char msg[1024];
	Jast_t *jast=NULL;
	JastSession_t *session=NULL;
	const char *format="JAST/1.0 200 OK\r\n"\
			"Location: %s:%d\r\n"\
			"\r\n";

	pthread_detach(pthread_self());
	printf("JAST server enter!!!!!!!!\n");

    server_fd=SOCK_udp_init(JAST_DISCOVERY_SPORT,JAST_SOCK_TIMEOUT);
	if(server_fd < JAST_RET_FAIL) return NULL;
#ifndef JAST_USE_BROADCAST
	if(SOCK_add_membership(server_fd,JAST_MULTICAST_ADDR) < JAST_RET_FAIL) return NULL;
	system("route add -net 224.0.0.0 netmask 224.0.0.0 dev eth0");
#endif
	do {
		FD_ZERO(&read_set);
		FD_SET(server_fd,&read_set);
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000*50;
		ret = select(server_fd + 1,&read_set,NULL,NULL,&timeout);
		if(ret < 0){
			printf("JAST server select failed!\n");
			return NULL;
		}else if(ret == 0){
			// timeout
			//printf("select timeout!!\n");
		}else{
			if(FD_ISSET(server_fd,&read_set) == true){
				ret = SOCK_recvfrom(server_fd,dst_ip,&dst_port,buff,sizeof(buff));
				if(ret < 0) return NULL;
				buff[ret]=0;
				VLOG(VLOG_DEBUG,"recv %d ok,recv data:\n%s\n",ret,buff);
				if(memcmp(buff,"DISCOVERY * JAST/1.0",strlen("DISCOVERY * JAST/1.0"))==0){
					printf("Discovery from %s:%d!\n",dst_ip,dst_port);
					// send response
					//SOCK_getsockname(server_fd,mine_ip);
					SOCK_gethostname(mine_ip);

					// init a jast session
					if((session=JAST_session_find(dst_ip)) == NULL){
						mine_port = g_JastServerPort;
						sprintf(msg,format,mine_ip,mine_port);
						g_JastServerPort++;
						//
						printf("add new session\n",dst_ip,dst_port);
						jast=JAST_server_init(dst_ip,mine_port);
						if(jast == NULL) return NULL;
						session=JAST_session_add(dst_ip,jast->sock,jast);
						if(session == NULL) return NULL;
					}else{
						jast = (Jast_t *)session->data.context;
						jast->bLogin = false;
						//
						sprintf(msg,format,jast->ip_me,jast->port_me);
						printf("session:%s exist!!!\n",dst_ip);
					}
					VLOG(VLOG_DEBUG2,"ack:\n%s\n",msg);
					ret = SOCK_sendto(server_fd,dst_ip,dst_port,msg,strlen(msg));
					if( ret == JAST_RET_FAIL) break;
				}
			}else{
				printf("JAST select:something wrong!\n");
				return NULL;
			}
		}
    } while (m_toggle);
	
	printf("JAST server exit!!!!!!!!\n");
	
    return (void *)1;
}

int JASTS_SEARCH_proc_start() 
{
	ThreadId_t tid;
	THREAD_create(tid,JASTS_SEARCH_proc,NULL);
    return 0;
}

void JASTS_SEARCH_proc_stop() 
{
	m_toggle = false;
}

