
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef APP_MOTION_DETECT_H_
#define APP_MOTION_DETECT_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef enum APP_MD_SENSITIVITY {
	kAPP_MD_SENSITIVITY_INVALID = -1,
	kAPP_MD_SENSITIVITY_LOWEST = 0,
	kAPP_MD_SENSITIVITY_LOW,
	kAPP_MD_SENSITIVITY_MEDIUM,
	kAPP_MD_SENSITIVITY_HIGH,
	kAPP_MD_SENSITIVITY_HIGHEST,
}enAPP_MD_SENSITIVITY, *lpAPP_MD_SENSITIVITY;

extern void APP_MD_do_trap(int vin, const char *rect_name);

extern int APP_MD_clear_mask(int vin);
extern int APP_MD_set_bitmap_one_mask(int vin, int x, int y, bool mask_flag);
extern int APP_MD_set_bitmap_mask(int vin, const uint8_t *const mask_bitflag);
extern bool APP_MD_get_bitmap_one_mask(int vin, int x, int y);
extern const uint8_t *APP_MD_get_bitmap_mask(int vin, int *h_mblock, int *v_mblock);

extern int APP_MD_add_rect_mask(int vin, float x_ratio, float y_ratio, float width_ratio, float height_ratio);
extern int APP_MD_commit_rect_mask(int vin);

extern int APP_MD_set_ref_freq(int vin, int freq);
extern int APP_MD_get_ref_freq(int vin);

extern int APP_MD_set_sensitivity(int vin, enAPP_MD_SENSITIVITY sensi);
extern enAPP_MD_SENSITIVITY APP_MD_get_sensitivity(int vin);

extern int APP_MD_init(int n_md);
extern void APP_MD_destroy();

#ifdef __cplusplus
};
#endif
#endif //APP_MOTION_DETECT_H_

