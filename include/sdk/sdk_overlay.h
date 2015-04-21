
#ifndef __SDK_OVERLAY_H__
#define __SDK_OVERLAY_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct SDK_OVERLAY_COLOR
{
	union
	{
		struct
		{
			uint8_t blue;
			uint8_t green;
			uint8_t red;
			uint8_t alpha;
		};
		uint32_t val;
	};
}SDK_OVERLAY_COLOR_t;

typedef struct SDK_OVERLAY_COLOR_FORMAT
{
	uint32_t rmask;
	uint32_t gmask;
	uint32_t bmask;
	uint32_t amask;
}SDK_OVERLAY_COLOR_FORMAT_t;

typedef struct SDK_OVERLAY_SURFACE
{
	uint32_t width;
	uint32_t height;
	SDK_OVERLAY_COLOR_FORMAT_t color_format;
}SDK_OVERLAY_SURFACE_t;

typedef struct SDK_OVERLAY_REGION
{
	int venc;
	int id; // which region of each ch, hi3507 support 4 regions in total
	int x;
	int y;
	SDK_OVERLAY_SURFACE_t* surface;
}SDK_OVERLAY_REGION_t;

extern SDK_API_t SDK_OVERLAY_surface_create(uint32_t width, uint32_t height, SDK_OVERLAY_SURFACE_t** ret_surface);
extern SDK_API_t SDK_OVERLAY_surface_release(SDK_OVERLAY_SURFACE_t** surface);
extern SDK_API_t SDK_OVERLAY_surface_putpixel(SDK_OVERLAY_SURFACE_t* surface, int x, int y, SDK_OVERLAY_COLOR_t color);
extern SDK_API_t SDK_OVERLAY_surface_getpixel(SDK_OVERLAY_SURFACE_t* surface, int x, int y, SDK_OVERLAY_COLOR_t* ret_color);

extern SDK_API_t SDK_OVERLAY_region_create(int venc, int x, int y, SDK_OVERLAY_SURFACE_t* surface, SDK_OVERLAY_REGION_t** ret_region);
extern SDK_API_t SDK_OVERLAY_region_release(SDK_OVERLAY_REGION_t* region);
extern SDK_API_t SDK_OVERLAY_region_visible(SDK_OVERLAY_REGION_t* region, uint32_t flag);
extern SDK_API_t SDK_OVERLAY_region_flip(SDK_OVERLAY_REGION_t* region);

extern SDK_API_t SDK_OVERLAY_init(int overlay_cnt);
extern SDK_API_t SDK_OVERLAY_destroy();

#ifdef __cplusplus
};
#endif
#endif //__SDK_OVERLAY_H__

