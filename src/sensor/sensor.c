
#include "bsp/i2cm.h"
#include "mt9d131.h"
#include "ov2643.h"
#include "sensor.h"
#include "sysconf.h"
#include "sensor_callback.h"
//#include "sdk/sdk_isp.h"
#include "sdk_isp.h" // in hi_isp_tmp


typedef struct _sensor_func
{
	void (*MIRROR_FLIP_SET)(uint8_t);
	void (*HUE_SET)(uint16_t);
	void (*CONTRAST_SET)(uint8_t);
	void (*BRIGHTNESS_SET)(uint8_t);
	void (*SATURATION_SET)(uint8_t);
	void (*LIGHT_MODE_SET)(uint8_t);
	void (*TEST_MODE_SET)(uint8_t);
	void (*COLOR_MODE_SET)(uint8_t);
	void (*REG_WRITE)(uint8_t,uint8_t);
	uint8_t (*REG_READ)(uint8_t);
	void (*SPEC_RED_WRITE)(uint8_t, uint8_t, uint16_t);
	uint16_t (*SPEC_REG_READ)(uint8_t, uint8_t);
	ColorMaxValue (*GET_COLOR_MAX_VALUE)(void);
	void (*SHUTTER_SET)(uint8_t);
	void (*IRCUT_AUTO_SWITCH)(uint8_t, uint8_t);
	void (*VI_FLICKER)(uint8_t, uint8_t);
	void (*SHARPEN_SET)(uint8_t);
	uint8_t (*SHARPEN_GET)(void);
	void (*SCENE_MODE_SET)(uint32_t);
	void (*WB_MODE_SET)(uint32_t);
	void (*IRCUT_CONTROL_MODE_SET)(uint32_t);
	void (*IRCUT_MODE_SET)(uint32_t);
	void (*WDR_MODE_ENABLE)(uint8_t);
	void (*WDR_STRENGTH_SET)(uint8_t);
	void (*EXPOSURE_MODE_SET)(uint32_t);
	void (*AE_COMPENSATION_SET)(uint8_t);
	void (*DENOISE_ENABLE)(uint32_t);
	void (*DENOISE_STRENGTH_SET)(uint8_t);
	uint8_t (*DENOISE_STRENGTH_GET)(void);
	void (*ANTI_FOG_ENABLE)(uint8_t);
	void (*LOWLIGHT_ENABLE)(uint8_t);
	void (*GAMMA_TABLE_SET)(uint8_t);
	void (*DEFECT_PIXEL_ENABLE)(uint8_t);
}st_sensor_func;
st_sensor_func stSENSOR;

static ColorMaxValue sensor_max_value;
extern SDK_ISP_SENSOR_MODEL_t g_sensor_type;

char *sensor_model_str[] = {
	"ar0130",
	"ov9712", 
	"soih22",
};

static void _sensor_set_sysconf(SYSCONF_t *sysconf)
{
	sensor_max_value = stSENSOR.GET_COLOR_MAX_VALUE();
	//SYSCONF_t *sysconf = SYSCONF_dup();
	sysconf->ipcam.vin[0].brightness.max = sensor_max_value.BrightnessMax;
	sysconf->ipcam.vin[0].contrast.max = sensor_max_value.ContrastMax;
	sysconf->ipcam.vin[0].hue.max = sensor_max_value.HueMax;
	sysconf->ipcam.vin[0].saturation.max = sensor_max_value.SaturationMax;
	
	sysconf->ipcam.isp.image_attr.brightness.max = sensor_max_value.BrightnessMax;
	sysconf->ipcam.isp.image_attr.contrast.max = sensor_max_value.ContrastMax;
	sysconf->ipcam.isp.image_attr.hue.max = sensor_max_value.HueMax;
	sysconf->ipcam.isp.image_attr.saturation.max = sensor_max_value.SaturationMax;
	stSENSOR.SHUTTER_SET(sysconf->ipcam.vin[0].digital_shutter.val);
	stSENSOR.SATURATION_SET(sysconf->ipcam.vin[0].saturation.val);
	stSENSOR.CONTRAST_SET(sysconf->ipcam.vin[0].contrast.val);
	stSENSOR.HUE_SET(sysconf->ipcam.vin[0].hue.val);
	stSENSOR.BRIGHTNESS_SET(sysconf->ipcam.vin[0].brightness.val);
	//stSENSOR.VI_FLICKER(1, sysconf->ipcam.vin[0].digital_shutter.val == SYS_VIN_DIGITAL_SHUTTER_50HZ ? 50 : 60);
	if(sysconf->ipcam.vin[0].flip){
		stSENSOR.MIRROR_FLIP_SET(MODE_FLIP);
	}else{
		stSENSOR.MIRROR_FLIP_SET(MODE_UNFLIP);
	}
	if(sysconf->ipcam.vin[0].mirror){
		stSENSOR.MIRROR_FLIP_SET(MODE_MIRROR);
	}else{
		stSENSOR.MIRROR_FLIP_SET(MODE_UNMIRROR);
	}

	//stSENSOR.SHARPEN_SET(sysconf->ipcam.isp.image_attr.sharpen.val);
	sysconf->ipcam.isp.image_attr.sharpen.val = stSENSOR.SHARPEN_GET();
	//stSENSOR.DEFECT_PIXEL_ENABLE(sysconf->ipcam.isp.advance.defect_pixel_enable);
	stSENSOR.SCENE_MODE_SET(sysconf->ipcam.isp.scene_mode);
	stSENSOR.WB_MODE_SET(sysconf->ipcam.isp.white_balance_mode);
	stSENSOR.IRCUT_CONTROL_MODE_SET(sysconf->ipcam.isp.day_night_mode.ircut_control_mode);
	stSENSOR.IRCUT_MODE_SET(sysconf->ipcam.isp.day_night_mode.ircut_mode);
	stSENSOR.WDR_MODE_ENABLE(sysconf->ipcam.isp.wide_dynamic_range.enable);
	//stSENSOR.WDR_STRENGTH_SET(sysconf->ipcam.isp.wide_dynamic_range.strength.val);
	stSENSOR.EXPOSURE_MODE_SET(sysconf->ipcam.isp.exposure.mode);
	//stSENSOR.AE_COMPENSATION_SET(sysconf->ipcam.isp.exposure.ae_compensation.val);
	stSENSOR.DENOISE_ENABLE(sysconf->ipcam.isp.denoise.denoise_enable);
	sysconf->ipcam.isp.denoise.denoise_strength.val = stSENSOR.DENOISE_STRENGTH_GET();
	//stSENSOR.LOWLIGHT_ENABLE(sysconf->ipcam.isp.advance.lowlight_enable);
	//stSENSOR.GAMMA_TABLE_SET(sysconf->ipcam.isp.advance.gamma);
	stSENSOR.ANTI_FOG_ENABLE(sysconf->ipcam.isp.advance.anti_fog_enable);
	//stSENSOR.VI_FLICKER(1, 50);
	//SENSOR_shutter_set(25);
	
}

void _sensor_setup_tools(uint32_t sensor_type)
{
	char cmd[128] = {0};
	system("kill -9 `pidof ittb_control`");
	usleep(200000);
	//sprintf(cmd, "/root/nfs/gm_ipc/HiPCTools_Board/release_hi3518/ittb_control -n -s %s &", sensor_model_str[sensor_type]);
	sprintf(cmd, "/root/tools/ittb_control -n -s %s &", sensor_model_str[sensor_type]);
	system(cmd);
}

void _sensor_read_value(uint32_t sensor_type, SYSCONF_t *sysconf)
{
	
}

static void _sensor_check_default_value(uint32_t sensor_type)
{
	SYSCONF_t *sysconf = SYSCONF_dup();
	memset(sysconf->ipcam.info.sensor_type, 0, sizeof(sysconf->ipcam.info.sensor_type));
	strcpy(sysconf->ipcam.info.sensor_type, sensor_model_str[sensor_type]);
	if(sysconf->ipcam.isp.read_default_val){
		_sensor_read_value(sensor_type, sysconf);
		//SYSCONF_save(sysconf);
	}
	_sensor_set_sysconf(sysconf);	
	SYSCONF_save(sysconf);
}

void SENSOR_set_sysconf()
{
	SYSCONF_t *sysconf = SYSCONF_dup();
//	stSENSOR.SHUTTER_SET(sysconf->ipcam.vin[0].digital_shutter.val);
	stSENSOR.SATURATION_SET(sysconf->ipcam.vin[0].saturation.val);
	stSENSOR.CONTRAST_SET(sysconf->ipcam.vin[0].contrast.val);
	stSENSOR.HUE_SET(sysconf->ipcam.vin[0].hue.val);
	stSENSOR.BRIGHTNESS_SET(sysconf->ipcam.vin[0].brightness.val);
}

int SENSOR_init()
{
	// init i2c bus
	/*
	I2CM_init();
	// install sensor module buses
	MT9D131_install(I2CM_read_bus8, I2CM_write_bus8);
	OV2643_install(I2CM_read_sccb, I2CM_write_sccb);
	// init sensor
	if(0 == MT9D131_check()){
		MT9D131_init(60);

		stSENSOR.MIRROR_FLIP_SET=MT9D131_mirror_flip;
		stSENSOR.HUE_SET=MT9D131_set_hue;
		stSENSOR.CONTRAST_SET=MT9D131_set_contrast;
		stSENSOR.BRIGHTNESS_SET=MT9D131_set_brightness;
		stSENSOR.SATURATION_SET=MT9D131_set_saturation;
		stSENSOR.EXPOSURE_SET=MT9D131_set_exposure;
		stSENSOR.SHARPNESS_SET=MT9D131_set_sharpness;
		stSENSOR.AWB_MODE_SET=MT9D131_AWB;
		stSENSOR.LIGHT_MODE_SET=MT9D131_light_mode;
		stSENSOR.TEST_MODE_SET=MT9D131_test_mode;
		stSENSOR.COLOR_MODE_SET=MT9D131_color_mode;
		stSENSOR.REG_READ=MT9D131_reg_read;
		stSENSOR.REG_WRITE=MT9D131_reg_write;
		//for special reg R/W (need client)
		stSENSOR.SPEC_RED_WRITE=SPEC_MT9D131_reg_write;
		stSENSOR.SPEC_REG_READ=SPEC_MT9D131_reg_read;
		stSENSOR.GET_COLOR_MAX_VALUE=MT9D131_get_color_max_value;
		stSENSOR.SHUTTER_SET=MT9D131_set_shutter;
	}else if(0 == OV2643_check()){
		OV2643_init(50);

		stSENSOR.MIRROR_FLIP_SET=OV2643_mirror_flip;
		stSENSOR.HUE_SET=OV2643_set_hue;
		stSENSOR.CONTRAST_SET=OV2643_set_contrast;
		stSENSOR.BRIGHTNESS_SET=OV2643_set_brightness;
		stSENSOR.SATURATION_SET=OV2643_set_saturation;
		stSENSOR.EXPOSURE_SET=OV2643_set_exposure;
		stSENSOR.SHARPNESS_SET=OV2643_set_sharpness;
		stSENSOR.AWB_MODE_SET=OV2643_AWB;
		stSENSOR.LIGHT_MODE_SET=OV2643_light_mode;
		stSENSOR.TEST_MODE_SET=OV2643_test_mode;
		stSENSOR.COLOR_MODE_SET=OV2643_color_mode;
		stSENSOR.REG_READ=OV2643_reg_read;
		stSENSOR.REG_WRITE=OV2643_reg_write;
		//for special reg R/W (need client)
		stSENSOR.SPEC_RED_WRITE=SPEC_OV2643_reg_write;
		stSENSOR.SPEC_REG_READ=SPEC_OV2643_reg_read;
		stSENSOR.GET_COLOR_MAX_VALUE=OV2643_get_color_max_value;
		stSENSOR.SHUTTER_SET=OV2643_set_shutter;
	}else{
		//return -1;
	}*/
	stSENSOR.MIRROR_FLIP_SET=sensor_callback_mirror_flip;
	stSENSOR.HUE_SET=sensor_callback_set_hue;
	stSENSOR.CONTRAST_SET=sensor_callback_set_contrast;
	stSENSOR.BRIGHTNESS_SET=sensor_callback_set_brightness;
	stSENSOR.SATURATION_SET=sensor_callback_set_saturation;
	stSENSOR.LIGHT_MODE_SET=sensor_callback_light_mode;
	stSENSOR.TEST_MODE_SET=sensor_callback_test_mode;
	stSENSOR.COLOR_MODE_SET=sensor_callback_color_mode;
	stSENSOR.REG_READ=sensor_callback_reg_read;
	stSENSOR.REG_WRITE=sensor_callback_reg_write;
	//for special reg R/W (need client)
	stSENSOR.SPEC_RED_WRITE=SPEC_sensor_callback_reg_write;
	stSENSOR.SPEC_REG_READ=SPEC_sensor_callback_reg_read;
	stSENSOR.GET_COLOR_MAX_VALUE=sensor_callback_get_color_max_value;
	stSENSOR.SHUTTER_SET=sensor_callback_set_shutter;
	stSENSOR.IRCUT_AUTO_SWITCH=sensor_callback_ircut_auto_switch;
	stSENSOR.VI_FLICKER=sensor_callback_vi_flicker;
	stSENSOR.SHARPEN_SET=sensor_callback_set_sharpen;
	stSENSOR.SHARPEN_GET=sensor_callback_get_sharpen;
	stSENSOR.SCENE_MODE_SET=sensor_callback_set_scene_mode;
	stSENSOR.WB_MODE_SET=sensor_callback_set_WB_mode;
	stSENSOR.IRCUT_CONTROL_MODE_SET=sensor_callback_set_ircut_control_mode;
	stSENSOR.IRCUT_MODE_SET=sensor_callback_set_ircut_mode;
	stSENSOR.WDR_MODE_ENABLE=sensor_callback_set_WDR_enable;
	stSENSOR.WDR_STRENGTH_SET=sensor_callback_set_WDR_strength;
	stSENSOR.EXPOSURE_MODE_SET=sensor_callback_set_exposure_mode;
	stSENSOR.AE_COMPENSATION_SET=sensor_callback_set_AEcompensation;
	stSENSOR.DENOISE_ENABLE=sensor_callback_set_denoise_enable;
	stSENSOR.DENOISE_STRENGTH_SET=sensor_callback_set_denoise_strength;
	stSENSOR.DENOISE_STRENGTH_GET=sensor_callback_get_denoise_strength;
	stSENSOR.ANTI_FOG_ENABLE=sensor_callback_set_anti_fog_enable;
	stSENSOR.LOWLIGHT_ENABLE=sensor_callback_set_lowlight_enable;
	stSENSOR.GAMMA_TABLE_SET=sensor_callback_set_gamma_table;
	stSENSOR.DEFECT_PIXEL_ENABLE=sensor_callback_set_defect_pixel_enable;
	
	_sensor_check_default_value((uint32_t)g_sensor_type);
	//_sensor_setup_tools((uint32_t)g_sensor_type);
	return 0;
}

void SENSOR_destroy()
{
	// deinit i2c bus
	I2CM_destroy();
}

void SENSOR_mirror_flip(uint8_t mode)
{
	stSENSOR.MIRROR_FLIP_SET(mode);
}
void SENSOR_hue_set(uint16_t val)
{
	if(val <= sensor_max_value.HueMax){
		stSENSOR.HUE_SET(val);
	}
}
void SENSOR_contrast_set(uint8_t val)
{
	if(val <= sensor_max_value.ContrastMax)
		stSENSOR.CONTRAST_SET(val);
}
void SENSOR_brightness_set(uint8_t val)
{
	if(val <= sensor_max_value.BrightnessMax)
		stSENSOR.BRIGHTNESS_SET(val);
}
void SENSOR_saturation_set(uint8_t val)
{
	if(val <= sensor_max_value.SaturationMax)
	stSENSOR.SATURATION_SET(val);
}

void SENSOR_lightmode_set(uint8_t mode)
{
	stSENSOR.LIGHT_MODE_SET(mode);
}
void SENSOR_test_mode(uint8_t enable)
{
	stSENSOR.TEST_MODE_SET(enable);
}
void SENSOR_color_mode(uint8_t mode)
{
	stSENSOR.COLOR_MODE_SET(mode);
}
void SENSOR_reg_write(uint8_t addr,uint8_t val)
{
	stSENSOR.REG_WRITE(addr,val);
}
uint8_t SENSOR_reg_read(uint8_t addr)
{
	return stSENSOR.REG_READ(addr);
}
void SENSOR_spec_reg_write(uint8_t page, uint8_t addr, uint16_t val)
{
	stSENSOR.SPEC_RED_WRITE(page, addr, val);
}
uint16_t SENSOR_spec_reg_read(uint8_t page,uint8_t addr)
{
	return stSENSOR.SPEC_REG_READ(page, addr);
}

ColorMaxValue SENSOR_get_color_max_value()
{
	return stSENSOR.GET_COLOR_MAX_VALUE();
}

void SENSOR_shutter_set(uint8_t val)
{
	stSENSOR.SHUTTER_SET(val);
}

void SENSOR_ircut_auto_switch(uint8_t type, uint8_t bEnable)//0:software   1: hardware
{
	stSENSOR.IRCUT_AUTO_SWITCH(type, bEnable);
}

void SENSOR_vi_flicker(uint8_t bEnable, uint8_t frequency)
{
	stSENSOR.VI_FLICKER(bEnable, frequency);
}

void SENSOR_sharpen_set(uint8_t val)
{
	stSENSOR.SHARPEN_SET(val);
}

void SENSOR_scene_mode_set(uint32_t mode)
{
	stSENSOR.SCENE_MODE_SET(mode);
}

void SENSOR_WB_mode_set(uint32_t mode)
{
	stSENSOR.WB_MODE_SET(mode);
}

void SENSOR_ircut_control_mode_set(uint32_t mode)
{
	stSENSOR.IRCUT_CONTROL_MODE_SET(mode);
}

void SENSOR_ircut_mode_set(uint32_t mode)
{
	stSENSOR.IRCUT_MODE_SET(mode);
}

void SENSOR_WDR_mode_enable(uint8_t bEnable)
{
	stSENSOR.WDR_MODE_ENABLE(bEnable);
}

void SENSOR_WDR_strength_set(uint8_t val)
{
	stSENSOR.WDR_STRENGTH_SET(val);
}

void SENSOR_exposure_mode_set(uint32_t mode)
{
	stSENSOR.EXPOSURE_MODE_SET(mode);
}

void SENSOR_AEcompensation_set(uint8_t val)
{
	stSENSOR.AE_COMPENSATION_SET(val);
}

void SENSOR_denoise_enable(uint8_t bEnable)
{
	stSENSOR.DENOISE_ENABLE(bEnable);
}

void SENSOR_denoise_strength(uint8_t val)
{
	stSENSOR.DENOISE_STRENGTH_SET(val);
}

void SENSOR_anti_fog_enable(uint8_t bEnable)
{
	stSENSOR.ANTI_FOG_ENABLE(bEnable);
}

void SENSOR_lowlight_enable(uint8_t bEnable)
{
	stSENSOR.LOWLIGHT_ENABLE(bEnable);
}

void SENSOR_gamma_table_set(uint8_t val)
{
	stSENSOR.GAMMA_TABLE_SET(val);
}

void SENSOR_defect_pixel_enable(uint8_t bEnable)
{
	stSENSOR.DEFECT_PIXEL_ENABLE(bEnable);
}


