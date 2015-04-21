
#ifndef __OV2643_H__
#define __OV2643_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensor.h"

// bit8 i2c read /write
typedef int (*OV2643_WRITE_FUNC)(uint8_t dev, uint8_t addr, uint8_t val);
typedef uint8_t (*OV2643_READ_FUNC)(uint8_t dev, uint8_t addr);

extern int OV2643_install(OV2643_READ_FUNC rfunc, OV2643_WRITE_FUNC wfunc);
extern int OV2643_check();
extern int OV2643_init(int freq);


extern void OV2643_mirror_flip(unsigned char mode);
extern void OV2643_test_mode(unsigned char enable);
extern void OV2643_AWB(unsigned char mode);
extern void OV2643_light_mode(unsigned char mode);
extern void OV2643_set_hue(unsigned short val);
extern void OV2643_set_saturation(unsigned char val);
extern unsigned char  OV2643_get_saturation();
extern void OV2643_set_brightness(unsigned char val);
extern void OV2643_set_contrast(unsigned char val);
extern void OV2643_set_exposure(unsigned char val);
extern void OV2643_set_sharpness(unsigned char val);
extern void OV2643_color_mode(unsigned char mode);
extern void OV2643_reg_write(unsigned char addr,unsigned char val);
extern unsigned char  OV2643_reg_read(unsigned char addr);
extern uint16_t SPEC_OV2643_reg_read(unsigned char page, unsigned char addr);
extern void SPEC_OV2643_reg_write(unsigned char page, unsigned char addr,uint16_t val);
extern ColorMaxValue OV2643_get_color_max_value();
extern void OV2643_set_shutter(unsigned char val);


#endif //__OV2643_H__

