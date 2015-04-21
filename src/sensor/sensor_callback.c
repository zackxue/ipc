
#include "sdk_isp.h"

#include "sensor_callback.h"
#include "sensor.h"
#include "sysconf.h"

static ColorMaxValue gs_value_info;

void sensor_callback_mirror_flip(unsigned char mode)
{
	switch(mode){
		case MODE_MIRROR:
			SDK_ISP_set_mirror(0, SYS_TRUE);
			break;
		case MODE_UNMIRROR:
			SDK_ISP_set_mirror(0, SYS_FALSE);
			break;
		case MODE_FLIP:
			SDK_ISP_set_flip(0, SYS_TRUE);
			break;
		case MODE_UNFLIP:
			SDK_ISP_set_flip(0, SYS_FALSE);
			break;
		default:
		case MODE_NORMAL:
			SDK_ISP_set_flip(0, SYS_FALSE);
			SDK_ISP_set_mirror(0, SYS_FALSE);
			break;
	}	
}
void sensor_callback_test_mode(unsigned char enable){}
void sensor_callback_light_mode(unsigned char mode){}
void sensor_callback_set_hue(unsigned short val)
{
	if(gs_value_info.HueMax>= val){
		SDK_ISP_set_hue(0, val);
	}else{
		SDK_ISP_set_hue(0, gs_value_info.HueMax);
	}
}

void sensor_callback_set_saturation(unsigned char val)//val should be 0,1,2,3,4,5,6,7
{
	if(gs_value_info.SaturationMax >= val){
		SDK_ISP_set_saturation(0, val);
	}
}
unsigned char  sensor_callback_get_saturation()
{
#if 0
	unsigned char val;
	SDK_ISP_get_saturation(0, &val);
	return val;
#endif
}
void sensor_callback_set_brightness(unsigned char val)
{
	if(gs_value_info.BrightnessMax >= val){
		SDK_ISP_set_brightness(0, val);
	}
}
void sensor_callback_set_contrast(unsigned char val)
{
	if(gs_value_info.ContrastMax >= val){
		SDK_ISP_set_contrast(0, val);
	}	
}

void sensor_callback_color_mode(unsigned char mode)
{

}
void sensor_callback_reg_write(unsigned char addr,unsigned char val){}
unsigned char  sensor_callback_reg_read(unsigned char addr){}

void SPEC_sensor_callback_reg_write(unsigned char page, unsigned char addr,uint16_t val)
{

}
uint16_t SPEC_sensor_callback_reg_read(unsigned char page, unsigned char addr)
{

}

ColorMaxValue sensor_callback_get_color_max_value()
{
	gs_value_info.HueMax = 100;
	gs_value_info.SaturationMax = 100;
	gs_value_info.ContrastMax = 100;
	gs_value_info.BrightnessMax = 100;
	return gs_value_info;
}

void sensor_callback_set_shutter(unsigned char val)
{
	switch(val)
	{
		default:
		case SYS_VIN_DIGITAL_SHUTTER_50HZ:
			//SDK_ISP_set_src_framerate(25);
			SDK_ISP_sensor_flicker(1, 50);
			break;
		case SYS_VIN_DIGITAL_SHUTTER_60HZ:
			SDK_ISP_sensor_flicker(1, 60);
			//SDK_ISP_set_src_framerate(30);
			break;
	}
}

void sensor_callback_ircut_auto_switch(uint8_t type, uint8_t bEnable)//1:software   0: hardware
{
	if(bEnable){
		SDK_ISP_ircut_auto_switch(0, type);
	}
}

void sensor_callback_vi_flicker(uint8_t bEnable,uint8_t frequency)
{
	SDK_ISP_sensor_flicker(bEnable, frequency);
}

uint8_t sensor_callback_get_sharpen(void)
{
	uint8_t ret_val = 0;
	SDK_ISP_get_sharpen(&ret_val);
	return ret_val;
}

void sensor_callback_set_sharpen(uint8_t val)
{
	SDK_ISP_set_sharpen(val);
}

void sensor_callback_set_scene_mode(uint32_t mode)
{
	SDK_ISP_set_scene_mode(mode);
}

void sensor_callback_set_WB_mode(uint32_t mode)
{
	SDK_ISP_set_WB_mode(mode);
}

void sensor_callback_set_ircut_control_mode(uint32_t mode)
{
	SDK_ISP_set_ircut_control_mode(mode);
}

void sensor_callback_set_ircut_mode(uint32_t mode)
{
	SDK_ISP_set_ircut_mode(mode);
}

void sensor_callback_set_WDR_enable(uint8_t bEnable)
{
	SDK_ISP_set_WDR_enable(bEnable);
}

void sensor_callback_set_WDR_strength(uint8_t val)
{
	SDK_ISP_set_WDR_strength(val);
}

void sensor_callback_set_exposure_mode(uint32_t mode)
{
	SDK_ISP_set_exposure_mode(mode);
}

void sensor_callback_set_AEcompensation(uint8_t val)
{
	SDK_ISP_set_AEcompensation(val);
}

void sensor_callback_set_denoise_enable(uint8_t bEnable)
{
	SDK_ISP_set_denoise_enable(bEnable);
}

uint8_t sensor_callback_get_denoise_strength(void)
{
	uint8_t ret_val;
	SDK_ISP_get_denoise_strength(&ret_val);
	return ret_val;
}

void sensor_callback_set_denoise_strength(uint8_t val)
{
	SDK_ISP_set_denoise_strength(val);
}

void sensor_callback_set_anti_fog_enable(uint8_t bEnable)
{
	SDK_ISP_set_advance_anti_fog_enable(bEnable);
}

void sensor_callback_set_lowlight_enable(uint8_t bEnable)
{
	SDK_ISP_set_advance_lowlight_enable(bEnable);
}

void sensor_callback_set_gamma_table(uint8_t val)
{
	SDK_ISP_set_advance_gamma_table(val);
}

void sensor_callback_set_defect_pixel_enable(uint8_t bEnable)
{
	SDK_ISP_set_advance_defect_pixel_enable(bEnable);
}



