
#include "version.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
//#include "rtspd.h"
#include "sensor.h"
//#include "spook/spook.h"
#include "sysconf.h"
#include "bsp/watchdog.h"

#include "firmware.h"

#include "ipcam_network.h"
#include "ipcam_timer.h"
#include "ipcam.h"
#include "gpio.h"
#include "app_debug.h"
#include "generic.h"
#include "ucode.h"
#include "bsp/rtc.h"

#include "app_overlay.h"
#include "usrm.h"
#include "app_motion_detect.h"


static void ipcam_signal_escape(int sig)
{
	APP_TRACE("Hey!");
	fclose(stdin);
}

static void ipcam_signal_init()
{
	signal(SIGINT, ipcam_signal_escape);
	signal(SIGQUIT, ipcam_signal_escape);
}

static void ipcam_signal_destroy()
{
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
}

static void ipcam_mediabuf_avenc_init()
{
	SYSCONF_t* sysconf = SYSCONF_dup();
	int i = 0, ii = 0, iii = 0;

	MEDIABUF_init();
	SDK_init_enc();
	
	APP_OVERLAY_init();

	sdk_enc->do_buffer_request = MEDIABUF_in_request;
	sdk_enc->do_buffer_append = MEDIABUF_in_append;
	sdk_enc->do_buffer_commit = MEDIABUF_in_commit;

	for(i = 0; i < sysconf->ipcam.spec.vin; ++i){
		// video input
		SYS_VIN_t* const vin = sysconf->ipcam.vin + i;
		for(ii = 0; ii < vin->enc_h264_ch; ++ii){
			// video h264 encode
			SYS_VIN_ENC_H264_t* const enc_h264 = vin->enc_h264 + ii;
			for(iii = 0; iii < enc_h264->stream_ch; ++iii){
				// video h264 encode stream
				SYS_VIN_ENC_H264_STREAM_t* const h264_stream = enc_h264->stream + iii;
				char preferred_name[32] = {""};
				char alternate_name[32] = {""};
				sprintf(preferred_name, "ch%d_%d.264", i, ii);
				sprintf(alternate_name, "%s", h264_stream->name);
				
				// create mediabuf for buffering

				int const enc_fps = h264_stream->fps;
				int const enc_gop = h264_stream->gop;
				int const enc_width = 0;

				int const entry_available = 100;
				int const entry_key_available = 1;
				int const user_available = h264_stream->on_demand;
				
				
				if(0 == MEDIABUF_new(0, preferred_name, alternate_name, entry_available, entry_key_available, user_available)){
					// success to new a mediabuf
					stSDK_ENC_H264_STREAM_ATTR video_stream_attr;
					
					int venc_id = 0;
					int const buf_id = MEDIABUF_lookup_byname(preferred_name);

					//uint32_t stream_vin_fps = (SYS_VIN_DIGITAL_SHUTTER_50HZ == sysconf->ipcam.vin[0].digital_shutter.val) ? 25 : 30;
					uint32_t stream_vin_fps = 30;
					uint32_t stream_fps = h264_stream->fps;
					if(stream_fps < 4){
						stream_fps = 4;
					}
					//uint32_t stream_gop = h264_stream->gop;
					uint32_t stream_gop = stream_fps * 2;
					uint32_t stream_bps = h264_stream->bps;
					uint32_t stream_width = 0, stream_height = 0, stream_max_frame = 0;
					uint32_t stream_quality = h264_stream->quality.val;
					enSDK_ENC_H264_RC_MODE stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR;
					SYS_VIN_SIZE_WIDTH_HEIGHT(h264_stream->size, stream_width, stream_height);
					
					assert(stream_width > 0 && stream_height > 0);
					switch(h264_stream->mode.val){
					case SYS_VENC_H264_MODE_VBR:
						stream_rc_mode = kSDK_ENC_H264_RC_MODE_VBR; break;
					case SYS_VENC_H264_MODE_CBR:
						stream_rc_mode = kSDK_ENC_H264_RC_MODE_CBR; break;
					case SYS_VENC_H264_MODE_ABR:
						stream_rc_mode = kSDK_ENC_H264_RC_MODE_ABR; break;
					default:
						stream_rc_mode = kSDK_ENC_H264_RC_MODE_AUTO; break;
					}

					video_stream_attr.vin_fps = stream_vin_fps;
					video_stream_attr.width = stream_width;
					video_stream_attr.height = stream_height;
					//video_stream_attr.fps = h264_stream->fps;
					video_stream_attr.fps = stream_fps;
					//video_stream_attr.gop = h264_stream->gop;
					video_stream_attr.gop = stream_gop;
					video_stream_attr.profile = kSDK_ENC_H264_PROFILE_BASELINE;
					video_stream_attr.rc_mode = stream_rc_mode;
					video_stream_attr.bps = h264_stream->bps;
					video_stream_attr.quality = 0;
					video_stream_attr.frame_limit = 0;
					video_stream_attr.buf_id = buf_id;
					video_stream_attr.start = false;

					// start video stream
					sdk_enc->create_h264_stream(preferred_name, i, ii, &video_stream_attr);
					//sdk_enc->create_g711a_stream(i, i);

					// start audio stream

					if(stream_width > 640){
						APP_OVERLAY_create_clock(i, ii, 0, 0, OVERLAY_FONT_SIZE_32);
						APP_OVERLAY_create_id(i, ii, stream_width/2 + 20, 0, OVERLAY_FONT_SIZE_32);
						APP_OVERLAY_create_title(i, ii, 0, stream_height - 32, OVERLAY_FONT_SIZE_32);
					}else{
						APP_OVERLAY_create_clock(i, ii, 0, 0, OVERLAY_FONT_SIZE_16);
						APP_OVERLAY_create_title(i, ii, 0, stream_height - 16, OVERLAY_FONT_SIZE_16);
					}

					APP_OVERLAY_update_clock(i, ii, 0, 0);
					APP_OVERLAY_show_clock(i, ii, true);
					APP_OVERLAY_update_id(i, ii, 0, 0);
					APP_OVERLAY_show_id(i, ii, true);
					APP_OVERLAY_update_title(i, ii, 0, 0);
					APP_OVERLAY_show_title(i, ii, true);
						
					sdk_enc->start_h264_stream(i, ii);
				}
			}
		}
	}
}

static void ipcam_mediabuf_avenc_destroy()
{
	APP_OVERLAY_destroy();
	SDK_destroy_enc();
	SDK_destroy_vin();
	MEDIABUF_destroy();
}


void set_env()
{
	unsigned char str[128] = {0};
	setenv("DATADIR", "/root/data", 1);
	setenv("FONTDIR", "/root/data/font", 1);
#ifdef MAKE_IMAGE
	setenv("WEBDIR", "/root/data/web", 1);
	//setenv("WEBDIR", "./web", 1);
#else
	setenv("WEBDIR", "./web", 1);
#endif
	sprintf(str, "/root/data/%s_ipcam.ini", SOC_MODEL);
	setenv("FLASHMAP", str, 1);
	setenv("SYSCONF", "/dev/mtdblock3", 1);
}


int IPCAM_init()
{
	int i = 0;
	SYSCONF_t *sysconf = NULL;
	
	ipcam_signal_init();
	
	//GPIO_init();
	set_env();
	RTC_init(1);
	SYSCONF_init(SOC_MODEL, PRODUCT_MODEL, "/dev/mtdblock3",
		SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	USRM_init("/dev/mtdblock2", NULL, NULL);
	ipcam_signal_init();
	WATCHDOG_init(10);

	SDK_init_sys(PRODUCT_MODEL);
	
	printf("%s\r\n", PRODUCT_MODEL);
	//LVIEW_init();
	SDK_ISP_init();

	// init audio 
	SDK_init_audio(kSDK_AUDIO_HW_SPEC_IGNORE);
	sdk_audio->init_ain(kSDK_AUDIO_SAMPLE_RATE_11025, 16);
	sdk_audio->create_ain_ch(0);
	sdk_audio->set_aout_loop(0);

	// sdk init video input
	SDK_init_vin(kSDK_VIN_HW_SPEC_IGNORE);
	
	
	ipcam_mediabuf_avenc_init();

	// motion detection
	APP_MD_init(1); // only one channel motion detection

	SENSOR_init();//sensor init must be later than avenc
	//FIRMWARE_init(getenv("FLASHMAP"), NULL);
	
	IPCAM_timer_init();
	IPCAM_network_init();

	sdk_enc->start();
	sdk_vin->start();

/*
	APP_MD_clear_mask(0);
	sdk_vin->set_md_trap(0, APP_MD_do_trap);
	sdk_vin->set_md_ref_freq(0, 1);
	APP_TRACE("haha");getchar();
	APP_MD_add_rect_mask(0, 0.0, 0.0, 0.2, 0.2);
	APP_TRACE("haha");getchar();
	APP_MD_add_rect_mask(0, 0.2, 0.2, 0.2, 0.2);
	APP_TRACE("haha");getchar();
	APP_MD_add_rect_mask(0, 0.4, 0.4, 0.2, 0.2);
	APP_TRACE("haha");getchar();
	APP_MD_add_rect_mask(0, 0.6, 0.6, 0.2, 0.2);
	APP_TRACE("haha");getchar();
	APP_MD_add_rect_mask(0, 0.8, 0.8, 0.2, 0.2);
	APP_TRACE("haha");getchar();
	APP_MD_commit_rect_mask(0);
	APP_TRACE("haha");getchar();
	sdk_vin->add_cover(0, "1", 0.2, 0.2, 0.6, 0.6, 0x0000ff, false);
	APP_TRACE("haha");getchar();
	sdk_vin->add_cover(0, "2", 0.0, 0.0, 0.2, 0.2, 0xff00ff, false);
	APP_TRACE("haha");getchar();
	sdk_vin->del_cover(0, "1");
	APP_TRACE("haha");getchar();
	*/
	return 0;
}

void IPCAM_destroy()
{

	IPCAM_timer_destroy();
	IPCAM_network_destroy();

	FIRMWARE_destroy();

	ipcam_mediabuf_avenc_destroy();

	SDK_destroy_vin();
	sdk_audio->release_ain_ch(0);
	sdk_audio->destroy_ain();
	SDK_destroy_audio();
	
	SDK_ISP_destroy();
	//LVIEW_destroy();
	SENSOR_destroy();
	SDK_destroy_sys();

	WATCHDOG_destroy();
	ipcam_signal_destroy();

}

int IPCAM_exec()
{
	char inbuf[128];
#define _IAPP_EXEC_POLL_INPUT() \
	(memset(inbuf, 0, sizeof(inbuf)), fgets(inbuf, sizeof(inbuf) - 1, stdin))
#define _IAPP_EXEC_MATCH_INPUT(cmd) \
	(0 == strncasecmp(inbuf, (cmd), strlen(cmd)))

	if(_IAPP_EXEC_POLL_INPUT()){
		if(_IAPP_EXEC_MATCH_INPUT("quit")){
			exit(1);
			return -1;
		}
		return 0;
	}
	exit(1);
	return -1;
}

static void ipcam_env_usage(const char* name)
{
	printf("%s not found! Please export %s=[%s]\r\n", name, name, name);
}

static void ipcam_env_check()
{
	int i = 0;
	const char* env_listp[] =
	{
		"FONTDIR", "WEBDIR", "FLASHMAP", "SYSCONF",
	};
	// check
	for(i = 0; i < sizeof(env_listp) / sizeof(env_listp[0]); ++i){
		if(!getenv(env_listp[i])){
			ipcam_env_usage(env_listp[i]);
		}
	}
	// printf
	for(i = 0; i < sizeof(env_listp) / sizeof(env_listp[0]); ++i){
		printf("%s = %s\r\n", env_listp[i], getenv(env_listp[i]));
	}

}

char* UCODEC_get_id()
{
	char* id_buf[64];
	memset(id_buf, 0, sizeof(id_buf));

	if(!UCODE_check(UCODE_SN_MTD, -1)){
		UCODE_read(UCODE_SN_MTD, -1, id_buf);
	}
	return id_buf;
}

void IPCAM_Info()
{
	set_env();
	SYSCONF_init(SOC_MODEL, PRODUCT_MODEL, "/dev/mtdblock3",
		SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	SYSCONF_t *sysconf = SYSCONF_dup();
	printf("Device Name: %s\r\n", sysconf->ipcam.info.device_name);
	printf("Device Model: %s\r\n", sysconf->ipcam.info.device_model);
	printf("Device ID: %s\r\n", UCODEC_get_id());
	printf("Device Software Version: %d.%d.%d %s\r\n", SWVER_MAJ, SWVER_MIN, SWVER_REV, SWVER_EXT);
	printf("\r\n");
	printf("Camera: %d\r\n", sysconf->ipcam.spec.vin);
	printf("Audio: %d\r\n", sysconf->ipcam.spec.ain);
	printf("Sensor: %d\r\n", sysconf->ipcam.spec.io_sensor);
	printf("Alarm: %d\r\n", sysconf->ipcam.spec.io_alarm);
	printf("Hard Disk Driver: 0\r\n");
	printf("Series Code: %s\r\n", SERISE_CODE);
	printf("\r\n\r\n");
}

int IPCAM_main(int argc, char** argv)
{
	if(strstr(argv[0], "READ_INFO") != NULL)
	{
		IPCAM_Info();
		exit(0);
	}
	else if(strstr(argv[0], "WSERIAL_NUM") != NULL)
	{
		UCODE_write(UCODE_SN_MTD, -1, argv[1], strlen(argv[1]));
		printf("write serial number finish\n");
		exit(0);
	}
	else if(strstr(argv[0], "RSERIAL_NUM") != NULL)
	{
		if(!UCODE_check(UCODE_SN_MTD, -1))
		{
			char ucode[32];
			UCODE_read(UCODE_SN_MTD, -1, ucode);
			printf("serial number:%s\n", ucode);
		}
		else
		{
			printf("serial number NOT init\n");
		}
		exit(0);
	}
	else if(strstr(argv[0], "RRTC") != NULL)
	{
		RTC_init(0);
		rtc_time_t rtc_tm;
		RTC_gettime(&rtc_tm);
		RTC_destroy();
	}
	else if(strstr(argv[0], "WRTC") != NULL && argv[1] != NULL)
	{
		RTC_init(0);
		struct timeval tv;
		//printf("%s\r\n", argv[1]);		
		time_t curtime = (time_t)atoi(argv[1]);
		tv.tv_sec = curtime;
		settimeofday(&tv,NULL);
		printf("int:%d\r\n", curtime);
		RTC_settime((time_t)tv.tv_sec);
		RTC_destroy();
	}
	else if(strstr(argv[0], "ipcam_app") != NULL)
	{
		printf("enter main application\r\n");
		IPCAM_Info();
		IPCAM_init();
		while(0 == IPCAM_exec());
		IPCAM_destroy();
	}

	exit(0);
}

