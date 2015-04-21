
#include "unistruct_debug.h"
#include "sysconf.h"
#include "generic.h"
#include "unistruct_gfun.h"
//#include "sdk/sdk_isp.h"
#include "sdk_isp.h" // in hi_isp_tmp


/////////////////////////////////////////////////////////////////////////////////////////
// map tables
//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV

typedef const char* const SYSCONF_MAPTBL_t;
typedef struct SYSCONF_ENUMTBL
{
	SYS_U64_t val;
	const SYS_CHR_t* text;
}SYSCONF_ENUMTBL_t;

static SYSCONF_MAPTBL_t level_map[] =
{
	[SYS_LEVEL_HIGHEST] = "highest",
	[SYS_LEVEL_HIGH] = "high",
	[SYS_LEVEL_MEDIUM] = "medium",
	[SYS_LEVEL_LOW] = "low",
	[SYS_LEVEL_LOWEST] = "lowest",
};

static SYSCONF_MAPTBL_t date_format_map[] =
{
	[SYS_DATE_FORMAT_YYYYMMDD] = "yyyymmdd",
	[SYS_DATE_FORMAT_MMDDYYYY] = "mmddyyyy",
	[SYS_DATE_FORMAT_DDMMYYYY] = "ddmmyyyy",
};

static SYSCONF_MAPTBL_t date_separator_map[] =
{
	[SYS_DATE_SPEC_DOT] = ".",
	[SYS_DATE_SPEC_DASH] = "-",
	[SYS_DATE_SPEC_SLASH] = "/",
};

static SYSCONF_MAPTBL_t time_format_map[] =
{
	[SYS_TIME_FORMAT_12HOUR] = "12hour",
	[SYS_TIME_FORMATE_24HOUR] = "24hour",
};

static SYSCONF_MAPTBL_t ain_sample_rate_map[] =
{
	[SYS_AIN_SAMPLE_RATE_8KBPS] = "8kbps",
	[SYS_AIN_SAMPLE_RATE_16KBPS] = "16kbps",
};

static SYSCONF_MAPTBL_t ain_sample_width_map[] =
{
	[SYS_AIN_SAMPLE_WIDTH_8BITS] = "8bits",
	[SYS_AIN_SAMPLE_WIDTH_16BITS] = "16bits",
};

static SYSCONF_MAPTBL_t vin_digital_shutter_map[] =
{
	[SYS_VIN_DIGITAL_SHUTTER_INVALID] = "invalid",
	[SYS_VIN_DIGITAL_SHUTTER_50HZ] = "50hz",
	[SYS_VIN_DIGITAL_SHUTTER_60HZ] = "60hz",
};

static SYSCONF_MAPTBL_t vin_analog_standard_map[] =
{
	[SYS_VIN_ANALOG_STANDARD_INVALID] = "invalid",
	[SYS_VIN_ANALOG_STANDARD_PAL] = "pal",
	[SYS_VIN_ANALOG_STANDARD_NTSC] = "ntsc",
};

static SYSCONF_MAPTBL_t ptz_protocol_map[] =
{
	[SYS_PTZ_PROTOCOL_PELCO_D] = "pelco-d",
	[SYS_PTZ_PROTOCOL_PELCO_P] = "pelco-p",
};

static SYSCONF_MAPTBL_t ptz_baudrate_map[] =
{
	[SYS_PTZ_BAUDRATE_2400] = "2400",
	[SYS_PTZ_BAUDRATE_4800] = "4800",
	[SYS_PTZ_BAUDRATE_9600] = "9600",
	[SYS_PTZ_BAUDRATE_38400] = "38400",
	[SYS_PTZ_BAUDRATE_115200] = "115200",
};



static SYSCONF_ENUMTBL_t language_enum[] =
{
	{ SYS_LANGUAGE_ENGLISH, "english" },
	{ SYS_LANGUAGE_CHINESE_MAINLAND, "chinese mainland" },
	{ SYS_LANGUAGE_CHINESE_HONGKONG, "chinese hongkong" },
	{ SYS_LANGUAGE_CHINESE_TAIWAN, "chinese taiwan" },
};

static SYSCONF_ENUMTBL_t vin_size_enum[] =
{
	{ SYS_VIN_SIZE_QCIF, "qcif" },
	{ SYS_VIN_SIZE_CIF, "cif" },
	{ SYS_VIN_SIZE_HALFD1, "halfd1" },
	{ SYS_VIN_SIZE_D1, "d1" },
	{ SYS_VIN_SIZE_QQ960H, "qq960h" },
	{ SYS_VIN_SIZE_Q960H, "q960h" },
	{ SYS_VIN_SIZE_HALF960H, "half960h" },
	{ SYS_VIN_SIZE_960H, "960h" },
	{ SYS_VIN_SIZE_180P, "180p" },
	{ SYS_VIN_SIZE_360P, "360p" },
	{ SYS_VIN_SIZE_720P, "720p" },
	{ SYS_VIN_SIZE_QVGA, "qvga" },
	{ SYS_VIN_SIZE_VGA, "vga" },
};

static SYSCONF_ENUMTBL_t vin_enc_h264_mode_enum[] =
{
	{ SYS_VENC_H264_MODE_VBR, "vbr" },
	{ SYS_VENC_H264_MODE_CBR, "cbr" },
	{ SYS_VENC_H264_MODE_ABR, "abr" },
	{ SYS_VENC_H264_MODE_FIXQP, "fixqp" },
};

static SYSCONF_ENUMTBL_t ddns_provider_enum[] =
{
	{ SYS_DDNS_PROVIDER_DYNDNS, "dyndns" },
	{ SYS_DDNS_PROVIDER_NOIP, "noip" },
	{ SYS_DDNS_PROVIDER_3322, "3322" },
	{ SYS_DDNS_PROVIDER_CHANGEIP, "changeip" },
	{ SYS_DDNS_PROVIDER_POPDVR, "popdvr" },
	{ SYS_DDNS_PROVIDER_SKYBEST, "skybest" },
	{ SYS_DDNS_PROVIDER_DVRTOP, "dvrtop" },
};





static SYSCONF_ENUMTBL_t isp_scene_mode[] = 
{
	{ISP_SCENE_MODE_AUTO, "auto"},
		{ISP_SCENE_MODE_INDOOR, "indoor"},
		{ISP_SCENE_MODE_OUTDOOR, "outdoor"},
};

static SYSCONF_ENUMTBL_t isp_ircut_control_mode[] = 
{
	{ISP_IRCUT_CONTROL_MODE_HARDWARE, "hardware"},
	{ISP_IRCUT_CONTROL_MODE_SOFTWARE, "software"},
};

static SYSCONF_ENUMTBL_t isp_ircut_mode[] = 
{
	{ISP_IRCUT_MODE_AUTO, "auto"},
	{ISP_IRCUT_MODE_DAYLIGHT, "daylight"},
	{ISP_IRCUT_MODE_NIGHT, "night"},
};

static SYSCONF_ENUMTBL_t isp_exposure_mode[] =
{
	{ISP_EXPOSURE_MODE_AUTO, "auto"},
	{ISP_EXPOSURE_MODE_BRIGHT, "bright"},
	{ISP_EXPOSURE_MODE_DARK, "dark"},
};

static SYSCONF_ENUMTBL_t isp_gamma_table[] = 
{
	{ISP_ADVANCE_GAMMA_DEFAULT, "default"},
	{ISP_ADVANCE_GAMMA_NORMAL, "normal"},
	{ISP_ADVANCE_GAMMA_HIGH, "high"},
};


static const char* unistruct_map(int val, const char** map, ssize_t map_size)
{
	if(val < map_size){
		return map[val];
	}
	return SYS_NULL;
}

static int unistruct_umap(const char* text, const char** map, ssize_t map_size)
{
	int i = 0;
	for(i = 0; i < map_size; ++i){
		if(0 == strcmp(text, map[i])){
			return i;
		}
	}
	return -1;
}

static const char* unistruct_enum(SYS_U32_t val, SYSCONF_ENUMTBL_t* enu, ssize_t enu_size)
{
	int i = 0;
	for(i = 0; i < enu_size; ++i){
		if(val == enu[i].val){

			return enu[i].text;
		}
	}
	return SYS_NULL;
}

static SYS_U32_t unistruct_uenum(const char* text, SYSCONF_ENUMTBL_t* enu, ssize_t enu_size)
{
	int i = 0;
	for(i = 0; i < enu_size; ++i){
		if(0 == strcmp(text, enu[i].text)){
			return enu[i].val;
		}
	}
	return -1;
}

#define SYSCONF_MAP(int_val, tbl_map) unistruct_map(int_val, (const char**)tbl_map, ARRAY_ITEM(tbl_map))
#define SYSCONF_UMAP(str_text, tbl_map) unistruct_umap(str_text, (const char**)tbl_map, ARRAY_ITEM(tbl_map))
#define SYSCONF_ENUM(enu_val, tbl_enu) unistruct_enum(enu_val, tbl_enu, ARRAY_ITEM(tbl_enu))
#define SYSCONF_UENUM(str_text, tbl_enu) unistruct_uenum(str_text, tbl_enu, ARRAY_ITEM(tbl_enu))


//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
// map tables
/////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
typedef uint32_t SYSCONF_TYPE_t;


#define SYSCONF_TYPE_ENUM (0x0002)

#define SYSCONF_TYPE_BOOL (0x1000)

#define SYSCONF_TYPE_U8 (0x2000)
#define SYSCONF_TYPE_U16 (0x2001)
#define SYSCONF_TYPE_U32 (0x2002)
#define SYSCONF_TYPE_S8 (0x2003)
#define SYSCONF_TYPE_S16 (0x2004)
#define SYSCONF_TYPE_S32 (0x2005)

#define SYSCONF_TYPE_RANGE_U8 (0x3000)
#define SYSCONF_TYPE_RANGE_U16 (0x3001)
#define SYSCONF_TYPE_RANGE_U32 (0x3002)
#define SYSCONF_TYPE_RANGE_S8 (0x3003)
#define SYSCONF_TYPE_RANGE_S16 (0x3004)
#define SYSCONF_TYPE_RANGE_S32 (0x3005)

#define SYSCONF_TYPE_CHAR (0x4000)

#define SYSCONF_TYPE_STRING (0x5000)

#define SYSCONF_TYPE_IPV4 (0x6000)
#define SYSCONF_TYPE_MAC (0x6001)

#define SYSCONF_TYPE_DATE (0x7000)
#define SYSCONF_TYPE_TIME (0x7001)

#define SYSCONF_TYPE_MAP (0x8000)
#define SYSCONF_TYPE_RATE8 (0x8001)
#define SYSCONF_TYPE_RATE16 (0x8002)
#define SYSCONF_TYPE_RATE32 (0x8003)

#define SYSCONF_TYPE_LEVEL (0x9000)

#define SYSCONF_TYPE_RESERVED (0xa000)
//////////////////////////////////////////////////////


typedef struct UNISTRUCT_SYSCONF
{
	char* xpath;
	SYSCONF_TYPE_t type; // the type of value
	void* val_ptr; // the value offset to sysconf value
	void* map_enu;
	ssize_t map_enu_size;

	UNISTRUCT_DO_CHECK do_check;
	UNISTRUCT_DO_POLICY do_policy;
}UNISTRUCT_SYSCONF_t;


//SYSCONF_t* p_sysconf;
#define SYSCONF_UNISTRUCT_COMMON_TABEL(p_sysconf) \
	{\
		/* spec */\
		{ "spec@vin", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.vin, NULL, 0 , NULL, NULL, },\
		{ "spec@ain", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.ain, NULL, 0 , NULL, NULL, },\
		{ "spec@io_sensor", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.io_sensor, NULL, 0 , NULL, NULL, },\
		{ "spec@io_alarm", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.io_alarm, NULL, 0 , NULL, NULL, },\
		{ "spec@hdd", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.hdd, NULL, 0 , NULL, NULL, },\
		{ "spec@sd_card", SYSCONF_TYPE_U8, &p_sysconf->ipcam.spec.sd_card, NULL, 0 , NULL, NULL, },\
		/* info */\
		{ "info@device_name", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.device_name, NULL, 0 , NULL, NULL, },\
		{ "info@device_model", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.device_model, NULL, 0 , NULL, NULL, },\
		{ "info@device_soc", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.device_soc, NULL, 0 , NULL, NULL, },\
		{ "info@device_sn", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.device_sn, NULL, 0 , NULL, NULL, },\
		{ "info@device_sn_head", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.device_sn_head, NULL, 0 , NULL, NULL, },\
		{ "info@sensor_type", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.sensor_type, NULL, 0 , NULL, NULL, },\
		{ "info@hardware_version", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.hardware_version, NULL, 0 , NULL, NULL, },\
		{ "info@software_version", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.info.software_version, NULL, 0 , NULL, NULL, },\
		{ "info@build_date", SYSCONF_TYPE_DATE, &p_sysconf->ipcam.info.build_date, NULL, 0 , NULL, NULL, },\
		{ "info@build_time", SYSCONF_TYPE_TIME, &p_sysconf->ipcam.info.build_time, NULL, 0 , NULL, NULL, },\
		/* date time */\
		{ "datetime@date_format", SYSCONF_TYPE_MAP, &p_sysconf->ipcam.date_time.date_format, (char**)date_format_map, ARRAY_ITEM(date_format_map) , NULL, NULL, },\
		{ "datetime@date_separator", SYSCONF_TYPE_MAP, &p_sysconf->ipcam.date_time.date_separator, (char**)date_separator_map, ARRAY_ITEM(date_separator_map) , NULL, NULL, },\
		{ "datetime@time_format", SYSCONF_TYPE_MAP, &p_sysconf->ipcam.date_time.time_format, (char**)time_format_map, ARRAY_ITEM(time_format_map) , NULL, NULL, },\
		{ "datetime@time_zone", SYSCONF_TYPE_RANGE_S8, &p_sysconf->ipcam.date_time.time_zone, NULL, 0 , NULL, do_policy_timezone, },\
		{ "datetime@day_saving_time", SYSCONF_TYPE_RANGE_S8, &p_sysconf->ipcam.date_time.day_saving_time, NULL, 0 , NULL, NULL, },\
		{ "datetime@ntp_sync", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.date_time.ntp_sync, NULL, 0 , NULL, NULL, },\
		{ "datetime@ntp_user_domain", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.date_time.ntp_user_domain, NULL, 0 , NULL, NULL, },\
		/* generic */\
		{ "generic@key_buzzer", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.generic.key_buzzer, NULL, 0 , NULL, NULL, },\
		{ "generic@idle_timeout", SYSCONF_TYPE_U16, &p_sysconf->ipcam.generic.idle_timeout, NULL, 0 , NULL, NULL, },\
		{ "generic@irda_id", SYSCONF_TYPE_U16, &p_sysconf->ipcam.generic.irda_id, NULL, 0 , NULL, NULL, },\
		{ "generic@language", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.generic.language, (SYS_ENUM_t*)language_enum, ARRAY_ITEM(language_enum) , NULL, NULL, },\
		/* ain */\
		/* vin */\
		/* ptz */\
		/* network */\
		{ "network@mac", SYSCONF_TYPE_MAC, &p_sysconf->ipcam.network.mac, NULL, 0 , NULL, NULL, },\
		/* lan */\
		{ "network/lan@dhcp", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.lan.dhcp, NULL, 0 , NULL, do_policy_network, },\
		{ "network/lan@static_ip", SYSCONF_TYPE_IPV4, &p_sysconf->ipcam.network.lan.static_ip, NULL, 0 , NULL, do_policy_network, },\
		{ "network/lan@static_netmask", SYSCONF_TYPE_IPV4, &p_sysconf->ipcam.network.lan.static_netmask, NULL, 0 , NULL, do_policy_network, },\
		{ "network/lan@static_gateway", SYSCONF_TYPE_IPV4, &p_sysconf->ipcam.network.lan.static_gateway, NULL, 0 , NULL, do_policy_network, },\
		{ "network/lan@static_preferred_dns", SYSCONF_TYPE_IPV4, &p_sysconf->ipcam.network.lan.static_preferred_dns, NULL, 0 , NULL, do_policy_network, },\
		{ "network/lan@static_alternate_dns", SYSCONF_TYPE_IPV4, &p_sysconf->ipcam.network.lan.static_alternate_dns, NULL, 0 , NULL, NULL, },\
		/* pppoe */\
		{ "network/pppoe@enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.pppoe.enable, NULL, 0 , NULL, NULL, },\
		{ "network/pppoe@username", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.pppoe.username, NULL, 0 , NULL, NULL, },\
		{ "network/pppoe@password", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.pppoe.password, NULL, 0 , NULL, NULL, },\
		/* ddns */\
		{ "network/ddns@enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.ddns.enable, NULL, 0 , NULL, NULL, },\
		{ "network/ddns@provider", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.network.ddns.provider, (char**)ddns_provider_enum, ARRAY_ITEM(ddns_provider_enum) , NULL, NULL, },\
		{ "network/ddns@url", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.ddns.url, NULL, 0 , NULL, NULL, },\
		{ "network/ddns@username", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.ddns.username, NULL, 0 , NULL, NULL, },\
		{ "network/ddns@password", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.ddns.password, NULL, 0 , NULL, NULL, },\
		/* 3g */\
		{ "network/threeg@enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.threeg.enable, NULL, 0 , NULL, NULL, },\
		{ "network/threeg@apn", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.threeg.apn, NULL, 0 , NULL, NULL, },\
		{ "network/threeg@pin", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.threeg.pin, NULL, 0 , NULL, NULL, },\
		{ "network/threeg@username", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.threeg.username, NULL, 0 , NULL, NULL, },\
		{ "network/threeg@password", SYSCONF_TYPE_STRING, &p_sysconf->ipcam.network.threeg.password, NULL, 0 , NULL, NULL, },\
		/* esee */\
		{ "network/esee@enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.esee.enable, NULL, 0 , NULL, NULL, },\
		{ "network/esee@id_disp", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.network.esee.enable_display, NULL, 0 , NULL, NULL, },\
		/* isp */\
		{ "isp/image_attr@hue", SYSCONF_TYPE_RATE16, &p_sysconf->ipcam.isp.image_attr.hue, NULL, 0 , NULL, do_policy_hue, },\
		{ "isp/image_attr@contrast", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.image_attr.contrast, NULL, 0 , NULL, do_policy_contrast, },\
		{ "isp/image_attr@brightness", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.image_attr.brightness, NULL, 0 , NULL, do_policy_brightness, },\
		{ "isp/image_attr@saturation", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.image_attr.saturation, NULL, 0 , NULL, do_policy_saturation, },\
		{ "isp/image_attr@sharpen", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.image_attr.sharpen, NULL, 0 , NULL, do_policy_sharpen, },\
		{ "isp/image_attr@flip", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.image_attr.flip, NULL, 0 , NULL, do_policy_flip, },\
		{ "isp/image_attr@mirror", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.image_attr.mirror, NULL, 0 , NULL, do_policy_mirror, },\
		{ "isp/scene@mode", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.scene_mode, (SYS_ENUM_t*)isp_scene_mode, ARRAY_ITEM(isp_scene_mode) , NULL, do_policy_scene_mode, },\
		{ "isp/white_balance@mode", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.white_balance_mode, (SYS_ENUM_t*)isp_scene_mode, ARRAY_ITEM(isp_scene_mode) , NULL, do_policy_isp_white_balance_mode, },\
		{ "isp/day_night_mode@ircut_control_mode", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.day_night_mode.ircut_control_mode, (SYS_ENUM_t*)isp_ircut_control_mode, ARRAY_ITEM(isp_ircut_control_mode) , NULL, do_policy_isp_ircut_control_mode, },\
		{ "isp/day_night_mode@ircut_mode", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.day_night_mode.ircut_mode, (SYS_ENUM_t*)isp_ircut_mode, ARRAY_ITEM(isp_ircut_mode) , NULL, do_policy_isp_ircut_mode, },\
		{ "isp/wide_dynamic_range@enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.wide_dynamic_range.enable, NULL, 0 , NULL, do_policy_isp_wdr_enable, },\
		{ "isp/wide_dynamic_range@strength", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.wide_dynamic_range.strength, NULL, 0 , NULL, do_policy_isp_wdr_strength, },\
		{ "isp/exposure@mode", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.exposure.mode, (SYS_ENUM_t*)isp_exposure_mode, ARRAY_ITEM(isp_exposure_mode) , NULL, do_policy_isp_exposure_mode, },\
		{ "isp/exposure@ae_compensation", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.exposure.ae_compensation, NULL, 0 , NULL, do_policy_isp_exposure_ae_compensation, },\
		{ "isp/denoise@denoise_enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.denoise.denoise_enable, NULL, 0 , NULL, do_policy_isp_denoise_enable, },\
		{ "isp/denoise@denoise_strength", SYSCONF_TYPE_RATE8, &p_sysconf->ipcam.isp.denoise.denoise_strength, NULL, 0 , NULL, do_policy_isp_denoise_strength, },\
		{ "isp/advance@anti_fog_enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.advance.anti_fog_enable, NULL, 0 , NULL, do_policy_isp_anti_fog_enable, },\
		{ "isp/advance@lowlight_enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.advance.lowlight_enable, NULL, 0 , NULL, do_policy_isp_lowlight_enable, },\
		{ "isp/advance@gamma", SYSCONF_TYPE_ENUM, &p_sysconf->ipcam.isp.advance.gamma, (SYS_ENUM_t*)isp_gamma_table, ARRAY_ITEM(isp_gamma_table) , NULL, do_policy_isp_gamma, },\
		{ "isp/advance@defect_pixel_enable", SYSCONF_TYPE_BOOL, &p_sysconf->ipcam.isp.advance.defect_pixel_enable, NULL, 0 , NULL, do_policy_isp_defect_pixel_enable, },\	
	}

static UNISTRUCT_SYSCONF_t* unistruct_common_find(UNISTRUCT_SYSCONF_t* table, ssize_t table_size, const char* xpath)
{
	int i = 0;
	for(i = 0; i < table_size; ++i){
		if(0 == strcmp(xpath, table[i].xpath)){\
			return table + i;
		}
	}
	return NULL;
}

static int unistruct_item_rw(UNISTRUCT_SYSCONF_t* unistruct, char* io_text, SYS_BOOL_t opt_rw)
{
	int ret = -1;
	switch(unistruct->type)
	{
	case SYSCONF_TYPE_ENUM:
		{
			//SYS_ENUM_t* const val_ptr = (SYS_ENUM_t*)unistruct->val_ptr;
			SYS_ENUM_t val_ptr;
			memcpy(&val_ptr, unistruct->val_ptr, sizeof(SYS_ENUM_t));
			if(opt_rw){
				//sprintf(io_text, "%s", unistruct_enum(val_ptr->val, unistruct->map_enu, unistruct->map_enu_size));
				//printf("val_ptr.val = %d\r\n", val_ptr.val);
				sprintf(io_text, "%s", unistruct_enum(val_ptr.val, unistruct->map_enu, unistruct->map_enu_size));
				ret = 0;
			}else{
				int _val = unistruct_uenum(io_text, unistruct->map_enu, unistruct->map_enu_size);		
				//printf("%s:val = %d\r\n",io_text, _val);
				if(_val != -1){
					val_ptr.val = _val;
					memcpy(unistruct->val_ptr, &val_ptr, sizeof(SYS_ENUM_t));
					ret = 0;
				}
			}
		}
		break;

	case SYSCONF_TYPE_BOOL:
		{
			//SYS_BOOL_t* val_ptr = (SYS_BOOL_t*)unistruct->val_ptr;
			SYS_BOOL_t val_ptr;
			memcpy(&val_ptr, unistruct->val_ptr, sizeof(SYS_BOOL_t));
			if(opt_rw){
				sprintf(io_text, "%s", val_ptr ? "yes" : "no");
				ret = 0;
			}else{
				if(strcasecmp(io_text, "yes") == 0 || strcasecmp(io_text, "no") == 0){
					val_ptr = (0 == strcasecmp(io_text, "yes"));
					memcpy(unistruct->val_ptr, &val_ptr, sizeof(SYS_BOOL_t));
					ret = 0;
				}
			}
		}
		break;

#define SYSCONF_RANGE_INT_PROC(range_ptr, str_val, rw, type) \
	do{\
		type val_ptr;\
		memcpy(&val_ptr, range_ptr, sizeof(type));\
		if(rw){\		
			sprintf(str_val, "%d", val_ptr.val);\
			ret = 0;\
		}else{\
			type range_val;\
			range_val.val = atoi(str_val);\
			if(range_val.val >= val_ptr.min && range_val.val <= val_ptr.max){\
				val_ptr.val = range_val.val;\
				memcpy(range_ptr, &val_ptr, sizeof(type));\
				ret = 0; \
			}\
		}\
	}while(0)

	case SYSCONF_TYPE_RANGE_S8:
		SYSCONF_RANGE_INT_PROC((SYS_RANGE_S8_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RANGE_S8_t);
		break;

	case SYSCONF_TYPE_RANGE_S16:
		SYSCONF_RANGE_INT_PROC((SYS_RANGE_S16_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RANGE_S16_t);
		break;

	case SYSCONF_TYPE_RANGE_S32:
		SYSCONF_RANGE_INT_PROC((SYS_RANGE_S32_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RANGE_S32_t);
		break;

	case SYSCONF_TYPE_MAP:
		{
			//SYS_MAP_t* const map_ptr = (SYS_MAP_t*)unistruct->val_ptr;
			UNISTRUCT_ASSERT(NULL != unistruct->map_enu, "no map found! please check unistruct table");
			SYS_MAP_t map_ptr;
			memcpy(&map_ptr, unistruct->val_ptr, sizeof(SYS_MAP_t));
			if(opt_rw){
				const char** map = unistruct->map_enu;
				strcpy(io_text, map[map_ptr.val]);
				ret = 0;
			}else{
				int map_val = unistruct_umap(io_text, unistruct->map_enu, unistruct->map_enu_size);
				printf("map_val = %d-%s\r\n", map_val, io_text);
				if(map_val <= map_ptr.max){
					map_ptr.val = map_val;
					ret = 0;
				}
				map_ptr.max = unistruct->map_enu_size;
				map_ptr.val = map_val;
				memcpy(unistruct->val_ptr, &map_ptr, sizeof(SYS_MAP_t));					
			}
		}
		break;


#define SYSCONF_UINT_PROC(uint_ptr, str_val, rw, type) \
	do{\
		if(rw){\
			type read_val;\
			memcpy(&read_val, uint_ptr, sizeof(type));\
			sprintf(str_val, "%u", read_val);\
			ret = 0; \
		}else{\
			type write_val = (type)atoi(str_val);\
			memcpy(uint_ptr, &write_val, sizeof(type)); \
			ret = 0; \
		}\
	}while(0)


	case SYSCONF_TYPE_U8:
		SYSCONF_UINT_PROC((SYS_U8_t*)unistruct->val_ptr, io_text, opt_rw, SYS_U8_t);
		break;
	case SYSCONF_TYPE_U16:
		SYSCONF_UINT_PROC((SYS_U16_t*)unistruct->val_ptr, io_text, opt_rw, SYS_U16_t);
		break;
	case SYSCONF_TYPE_U32:
		SYSCONF_UINT_PROC((SYS_U32_t*)unistruct->val_ptr, io_text, opt_rw, SYS_U32_t);
		break;

	case SYSCONF_TYPE_STRING:
		if(opt_rw){
			strcpy(io_text, unistruct->val_ptr);
			ret = 0;
		}else{
			strcpy(unistruct->val_ptr, io_text);
			ret = 0;
		}
		break;

#define SYSCONF_RATE_PROC(rate_ptr, str_val, rw, type) \
		do{\
			type rate;\
			memcpy(&rate, rate_ptr, sizeof(type));\
			if(rw){\
				sprintf(io_text, "%u/%u", rate.val, rate.max);\
				ret = 0; \
			}else{\
				SYS_U32_t val,max;\
				if(2 == sscanf(str_val, "%u/%u", &(val), &(max))){\
					UNISTRUCT_ASSERT(max == rate.max, "invaild max rate %u->%u", max, max);\
					rate.val = val;\
					memcpy(rate_ptr, &rate, sizeof(type));\
					ret = 0; \
				}\
			}\
		}while(0)


	case SYSCONF_TYPE_RATE8:
		SYSCONF_RATE_PROC((SYS_RATE8_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RATE8_t);
		break;

	case SYSCONF_TYPE_RATE16:
		SYSCONF_RATE_PROC((SYS_RATE16_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RATE16_t);
		break;

	case SYSCONF_TYPE_RATE32:
		SYSCONF_RATE_PROC((SYS_RATE32_t*)unistruct->val_ptr, io_text, opt_rw, SYS_RATE32_t);
		break;

	case SYSCONF_TYPE_LEVEL:
		{
			SYS_LEVEL_t val_ptr;
			memcpy(&val_ptr, unistruct->val_ptr, sizeof(SYS_LEVEL_t));
			if(opt_rw){
				strcpy(io_text, SYSCONF_MAP(val_ptr.val, level_map));
				ret = 0;
			}else{
				int _val = SYSCONF_UMAP(io_text, level_map);
				if(_val != -1){
					val_ptr.val = _val;
					memcpy(unistruct->val_ptr, &val_ptr, sizeof(SYS_LEVEL_t));
					ret = 0;
				}
			}
		}
		break;

#define SYSCONF_TYPE_IPV4_FORMAT "%d.%d.%d.%d"

	case SYSCONF_TYPE_IPV4:
		{
			//printf("i am ipaddress, val=%s\n", ((SYS_IP_ADDR_t*)unistruct->val_ptr)->s[0], );
			SYS_IP_ADDR_t ipv4_val;
			if(opt_rw){
				memcpy(&ipv4_val, unistruct->val_ptr, sizeof(SYS_IP_ADDR_t));
				sprintf(io_text, SYSCONF_TYPE_IPV4_FORMAT, ipv4_val.s1, ipv4_val.s2, ipv4_val.s3, ipv4_val.s4);
				ret = 0;
			}else{
				int s1, s2, s3, s4;
				int ipv4_ret = sscanf(io_text, SYSCONF_TYPE_IPV4_FORMAT, &s1, &s2, &s3, &s4);	
				if(ipv4_ret == 4){
//					s1 = ipv4_val->s1, s2 = ipv4_val->s2, s3 = ipv4_val->s3, s4 = ipv4_val->s4;
					ipv4_val.s1 = s1, ipv4_val.s2 = s2, ipv4_val.s3 = s3, ipv4_val.s4 =s4;
					memcpy(unistruct->val_ptr, &ipv4_val, sizeof(SYS_IP_ADDR_t));
					ret = 0;
				}
			}
		}
		break;

#define SYSCONF_TYPE_MAC_FORMAT "%02x-%02x-%02x-%02x-%02x-%02x"

	case SYSCONF_TYPE_MAC:
		{
			SYS_MAC_ADDR_t mac_val;
			if(opt_rw){
				memcpy(&mac_val, unistruct->val_ptr, sizeof(SYS_MAC_ADDR_t));
				sprintf(io_text, SYSCONF_TYPE_MAC_FORMAT,
					mac_val.s1, mac_val.s2, mac_val.s3, mac_val.s4, mac_val.s5, mac_val.s6);
				ret = 0;
			}else{
				SYS_U32_t s1, s2, s3, s4, s5, s6;
				int mac_ret = sscanf(io_text, SYSCONF_TYPE_MAC_FORMAT,
					&s1, &s2, &s3, &s4, &s5, &s6);
				if(mac_ret == 6){
					mac_val.s1 = s1, mac_val.s2 = s2, mac_val.s3 = s3,
						mac_val.s4 = s4, mac_val.s5 = s5, mac_val.s6 = s6;
					memcpy(unistruct->val_ptr, &mac_val, sizeof(SYS_MAC_ADDR_t));
					ret = 0;
				}
			}
		}
		break;

#define SYSCONF_TYPE_DATE_FORMAT "%04d/%02d/%02d"

	case SYSCONF_TYPE_DATE:
		{
			SYS_DATE_t val_ptr;
			if(opt_rw){
				memcpy(&val_ptr, unistruct->val_ptr, sizeof(SYS_DATE_t));
				sprintf(io_text, SYSCONF_TYPE_DATE_FORMAT, val_ptr.year, val_ptr.month, val_ptr.day);
				ret = 0;
			}else{
				int year, month, mday;
				int date_ret = sscanf(io_text, SYSCONF_TYPE_DATE_FORMAT, &year, &month, &mday);
				if(date_ret == 3){
					val_ptr.year = year, val_ptr.month = month, val_ptr.day = mday;
					memcpy(unistruct->val_ptr, &val_ptr, sizeof(SYS_DATE_t));
					ret = 0;
				}
			}
		}
		break;

#define SYSCONF_TYPE_TIME_FORMAT "%02d:%02d:%02d"

	case SYSCONF_TYPE_TIME:
		{
			SYS_TIME_t val_ptr;
			if(opt_rw){
				memcpy(&val_ptr, unistruct->val_ptr, sizeof(SYS_TIME_t));
				sprintf(io_text, SYSCONF_TYPE_TIME_FORMAT, val_ptr.hour, val_ptr.min, val_ptr.sec);
				ret = 0;
			}else{
				int hour, minute, second;
				int time_ret = sscanf(io_text, SYSCONF_TYPE_TIME_FORMAT, &hour, &minute, &second);
				if(time_ret== 3){
					val_ptr.hour = hour, val_ptr.min = minute, val_ptr.sec = second;
					memcpy(unistruct->val_ptr, &val_ptr, sizeof(SYS_TIME_t));
					ret = 0;
				}
			}
		}
		break;

	default:
		UNISTRUCT_TRACE("type = %x", unistruct->type);
		ret = -1;
		break;
	}

//	if(ret < 0){
//		UNISTRUCT_TRACE("unknown xpath = \"%s\"", unistruct->xpath);
//	}
	if(!opt_rw){
		//if(unistruct->do_check!= NULL && !unistruct->do_check(NULL, unistruct->val_ptr)){
			if(unistruct->do_policy){
				unistruct->do_policy(NULL, unistruct->val_ptr);
			}
		//}
	}
	return ret;
}


static int unistruct_common_rw(SYSCONF_t* const sysconf, const char* xpath, char* io_text, SYS_BOOL_t opt_rw)
{
	UNISTRUCT_SYSCONF_t* unistruct = NULL;
	// declare the unistruct
	UNISTRUCT_SYSCONF_t uni_comm_tbl[] = SYSCONF_UNISTRUCT_COMMON_TABEL(sysconf);
	// find the relevant struct
	unistruct = unistruct_common_find(uni_comm_tbl, ARRAY_ITEM(uni_comm_tbl), xpath);
	if(!unistruct){
		return -1;
	}
	return unistruct_item_rw(unistruct, io_text,opt_rw);
}

#define UNISTRUCT_COMMON_GET(sysconf,xpath,io_text) unistruct_common_rw(sysconf, xpath, (char*)io_text, SYS_TRUE)
#define UNISTRUCT_COMMON_SET(sysconf,xpath,io_text) unistruct_common_rw(sysconf, xpath, (char*)io_text, SYS_FALSE)

#include "unistruct_sysconf_hi3507_inception.c"
#include "unistruct_sysconf_hi3518a_inception.c"

static int unistruct_platform_rw(SYSCONF_t* const sysconf, const char* xpath, char* io_text, SYS_BOOL_t opt_rw)
{
//	return unistruct_hi3507_inception_rw(sysconf, xpath, io_text, opt_rw);
	return unistruct_hi3518a_inception_rw(sysconf, xpath, io_text, opt_rw);
}

#define UNISTRUCT_PLATFORM_GET(sysconf,xpath,io_text) unistruct_platform_rw(sysconf, xpath, (char*)io_text, SYS_TRUE)
#define UNISTRUCT_PLATFORM_SET(sysconf,xpath,io_text) unistruct_platform_rw(sysconf, xpath, (char*)io_text, SYS_FALSE)

int SYSCONF_UNISTRUCT_get(SYSCONF_t* const sysconf, const char* xpath, char* ret_text)
{
	int ret = 0;
	ret = UNISTRUCT_COMMON_GET(sysconf, xpath, ret_text);
	if(ret < 0){
		ret = UNISTRUCT_PLATFORM_GET(sysconf, xpath, ret_text);
	}
	if(ret < 0){
		UNISTRUCT_TRACE("unkwon or invalid xpath = %s", xpath);
	}
	return ret;
}

int SYSCONF_UNISTRUCT_set(SYSCONF_t* const sysconf, const char* xpath, const char* text)
{
	int ret = 0;
	ret = UNISTRUCT_COMMON_SET(sysconf, xpath, text);
	if(ret < 0){
		ret = UNISTRUCT_PLATFORM_SET(sysconf, xpath, text);
	}
//	if(ret < 0){
//		UNISTRUCT_TRACE("unkwon or invalid xpath = %s", xpath);
//	}
	return ret;
}



