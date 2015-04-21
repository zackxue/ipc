

#ifndef __SYSCONF_H__
#define __SYSCONF_H__
#ifdef	__cplusplus
extern	"C"	{
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "sysconf_type.h"

#define SYS_PTZ_TOUR_POINT_REF (32)

#define SYS_VENC_H264_CH_REF (4)
#define SYS_VENC_H264_STREAM_CH_REF (2)
#define SYS_VENC_JPEG_CH_REF (1)

#define SYS_AENC_CH_REF (2)

#define SYS_MD_WIDTH_REF (640)
#define SYS_MD_HEIGHT_REF (480)

#define SYS_NETWORK_PORT_REF (4)

#pragma pack(4)
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// spec about
typedef struct SYS_SPEC
{
	SYS_U8_t vin; // read only
	SYS_U8_t ain; // read only
	SYS_U8_t ptz; // read only
	SYS_U8_t io_sensor; // read only
	SYS_U8_t io_alarm; // read only
	SYS_U8_t hdd; // read only
	SYS_U8_t sd_card; // read only
}SYS_SPEC_t;

///////////////////////////////////////////////////////////////////////////
// info about
typedef struct SYS_INFO
{
	SYS_CHR_t device_name[16]; // read only
	SYS_CHR_t device_model[16]; // read only
	SYS_CHR_t device_soc[16];
	SYS_CHR_t device_sn_head[16];
	SYS_CHR_t device_sn[16]; // read only
	SYS_CHR_t sensor_type[16];
	SYS_CHR_t hardware_version[16]; // read only
	SYS_CHR_t software_version[32]; // read only
	SYS_DATE_t build_date; // read only
	SYS_TIME_t build_time; // read only
}SYS_INFO_t;

///////////////////////////////////////////////////////////////////////////

typedef struct SYS_DATE_TIME
{
#define SYS_DATE_FORMAT_YYYYMMDD (0)
#define SYS_DATE_FORMAT_MMDDYYYY (1)
#define SYS_DATE_FORMAT_DDMMYYYY (2)
	SYS_MAP_t date_format; // SYS_DATE_FORMAT_YYYYMMDD <= date_format <= SYS_DATE_FORMAT_DDMMYYYY
	
#define SYS_DATE_SPEC_DOT (0) // ','
#define SYS_DATE_SPEC_DASH (1) // '-'
#define SYS_DATE_SPEC_SLASH (2) // '/'
	SYS_MAP_t date_separator;
#define SYS_TIME_FORMAT_12HOUR (0)
#define SYS_TIME_FORMATE_24HOUR (1)
	SYS_MAP_t time_format;
	SYS_RANGE_S8_t time_zone; // -12 <= time_zone <= 12
	SYS_RANGE_S8_t day_saving_time; // -3 <= dst <= 3;

	SYS_BOOL_t ntp_sync;
	SYS_IP_ADDR_t ntp_sys_ip[4]; // default 4 // read only
	SYS_CHR_t ntp_user_domain[32];

}SYS_DATE_TIME_t;

typedef struct SYS_GENERIC
{
	SYS_BOOL_t key_buzzer;
	SYS_U16_t idle_timeout;
	SYS_U16_t irda_id;
#define SYS_LANGUAGE_ENGLISH (1<<0)
#define SYS_LANGUAGE_CHINESE_MAINLAND (1<<1)
#define SYS_LANGUAGE_CHINESE_HONGKONG (1<<2)
#define SYS_LANGUAGE_CHINESE_TAIWAN (1<<3)
	SYS_ENUM_t language;
}SYS_GENERIC_t;

///////////////////////////////////////////////////////////////////////////
// ain about
typedef struct SYS_AIN_ENC
{
	SYS_CHR_t engine[16]; // "g711", "aac", ... // read only
	SYS_U16_t packet;
}SYS_AIN_ENC_t;


typedef struct SYS_AIN
{
#define SYS_AIN_SAMPLE_RATE_8KBPS (0)
#define SYS_AIN_SAMPLE_RATE_16KBPS (1)
	SYS_MAP_t sample_rate;
#define SYS_AIN_SAMPLE_WIDTH_8BITS (0)
#define SYS_AIN_SAMPLE_WIDTH_16BITS (1)
	SYS_MAP_t sample_width;

	SYS_U8_t enc_ch; // read only
	SYS_AIN_ENC_t enc[SYS_AENC_CH_REF];
}SYS_AIN_t;


///////////////////////////////////////////////////////////////////////////
// vin about


typedef struct SYS_VIN_MD
{
	SYS_LEVEL_t sensitivity;
	SYS_PLANE_t size; // read only
	SYS_U64_t mask[(SYS_MD_HEIGHT_REF + 15) / 16][(((SYS_MD_WIDTH_REF + 15) / 16) + 63) / 64]; // 40 x 30 limited
}SYS_VIN_MD_t;

#define SYS_VIN_SIZE_QCIF (1<<0)
#define SYS_VIN_SIZE_CIF (1<<1)
#define SYS_VIN_SIZE_HALFD1 (1<<2)
#define SYS_VIN_SIZE_D1 (1<<3)
#define SYS_VIN_SIZE_QQ960H (1<<4)
#define SYS_VIN_SIZE_Q960H (1<<5)
#define SYS_VIN_SIZE_HALF960H (1<<6)
#define SYS_VIN_SIZE_960H (1<<7)
#define SYS_VIN_SIZE_180P (1<<8)
#define SYS_VIN_SIZE_360P (1<<9)
#define SYS_VIN_SIZE_720P (1<<10)
#define SYS_VIN_SIZE_QVGA (1<<11)
#define SYS_VIN_SIZE_VGA (1<<12)
#define SYS_VIN_SIZE_1080P (1<<13)
#define SYS_VIN_SIZE_130M (1<<14)



#define SYS_VIN_SIZE_WIDTH_HEIGHT(size, ret_width, ret_height) \
	do{\
		if(SYS_VIN_SIZE_QCIF==size.val){\
		}else if(SYS_VIN_SIZE_CIF==size.val){\
		}else if(SYS_VIN_SIZE_HALFD1==size.val){\
		}else if(SYS_VIN_SIZE_D1==size.val){\
		}else if(SYS_VIN_SIZE_QQ960H==size.val){\
		}else if(SYS_VIN_SIZE_Q960H==size.val){\
		}else if(SYS_VIN_SIZE_HALF960H==size.val){\
		}else if(SYS_VIN_SIZE_960H==size.val){\
		}else if(SYS_VIN_SIZE_180P==size.val){\
			ret_width = 320, ret_height = 180;\
		}else if(SYS_VIN_SIZE_360P==size.val){\
			ret_width = 640, ret_height = 360;\
		}else if(SYS_VIN_SIZE_720P==size.val){\
			ret_width = 1280, ret_height = 720;\
		}else if(SYS_VIN_SIZE_QVGA==size.val){\
			ret_width = 320, ret_height = 240;\
		}else if(SYS_VIN_SIZE_130M==size.val){\
			ret_width = 1280, ret_height = 960;\
		}else if(SYS_VIN_SIZE_1080P==size.val){\
			ret_width = 1920, ret_height = 1080;\
		}else if(SYS_VIN_SIZE_VGA==size.val){\
		}else{\
			printf("size map %x not found!\r\n", size.val);\
			assert(0);\
		}\
	}while(0)

typedef struct SYS_VIN_ENC_H264_STREAM
{
	SYS_CHR_t name[16]; // read only
	SYS_CHR_t profile[16]; // read only
	SYS_ENUM_t size;
// engine == "h264"
#define SYS_VENC_H264_MODE_VBR (1<<0) // variable bitrate
#define SYS_VENC_H264_MODE_CBR (1<<1) // constant bitrate
#define SYS_VENC_H264_MODE_ABR (1<<2) // average bitrate
#define SYS_VENC_H264_MODE_FIXQP (1<<3) // fixed qp
	SYS_ENUM_t mode;
	SYS_U8_t on_demand; // how many user could be on demand // read only
	SYS_U8_t fps;
	SYS_U16_t gop; // read only
	SYS_U8_t ain_bind; // where ain ch binding to this vin ch // read only
	SYS_LEVEL_t quality;
	SYS_U32_t bps; // uint kbps
}SYS_VIN_ENC_H264_STREAM_t;

typedef struct SYS_VIN_ENC_H264
{
	SYS_U8_t stream_ch; // read only
	SYS_VIN_ENC_H264_STREAM_t stream[SYS_VENC_H264_STREAM_CH_REF];
}SYS_VIN_ENC_H264_t;

typedef struct SYS_VIN_ENC_JPEG
{
	SYS_CHR_t name[16]; // read only
	SYS_LEVEL_t quality;
	SYS_ENUM_t size; // 720x576 1280x720 ...
}SYS_VIN_ENC_JPEG_t;


typedef struct SYS_VIN
{
#define SYS_VIN_DIGITAL_SHUTTER_INVALID (0)
#define SYS_VIN_DIGITAL_SHUTTER_50HZ (1)
#define SYS_VIN_DIGITAL_SHUTTER_60HZ (2)
	SYS_MAP_t digital_shutter;
#define SYS_VIN_ANALOG_STANDARD_INVALID (0)
#define SYS_VIN_ANALOG_STANDARD_PAL (1)
#define SYS_VIN_ANALOG_STANDARD_NTSC (2)
	SYS_MAP_t analog_standard;

	SYS_RATE16_t hue; // hue / 64
	SYS_RATE8_t contrast; // contrast / 64
	SYS_RATE8_t brightness; // brightness / 64
	SYS_RATE8_t saturation; // saturation / 64
	SYS_BOOL_t flip;
	SYS_BOOL_t mirror;
	SYS_CHR_t channel_name[32];
	// motion detect
	SYS_VIN_MD_t motion_detect;

	// enc
	SYS_U8_t enc_h264_ch; // read only
	SYS_VIN_ENC_H264_t enc_h264[SYS_VENC_H264_CH_REF];
	SYS_U8_t enc_jpeg_ch; // read only
	SYS_VIN_ENC_JPEG_t enc_jpeg[SYS_VENC_JPEG_CH_REF];
	
}SYS_VIN_t;

///////////////////////////////////////////////////////////////////////////
// ptz about
typedef struct SYS_PTZ_TOUR_POINT
{
	SYS_U8_t preset;
	SYS_TIME_t time;
}SYS_PTZ_TOUR_POINT_t;

typedef struct SYS_PTZ_TOUR
{
	SYS_RATE16_t active;
	SYS_PTZ_TOUR_POINT_t point[SYS_PTZ_TOUR_POINT_REF];
}SYS_PTZ_TOUR_t;

typedef struct SYS_PTZ
{
	SYS_U8_t addr;
#define SYS_PTZ_PROTOCOL_PELCO_D (0)
#define SYS_PTZ_PROTOCOL_PELCO_P (1)
	SYS_MAP_t protocol;
#define SYS_PTZ_BAUDRATE_2400 (0)
#define SYS_PTZ_BAUDRATE_4800 (1)
#define SYS_PTZ_BAUDRATE_9600 (2)
#define SYS_PTZ_BAUDRATE_38400 (3)
#define SYS_PTZ_BAUDRATE_115200 (4)
	SYS_MAP_t baudrate;
	SYS_PTZ_TOUR_t tour;
}SYS_PTZ_t;


///////////////////////////////////////////////////////////////////////////
// network about
typedef struct SYS_NETWORK_PPPOE
{
	SYS_BOOL_t enable;
	SYS_CHR_t username[32];
	SYS_CHR_t password[32];
}SYS_NETWORK_PPPOE_t;

typedef struct SYS_NETWORK_DDNS
{
	SYS_BOOL_t enable;
	//SYS_CHR_t provider[16]; // such as dyndns.org
#define SYS_DDNS_PROVIDER_DYNDNS (1<<0)
#define SYS_DDNS_PROVIDER_NOIP (1<<1)
#define SYS_DDNS_PROVIDER_3322 (1<<2)
#define SYS_DDNS_PROVIDER_CHANGEIP (1<<3)
#define SYS_DDNS_PROVIDER_POPDVR (1<<16)
#define SYS_DDNS_PROVIDER_SKYBEST (1<<17)
#define SYS_DDNS_PROVIDER_DVRTOP (1<<18)
	SYS_ENUM_t provider;
	SYS_CHR_t url[32]; // all zero with popdvr, skybest, dvrtop;
	SYS_CHR_t username[32];
	SYS_CHR_t password[32];
}SYS_NETWORK_DDNS_t;

typedef struct SYS_NETWORK_3G
{
	SYS_BOOL_t enable;
	SYS_CHR_t apn[32];
	SYS_CHR_t pin[32];
	SYS_CHR_t username[32];
	SYS_CHR_t password[32];
}SYS_NETWORK_3G_t;

typedef struct SYS_NETWORK_LAN_PORT
{
	SYS_CHR_t name[16]; // read only
	SYS_U16_t value;
}SYS_NETWORK_LAN_PORT_t;

typedef struct SYS_NETWORK_LAN
{
	SYS_BOOL_t dhcp;
	SYS_IP_ADDR_t static_ip;
	SYS_IP_ADDR_t static_netmask;
	SYS_IP_ADDR_t static_gateway;
	SYS_IP_ADDR_t static_preferred_dns;
	SYS_IP_ADDR_t static_alternate_dns;
	SYS_BOOL_t upnp;
	SYS_RATE8_t port_active; // read only
	SYS_NETWORK_LAN_PORT_t port[SYS_NETWORK_PORT_REF];
}SYS_NETWORK_LAN_t;

typedef struct SYS_NETWORK_ESEE
{
	SYS_BOOL_t enable;
	SYS_BOOL_t enable_display;
}SYS_NETWORK_ESEE_t;


typedef struct SYS_NETWORK
{
	SYS_MAC_ADDR_t mac;
	SYS_NETWORK_LAN_t lan;
	SYS_NETWORK_LAN_t lan_vlan;
	SYS_NETWORK_PPPOE_t pppoe;
	SYS_NETWORK_DDNS_t ddns;
	SYS_NETWORK_3G_t threeg;
	SYS_NETWORK_ESEE_t esee;
}SYS_NETWORK_t;

// extend
typedef struct SYS_VSIP_PARAM
{
	char buf[2048];
}SYS_VSIP_PARAM_t;


typedef struct SYS_GB28181_PARAM
{
	char buf[2048];
}SYS_GB28181_PARAM_t;


///////////////////////////////////////////////////////////////////////////
// storage about
typedef struct SYS_STORAGE
{
	SYS_BOOL_t over_write;
}SYS_STORAGE_t;

///////////////////////////////////////////////////////////////////////////
//

typedef struct SYS_ISP_IMAGE_ATTR
{
	SYS_RATE16_t hue; // hue / 64
	SYS_RATE8_t contrast; // contrast / 64
	SYS_RATE8_t brightness; // brightness / 64
	SYS_RATE8_t saturation; // saturation / 64
	SYS_RATE8_t sharpen;
	SYS_BOOL_t flip;
	SYS_BOOL_t mirror;
}SYS_ISP_IMAGE_ATTR_t;

typedef struct SYS_ISP_DAY_NIGHT_MODE
{
	SYS_U32_t ircut_control_mode;//0:hardware 1:software
	SYS_U32_t ircut_mode;//0:auto 1:daylight 2:night
}SYS_ISP_DAY_NIGHT_MODE_t;	

typedef struct SYS_ISP_WDR
{
	SYS_BOOL_t enable;
	SYS_RATE8_t strength;//0~255
}SYS_ISP_WDR_t;

typedef struct SYS_ISP_EXPOSURE
{
	SYS_U32_t mode;//0:auto 1:bright 2:dark
	SYS_RATE8_t ae_compensation;//0~255
}SYS_ISP_EXPOSURE_t;

typedef struct SYS_ISP_DENOISE
{
	SYS_BOOL_t denoise_enable;
	SYS_RATE8_t denoise_strength;//0~255
}SYS_ISP_DENOISE_t;

typedef struct SYS_ISP_ADVANCE
{
	SYS_BOOL_t anti_fog_enable;
	SYS_BOOL_t lowlight_enable;
	SYS_U32_t gamma;//0:default 1:normal contrast 2:high contrast
	SYS_BOOL_t defect_pixel_enable;
}SYS_ISP_ADVANCE_t;

typedef struct SYS_ISP
{
	SYS_BOOL_t read_default_val; 
	SYS_ISP_IMAGE_ATTR_t image_attr;
	SYS_U32_t scene_mode;//1:indoor 2:outdoor
	SYS_U32_t white_balance_mode;//0:auto 1:indoor 2:outdoor
	SYS_ISP_DAY_NIGHT_MODE_t day_night_mode;
	SYS_ISP_WDR_t wide_dynamic_range;
	SYS_ISP_EXPOSURE_t exposure;
 	SYS_ISP_DENOISE_t denoise;
	SYS_ISP_ADVANCE_t advance;
}SYS_ISP_t;

typedef struct SYSCONF_HEADER
{
#define SYSCONF_HEADER_MAGIC "JUAN SYSCONF"
	SYS_CHR_t magic[32]; // must be SYSCONF_HEADER_MAGIC
	SYS_U32_t version;
	SYS_U32_t size;
	SYS_U32_t crc32;
}SYSCONF_HEADER_t;

typedef struct SYSCONF_IPCAM
{
	SYS_SPEC_t spec; // device spec
		
	SYS_INFO_t info; // device infomation
	SYS_DATE_TIME_t date_time;
	SYS_GENERIC_t generic;

	SYS_AIN_t ain[8];
	SYS_VIN_t vin[8];
	SYS_PTZ_t ptz[8];

	SYS_NETWORK_t network;
	SYS_STORAGE_t storage;
	SYS_VSIP_PARAM_t vsip_param;
	SYS_GB28181_PARAM_t gb28181_param;

	SYS_ISP_t isp;

}SYSCONF_IPCAM_t;

typedef struct SYSCONF_DVR
{

}SYSCONF_DVR_t;

typedef struct SYSCONF
{
	union
	{
		SYSCONF_IPCAM_t ipcam;
		SYSCONF_DVR_t dvr;
	};
}SYSCONF_t;

typedef struct SYSSTATE_IPCAM
{	
	char operation[32];
#define SYS_OPERATION_REBOOT (0)
#define SYS_OPERATION_DEFAULT_FACTORY (1)
	time_t time;
}SYSSTATE_IPCAM_t;

typedef struct SYSSTATE_DVR
{

}SYSSTATE_DVR_t;	

typedef struct SYSSTATE
{
	union
	{
		SYSSTATE_IPCAM_t ipcam;
		SYSSTATE_DVR_t dvr;
	};
}SYSSTATE_t;

typedef struct CompareInfo
{
	int time;
	int network;
	int video;
	int record;
	int screen;
	int sensor;
	int motiondetect;
	int reboot;
	int default_factory;
	int isp;
}CompareInfo_t;


#pragma pack()


extern SYSCONF_t* SYSCONF_open();
extern void SYSCONF_close(SYSCONF_t* sysenv);

extern int SYSCONF_save(SYSCONF_t* sysenv);

extern void SYSCONF_init(const SYS_CHR_t* soc, const SYS_CHR_t* model, const SYS_CHR_t* storage,
	int ver_maj, int ver_min, int ver_rev, const char* ver_ext);
extern void SYSCONF_destroy();

extern SYSCONF_t* SYSCONF_dup();

extern void SYSCONF_default_factory();

extern int SYSCONF_UNISTRUCT_get(SYSCONF_t* const sysconf, const char* xpath, char* ret_text);
extern int SYSCONF_UNISTRUCT_set(SYSCONF_t* const sysconf, const char* xpath, const char* text);
extern int SYSSTATE_UNISTRUCT_set(SYSSTATE_t* const sysstate, const char* xpath, const char* text);
extern int SYSSTATE_UNISTRUCT_get(SYSSTATE_t* const sysstate, const char* xpath, const char* text);

#ifdef	__cplusplus
};
#endif
#endif //__SYSCONF_H__

