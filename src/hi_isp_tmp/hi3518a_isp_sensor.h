
#ifndef __HI3518A_ISP_SENSOR_H__
#define __HI3518A_ISP_SENSOR_H__

typedef int (*SENSOR_APTINA_AR0130_DO_I2CRD)(uint16_t addr, uint16_t* ret_data);
typedef int (*SENSOR_APTINA_AR0130_DO_I2CWR)(uint16_t addr, uint16_t data);
typedef int (*SENSOR_OV9712_DO_I2CRD)(uint8_t addr, uint8_t* ret_data);
typedef int (*SENSOR_OV9712_DO_I2CWR)(uint8_t addr, uint8_t data);
typedef int (*SENSOR_SOIH22_DO_I2CRD)(uint8_t addr, uint8_t* ret_data);
typedef int (*SENSOR_SOIH22_DO_I2CWR)(uint8_t addr, uint8_t data);
typedef int (*SENSOR_SONY_IMX122_DO_I2CRD)(uint16_t addr, uint16_t* ret_data);
typedef int (*SENSOR_SONY_IMX122_DO_I2CWR)(uint16_t addr, uint16_t data);



extern void APTINA_AR0130_init(SENSOR_APTINA_AR0130_DO_I2CRD do_i2c_read, SENSOR_APTINA_AR0130_DO_I2CWR do_i2c_write);
extern void OV9712_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write);
extern void SOIH22_init(SENSOR_OV9712_DO_I2CRD do_i2c_read, SENSOR_OV9712_DO_I2CWR do_i2c_write);
extern void SONY_IMX122_init(SENSOR_SONY_IMX122_DO_I2CRD do_i2c_read,SENSOR_SONY_IMX122_DO_I2CWR do_i2c_write);




#endif //__HI3518A_ISP_SENSOR_H__

