
#ifndef __SENSOR_CALLBACK_H__
#define __SENSOR_CALLBACK_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sensor.h"

extern void sensor_callback_mirror_flip(unsigned char mode);
extern void sensor_callback_test_mode(unsigned char enable);
extern void sensor_callback_AWB(unsigned char mode);
extern void sensor_callback_light_mode(unsigned char mode);
extern void sensor_callback_set_hue(unsigned short val);
extern void sensor_callback_set_saturation(unsigned char val);
extern unsigned char  sensor_callback_get_saturation();
extern void sensor_callback_set_brightness(unsigned char val);
extern void sensor_callback_set_contrast(unsigned char val);
extern void sensor_callback_set_exposure(unsigned char val);
extern void sensor_callback_set_sharpness(unsigned char val);
extern void sensor_callback_color_mode(unsigned char mode);
extern void sensor_callback_reg_write(unsigned char addr,unsigned char val);
extern unsigned char  sensor_callback_reg_read(unsigned char addr);
extern uint16_t SPEC_sensor_callback_reg_read(unsigned char page, unsigned char addr);
extern void SPEC_sensor_callback_reg_write(unsigned char page, unsigned char addr,uint16_t val);
extern ColorMaxValue sensor_callback_get_color_max_value();
extern void sensor_callback_set_shutter(unsigned char val);
extern void sensor_callback_ircut_auto_switch(uint8_t type, uint8_t bEnable);
extern void sensor_callback_vi_flicker(uint8_t bEnable,uint8_t frequency);
extern uint8_t sensor_callback_get_sharpen(void);
extern void sensor_callback_set_sharpen(uint8_t val);
extern void sensor_callback_set_scene_mode(uint32_t mode);
extern void sensor_callback_set_WB_mode(uint32_t mode);
extern void sensor_callback_set_ircut_control_mode(uint32_t mode);
extern void sensor_callback_set_ircut_mode(uint32_t mode);
extern void sensor_callback_set_WDR_enable(uint8_t bEnable);
extern void sensor_callback_set_WDR_strength(uint8_t val);
extern void sensor_callback_set_exposure_mode(uint32_t mode);
extern void sensor_callback_set_AEcompensation(uint8_t val);
extern void sensor_callback_set_denoise_enable(uint8_t bEnable);
extern void sensor_callback_set_denoise_strength(uint8_t val);
extern uint8_t sensor_callback_get_denoise_strength(void);
extern void sensor_callback_set_anti_fog_enable(uint8_t bEnable);
extern void sensor_callback_set_lowlight_enable(uint8_t bEnable);
extern void sensor_callback_set_gamma_table(uint8_t val);
extern void sensor_callback_set_defect_pixel_enable(uint8_t bEnable);


#endif //__SENSOR_CALLBACK_H__

