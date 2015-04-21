
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main()
{
	char tmp[512];
	char name[32];
	time_t t;
	setlinebuf(stdout);
	setlinebuf(stdin);
	setvbuf(stderr,(char *)NULL,_IONBF,0);
	
	//printf("Please input your user name: ");
	//fflush(stdout);
	//scanf("%s",name);
	//printf("welcome %s!!!!!!!!\n",name);
	while(1){
		fprintf(stderr,"ERR message!\t");
		time(&t);
		strftime(tmp, sizeof(tmp), "\033[38;1;32m abcdefghijklmnopqrstuvwxyz \033[0m %a, %b %d %Y %H:%M:%S GMT\r\n", localtime(&t));
		printf(tmp);
		//printf("abcdefghijklmnopqrtsuvwxyz  %u\r\n", time(NULL));
		usleep(100*1000);
	}
	return 0;
}
