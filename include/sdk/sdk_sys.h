
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef SDK_SYS_H_
#define SDK_SYS_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct SDK_SYS_API {

	int (*read_reg)(uint32_t reg_addr, uint32_t *val32);
	int (*write_reg)(uint32_t reg_addr, uint32_t val32);
	int (*read_mask_reg)(uint32_t reg_addr, uint32_t mask32, uint32_t *val32);
	int (*write_mask_reg)(uint32_t reg_addr, uint32_t mask32, uint32_t val32);

	float (*temperature)();
	
	
}stSDK_SYS_API, *lpSDK_SYS_API;

extern lpSDK_SYS_API sdk_sys;
extern int SDK_init_sys(const char* solution);
extern int SDK_destroy_sys();

#ifdef __cplusplus
};
#endif
#endif //SDK_SYS_H_

