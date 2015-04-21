
#ifndef __HLS_SEGMENTOR_H__
#define __HLS_SEGMENTOR_H__

#include <stdint.h>
#include <stdbool.h>

#include "mpegts.h"

#define MAX_HLS_SEGMENTOR_PACKET (10240)

typedef struct HLS_SEGMENTOR
{
	int frame_counter;
	int continuity_counter;

	MPEGTS_PACKET trans_packet[MAX_HLS_SEGMENTOR_PACKET];
	int n_trans_packet;
	
}HLS_SEGMENTOR_t;

typedef struct HLS_SEGMENT_TASK
{
	bool trigger;
	pthread_t tid;
	int buf_id;
	
}HLS_SEGMENT_TASK_t;

typedef struct HLS_SEGMENT_INFO
{
	float duration_s;
}HLS_SEGMENT_INFO_t;


#endif //__HLS_SEGMENTOR_H__
