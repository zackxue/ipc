
#include "version.h"
#include "sysconf.h"

int SYSCONF_HI3516C_INCEPTION_reset(SYSCONF_t* sysconf)
{
	int i = 0;
	SYSCONF_IPCAM_t* const sysconf_hi3516 = &sysconf->ipcam;
	SYS_SPEC_t* const spec = &sysconf_hi3516->spec;
	SYS_INFO_t* const info = &sysconf_hi3516->info;
	SYS_GENERIC_t* const generic = &sysconf_hi3516->generic;
	SYS_AIN_t* const ain = sysconf_hi3516->ain;
	SYS_VIN_t* const vin = sysconf_hi3516->vin;
	SYS_PTZ_t* const ptz = sysconf_hi3516->ptz;
	SYS_NETWORK_LAN_t* const lan = &sysconf_hi3516->network.lan;

//	SYS_NETWORK_t* const network = &sysconf_hi3507->network;
//	SYS_STORAGE_t* const storage = &sysconf_hi3507->storage;

	// spec
	spec->vin = 1;
	spec->ain = 0;
	spec->io_sensor = 1;
	spec->io_alarm = 0;
	spec->hdd = 0;
	spec->sd_card = 0;
	spec->ptz = 0;
/*
//conf.h
#define CONF_PRODUCT_NAME "hi3507 ipcam"
#define CONF_PRODUCT_MODEL "hi3507"
#define CONF_PRODUCT_CLASS "ipcam"
#define CONF_PRODUCT_SOLUTION "hi3507"
#define CONF_PRODUCT_HARDWARE_VERSION "v1.0.0"
*/

	///////////////////////////////////////////////////////////////////
	// info
	strncpy(info->device_name, "ipcam", sizeof(info->device_name));
	strncpy(info->device_model, "IPC", sizeof(info->device_model));
	strncpy(info->device_soc, "N16C", sizeof(info->device_model));
	strncpy(info->device_sn, "ipcam123", sizeof(info->device_sn));
	memset(info->hardware_version, 0, sizeof(info->hardware_version));
	sprintf(info->hardware_version, "%d.%d.%d.%s", HWVER_MAJ, HWVER_MIN, HWVER_REV, HWVER_EXT);

	///////////////////////////////////////////////////////////////////
	// generic
	generic->language.val = SYS_LANGUAGE_ENGLISH;
	generic->language.mask = SYS_LANGUAGE_ENGLISH | SYS_LANGUAGE_CHINESE_MAINLAND;
	
	///////////////////////////////////////////////////////////////////
	// ain
	ain[0].sample_rate.max = SYS_AIN_SAMPLE_RATE_16KBPS;
	ain[0].sample_rate.val = SYS_AIN_SAMPLE_RATE_8KBPS;
	ain[0].sample_width.max = SYS_AIN_SAMPLE_WIDTH_16BITS;
	ain[0].sample_width.val = SYS_AIN_SAMPLE_WIDTH_16BITS;
	
	ain[0].enc_ch = 1;
	strcpy(ain[0].enc[0].engine, "g711a");
	ain[0].enc[0].packet = 320;

	///////////////////////////////////////////////////////////////////
	// vin
	memset(vin[0].channel_name, 0, sizeof(vin[0].channel_name));
	strcpy(vin[0].channel_name, "IPCAM");
	vin[0].digital_shutter.max = SYS_VIN_DIGITAL_SHUTTER_60HZ;
	vin[0].digital_shutter.val = SYS_VIN_DIGITAL_SHUTTER_50HZ;
	vin[0].analog_standard.max = SYS_VIN_ANALOG_STANDARD_INVALID;
	vin[0].analog_standard.val = SYS_VIN_ANALOG_STANDARD_INVALID;

	vin[0].motion_detect.sensitivity.val = SYS_LEVEL_HIGH;
	vin[0].motion_detect.sensitivity.max = SYS_LEVEL_LOWEST;
	vin[0].motion_detect.size.w = 320 / 16;
	vin[0].motion_detect.size.h = 180 / 16;
	memset(&vin[0].motion_detect.mask, 0xff, sizeof(vin[0].motion_detect.mask));
	// color
	vin[0].hue.max = 100;
	vin[0].contrast.max = 100;
	vin[0].brightness.max = 100;
	vin[0].saturation.max = 100;
	vin[0].hue.val = 50; // 0 - 63
	vin[0].contrast.val = 50;
	vin[0].brightness.val = 50;
	vin[0].saturation.val = 50;
	vin[0].flip = 0;
	vin[0].mirror = 0;

	// enc h264
	vin[0].enc_h264_ch = 2; // hi3518c  only 2 encode engine supported 
	vin[0].enc_h264[0].stream_ch = 1; // 720p enc
	vin[0].enc_h264[1].stream_ch = 1; // 360p enc
	vin[0].enc_h264[2].stream_ch = 1; // qvga enc
	do
	{
		SYS_VIN_ENC_H264_STREAM_t* const stream_720p = &vin[0].enc_h264[0].stream[0];
		SYS_VIN_ENC_H264_STREAM_t* const stream_360p = &vin[0].enc_h264[1].stream[0];
		SYS_VIN_ENC_H264_STREAM_t* const stream_qvga = &vin[0].enc_h264[2].stream[0];
		
		// encode 720p stream
		strcpy(stream_720p->name, "720p.264");
		strcpy(stream_720p->profile, "baseline");
		stream_720p->size.val = SYS_VIN_SIZE_1080P;
		stream_720p->size.mask = SYS_VIN_SIZE_1080P;
		stream_720p->mode.val = SYS_VENC_H264_MODE_VBR;
		stream_720p->mode.mask = SYS_VENC_H264_MODE_VBR | SYS_VENC_H264_MODE_CBR;
		stream_720p->on_demand = 5; // 4 users on demand

		/*if(SYS_VIN_DIGITAL_SHUTTER_50HZ == vin[0].digital_shutter.val){
			stream_720p->fps = 25;
		}else if(SYS_VIN_DIGITAL_SHUTTER_60HZ == vin[0].digital_shutter.val){
			stream_720p->fps = 30;
		}*/
		stream_720p->fps = 30;
		stream_720p->gop = 2 * stream_720p->fps; // iframe per 2 seconds intervals
		stream_720p->ain_bind = 0; // bind to aenc 0
		stream_720p->quality.max = SYS_LEVEL_LOWEST;
		stream_720p->quality.val = SYS_LEVEL_HIGHEST;
		stream_720p->bps = 2048; // if zero default bitrate, all follow

		// encode 360p stream
		strcpy(stream_360p->name, "360p.264");
		strcpy(stream_360p->profile, "baseline");
		stream_360p->size.val = SYS_VIN_SIZE_360P;
		stream_360p->size.mask = SYS_VIN_SIZE_360P;
		stream_360p->mode.val = SYS_VENC_H264_MODE_VBR;
		stream_360p->mode.mask = SYS_VENC_H264_MODE_VBR | SYS_VENC_H264_MODE_CBR;
		stream_360p->on_demand = 8; // 5 users on demand
		
		/*if(SYS_VIN_DIGITAL_SHUTTER_50HZ == vin[0].digital_shutter.val){
			stream_360p->fps = 25;
		}else if(SYS_VIN_DIGITAL_SHUTTER_60HZ == vin[0].digital_shutter.val){
			stream_360p->fps = 30;
		}*/
		stream_360p->fps = 25;
		stream_360p->gop = 3 * stream_360p->fps; // iframe per 2 seconds intervals
		stream_360p->ain_bind = 0; // bind to aenc 0
		stream_360p->quality.max = SYS_LEVEL_LOWEST;
		stream_360p->quality.val = SYS_LEVEL_HIGHEST;
		stream_360p->bps = 768; // zero == vbr

		/*
		// encode qvga stream
		strcpy(stream_qvga->name, "qvga.264");
		strcpy(stream_qvga->profile, "baseline");
		stream_qvga->size.val = SYS_VIN_SIZE_QVGA;
		stream_qvga->size.mask = SYS_VIN_SIZE_QVGA;
		stream_qvga->mode.val = SYS_VENC_H264_MODE_VBR;
		stream_qvga->mode.mask = SYS_VENC_H264_MODE_CBR | SYS_VENC_H264_MODE_VBR;
		stream_qvga->on_demand = 16; // 16 users on demand
		stream_qvga->fps = 30;
		stream_qvga->gop = 3 * stream_qvga->fps;
		stream_qvga->ain_bind = 0; // bind to aenc 0
		stream_qvga->quality.max = SYS_LEVEL_LOWEST;
		stream_qvga->quality.val = SYS_LEVEL_HIGHEST;
		stream_qvga->bps = 256; // if zero == vbr
		*/
		
	}while(0);

	///////////////////////////////////////////////////////////////////
	// ptz
	ptz[0].addr = 1;
	ptz[0].protocol.max = SYS_PTZ_PROTOCOL_PELCO_P;
	ptz[0].protocol.val = SYS_PTZ_PROTOCOL_PELCO_D;
	ptz[0].baudrate.max = SYS_PTZ_BAUDRATE_115200;
	ptz[0].baudrate.val = SYS_PTZ_BAUDRATE_4800;
	ptz[0].tour.active.max = sizeof(ptz[0].tour.point) / sizeof(ptz[0].tour.point[0]);
	ptz[0].tour.active.val = ptz[0].tour.active.max;
	for(i = 0; i < ptz[0].tour.active.val; ++i){
		ptz[0].tour.point[i].preset = i;
		ptz[0].tour.point[i].time.hour = 0;
		ptz[0].tour.point[i].time.min = 0;
		ptz[0].tour.point[i].time.sec = 5;
	}

	///////////////////////////////////////////////////////////////////
	// network
	lan->port_active.max = sizeof(lan->port) / sizeof(lan->port[0]);
	lan->port_active.val = 1;
	strcpy(lan->port[0].name, "universal");
	lan->port[0].value = 80;
	strcpy(lan->port[1].name, "ants port");
	lan->port[1].value = 5050;
	return 0;
}



