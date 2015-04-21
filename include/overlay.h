

#ifndef __OVERLAY_H__
#define __OVERLAY_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum{
	OVERLAY_HIDE = 0,
	OVERLAY_SHOW,
	OVERLAY_CTRL_CNT,
};

extern void OVERLAY_update_time(time_t update_time);
extern void OVERLAY_update_channel_name(char * name_str);
extern void OVERLAY_control(int region, uint32_t mode);//show/hide

//extern void OVERLAY_update_status();

extern int OVERLAY_init();
extern void OVERLAY_destroy();

#endif //__OVERLAY_H__

