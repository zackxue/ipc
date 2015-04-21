
#ifndef __SDK_ISP_H__
#define __SDK_ISP_H__
#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum SDK_ISP_SENSOR_MODEL
{
	SDK_ISP_SENSOR_MODEL_APINA_AR0130 = 0,
	SDK_ISP_SENSOR_MODEL_SONY_ICX692,
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

extern SDK_API_t SDK_ISP_set_mirror(int vin, bool mirror);
extern SDK_API_t SDK_ISP_set_flip(int vin, bool flip);

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

