
#include "timertask.h"
#include "overlay.h"
#include "esee_client.h"
#include "sdk/sdk_api.h"
#include "generic.h"
#include "gpio.h"
#include "sensor.h"
#include "sysconf.h"
#include "app_overlay.h"
#include "media_buf.h"

static void ipcam_watchdog_autofeed(time_t cur_time)
{
	WATCHDOG_feed();
}

void get_rawdata()
{
	lpMEDIABUF_USER user;
	int mediabuf_ch = MEDIABUF_lookup_byname("720p.264"); 
	int success = 0; 
	if(mediabuf_ch >= 0){
		user = MEDIABUF_attach(mediabuf_ch);

		while(1){
			if(0 == MEDIABUF_out_lock(user)){
				void* send_ptr = 0;
				ssize_t send_sz = 0;
				if(0 == MEDIABUF_out(user, &send_ptr, NULL, &send_sz)){
					const lpSDK_ENC_BUF_ATTR const attr = (lpSDK_ENC_BUF_ATTR)send_ptr;
					printf("MEDIABUF_out:%s-%d    %d/%d\r\n", attr->h264.keyframe?"I":"P", send_sz,
						attr->h264.width, attr->h264.height);
					if(attr->h264.keyframe){
						printf("size :%d\r\n", send_sz - sizeof(stSDK_ENC_BUF_ATTR));
						FILE* file_fd = NULL;
						file_fd = fopen("/root/nfs/gm_ipc/rawdata", "wb+");
						fwrite(send_ptr + sizeof(stSDK_ENC_BUF_ATTR),
							send_sz - sizeof(stSDK_ENC_BUF_ATTR),1 , file_fd);
						// out unlock
						fclose(file_fd);						
						success = 1;
					}
				}
				MEDIABUF_out_unlock(user);
				if(success){
					break;
				}
			}
		}
	}
	if(user){
		MEDIABUF_detach(user);
		user = NULL;
	}
}

static void ipcam_eseeclient_report(time_t cur_time)
{
	ESEE_CLIENT_INFO_t info;
	if(0 == ESEE_CLIENT_get_info(&info)){
		/*
		int i = 0;
		printf("id = %s\r\n", info.id);
		printf("ip4 = %s\r\n", info.ip4);
		printf("heartbeat = %u\r\n", info.heartbeat_port);
		printf("port total map: %d\r\n", info.port_cnt);
		for(i = 0; i < info.port_cnt; ++i){
			printf("port(%d) = %u\r\n", i, info.port[i]);
		}*/
	}
}

static void ipcam_ircut_auto_switch(time_t cur_time)
{
	/*static unsigned char gpio_status_old = 1;//daytime
	unsigned char gpio_status_cur = IRCUT_READ_PIN;
	if(gpio_status_old != gpio_status_cur){
		gpio_status_old = gpio_status_cur;
		if(gpio_status_cur){			
			printf("daylight mode!\r\n");
			//SENSOR_color_mode(COLOR_MODE_NORMAL);
		}else{			
			printf("night mode!\r\n");
			//SENSOR_color_mode(COLOR_MODE_BW);
		}
	}*/
	SYSCONF_t *sysconf = SYSCONF_dup();
	SENSOR_ircut_auto_switch(sysconf->ipcam.isp.day_night_mode.ircut_control_mode, sysconf->ipcam.isp.day_night_mode.ircut_mode?0:1);
}

static int tody_snapshot(time_t cur_time)
{
	static int today_counter = 0;
	int ret = 0;
	char file_name[32] = {""};
	const char* const today_path = "/tmp/today/";
	struct tm cur_tm;
	FILE* fid = NULL;

	// get local time
	localtime_r(&cur_time, &cur_tm);

	if(0 == cur_tm.tm_min){
		// come on, snapshot!
		mkdir(today_path, 0);
		ret = snprintf(file_name, ARRAY_ITEM(file_name), "%s/%02d.jpg", today_path, today_counter);
		// remember to clear the last photo
		unlink(file_name);
		remove(file_name);
		// write a new file
		fid = fopen(file_name, "w+b");
		if(fid){
			sdk_enc->snapshot(0, kSDK_ENC_SNAPSHOT_QUALITY_HIGH, 640, 480, fid);
			today_counter++;
			today_counter %= 24;
			fclose(fid);
			fid = NULL;
		}
	}
	return 0;
}

extern uint8_t sdk_isp_calculate_exposure(uint32_t old_state);

extern int exposure_calculate(time_t cur_time);



int IPCAM_timer_init()
{
	TTASK_init();
	TTASK_add(ipcam_watchdog_autofeed, 5);
	TTASK_add(ipcam_eseeclient_report, 60);
	TTASK_add(ipcam_ircut_auto_switch, 5);
	//TTASK_add(cpu_get_status, 10);
	//TTASK_add(exposure_calculate, 2);
	//TTASK_add(tody_snapshot, 5);
	TTASK_add(APP_OVERLAY_task, 1);
	TTASK_add(APP_OVERLAY_id_display, 5);
	//TTASK_add(get_rawdata, 60);
	return 0;
}

void IPCAM_timer_destroy()
{
	TTASK_remove(APP_OVERLAY_task);
	//TTASK_remove(tody_snapshot);
	//TTASK_remove(exposure_calculate);
	//TTASK_remove(cpu_get_status);
	TTASK_remove(ipcam_ircut_auto_switch);
	TTASK_remove(ipcam_eseeclient_report);
	TTASK_remove(ipcam_watchdog_autofeed);
	TTASK_destroy();
}

