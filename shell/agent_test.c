

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "shell_agent.h"



int main(int argc, char *argv[])
{
#define MAXLINE	1024
	pid_t child_pid = -1;
	char message[MAXLINE] = {""};
	int pipe_shell2app[2] = {0};	
	int pipe_app2shell[2] = {0};
	
	// create two pipes
	if((pipe(pipe_shell2app) < 0) || (pipe(pipe_app2shell) < 0)){
		printf("create pipe failed!\n");
		exit(1);
	}

	if((child_pid = fork()) > 0){
		// self process
		int wait_status = 0;
		struct timeval timeout;
		fd_set read_set;
		int ret;
		int max_fd=0;

		// close one direct of each pipe
		close(pipe_shell2app[0]);
		close(pipe_app2shell[1]);

		printf("parent: %s\r\n", "haha");
		fprintf(stderr,"parent: haha ERR!");

		// loop to send out the
		while(1){
			timeout.tv_usec = 5*1000;
			timeout.tv_sec = 0;
			FD_ZERO(&read_set);
			FD_SET(0,&read_set);
			FD_SET(pipe_app2shell[0],&read_set);
			max_fd = 0;
			if(pipe_app2shell[0] > max_fd) max_fd = pipe_app2shell[0];
			ret = select(max_fd+1,&read_set,NULL,NULL,&timeout);
			if(ret < 0){
				printf("select failed\n");
				break;
			}else if(ret == 0){
				//printf("select timeout\n");
				continue;
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
					fprintf(stderr,"parent readed %d: %s", ret,message);
				}
			}
		}

		waitpid(child_pid, &wait_status, 0);

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
		execl("./trace_test","./trace_test",NULL);
	}else{
		// error
		printf("fork failed,errno:%s\n",strerror(errno));
	}

	return 0;
}
