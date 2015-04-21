
#ifndef __AR0130_H__
#define __AR0130_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sensor.h"

extern void AR0130_mirror_flip(unsigned char mode);
extern void AR0130_test_mode(unsigned char enable);
extern void AR0130_AWB(unsigned char mode);
extern void AR0130_light_mode(unsigned char mode);
extern void AR0130_set_hue(unsigned short val);
extern void AR0130_set_saturation(unsigned char val);
extern unsigned char  AR0130_get_saturation();
extern void AR0130_set_brightness(unsigned char val);
extern void AR0130_set_contrast(unsigned char val);
extern void AR0130_set_exposure(unsigned char val);
extern void AR0130_set_sharpness(unsigned char val);
extern void AR0130_color_mode(unsigned char mode);
extern void AR0130_reg_write(unsigned char addr,unsigned char val);
extern unsigned char  AR0130_reg_read(unsigned char addr);
extern uint16_t SPEC_AR0130_reg_read(unsigned char page, unsigned char addr);
extern void SPEC_AR0130_reg_write(unsigned char page, unsigned char addr,uint16_t val);
extern ColorMaxValue AR0130_get_color_max_value();
extern void AR0130_set_shutter(unsigned char val);
extern void AR0130_ircut_auto_switch();

#endif //__AR0130_H__

