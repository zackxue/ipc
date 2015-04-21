#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "shell_agent.h"
#include "jastlib.h"
#include "jastsession.h"

void exit_done()
{
	int i;
	JastSession_t *s = NULL;
	Jast_t *j =NULL;
	
	for(i=1;i<=JAST_session_entries();i++){
		if((s=JAST_session_get(i)) == JAST_RET_FAIL) break;
		j = (Jast_t *)s->data.context;
		JAST_destroy(j);
	}
	JAST_session_destroy();
	
	printf("do something cleaning!!!!\n");
}

static void signal_handle(int sign_no) 
{
    printf("shell agent kill!\n");
	exit(0);
}

int main(int argc, char *argv[])
{
#define MAXLINE	1024
	pid_t child_pid = -1;
	char message[MAXLINE] = {""};
	int pipe_shell2app[2] = {0};	
	int pipe_app2shell[2] = {0};
	MillisecondTimer_t t_tmp;
	MilliSecond_t t_lasttime;

	if(argc != 2){
		printf("usage: shellagent [app]\n");
		return 0;
	}
	if(atexit(exit_done)!=0){
		printf("register exit funcition failed!\n");
		return 0;
	}
	signal(SIGINT,signal_handle);
	
	// create two pipes
	if((pipe(pipe_shell2app) < 0) || (pipe(pipe_app2shell) < 0)){
		printf("create pipe failed!\n");
		exit(1);
	}

	if((child_pid = fork()) > 0){
		// self process
		int i;
		int wait_status = 0;
		struct timeval timeout;
		fd_set read_set;
		int ret;
		int max_fd=0;
		Jast_t *j=NULL;
		JastSession_t *s=NULL;
		char from_ip[20];
		int from_port;
		int trigger = true;

		// close one direct of each pipe
		close(pipe_shell2app[0]);
		close(pipe_app2shell[1]);

		printf("parent: %s\r\n", "haha");
		fprintf(stderr,"parent: haha ERR!");

		// start the UDP search proc
		
		if(JAST_session_init()==JAST_RET_FAIL) return -1;
		JASTS_SEARCH_proc_start();
		
		// loop to send out the
		while(trigger){
			timeout.tv_usec = 5*1000;
			timeout.tv_sec = 0;
			FD_ZERO(&read_set);
			FD_SET(0,&read_set);
			FD_SET(pipe_app2shell[0],&read_set);
			max_fd = 0;
			if(pipe_app2shell[0] > max_fd) max_fd = pipe_app2shell[0];
			for(i=1;i<=JAST_session_entries();i++){
				if((s=JAST_session_get(i)) == JAST_RET_FAIL) break;
				FD_SET(s->data.fd,&read_set);
				if(s->data.fd > max_fd) max_fd = s->data.fd;
			}
			ret = select(max_fd+1,&read_set,NULL,NULL,&timeout);
			if(ret < 0){
				printf("select failed\n");
				break;
			}else if(ret == 0){
				//printf("select timeout\n");
				//continue;
			}else{
				if(FD_ISSET(0,&read_set)){
					if(fgets(message,sizeof(message),stdin) != NULL){
						if(write(pipe_shell2app[1],message,strlen(message)) != strlen(message)){
							printf("write to pipe failed!\n");
							break;
						}
					}else{
						printf("fgets failed!\n");
					}
				}else if(FD_ISSET(pipe_app2shell[0],&read_set)){
					ret = read(pipe_app2shell[0],message,sizeof(message));
					if(ret < 0){
						printf("read failed,err:%s",strerror(errno));
						break;
					}else if(ret == 0){
						continue;
					}
					message[ret] = 0;
					//printf("parent readed %d: %s", ret,message);
					// send stdout
					for(i=1;i<=JAST_session_entries();i++){
						if((s=JAST_session_get(i)) == JAST_RET_FAIL) trigger = false;
						j=(Jast_t *)s->data.context;
						if(j->bLogin == true){
							if(JAST_send_stdout(j,message,ret) == JAST_RET_FAIL){
								JAST_destroy(j);
								JAST_session_del(s->data.ip);
							}
						}
					}
				}else{
					for(i=1;i<=JAST_session_entries();i++){
						if((s=JAST_session_get(i)) == JAST_RET_FAIL) trigger=false;
						if(FD_ISSET(s->data.fd,&read_set)){
							if(JAST_read_message(s->data.context,from_ip,&from_port)==JAST_RET_FAIL)
								trigger = false;
							if(JAST_parse_request(s->data.context,from_ip,from_port) == JAST_RET_FAIL)
								break;
						}
					}
				}
			}
			// send hearbreak			
			for(i=1;i<=JAST_session_entries();i++){
				if((s=JAST_session_get(i)) == JAST_RET_FAIL) break;
				j=(Jast_t *)s->data.context;
				MilliTimerStop(j->hb_timer,t_tmp,t_lasttime);
				if((j->bLogin == true) && (t_lasttime >=JAST_HEARTBREAK_TIME)){
					if(JAST_send_heartbreak(j) == JAST_RET_FAIL){
						JAST_destroy(j);
						JAST_session_del(s->data.ip);
					}
					MilliTimerStart(j->hb_timer);
				}
			}
		}

		waitpid(child_pid, &wait_status, 0);

		// send out the core and appliction binary

		// clean up the agent

	}else if(0 == child_pid){
		// child process
		// close one direct of each pipe
		close(pipe_shell2app[1]);
		close(pipe_app2shell[0]);

		// redirect stderr to pipe
		dup2(pipe_app2shell[1], 2);
		//close(pipe_app2shell[1]);
		// redirect stdout to pipe
		dup2(pipe_app2shell[1], 1);
		close(pipe_app2shell[1]);

		// redirect stdin
		dup2(pipe_shell2app[0],0);
		close(pipe_shell2app[0]);
		
		// startup the vitrual child process
		execl(argv[1],argv[1],NULL);
	}else{
		// error
		printf("fork failed,errno:%s\n",strerror(errno));
	}

	return 0;
}
