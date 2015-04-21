
#ifndef __MT9D131_H__
#define __MT9D131_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sensor.h"
// bit8 i2c read /write
typedef int (*MT9D131_WRITE_FUNC)(uint8_t dev, uint8_t addr, uint8_t val);
typedef uint8_t (*MT9D131_READ_FUNC)(uint8_t dev, uint8_t addr);

extern int MT9D131_install(MT9D131_READ_FUNC rfunc, MT9D131_WRITE_FUNC wfunc);
extern int MT9D131_check();
extern int MT9D131_init(int freq);

extern void MT9D131_mirror_flip(unsigned char mode);
extern void MT9D131_test_mode(unsigned char enable);
extern void MT9D131_AWB(unsigned char mode);
extern void MT9D131_light_mode(unsigned char mode);
extern void MT9D131_set_hue(unsigned short val);
extern void MT9D131_set_saturation(unsigned char val);
extern unsigned char  MT9D131_get_saturation();
extern void MT9D131_set_brightness(unsigned char val);
extern void MT9D131_set_contrast(unsigned char val);
extern void MT9D131_set_exposure(unsigned char val);
extern void MT9D131_set_sharpness(unsigned char val);
extern void MT9D131_color_mode(unsigned char mode);
extern void MT9D131_reg_write(unsigned char addr,unsigned char val);
extern unsigned char  MT9D131_reg_read(unsigned char addr);
extern uint16_t SPEC_MT9D131_reg_read(unsigned char page, unsigned char addr);
extern void SPEC_MT9D131_reg_write(unsigned char page, unsigned char addr,uint16_t val);
extern ColorMaxValue MT9D131_get_color_max_value();
extern void MT9D131_set_shutter(unsigned char val);

#endif //__MT9D131_H__

