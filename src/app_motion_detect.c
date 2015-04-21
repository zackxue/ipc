
#include "app_motion_detect.h"
#include "app_debug.h"
#include "sdk/sdk_api.h"

static const float const _app_md_sensitivity_map[] = {
	[kAPP_MD_SENSITIVITY_LOWEST] = 0.8,
	[kAPP_MD_SENSITIVITY_LOW] = 0.6,
	[kAPP_MD_SENSITIVITY_MEDIUM] = 0.5,
	[kAPP_MD_SENSITIVITY_HIGH] = 0.3,
	[kAPP_MD_SENSITIVITY_HIGHEST] = 0.2,
};

typedef struct APP_MD_CH_ATTR {
	size_t h_mblock, v_mblock; 
	//uint8_t *mask_bitflag;
	enAPP_MD_SENSITIVITY sensitivity;
}stAPP_MD_CH_ATTR, *lpAPP_MD_CH_ATTR;

typedef struct APP_MD_ATTR {
	int n_md;
	lpAPP_MD_CH_ATTR md_attr;
}stAPP_MD_ATTR, *lpAPP_MD_ATTR;

static stAPP_MD_ATTR _md_attr = {
	.n_md = 0,
};
static lpAPP_MD_ATTR _lp_md_attr = NULL;

void APP_MD_do_trap(int vin, const char *rect_name)
{
	//sdk_vin->dump(vin);
	APP_TRACE("VIN(%d) motion occurs", vin);
}

int APP_MD_clear_mask(int vin)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		sdk_vin->clear_md_bitmap_mask(vin);
		return 0;
	}
	return -1;
}

int APP_MD_set_bitmap_one_mask(int vin, int x, int y, bool mask_flag)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->set_md_bitmap_one_mask(vin, x, y, mask_flag);
	}
	return -1;
}

int APP_MD_set_bitmap_mask(int vin, const uint8_t *const mask_bitflag)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->set_md_bitmap_mask(vin, mask_bitflag);
	}
	return -1;
}

bool APP_MD_get_bitmap_one_mask(int vin, int x, int y)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->get_md_bitmap_one_mask(vin, x, y);
	}
	return false;
}

const uint8_t *APP_MD_get_bitmap_mask(int vin, int *h_mblock, int *v_mblock)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->get_md_bitmap_mask(vin, h_mblock, v_mblock);
	}
	return NULL;
}

int APP_MD_add_rect_mask(int vin, float x_ratio, float y_ratio, float width_ratio, float height_ratio)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		if(x_ratio < 1.0 && y_ratio < 1.0
			&& width_ratio <= 1.0 && height_ratio <= 1.0){
		
			lpAPP_MD_CH_ATTR const md_ch = &_lp_md_attr->md_attr[vin];
			int x = 0, y = 0;
			int x1_abs = (int)((float)md_ch->h_mblock * x_ratio);
			int y1_abs = (int)((float)md_ch->v_mblock * y_ratio);
			int x2_abs = (int)((float)md_ch->h_mblock * (x_ratio + width_ratio) + 0.09);
			int y2_abs = (int)((float)md_ch->v_mblock * (y_ratio + height_ratio) + 0.09);

			// set the mask one by one
			for(y = y1_abs; y <= y2_abs; ++y){
				for(x = x1_abs; x <= x2_abs; ++x){
					APP_MD_set_bitmap_one_mask(vin, x, y, true);
				}
			}

			APP_TRACE("MD add rect [%d,%d,%d,%d]", x1_abs, y1_abs, x2_abs, y2_abs);
			return 0;
		}else{
			APP_TRACE("Invalid x/y/width/height ratio");
		}
	}
	return -1;
}

int APP_MD_commit_rect_mask(int vin)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		size_t h_mblock, v_mblock;
		const uint8_t *const mask_bitflag = sdk_vin->get_md_bitmap_mask(vin, &h_mblock, &v_mblock);
		// dump the mask status
		sdk_vin->dump_md(vin);

		// FIXME: store the motion detection mask to flash
		return 0;
	}
	return -1;
}

int APP_MD_set_ref_freq(int vin, int freq)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->set_md_ref_freq(vin, freq);
	}
	return -1;
}

int APP_MD_get_ref_freq(int vin)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		return sdk_vin->get_md_ref_freq(vin);
	}
	return -1;
}

int APP_MD_set_sensitivity(int vin, enAPP_MD_SENSITIVITY sensi)
{
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		if(sensi >= kAPP_MD_SENSITIVITY_LOWEST
			&&  sensi <= kAPP_MD_SENSITIVITY_HIGHEST){

			lpAPP_MD_CH_ATTR const md_ch = &_lp_md_attr->md_attr[vin];
			md_ch->sensitivity = sensi;
			return sdk_vin->set_md_bitmap_threshold(vin, _app_md_sensitivity_map[md_ch->sensitivity]);
		}
	}
	return -1;
}

enAPP_MD_SENSITIVITY APP_MD_get_sensitivity(int vin)
{
	int i = 0;
	if(NULL != _lp_md_attr && NULL != sdk_vin){
		enAPP_MD_SENSITIVITY sensitivity = kAPP_MD_SENSITIVITY_INVALID;
		float const threshold_ratio = sdk_vin->get_md_bitmap_threshold(vin);
		if(threshold_ratio >= 1.0){
			return -1;
		}

		if(threshold_ratio >= _app_md_sensitivity_map[kAPP_MD_SENSITIVITY_LOWEST]){
			sensitivity = kAPP_MD_SENSITIVITY_LOWEST;
		}else if(threshold_ratio >= _app_md_sensitivity_map[kAPP_MD_SENSITIVITY_LOW]){
			sensitivity = kAPP_MD_SENSITIVITY_LOW;
		}else if(threshold_ratio >= _app_md_sensitivity_map[kAPP_MD_SENSITIVITY_MEDIUM]){
			sensitivity = kAPP_MD_SENSITIVITY_MEDIUM;
		}else if(threshold_ratio >= _app_md_sensitivity_map[kAPP_MD_SENSITIVITY_HIGH]){
			sensitivity = kAPP_MD_SENSITIVITY_HIGH;
		}else{
			sensitivity = kAPP_MD_SENSITIVITY_HIGH;
		}
		return sensitivity;
	}
	return kAPP_MD_SENSITIVITY_INVALID;
}

int APP_MD_init(int n_md)
{
	int i = 0;
	//int x = 0, y = 0;
	
	if(NULL == _lp_md_attr && NULL != sdk_vin){
		// init the pointer
		_lp_md_attr = &_md_attr;
		// init element
		_lp_md_attr->n_md = n_md;
		_lp_md_attr->md_attr = calloc(sizeof(stAPP_MD_CH_ATTR) * _lp_md_attr->n_md, 1);
		// create the every channel motion detection
		for(i = 0; i < _lp_md_attr->n_md; ++i){
			//size_t bitflag_bytes = 0;
			
			sdk_vin->create_md(i, 320 / 16, 240 / 16, 10);
			
			// get the w/h info and init the local mask array
			APP_ASSERT(NULL != sdk_vin->get_md_bitmap_mask(i, &_lp_md_attr->md_attr[i].h_mblock, &_lp_md_attr->md_attr[i].v_mblock),
				"SDK get md(%d) mask error!", i);
			//bitflag_bytes = SDK_ALIGNED_BIG_ENDIAN(_lp_md_attr->md_attr[i].h_mblock * _lp_md_attr->md_attr[i].v_mblock, 8);
			//bitflag_bytes /= 8;
			//_lp_md_attr->md_attr[i].mask_bitflag = calloc(sizeof(uint8_t) * bitflag_bytes, 1);

			// set defautl reference frequency
			sdk_vin->set_md_ref_freq(i, 3); // 3 times /s

			// set the default sensitivity
			APP_MD_set_sensitivity(i, kAPP_MD_SENSITIVITY_MEDIUM);

			// dump and start
			sdk_vin->dump_md(i);
			sdk_vin->start_md(i);	
		}

		return 0;
	}
	return -1;
}

void APP_MD_destroy()
{
	int i = 0;
	if(NULL != _lp_md_attr){
		for(i = 0; i < _lp_md_attr->n_md; ++i){
			// stop motion detection of each channel
			sdk_vin->stop_md(i);
			// free the bitflag mask
			//if(_lp_md_attr->md_attr[i].mask_bitflag){
			//	free(_lp_md_attr->md_attr[i].mask_bitflag);
			//	_lp_md_attr->md_attr[i].mask_bitflag = NULL;
			//}
			// release sdk motion detection
			sdk_vin->release_md(i);
		}
		// free the application attributes
		free(_lp_md_attr->md_attr);
		_lp_md_attr->md_attr = NULL;
		_lp_md_attr->n_md = 0;
		// clear the pointer
		_lp_md_attr = NULL;
	}
}

