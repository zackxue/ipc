
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef APP_GPIO_H_
#define APP_GPIO_H_
#ifdef __cplusplus
extern "C" {
#endif

extern int APP_GPIO_init();
extern void APP_GPIO_destroy();

extern int APP_GPIO_add(const char *name,
	uint32_t conf_addr32, uint32_t conf_mask32, uint32_t conf_val32,
	uint32_t dir_addr32, uint32_t dir_mask32, uint32_t dir_in_val32, uint32_t dir_out_val32,
	uint32_t data_addr32, uint32_t data_mask32);
extern int APP_GPIO_del(const char *name);

extern int APP_GPIO_set_pin(const char *name, bool pin_high);
extern int APP_GPIO_get_pin(const char *name, bool *pin_high);


#ifdef __cplusplus
};
#endif
#endif //APP_GPIO_H_

