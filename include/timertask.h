
//
// a timer task module
//

#ifndef __TIMER_TASK_H__
#define __TIMER_TASK_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

typedef struct TollTickReckon
{
	struct timeval tv_pts;
}TollTickReckon_t;

extern TollTickReckon_t* TollTick_in();
extern void TollTick_out(TollTickReckon_t* reckon);

typedef void (*TTASK_CALLBACK)(time_t cur_time);

extern int TTASK_add(TTASK_CALLBACK task, int interleave); // interleave uint: sec must be greater than 1
extern int TTASK_remove(TTASK_CALLBACK task);

extern int TTASK_init();
extern void TTASK_destroy();
extern void TTASK_syn_time(time_t cur_time);

//#define _TEST_TIMERTASK
#ifdef _TEST_TIMERTASK
static void TASK_1sec(time_t cur_time)
{
	printf("task 1 second -> %s", ctime(&cur_time));
}

static void TASK_5sec(time_t cur_time)
{
	printf("task 5 second -> %s", ctime(&cur_time));
}

static void TASK_10sec(time_t cur_time)
{
	printf("task 10 second -> %s", ctime(&cur_time));
}

static void TASK_100sec(time_t cur_time)
{
	printf("task 100 second -> %s", ctime(&cur_time));
}

static void TASK_500sec(time_t cur_time)
{
	printf("task 500 second -> %s", ctime(&cur_time));
}

#include <signal.h>

static unsigned char bTimeTick = 1;
static void ouch(int sig)
{
	printf("Ouch!!\r\n");
	bTimeTick = 0;
}

int main(int argc, char** argv)
{
	(void)signal(SIGINT, ouch);
	TTASK_init();
	TTASK_add(TASK_1sec, 1);
	TTASK_add(TASK_5sec, 5);
	TTASK_add(TASK_10sec, 10);
	TTASK_add(TASK_100sec, 100);
	TTASK_add(TASK_500sec, 500);
	while(bTimeTick){
		usleep(10000);
	}
	TTASK_destroy();
	return 0;
}
#endif

#ifdef __cplusplus
};
#endif
#endif //__TIMER_TASK_H__

