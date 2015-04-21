#include "unistruct_gfun.h"
#include "sensor.h"
#include "sysconf.h"
#include "sysconf_type.h"
#include "timertask.h"
#include "overlay.h"
#include "generic.h"
#include "bsp/rtc.h"


extern CompareInfo_t g_compare_info;

void proc_module_reboot()
{
	if(g_compare_info.network){
		printf("%s-%d-------network changed!\r\n", __FUNCTION__, __LINE__);
		exit(0);
	}else if(g_compare_info.video){
		printf("%s-%d-------video changed!\r\n", __FUNCTION__, __LINE__);
		exit(0);
	}else if(g_compare_info.time){
		printf("%s-%d-------time changed!\r\n", __FUNCTION__, __LINE__);
		//time_t cur_time = time(NULL);
		//TTASK_syn_time(cur_time);
		//exit(0);
	}
	else if(g_compare_info.reboot){
		printf("%s-%d-------system reboot!\r\n", __FUNCTION__, __LINE__);
		system("reboot");
//		exit(0);
	}
	else if(g_compare_info.default_factory){
		printf("%s-%d-------default factory!\r\n", __FUNCTION__, __LINE__);
		SYSCONF_default_factory();
		exit(0);
	}
	else if(g_compare_info.screen){
		//SENSOR_set_sysconf();
		g_compare_info.screen = 0;
	}
}


UNISTRUCT_DO_CHECK do_check_saturation(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	if(val.max >= val.val){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_POLICY do_policy_saturation(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	SENSOR_saturation_set(val.val);
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_CHECK do_check_contrast(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	if(val.max >= val.val){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_POLICY do_policy_contrast(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	printf("%s %d-%d\r\n", __FUNCTION__, __LINE__, val.val);
	SENSOR_contrast_set(val.val);
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_CHECK do_check_hue(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	if(val.max >= val.val){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_POLICY do_policy_hue(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	printf("%s %d-%d\r\n", __FUNCTION__, __LINE__, val.val);
	SENSOR_hue_set(val.val);
	g_compare_info.screen = 1;
	return 0;
}


UNISTRUCT_DO_CHECK do_check_brightness(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	if(val.max >= val.val){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_POLICY do_policy_brightness(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	SENSOR_brightness_set(val.val);
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_network(void *val_ptr, void *arg)
{
	g_compare_info.network = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_system_operation(void *val_ptr, void *arg)
{
	SYS_MAP_t map_ptr;
	memcpy(&map_ptr, arg, sizeof(SYS_MAP_t));
	switch(map_ptr.val)
	{
		case SYS_OPERATION_REBOOT:
			{
				g_compare_info.reboot = 1;
			}
			break;
		case SYS_OPERATION_DEFAULT_FACTORY:
			{
				g_compare_info.default_factory = 1;
			}
			break;
	}
	return 0;
}

/*UNISTRUCT_DO_CHECK do_check_bps(void *val_ptr, void *arg)
{
	if(){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_CHECK do_check_fps(void *val_ptr, void *arg)
{
	if(){
		return 0;
	}else{
		return -1;
	}
}*/


UNISTRUCT_DO_POLICY do_policy_video(void *val_ptr, void *arg)
{
	g_compare_info.video = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_time(void *val_ptr, void *arg)
{
	time_t cur_time, *timet;
	struct timeval tv;
	timet = (time_t *)arg;
	tv.tv_sec = *timet;
	tv.tv_usec = 0;
	cur_time = time(NULL);
	// default to GMT+8
	//struct timezone tz;
	//tz.tz_minuteswest = -1 * 8 * 60;
	//tz.tz_dsttime = 0;
	//settimeofday(NULL,&tz);
	struct tm cur_tm;
	localtime_r(&tv.tv_sec, &cur_tm);
	printf("%s-timt_t:%d\r\n", __FUNCTION__, tv.tv_sec);
	printf("set time:%04d/%02d/%02d %02d:%02d:%02d\r\n", cur_tm.tm_year + 1900, cur_tm.tm_mon + 1, cur_tm.tm_mday,
		cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);
	settimeofday(&tv,NULL);
	RTC_settime(tv.tv_sec);
	system("date");
	system("date -u");
	printf("cur_time:%d/%d", cur_time, *timet);
	TTASK_syn_time(tv.tv_sec);
	/*if(cur_time > *timet){
		g_compare_info.time = 1;
		//TTASK_syn_time(cur_time);
	}else{
		g_compare_info.time = 0;
	}*/
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_channel_name(void *val_ptr, void *arg)
{
	APP_OVERLAY_set_title(0, arg);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_flip(void *val_ptr, void *arg)
{
	if(*(unsigned char*)arg){
		SENSOR_mirror_flip(MODE_FLIP);
	}
	else{
		SENSOR_mirror_flip(MODE_UNFLIP);
	}
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_mirror(void *val_ptr, void *arg)
{
	if(*(unsigned char*)arg){
		SENSOR_mirror_flip(MODE_MIRROR);
	}
	else{
		SENSOR_mirror_flip(MODE_UNMIRROR);
	}
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_timezone(void *val_ptr, void *arg)
{
	if(arg){
		SYS_RANGE_S8_t timezone;
		memcpy(&timezone, arg, sizeof(SYS_RANGE_S8_t));
		printf("set timezone %d\r\n", timezone.val);
		TIMEZONE_SYNC((int)timezone.val);
	}
	return 0;
}

UNISTRUCT_DO_CHECK do_check_sharpen(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	if(val.max >= val.val){
		return 0;
	}else{
		return -1;
	}
}

UNISTRUCT_DO_POLICY do_policy_sharpen(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));
	printf("%s-%d\r\n", __FUNCTION__, val.val);
	SENSOR_sharpen_set(val.val);
	g_compare_info.screen = 1;
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_scene_mode(void *val_ptr, void *arg)
{
	uint32_t mode = *(uint32_t *)arg;
	SENSOR_scene_mode_set(mode);
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_white_balance_mode(void *val_ptr, void *arg)
{
	uint32_t mode = *(uint32_t *)arg;
	SENSOR_WB_mode_set(mode);
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_ircut_control_mode(void *val_ptr, void *arg)
{
	uint32_t mode = *(uint32_t *)arg;
	SENSOR_ircut_control_mode_set(mode);
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_ircut_mode(void *val_ptr, void *arg)
{
	uint32_t mode = *(uint32_t *)arg;
	SENSOR_ircut_mode_set(mode);
	printf("%s:%d\r\n", __FUNCTION__, mode);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_wdr_enable(void *val_ptr, void *arg)
{
	uint8_t bEnable = *(uint8_t *)arg;
	SENSOR_WDR_mode_enable(bEnable);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_wdr_strength(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	//SENSOR_WDR_strength_set(val.val);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_exposure_mode(void *val_ptr, void *arg)
{
	uint32_t mode = *(uint32_t *)arg;
	SENSOR_exposure_mode_set(mode);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_exposure_ae_compensation(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	//SENSOR_AEcompensation_set(val.val);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_denoise_enable(void *val_ptr, void *arg)
{
	uint8_t bEnable = *(uint8_t *)arg;
	SENSOR_denoise_enable(bEnable);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_denoise_strength(void *val_ptr, void *arg)
{
	SYS_RATE8_t val;	
	memcpy(&val, arg, sizeof(SYS_RATE8_t));	
	SENSOR_denoise_strength(val.val);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_anti_fog_enable(void *val_ptr, void *arg)
{
	uint8_t bEnable = *(uint8_t *)arg;
	SENSOR_anti_fog_enable(bEnable);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_lowlight_enable(void *val_ptr, void *arg)
{
	uint8_t bEnable = *(uint8_t *)arg;
	//SENSOR_lowlight_enable(bEnable);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_gamma(void *val_ptr, void *arg)
{
	uint8_t val = *(uint8_t *)arg;
	//SENSOR_gamma_table_set(val);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

UNISTRUCT_DO_POLICY do_policy_isp_defect_pixel_enable(void *val_ptr, void *arg)
{
	uint8_t bEnable = *(uint8_t *)arg;
	//SENSOR_defect_pixel_enable(bEnable);
	printf("%s\r\n", __FUNCTION__);
	return 0;
}

















