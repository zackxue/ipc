
//
// a timer task module
//

#include "timertask.h"

#define DEBUG_TIMERTASK
#ifdef DEBUG_TIMERTASK
#define TTASK_TRACE(fmt...)	\
	do{printf("\033[0;37mTIMERTASK->[%s]:%d ", __FILE__, __LINE__);printf(fmt);printf("\033[m\r\n");}while(0)
#else
#define TTASK_TRACE(fmt...)
#endif
#define TIMERTASK_MAX_TASK (16)


typedef struct TIMER_TASK
{
	time_t setup_pts;
	
	uint32_t daemon_trigger;
	pthread_t daemon_tid;

	int task_cnt;
	int task_interleave[TIMERTASK_MAX_TASK];
	time_t task_time[TIMERTASK_MAX_TASK]; // mark task running time
	TTASK_CALLBACK task_callback[TIMERTASK_MAX_TASK]; // keep task function pointer
	
}TIMER_TASK_t;
static TIMER_TASK_t* _timer_task = NULL;


static void* ttask_daemon(void* arg)
{
	time_t last_time = 0;
	time_t cur_time = 0;
	while(_timer_task->daemon_trigger)
	{
		cur_time = time(NULL);
		if(cur_time < _timer_task->setup_pts){
			// FIXME: the cur_time must greater than setup_pts, if not, the system time has been adjusted, need to reset
		}
		// task management
		if(last_time != cur_time){
			int i = 0;
			// for earch task
			for(i = 0; i < TIMERTASK_MAX_TASK && _timer_task->daemon_trigger; ++i){
				TTASK_CALLBACK const task = _timer_task->task_callback[i];
				if(task){
					if(cur_time - _timer_task->task_time[i] >= _timer_task->task_interleave[i]){
						task(cur_time);
						// backup
						_timer_task->task_time[i] = cur_time;
					}
				}
			}
			last_time = cur_time;
		}
		usleep(100000);
	}
	pthread_exit(NULL);
}

static void ttask_daemon_start()
{
	if(!_timer_task->daemon_tid){
		int ret = 0;
		_timer_task->daemon_trigger = true;
		ret = pthread_create(&_timer_task->daemon_tid, 0, ttask_daemon, NULL);
		assert(0 == ret);
	}
}

static void ttask_daemon_stop()
{
	if(_timer_task->daemon_tid){
		_timer_task->daemon_trigger = false;
		pthread_join(_timer_task->daemon_tid, NULL);
		_timer_task->daemon_tid = (pthread_t)NULL;
	}
}

int TTASK_add(TTASK_CALLBACK task, int interleave) // interleave uint: sec must be greater than 1
{
	int i = 0;
	if(!task){
		return -1;
	}
	// scan one time to avoid duplicate task
	for(i = 0; i < TIMERTASK_MAX_TASK; ++i){
		if(_timer_task->task_callback[i] == task){
			TTASK_TRACE("duplicate task");
			return -1;
		}
	}
	for(i = 0; i < TIMERTASK_MAX_TASK; ++i){
		if(!_timer_task->task_callback[i]){
			// find 1 position
			_timer_task->task_interleave[i] = interleave > 0 ? interleave : 1;
			_timer_task->task_time[i] = 0;
			_timer_task->task_callback[i] = task;
			++_timer_task->task_cnt;
			TTASK_TRACE("add a task @ %d interval %d", i, _timer_task->task_interleave[i]);
			return 0;
		}
	}
	return -1;
}

int TTASK_remove(TTASK_CALLBACK task)
{
	int i = 0;
	for(i = 0; i < TIMERTASK_MAX_TASK; ++i){
		if(_timer_task->task_callback[i] == task){
			_timer_task->task_interleave[i] = 0;
			_timer_task->task_time[i] = 0;
			_timer_task->task_callback[i] = NULL;
			--_timer_task->task_cnt;
			TTASK_TRACE("remove a task @ %d", i);
			return 0;
		}
	}
	return -1;
}

int TTASK_init()
{
	if(!_timer_task){
		_timer_task = calloc(sizeof(TIMER_TASK_t), 1);
		// init	
		time_t cur_time;
		cur_time = time(NULL);
		
		if(cur_time < 0){//fix bug that the time before 1970/1/1 when boot sometimes
			struct timeval tv;
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			settimeofday(&tv,NULL);
			printf("set date:1970/1/1");
			cur_time = time(NULL);
		}		
		_timer_task->setup_pts = cur_time;
		// start daemon
		ttask_daemon_start();
		return 0;
	}
	return -1;
}

void TTASK_destroy()
{
	if(_timer_task){
		// stop daemon
		ttask_daemon_stop();

		free(_timer_task);
		_timer_task = NULL;
	}
}

TollTickReckon_t* TollTick_in()
{
	TollTickReckon_t* reckon = calloc(sizeof(TollTickReckon_t), 1);
	gettimeofday(&reckon->tv_pts, NULL);
	return reckon;
}

void TollTick_out(TollTickReckon_t* reckon)
{
	struct timeval tv_pts;
	struct timeval tv_diff;
	gettimeofday(&tv_pts, NULL);
	timersub(&tv_pts, &reckon->tv_pts, &tv_diff);
	printf("exhaust: %d.%06d\r\n", tv_diff.tv_sec, tv_diff.tv_usec);
	free(reckon);
}

void TTASK_syn_time(time_t cur_time)
{
//	printf("setup_pts:%d/%d\r\n", _timer_task->setup_pts, cur_time);
	_timer_task->setup_pts = cur_time;
	int i = 0;
	// for earch task
	for(i = 0; i < TIMERTASK_MAX_TASK && _timer_task->daemon_trigger; ++i){
//		printf("task_time[%d]:%d/%d\r\n",i, _timer_task->task_time[i], cur_time);
		_timer_task->task_time[i] = cur_time;
	}
}


