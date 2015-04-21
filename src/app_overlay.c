
#include "app_overlay.h"
#include "conf.h"
#include "sysconf.h"
#include "sdk/sdk_api.h"
#include "app_debug.h"
#include "esee_client.h"

int APP_OVERLAY_create_title(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size)
{
	if(NULL == sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_TITLE_NAME)){
		lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas_title = NULL;
		canvas_title = sdk_enc->create_overlay_canvas(font_size * 16, font_size);
		if(NULL != canvas_title){
			SYSCONF_t *sysconf = SYSCONF_dup();
			char stream_title[32] = {""};
			strcpy(stream_title, sysconf->ipcam.vin[0].channel_name);
			OVERLAY_put_text(canvas_title, 0, 0, font_size, stream_title, 0);
			sdk_enc->create_overlay(vin, stream, APP_OVERLAY_TITLE_NAME, x, y, canvas_title->width, canvas_title->height, canvas_title);
			return 0;
		}
	}
	return -1;
}

int APP_OVERLAY_create_id(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size)
{
	if(NULL == sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_ID_NAME)){
		lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas_id = NULL;
		canvas_id = sdk_enc->create_overlay_canvas(font_size * 16, font_size);
		if(NULL != canvas_id){
			SYSCONF_t *sysconf = SYSCONF_dup();
			char stream_id[32];
			memset(stream_id, 0, sizeof(stream_id));
			sprintf(stream_id, " ");
			OVERLAY_put_text(canvas_id, 0, 0, font_size, stream_id, 0);
			sdk_enc->create_overlay(vin, stream, APP_OVERLAY_ID_NAME, x, y, canvas_id->width, canvas_id->height, canvas_id);
			return 0;
		}
	}
	return -1;
}


static stSDK_ENC_VIDEO_OVERLAY_CANVAS *_canvas_clock_font_16 = NULL;
static stSDK_ENC_VIDEO_OVERLAY_CANVAS *_canvas_clock_font_20 = NULL;
static stSDK_ENC_VIDEO_OVERLAY_CANVAS *_canvas_clock_font_32 = NULL;
int APP_OVERLAY_create_clock(int vin, int stream, int x, int y, OVERLAY_FONT_SIZE_t font_size)
{
	if(NULL == sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_CLOCK_NAME)){
		lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas_clock = NULL;
		// check the size
		if(16 == font_size){
			if(!_canvas_clock_font_16){
				_canvas_clock_font_16 = sdk_enc->create_overlay_canvas(font_size * 16, font_size);
			}
			canvas_clock = _canvas_clock_font_16;
		}else if(20 == font_size){
			if(!_canvas_clock_font_20){
				_canvas_clock_font_20 = sdk_enc->create_overlay_canvas(font_size * 20, font_size);
			}
			canvas_clock = _canvas_clock_font_32;
		}else if(32 == font_size){
			if(!_canvas_clock_font_32){
				_canvas_clock_font_32 = sdk_enc->create_overlay_canvas(font_size * 32, font_size);
			}
			canvas_clock = _canvas_clock_font_32;
		}else{
			return -1;
		}

		sdk_enc->create_overlay(vin, stream, APP_OVERLAY_CLOCK_NAME, x, y, canvas_clock->width, canvas_clock->height, canvas_clock);
		return 0;
	}
	return -1;
}

int APP_OVERLAY_release_title(int vin, int stream)
{
	if(NULL != sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_TITLE_NAME)){
		sdk_enc->release_overlay(vin, stream, APP_OVERLAY_TITLE_NAME);
		return 0;
	}
	return -1;
}

int APP_OVERLAY_release_id(int vin, int stream)
{
	if(NULL != sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_ID_NAME)){
		sdk_enc->release_overlay(vin, stream, APP_OVERLAY_ID_NAME);
		return 0;
	}
	return -1;
}


int APP_OVERLAY_release_clock(int vin, int stream)
{
	if(NULL != sdk_enc->get_overlay_canvas(vin, stream, APP_OVERLAY_CLOCK_NAME)){
		sdk_enc->release_overlay(vin, stream, APP_OVERLAY_CLOCK_NAME);
		return 0;
	}
	return -1;
}

int APP_OVERLAY_update_title(int vin, int stream, size_t width, size_t height)
{
	sdk_enc->update_overlay(vin, stream, APP_OVERLAY_TITLE_NAME);
	return 0;
}

int APP_OVERLAY_update_id(int vin, int stream, size_t width, size_t height)
{
	sdk_enc->update_overlay(vin, stream, APP_OVERLAY_ID_NAME);
	return 0;
}


int APP_OVERLAY_update_clock(int vin, int stream, size_t width, size_t height)
{
	sdk_enc->update_overlay(vin, stream, APP_OVERLAY_CLOCK_NAME);
	return 0;
}

int APP_OVERLAY_show_title(int vin, int stream, bool show)
{
	sdk_enc->show_overlay(vin, stream, APP_OVERLAY_TITLE_NAME, show);
	return 0;
}

int APP_OVERLAY_show_id(int vin, int stream, bool show)
{
	sdk_enc->show_overlay(vin, stream, APP_OVERLAY_ID_NAME, show);
	return 0;
}


int APP_OVERLAY_show_clock(int vin, int stream, bool show)
{
	sdk_enc->show_overlay(vin, stream, APP_OVERLAY_CLOCK_NAME, show);
	return 0; 
}

void APP_OVERLAY_id_display()
{
	SYSCONF_t *sysconf = SYSCONF_dup();
	int i, ii;
	char id_text[64];
	//memset(id_text, 0, sizeof(id_text));
	if(sysconf->ipcam.network.esee.enable_display){
		ESEE_CLIENT_INFO_t ret_info;
		ESEE_CLIENT_get_info(&ret_info);
		sprintf(id_text, "ID:%s", ret_info.id);
		for(i = 0; i < 1; ++i){
			for(ii = 0; ii < sysconf->ipcam.vin[0].enc_h264_ch; ++ii){
				lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas = sdk_enc->get_overlay_canvas(0, i, "id");
				OVERLAY_put_text(canvas, 0, 0, canvas->height, id_text, 0);
				sdk_enc->update_overlay(i, ii, APP_OVERLAY_ID_NAME);
			}
		}
	}
}

void APP_OVERLAY_task() // per second
{
	int i = 0, ii = 0;
	char clock_text[128] = {""};
	time_t t_now = 0;
	struct tm tm_now = {0};
	size_t text_width = 0, text_height = 0;
	char* weekday_map_chs[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六",};
	char* weekday_map_eng[] = {"Sun", "Mon", "Tue", "Web", "Thu", "Fri", "Sat",};
	
	int year = 0, month = 0, mday = 0, hour = 0;
	const char* date_format_yyyymmdd = "%04d/%02d/%02d";
	const char* date_format_mmddyyyy = "%02d/%02d/%04d";
	const char* date_format_ddmmyyyy = "%02d/%02d/%04d";
	const char* time_symbol_prefix = "";
	const char* time_symbol_suffix = "";
	const char* weekday = "";

	SYSCONF_t *sysconf = SYSCONF_dup();
	//printf("%s\r\n", __FUNCTION__);
	// all the clock overlay is reference to these 2 canvas
	// erase the canvas firstly
	if(_canvas_clock_font_32){
		_canvas_clock_font_32->erase_rect(_canvas_clock_font_32, 0, 0, 0, 0);
	}
	if(_canvas_clock_font_16){
		_canvas_clock_font_16->erase_rect(_canvas_clock_font_16, 0, 0, 0, 0);
	}

	time(&t_now);
	localtime_r(&t_now, &tm_now);
	hour = tm_now.tm_hour % 12;

	switch(sysconf->ipcam.generic.language.val){
		case SYS_LANGUAGE_CHINESE_MAINLAND:
			if(hour < 6){
				time_symbol_prefix = "凌晨";
			}else if(hour < 12){
				time_symbol_prefix = "上午";
			}else if(hour < 18){
				time_symbol_prefix = "下午";
			}else if(hour < 20){
				time_symbol_prefix = "傍晚";
			}else{
				time_symbol_prefix = "晚上";
			}
			weekday = weekday_map_chs[tm_now.tm_wday];
			date_format_yyyymmdd = "%04d年%02d月%02d日";
			date_format_mmddyyyy = "%02d月%02d日%04d年";
			date_format_ddmmyyyy = "%02d日%02d月%04d年";
			break;

		default:
			time_symbol_suffix = hour < 12 ? "a.m." : "p.m.";
			weekday = weekday_map_eng[tm_now.tm_wday];
			date_format_yyyymmdd = "%04d/%02d/%02d";
			date_format_mmddyyyy = "%02d/%02d/%04d";
			date_format_ddmmyyyy = "%02d/%02d/%04d";
			break;
	}

	// FIXME:
	hour = tm_now.tm_hour;
	time_symbol_prefix = "";
	time_symbol_suffix = "";

	// the date
	year = tm_now.tm_year + 1900;
	month = tm_now.tm_mon + 1;
	mday = tm_now.tm_mday;
	switch(sysconf->ipcam.date_time.date_format.val){
		case SYS_DATE_FORMAT_YYYYMMDD:
			snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), date_format_yyyymmdd,
				year, month, mday);
			break;
		case SYS_DATE_FORMAT_MMDDYYYY:
			snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), date_format_mmddyyyy,
				month, mday, year);
			break;
		case SYS_DATE_FORMAT_DDMMYYYY:
			snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), date_format_ddmmyyyy,
				mday, month, year);
			break;
		default:
			// invalid format
			return;
	}

	// the time
	strcat(clock_text, " ");

	// add symbol prefix
	snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), "%s", time_symbol_prefix);
	// add time clock
	snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), "%2d:%02d:%02d",
		hour, tm_now.tm_min, tm_now.tm_sec);
	snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), "%s", time_symbol_suffix);

	// weekday
	strcat(clock_text, " ");
	snprintf(clock_text + strlen(clock_text), sizeof(clock_text) - strlen(clock_text), "%s", weekday);

	// put text
	text_width = OVERLAY_put_text(_canvas_clock_font_32, 0, 0, OVERLAY_FONT_SIZE_32, clock_text, 0);
	text_width = OVERLAY_put_text(_canvas_clock_font_16, 0, 0, OVERLAY_FONT_SIZE_16, clock_text, 0);
	
	for(i = 0; i < 1; ++i){
		for(ii = 0; ii < sysconf->ipcam.vin[0].enc_h264_ch; ++ii){
			sdk_enc->update_overlay(i, ii, APP_OVERLAY_CLOCK_NAME);
		}
	}
}

int APP_OVERLAY_set_title(int vin, const char *title)
{
	int i = 0;
	SYSCONF_t* sysconf = SYSCONF_dup();
	
	for(i = 0; i < sysconf->ipcam.vin[vin].enc_h264_ch; ++i){
		lpSDK_ENC_VIDEO_OVERLAY_CANVAS canvas = sdk_enc->get_overlay_canvas(vin, i, "title");
		OVERLAY_put_text(canvas, 0, 0, canvas->height, title, 0);
	}
	return 0;
}

static char _overlay_file[128];
#define OVERLAY_FILE_PATH(bname) \
	(strcpy(_overlay_file, getenv("FONTDIR")), \
	'/' != _overlay_file[strlen(_overlay_file) - 1] ? strcat(_overlay_file, "/") : 0, \
	strcat(_overlay_file, (bname)), \
	printf("%s\r\n", _overlay_file), \
	_overlay_file)


int APP_OVERLAY_init()
{	
	OVERLAY_init();
    //OVERLAY_load_font(OVERLAY_FONT_SIZE_16, OVERLAY_FILE_PATH("asc16"), OVERLAY_FILE_PATH("hzk16")); // FIXME:
    //OVERLAY_load_font(OVERLAY_FONT_SIZE_32, OVERLAY_FILE_PATH("asc32"), OVERLAY_FILE_PATH("hzk32")); // FIXME:
    OVERLAY_load_font(OVERLAY_FONT_SIZE_16, "/root/data/font/asc16", "/root/data/font/hzk16"); // FIXME:
    OVERLAY_load_font(OVERLAY_FONT_SIZE_32, "/root/data/font/asc32", "/root/data/font/hzk32");
	return 0;
}

void APP_OVERLAY_destroy()
{
	if(_canvas_clock_font_16){
		sdk_enc->release_overlay_canvas(_canvas_clock_font_16);
		_canvas_clock_font_16 = NULL;
	}
	if(_canvas_clock_font_32){
		sdk_enc->release_overlay_canvas(_canvas_clock_font_32);
		_canvas_clock_font_32 = NULL;
	}
}


