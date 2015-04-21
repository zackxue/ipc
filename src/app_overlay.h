
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "overlay.h"

#ifndef APP_OVERLAY_H_
#define APP_OVERLAY_H_
#ifdef __cplusplus
extern "C" {
#endif

#define APP_OVERLAY_CLOCK_NAME "clock"
#define APP_OVERLAY_TITLE_NAME "title"
#define APP_OVERLAY_ID_NAME "id"

extern void APP_OVERLAY_task();
extern void APP_OVERLAY_id_display();

extern int APP_OVERLAY_create_title(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size);
extern int APP_OVERLAY_create_id(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size);
extern int APP_OVERLAY_create_clock(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size);

extern int APP_OVERLAY_release_title(int vin, int stream);
extern int APP_OVERLAY_release_id(int vin, int stream);
extern int APP_OVERLAY_release_clock(int vin, int stream);

extern int APP_OVERLAY_update_title(int vin, int stream, size_t width, size_t height);
extern int APP_OVERLAY_update_id(int vin, int stream, size_t width, size_t height);
extern int APP_OVERLAY_update_clock(int vin, int stream, size_t width, size_t height);

extern int APP_OVERLAY_show_title(int vin, int stream, bool show);
extern int APP_OVERLAY_show_id(int vin, int stream, bool show);
extern int APP_OVERLAY_show_clock(int vin, int stream, bool show);

extern int APP_OVERLAY_set_title(int vin, const char *title);
extern int APP_OVERLAY_set_id(int vin, const char *title);


extern int APP_OVERLAY_init();
extern void APP_OVERLAY_destroy();


#ifdef __cplusplus
};
#endif
#endif //APP_OVERLAY_H_

