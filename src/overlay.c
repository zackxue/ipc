
#include "overlay.h"
#include "sdk/sdk_api.h"
#include "app_debug.h"

static uint8_t _def_alpha = 64;
static OVERLAY_TEXT_STYLE_t _def_sytle = OVERLAY_TEXT_STYLE_DEFAULT; 

static uint8_t *_asc_16 = NULL; // bytes 128 x 16 x 1 size 8x16
static uint8_t *_asc_20 = NULL; // bytes 128 x 20 x 2 size 10x20
static uint8_t *_asc_24 = NULL; // bytes 128 x 24 x 2 size 12x24
static uint8_t *_asc_32 = NULL; // bytes 128 x 32 x 2 size 16x32

static uint8_t* _gb2312_16 = NULL; // bytes 87 x 94 x 16 x 2 size 16x16
static uint8_t* _gb2312_20 = NULL; // bytes 87 x 94 x 20 x 3 size 20x20
static uint8_t* _gb2312_24 = NULL; // bytes 87 x 94 x 24 x 3 size 24x24
static uint8_t* _gb2312_32 = NULL; // bytes 87 x 94 x 32 x 4 size 32x32


lpSDK_ENC_VIDEO_OVERLAY_CANVAS OVERLAY_create_canvas(size_t width, size_t height)
{
	return NULL;
}

void OVERLAY_release_canvas(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas)
{
}

ssize_t OVERLAY_put_text(lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas,
	int x, int y, OVERLAY_FONT_SIZE_t font_size, const char *text, OVERLAY_TEXT_STYLE_t style)
{

	int i = 0, ii = 0, iii = 0;
	int const x_base = x;
	int const y_base = y;
	
	char *ch = text; // at the beginning of the text
	size_t asc_width = 0;
	size_t gb2312_width = 0;
	size_t text_width = 0, text_height = 0;

	stSDK_ENC_VIDEO_OVERLAY_PIXEL fg_color, bg_color, enclosed_color;
	bool is_enclosed = false;

	if(!canvas){
		return -1;
	}
	
	switch(font_size){
		case OVERLAY_FONT_SIZE_16:
			asc_width = 8;
			gb2312_width = 16;
			if(!_asc_16 || !_gb2312_16){
				return -1;
			}
			break;

		case OVERLAY_FONT_SIZE_20:
			asc_width = 10;
			gb2312_width = 20;
			if(!_asc_20 || !_gb2312_20){
				return -1;
			}
			break;

		case OVERLAY_FONT_SIZE_24:
			asc_width = 12;
			gb2312_width = 24;
			if(!_asc_24 || !_gb2312_24){
				return -1;
			}
			break;

		case OVERLAY_FONT_SIZE_32:
			asc_width = 16;
			gb2312_width = 32;
			if(!_asc_32 || !_gb2312_32){
				return -1;
			}
			break;
			
		default:
			// not support font size
			return -1;
	}

	text_height = font_size < canvas->height ? font_size : canvas->height;

	if(!style){
		style = _def_sytle;
	}

	// check style
	if(style & OVERLAY_TEXT_STYLE_BACKGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, 0, 255, 255, 0);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, _def_alpha, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_BACKGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(bg_color, _def_alpha, 0, 0, 0);
	}
	
	if(style & OVERLAY_TEXT_STYLE_FOREGROUND_TRANSPARENT){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, 0, 255, 0, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_WHRITE){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, _def_alpha ^ 0x11, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_FOREGROUND_BLACK){
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(fg_color, _def_alpha ^ 0x11, 0, 0, 0);
	}

	if(style & OVERLAY_TEXT_STYLE_ENCLOSED_TRANSPARENT){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, 0, 0, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_WHRITE){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, _def_alpha ^ 0x22, 255, 255, 255);
	}else if(style & OVERLAY_TEXT_STYLE_ENCLOSED_BLACK){
		is_enclosed = true;
		SDK_ENC_VIDEO_OVERLAY_PIXEL_RGB(enclosed_color, _def_alpha ^ 0x22, 0, 0, 0);
	}

	// the font item stride bytes number, align 8bits / 1byte
	while('\0' != *ch){
		if(*ch < 0x7f){
			// ascii
			int const stride_bytes = (asc_width + 7) / 8;
			int const asc_code = *ch;

			// check width limited
			if(x + asc_width > canvas->width){
				break;
			}
			
			for(i = 0; i < font_size; ++i){
				int stride_pixel = 0;
				for(ii = 0; ii < stride_bytes && stride_pixel < asc_width; ++ii){
					off_t const offset_byte = asc_code * font_size * stride_bytes + i * stride_bytes + ii;
					for(iii = 0; iii < 8 && stride_pixel < asc_width; ++iii){
						uint8_t const actived_px = (1<<(8-1-iii));
						
						if((OVERLAY_FONT_SIZE_16 == font_size && (_asc_16[offset_byte] & actived_px))
							|| (OVERLAY_FONT_SIZE_20 == font_size && (_asc_20[offset_byte] & actived_px))
							|| (OVERLAY_FONT_SIZE_24 == font_size && (_asc_24[offset_byte] & actived_px))
							|| (OVERLAY_FONT_SIZE_32 == font_size && (_asc_32[offset_byte] & actived_px))){
							canvas->put_pixel(canvas, x + ii * 8 + iii, y + i, fg_color);
						}else{
							canvas->put_pixel(canvas, x + ii * 8 + iii, y + i, bg_color);
						}
						++stride_pixel;
					}
				}
			}
			x += asc_width;
			ch += 1;
		}else if(*ch > 0xa0){
			int const stride_bytes = (gb2312_width + 7) / 8;
			// get qu code and wei code
			int const qu_code = ch[0] - 0xa0 - 1; // 87
			int const wei_code = ch[1] - 0xa0 - 1; // 94

			// check width limited
			if(x + gb2312_width > canvas->width){
				break;
			}
			
			if(6 == qu_code){
				// russian
			}else{
				for(i = 0; i < font_size; ++i){
					int stride_pixel = 0;
					for(ii = 0; ii < stride_bytes && stride_pixel < gb2312_width; ++ii){
						off_t const offset_byte = (qu_code * 94 + wei_code) * font_size * stride_bytes + i * stride_bytes + ii;
						for(iii = 0; iii < 8 && stride_pixel < gb2312_width; ++iii){
							uint8_t const actived_px = (1<<(8-1-iii));
							
							if((OVERLAY_FONT_SIZE_16 == font_size && (_gb2312_16[offset_byte] & actived_px))
								|| (OVERLAY_FONT_SIZE_20 == font_size && (_gb2312_20[offset_byte] & actived_px))
								|| (OVERLAY_FONT_SIZE_24 == font_size && (_gb2312_24[offset_byte] & actived_px))
								|| (OVERLAY_FONT_SIZE_32 == font_size && (_gb2312_32[offset_byte] & actived_px))){
								canvas->put_pixel(canvas, x + ii * 8 + iii, y + i, fg_color);
							}else{
								canvas->put_pixel(canvas, x + ii * 8 + iii, y + i, bg_color);
							}
							++stride_pixel;
						}
					}
				}
			}
			x += gb2312_width;
			ch += 2;
		}
	}

	text_width = x - x_base;

	if(is_enclosed){
		for(i = 0; i < text_height - 1 ; ++i){
			for(ii = 0; ii < text_width - 1 ; ++ii){
				stSDK_ENC_VIDEO_OVERLAY_PIXEL center_color;
				canvas->get_pixel(canvas, x_base + ii, y_base + i, &center_color);
				if(0 == i || text_height - 1 == i
					|| 0 == ii || text_width - 1 == ii){
					// on the edage
					if(canvas->match_pixel(canvas, fg_color, center_color)){
						canvas->put_pixel(canvas, x_base + ii, y_base + i, enclosed_color);
					}
				}else{
					stSDK_ENC_VIDEO_OVERLAY_PIXEL right_color, bottom_color;
					
					canvas->get_pixel(canvas, x_base + ii + 1, y_base + i, &right_color); // to the right
					canvas->get_pixel(canvas, x_base + ii, y_base + i + 1, &bottom_color); // to the bottom

					if(canvas->match_pixel(canvas, bg_color, center_color)){
						// background
						if(canvas->match_pixel(canvas, fg_color, right_color)
							|| canvas->match_pixel(canvas, fg_color, bottom_color)){
							canvas->put_pixel(canvas, x_base + ii, y_base + i, enclosed_color);
						}
					}else if(canvas->match_pixel(canvas, fg_color, center_color)){
						// forground
						if(canvas->match_pixel(canvas, bg_color, right_color)){
							canvas->put_pixel(canvas, x_base + ii + 1, y_base + i, enclosed_color);
						}
						if(canvas->match_pixel(canvas, bg_color, bottom_color)){
							canvas->put_pixel(canvas, x_base + ii, y_base + i + 1, enclosed_color);
						}
					}
				}
			}
		}
	}
	
	return x - x_base; // write put size
}

static int overlay_load_font_2_mem(const char* file_name, uint8_t **ret_mem)
{
	struct stat file_stat={0};
	FILE* fid = NULL;
	if(0 == stat(file_name, &file_stat)){
		fid = fopen(file_name, "rb");
		if(NULL != fid){
			if(NULL != *ret_mem){
				free(*ret_mem);
			}
			*ret_mem = calloc(file_stat.st_size, 1);
			fread(*ret_mem, 1, file_stat.st_size, fid);
			fclose(fid);
			fid = NULL;
			return 0;
		}
	}
	return -1;
}

static void overlay_free_font_mem(uint8_t **mem)
{
    if(mem && *mem){
        free(*mem);
        *mem = NULL;
    }
}

int OVERLAY_load_font(OVERLAY_FONT_SIZE_t font_size, const char* asc_font, const char* gb2312_font)
{
	uint8_t **asc_mem = NULL, **gb2312_mem = NULL;
	
	switch(font_size){
		case OVERLAY_FONT_SIZE_16:
			asc_mem = &_asc_16;
			gb2312_mem = &_gb2312_16;
			break;

		case OVERLAY_FONT_SIZE_20:
			asc_mem = &_asc_20;
			gb2312_mem = &_gb2312_20;
			break;

		case OVERLAY_FONT_SIZE_24:
			asc_mem = &_asc_24;
			gb2312_mem = &_gb2312_24;
			break;

		case OVERLAY_FONT_SIZE_32:
			asc_mem = &_asc_32;
			gb2312_mem = &_gb2312_32;
			break;

		default:
			return -1;
	}

	if(0 == overlay_load_font_2_mem(asc_font, asc_mem)
		&& 0 == overlay_load_font_2_mem(gb2312_font, gb2312_mem)){
		return 0;
	}

	free(*asc_mem);
	free(*gb2312_mem);
	return -1;
	
}

void OVERLAY_set_alpha(uint8_t alpha)
{
	_def_alpha = alpha;
}

bool OVERLAY_font_available(OVERLAY_FONT_SIZE_t font_size)
{
	switch(font_size){
		case OVERLAY_FONT_SIZE_16: 	return (NULL != _asc_16 && (NULL != _gb2312_16));
		case OVERLAY_FONT_SIZE_20: 	return (NULL != _asc_20 && (NULL != _gb2312_20));
		case OVERLAY_FONT_SIZE_24: 	return (NULL != _asc_24 && (NULL != _gb2312_24));
		case OVERLAY_FONT_SIZE_32: 	return (NULL != _asc_32 && (NULL != _gb2312_32));
		default: ;
	}
	return false;
}

int OVERLAY_init()
{
//	OVERLAY_load_font(OVERLAY_FONT_SIZE_16, "asc16", "hzk16");
//	OVERLAY_load_font(OVERLAY_FONT_SIZE_20, "asc20", "hzk20");
//	OVERLAY_load_font(OVERLAY_FONT_SIZE_24, "asc24", "hzk24");
//	OVERLAY_load_font(OVERLAY_FONT_SIZE_32, "asc32", "hzk32");
	return 0;
}

void OVERLAY_destroy()
{
	overlay_free_font_mem(&_asc_16);
	overlay_free_font_mem(&_asc_20);
	overlay_free_font_mem(&_asc_24);	
	overlay_free_font_mem(&_asc_32);
	overlay_free_font_mem(&_gb2312_16);
	overlay_free_font_mem(&_gb2312_20);
	overlay_free_font_mem(&_gb2312_24);
	overlay_free_font_mem(&_gb2312_32);
}



