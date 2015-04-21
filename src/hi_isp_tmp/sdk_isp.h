
#ifndef __SDK_ISP_H__
#define __SDK_ISP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>
#include "sdk_api_def.h"

typedef enum SDK_ISP_SENSOR_MODEL
{
	SDK_ISP_SENSOR_MODEL_APINA_AR0130 = 0,
	SDK_ISP_SENSOR_MODEL_OV_OV9712,
	SDK_ISP_SENSOR_MODEL_SOI_H22,
	SDK_ISP_SENSOR_MODEL_SONY_IMX122,
	SDK_ISP_SENSOR_MODEL_SONY_IMX104,
}SDK_ISP_SENSOR_MODEL_t;

typedef struct SDK_ISP_SENSOR_APINA_AR0130
{
	SDK_ISP_SENSOR_MODEL_t model; // must be 'SDK_ISP_SENSOR_MODEL_APINA_AR0130'
	// i2c op
	int (*do_i2c_read)(uint16_t addr, uint16_t* ret_data);
	int (*do_i2c_write)(uint16_t addr, uint16_t data);

}SDK_ISP_SENSOR_APINA_AR0130_t;

typedef struct SDK_ISP_SENSOR_SONY_ICX692
{
	SDK_ISP_SENSOR_MODEL_t model; // must be 'SDK_ISP_SENSOR_MODEL_SONY_ICX692'
}SDK_ISP_SENSOR_SONY_ICX692_t;

typedef struct SDK_ISP_SENSOR_SONY_IMX104
{
	SDK_ISP_SENSOR_MODEL_t model; // must be 'SDK_ISP_SENSOR_MODEL_SONY_IMX104'
}SDK_ISP_SENSOR_SONY_IMX104_t;


typedef union SDK_ISP_SENSOR_SET
{
	SDK_ISP_SENSOR_APINA_AR0130_t aptina_ar0130;
	SDK_ISP_SENSOR_SONY_ICX692_t sony_icx692;
	SDK_ISP_SENSOR_SONY_IMX104_t sony_imx104;

}SDK_ISP_SENSOR_SET_t;

#define ISP_SCENE_MODE_AUTO (0)
#define ISP_SCENE_MODE_INDOOR (1)
#define ISP_SCENE_MODE_OUTDOOR (2)

#define ISP_IRCUT_CONTROL_MODE_HARDWARE (0)
#define ISP_IRCUT_CONTROL_MODE_SOFTWARE (1)

#define ISP_IRCUT_MODE_AUTO (0)
#define ISP_IRCUT_MODE_DAYLIGHT (1)
#define ISP_IRCUT_MODE_NIGHT (2)

#define ISP_EXPOSURE_MODE_AUTO (0)
#define ISP_EXPOSURE_MODE_BRIGHT (1)
#define ISP_EXPOSURE_MODE_DARK (2)

#define ISP_ADVANCE_GAMMA_DEFAULT (0)
#define ISP_ADVANCE_GAMMA_NORMAL (1)
#define ISP_ADVANCE_GAMMA_HIGH (2)


extern SDK_API_t SDK_ISP_set_mirror(int vin, bool mirror);
extern SDK_API_t SDK_ISP_set_flip(int vin, bool flip);
extern SDK_API_t SDK_ISP_ircut_auto_switch(int vin, uint8_t type);
extern SDK_API_t SDK_ISP_set_saturation(int vin, uint16_t val);
extern SDK_API_t SDK_ISP_get_saturation(int vin, uint16_t *val);
extern SDK_API_t SDK_ISP_set_contrast(int vin, uint16_t val);
extern SDK_API_t SDK_ISP_set_hue(int vin, uint16_t val);
extern SDK_API_t SDK_ISP_set_brightness(int vin, uint16_t val);
extern SDK_API_t SDK_ISP_sensor_flicker(uint8_t bEnable, uint8_t frequency);
extern SDK_API_t SDK_ISP_set_src_framerate(unsigned int framerate);
extern SDK_API_t SDK_ISP_set_sharpen(uint8_t val);
extern SDK_API_t SDK_ISP_get_sharpen(uint8_t *val);
extern SDK_API_t SDK_ISP_set_scene_mode(uint32_t mode);
extern SDK_API_t SDK_ISP_set_WB_mode(uint32_t mode);
extern SDK_API_t SDK_ISP_set_ircut_control_mode(uint32_t mode);
extern SDK_API_t SDK_ISP_set_ircut_mode(uint32_t mode);
extern SDK_API_t SDK_ISP_set_WDR_enable(uint8_t bEnable);
extern SDK_API_t SDK_ISP_set_WDR_strength(uint8_t val);
extern SDK_API_t SDK_ISP_set_exposure_mode(uint32_t mode);
extern SDK_API_t SDK_ISP_set_AEcompensation(uint8_t val);
extern SDK_API_t SDK_ISP_set_denoise_enable(uint8_t bEnable);
extern SDK_API_t SDK_ISP_set_denoise_strength(uint8_t val);
extern SDK_API_t SDK_ISP_get_denoise_strength(uint8_t *val);
extern SDK_API_t SDK_ISP_set_advance_anti_fog_enable(uint8_t bEnable);
extern SDK_API_t SDK_ISP_set_advance_lowlight_enable(uint8_t bEnable);
extern SDK_API_t SDK_ISP_set_advance_gamma_table(uint8_t val);
extern SDK_API_t SDK_ISP_set_advance_defect_pixel_enable(uint8_t bEnable);
extern SDK_API_t SDK_ISP_set_isp_default_value(int sensor_type, int mode);


#define SDK_ISP_ROTATE_0 (1<<0)
#define SDK_ISP_ROTATE_90 (1<<1)
#define SDK_ISP_ROTATE_180 (1<<2)
#define SDK_ISP_ROTATE_270 (1<<3)

extern SDK_API_t SDK_ISP_set_rotate(int vin, int rotate_n);

extern SDK_API_t SDK_ISP_init();
extern SDK_API_t SDK_ISP_destroy();


#ifdef __cplusplus
};
#endif
#endif //__SDK_ISP_H__

