
#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>

//AWB
#define AWB_SIMPLE		0
#define AWB_ADVANCED	1
//mirror  or flip
enum{
	MODE_NORMAL,
	MODE_MIRROR,
	MODE_FLIP,
	MODE_MIRROR_FLIP,
	MODE_UNMIRROR,
	MODE_UNFLIP,
	MODE_END
};
//Light mode
enum{
	LIGHT_MODE_AUTO,
	LIGHT_MODE_SUNNY,
	LIGHT_MODE_CLOUDY,
	LIGHT_MODE_OFFICE,
	LIGHT_MODE_HOME,
	LIGHT_MODE_END
};
//color mode
enum{
	COLOR_MODE_NORMAL,
	COLOR_MODE_ANTIQUE,
	COLOR_MODE_BLUISH,
	COLOR_MODE_GREENISH,
	COLOR_MODE_REDDISH,
	COLOR_MODE_BW,
	COLOR_MODE_NEGATIVE,
	COLOR_MODE_END
};

typedef struct _color_max_value
{
	unsigned short HueMax;
	unsigned short SaturationMax;
	unsigned short ContrastMax;
	unsigned short BrightnessMax;
}ColorMaxValue;


extern int SENSOR_init();
extern void SENSOR_destroy();

extern void SENSOR_mirror_flip(uint8_t mode);
extern void SENSOR_hue_set(uint16_t val);
extern void SENSOR_contrast_set(uint8_t val);
extern void SENSOR_brightness_set(uint8_t val);
extern void SENSOR_saturation_set(uint8_t val);
extern void SENSOR_lightmode_set(uint8_t mode);
extern void SENSOR_test_mode(uint8_t enable);
extern void SENSOR_color_mode(uint8_t mode);
extern void SENSOR_reg_write(uint8_t addr,uint8_t val);
extern uint8_t SENSOR_reg_read(uint8_t addr);
extern void SENSOR_spec_reg_write(uint8_t page, uint8_t addr, uint16_t val);
extern uint16_t SENSOR_spec_reg_read(uint8_t page,uint8_t addr);
extern ColorMaxValue SENSOR_get_color_max_value();
extern void SENSOR_shutter_set(uint8_t val);
extern void SENSOR_set_sysconf();
extern void SENSOR_ircut_auto_switch(uint8_t type, uint8_t bEnable);
extern void SENSOR_vi_flicker(uint8_t bEnable, uint8_t frequency);
extern void SENSOR_sharpen_set(uint8_t val);
extern void SENSOR_scene_mode_set(uint32_t mode);
extern void SENSOR_WB_mode_set(uint32_t mode);
extern void SENSOR_ircut_control_mode_set(uint32_t mode);
extern void SENSOR_ircut_mode_set(uint32_t mode);
extern void SENSOR_WDR_mode_enable(uint8_t bEnable);
extern void SENSOR_WDR_strength_set(uint8_t val);
extern void SENSOR_exposure_mode_set(uint32_t mode);
extern void SENSOR_AEcompensation_set(uint8_t val);
extern void SENSOR_denoise_3d_enable(uint8_t bEnable);
extern void SENSOR_denoise_3d_strength(uint8_t val);
extern void SENSOR_anti_fog_enable(uint8_t bEnable);
extern void SENSOR_lowlight_enable(uint8_t bEnable);
extern void SENSOR_gamma_table_set(uint8_t val);
extern void SENSOR_defect_pixel_enable(uint8_t bEnable);
#endif //__SENSOR_H__

