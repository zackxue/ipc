/*   extdrv/peripheral/dc/mt9v131.c
 *
 * Copyright (c) 2006 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 *
 * History:
 *     04-Apr-2006 create this file
 *
 */
#include "sdk/sdk_isp.h"

#include "ar0130.h"
#include "sensor.h"
#include "sysconf.h"

static ColorMaxValue gs_value_info;

void AR0130_mirror_flip(unsigned char mode)
{

}
void AR0130_test_mode(unsigned char enable){}
void AR0130_AWB(unsigned char mode){}
void AR0130_light_mode(unsigned char mode){}
void AR0130_set_hue(unsigned short val)
{
	if(gs_value_info.HueMax>= val){
		SDK_ISP_set_hue(0, val);
	}else{
		SDK_ISP_set_hue(0, gs_value_info.HueMax);
	}
}

void AR0130_set_saturation(unsigned char val)//val should be 0,1,2,3,4,5,6,7
{
	if(gs_value_info.SaturationMax >= val){
		SDK_ISP_set_saturation(0, val);
	}
}
unsigned char  AR0130_get_saturation()
{
#if 0
	unsigned char val;
	SDK_ISP_get_saturation(0, &val);
	return val;
#endif
}
void AR0130_set_brightness(unsigned char val)
{
	if(gs_value_info.BrightnessMax >= val){
		SDK_ISP_set_brightness(0, val);
	}
}
void AR0130_set_contrast(unsigned char val)
{
	if(gs_value_info.ContrastMax >= val){
		SDK_ISP_set_contrast(0, val);
	}	
}

void AR0130_set_exposure(unsigned char val){}
void AR0130_set_sharpness(unsigned char val){}
void AR0130_color_mode(unsigned char mode)
{

}
void AR0130_reg_write(unsigned char addr,unsigned char val){}
unsigned char  AR0130_reg_read(unsigned char addr){}

void SPEC_AR0130_reg_write(unsigned char page, unsigned char addr,uint16_t val)
{

}
uint16_t SPEC_AR0130_reg_read(unsigned char page, unsigned char addr)
{

}

ColorMaxValue AR0130_get_color_max_value()
{
	gs_value_info.HueMax = 100;
	gs_value_info.SaturationMax = 100;
	gs_value_info.ContrastMax= 100;
	gs_value_info.BrightnessMax= 100;

	return gs_value_info;
}

void AR0130_set_shutter(unsigned char val)
{

}

void AR0130_ircut_auto_switch()
{
	SDK_ISP_ircut_auto_switch(0);
}

