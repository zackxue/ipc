
#include "sysconf.h"
#include "sysconf_debug.h"
#include "generic.h"
#include "version.h"
//#include "sdk/sdk_isp.h"
#include "sdk_isp.h" // in hi_isp_tmp
#include "ucode.h"

typedef int (*SYSCONF_RESET_FUNC)(SYSCONF_t* sysconf);

static SYSCONF_t* _sysconf_shadow = SYS_NULL;
static SYS_CHR_t* _sysconf_storage = SYS_NULL;
static SYSCONF_RESET_FUNC _sysconf_reset_func = SYS_NULL;

static SYS_U32_t _sysconf_sw_ver_maj = 1;
static SYS_U32_t _sysconf_sw_ver_min = 0;
static SYS_U32_t _sysconf_sw_ver_rev = 0;
static SYS_CHR_t _sysconf_sw_ver_ext[16] = {""};
static SYS_CHR_t _sysconf_serial_code_head[16] = {""};

static SYS_U32_t _sysconf_build_year = 2012;
static SYS_U32_t _sysconf_build_month = 12;
static SYS_U32_t _sysconf_build_mday = 18;
static SYS_U32_t _sysconf_build_hour = 16;
static SYS_U32_t _sysconf_build_min = 22;
static SYS_U32_t _sysconf_build_sec = 30;

static void sysconf_get_build_time()
{
	int i = 0;
	const SYS_CHR_t* month_title[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov","Dec"};
	SYS_CHR_t month_str[16] = {""};
	sscanf(__DATE__, "%s %2d %4d", month_str, &_sysconf_build_mday, &_sysconf_build_year);
	sscanf(__TIME__, "%2d:%2d:%2d", &_sysconf_build_hour, &_sysconf_build_min, &_sysconf_build_sec);
	for (i = 0; i < 12; i++){
		if(0 == strncasecmp(month_str, month_title[i], 3)){
			_sysconf_build_month = i + 1;
			break;
		}
	}
}



static void sysconf_reset(SYSCONF_t* sysconf)
{
	int i = 0;

	if(_sysconf_reset_func){
//		SYS_SPEC_t* const spec = &sysconf->ipcam.spec;
		SYS_INFO_t* const info = &sysconf->ipcam.info;
		SYS_DATE_TIME_t* const date_time = &sysconf->ipcam.date_time;
//		SYS_GENERIC_t* const generic = &sysconf->ipcam.generic;
//		SYS_VIN_t* const vin = sysconf->ipcam.vin;
//		SYS_PTZ_t* const ptz = sysconf->ipcam.ptz;
//		SYS_AENC_t* const aenc = sysconf->ipcam.aenc;
//		SYS_VENC_t* const venc = sysconf->ipcam.venc;
		SYS_NETWORK_t* const network = &sysconf->ipcam.network;
		SYS_STORAGE_t* const storage = &sysconf->ipcam.storage;
		SYS_ISP_t* const isp = &sysconf->ipcam.isp;

		memset(sysconf, 0, sizeof(SYSCONF_t));

		// info
		memset(info->software_version, 0, sizeof(info->software_version));
		sprintf(info->software_version, "%d.%d.%d.%s",
			_sysconf_sw_ver_maj, _sysconf_sw_ver_min, _sysconf_sw_ver_rev, _sysconf_sw_ver_ext);
		info->build_date.year = (SYS_U16_t)_sysconf_build_year;
		info->build_date.month = (SYS_U8_t)_sysconf_build_month;
		info->build_date.day = (SYS_U8_t)_sysconf_build_mday;
		info->build_time.hour = (SYS_U8_t)_sysconf_build_hour;
		info->build_time.min = (SYS_U8_t)_sysconf_build_min;
		info->build_time.sec = (SYS_U8_t)_sysconf_build_sec;

		// date time
		date_time->date_format.val = SYS_DATE_FORMAT_YYYYMMDD;
		date_time->date_separator.val = SYS_DATE_SPEC_SLASH;
		date_time->time_format.val = SYS_TIME_FORMATE_24HOUR;
		date_time->time_zone.min = -12;
		date_time->time_zone.max = 12;
		date_time->time_zone.val = 8;
		date_time->day_saving_time.min = -3;
		date_time->day_saving_time.max= 3;
		date_time->day_saving_time.val = 0;

		// default ntp server
		date_time->ntp_sync = SYS_FALSE;
		date_time->ntp_sys_ip[0].s1 = 64; // nist1-ny.ustiming.org  New York City, NY  All services available
		date_time->ntp_sys_ip[0].s2 = 90;
		date_time->ntp_sys_ip[0].s3 = 182;
		date_time->ntp_sys_ip[0].s4 = 55;
		date_time->ntp_sys_ip[1].s1 = 96; // nist1-nj.ustiming.org  Bridgewater, NJ  All services available
		date_time->ntp_sys_ip[1].s2 = 47;
		date_time->ntp_sys_ip[1].s3 = 67;
		date_time->ntp_sys_ip[1].s4 = 105;
		date_time->ntp_sys_ip[2].s1 = 206; // nist1-pa.ustiming.org  Hatfield, PA  All services available
		date_time->ntp_sys_ip[2].s2 = 246;
		date_time->ntp_sys_ip[2].s3 = 122;
		date_time->ntp_sys_ip[2].s4 = 250;
		date_time->ntp_sys_ip[3].s1 = 129; // time-a.nist.gov  NIST, Gaithersburg, Maryland  ntp ok, time,daytime busy, not recommended
		date_time->ntp_sys_ip[3].s2 = 6;
		date_time->ntp_sys_ip[3].s3 = 15;
		date_time->ntp_sys_ip[3].s4 = 28;
		memset(date_time->ntp_user_domain, 0, sizeof(date_time->ntp_user_domain));


		///////////////////////////////////////////////////////////////////
		// network
		// ether
		struct timespec timetic;
		clock_gettime(CLOCK_MONOTONIC, &timetic);
		srand((unsigned) timetic.tv_nsec); 
		for(i = 0; i < sizeof(network->mac.s) / sizeof(network->mac.s[0]); ++i){
			network->mac.s[i] = rand();
		}
		network->mac.s[0] = 0x00;
		network->mac.s[1] = 0x9a; // from JUAN
		
		network->lan.dhcp = SYS_FALSE;
		network->lan.upnp = SYS_TRUE;
		network->lan_vlan.dhcp = SYS_FALSE;
		network->lan_vlan.upnp = SYS_FALSE;
		SYS_INET_ATON("192.168.3.33", &network->lan_vlan.static_ip);
		SYS_INET_ATON("192.168.3.1", &network->lan_vlan.static_gateway);
		SYS_INET_ATON("255.255.255.0", &network->lan_vlan.static_netmask);
#ifndef MAKE_IMAGE
		SYS_INET_ATON("192.168.2.32", &network->lan.static_ip);
		SYS_INET_ATON("192.168.2.1", &network->lan.static_gateway);
		SYS_INET_ATON("192.168.2.1", &network->lan.static_preferred_dns);
		SYS_INET_ATON("192.168.2.1", &network->lan.static_alternate_dns);
#else
		SYS_INET_ATON("192.168.1.168", &network->lan.static_ip);
		SYS_INET_ATON("192.168.1.1", &network->lan.static_gateway);
		SYS_INET_ATON("192.168.1.1", &network->lan.static_preferred_dns);
		SYS_INET_ATON("192.168.1.1", &network->lan.static_alternate_dns);
#endif	
		SYS_INET_ATON("255.255.255.0", &network->lan.static_netmask);

		
		// pppoe
		network->pppoe.enable = SYS_FALSE;
		strcpy(network->pppoe.username, "pppoe0123456789");
		strcpy(network->pppoe.password, "pppoe0123456789");
		// ddns
		network->ddns.enable = SYS_FALSE;
		//strcpy(network->ddns.provider, "changip.com");
		network->ddns.provider.val = SYS_DDNS_PROVIDER_CHANGEIP;
		network->ddns.provider.mask = \
			SYS_DDNS_PROVIDER_DYNDNS |\
			SYS_DDNS_PROVIDER_NOIP |\
			SYS_DDNS_PROVIDER_3322 |\
			SYS_DDNS_PROVIDER_CHANGEIP |\
			SYS_DDNS_PROVIDER_POPDVR |\
			SYS_DDNS_PROVIDER_SKYBEST |\
			SYS_DDNS_PROVIDER_DVRTOP;
		strcpy(network->ddns.url, "jatestddns.changeip.org");
		strcpy(network->ddns.username, "vocdvr201");
		strcpy(network->ddns.password, "111111");
		// 3g
		network->threeg.enable = SYS_FALSE;
		// esee
		network->esee.enable = SYS_TRUE;
		network->esee.enable_display= SYS_TRUE;
		///////////////////////////////////////////////////////////////////
		//isp
		isp->read_default_val = SYS_TRUE;
		isp->image_attr.hue.val = 50;
		isp->image_attr.contrast.val = 50;
		isp->image_attr.saturation.val = 50;
		isp->image_attr.brightness.val = 50;
		isp->image_attr.sharpen.val = 128;
		isp->image_attr.sharpen.max = 255;
		isp->image_attr.flip = SYS_FALSE;
		isp->image_attr.mirror = SYS_FALSE;
		isp->scene_mode = ISP_SCENE_MODE_AUTO;
		isp->white_balance_mode = ISP_SCENE_MODE_AUTO;
		isp->day_night_mode.ircut_control_mode = ISP_IRCUT_CONTROL_MODE_HARDWARE;
		isp->day_night_mode.ircut_mode = ISP_IRCUT_MODE_AUTO;
		isp->wide_dynamic_range.enable = SYS_TRUE;
		isp->wide_dynamic_range.strength.val = 0;
		isp->wide_dynamic_range.strength.max = 255;
		isp->exposure.mode = ISP_EXPOSURE_MODE_AUTO;
		isp->exposure.ae_compensation.val = 128;
		isp->exposure.ae_compensation.max = 255;
		isp->denoise.denoise_enable = SYS_TRUE;
		isp->denoise.denoise_strength.val = 0x10;
		isp->denoise.denoise_strength.max = 255;
		isp->advance.anti_fog_enable = SYS_FALSE;
		isp->advance.lowlight_enable = SYS_FALSE;
		isp->advance.gamma = ISP_ADVANCE_GAMMA_DEFAULT;
		isp->advance.defect_pixel_enable = SYS_FALSE;
		///////////////////////////////////////////////////////////////////
		// storage
		storage->over_write = SYS_FALSE;
		
		_sysconf_reset_func(sysconf);
	}
	
}

static SYS_U32_t sysconf_do_crc32(void* data, ssize_t data_sz)
{
	int i = 0;
	SYS_U32_t* data_32 = data;
	SYS_U32_t data_sz_32 = data_sz / sizeof(SYS_U32_t);
	SYS_U32_t const crc32_origin = 0xfefeef11;
	SYS_U32_t crc32_result = crc32_origin;
	for(i = 0; i < data_sz_32; ++i){
		crc32_result ^= data_32[i];
	}
	return crc32_result;
}

static void sysconf_set_info(SYSCONF_t* sysconf)
{
	SYS_INFO_t* const info = &sysconf->ipcam.info;
	memset(info->device_sn_head, 0, sizeof(info->device_sn_head));
	strncpy(info->device_sn_head, _sysconf_serial_code_head, sizeof(info->device_sn_head));
	memset(info->hardware_version, 0, sizeof(info->hardware_version));
	sprintf(info->hardware_version, "%d.%d.%d.%s", HWVER_MAJ, HWVER_MIN, HWVER_REV, HWVER_EXT);
	memset(info->software_version, 0, sizeof(info->software_version));
	sprintf(info->software_version, "%d.%d.%d.%s",
		_sysconf_sw_ver_maj, _sysconf_sw_ver_min, _sysconf_sw_ver_rev, _sysconf_sw_ver_ext);
	info->build_date.year = (SYS_U16_t)_sysconf_build_year;
	info->build_date.month = (SYS_U8_t)_sysconf_build_month;
	info->build_date.day = (SYS_U8_t)_sysconf_build_mday;
	info->build_time.hour = (SYS_U8_t)_sysconf_build_hour;
	info->build_time.min = (SYS_U8_t)_sysconf_build_min;
	info->build_time.sec = (SYS_U8_t)_sysconf_build_sec;
}

static void sysconf_get_dev_sn(SYSCONF_t* sysconf)
{
	SYS_INFO_t* const info = &sysconf->ipcam.info;
	char sn[16] = {0};
	memset(info->device_sn, 0, sizeof(info->device_sn));
	if(!UCODE_check(UCODE_SN_MTD, -1)){
		UCODE_read(UCODE_SN_MTD, -1, info->device_sn);
		printf("device sn:%s\r\n", info->device_sn);
	}
}

static void sysconf_load(SYSCONF_t* sysconf)
{
	bool success = false;
	int ret = 0;
	if(NULL != _sysconf_storage){
		FILE* fid = SYS_NULL;
		fid = fopen(_sysconf_storage, "wb+");
		if(NULL != fid){
			// load sysconf header
			// load header
			SYSCONF_HEADER_t header;
			ret = fread(&header, 1, sizeof(header), fid);
			if(sizeof(header) == ret){
				// check sysconf magic
				if(0 == memcmp(header.magic, SYSCONF_HEADER_MAGIC, strlen(SYSCONF_HEADER_MAGIC))){
					// check sysconf body size
					if(sizeof(SYSCONF_t) == header.size){
						// load sysconf body
						ret = fread(sysconf, 1, sizeof(SYSCONF_t), fid);
						if(sizeof(SYSCONF_t) == ret){
							// check sysconf crc32
							SYS_U32_t const crc32 = sysconf_do_crc32(sysconf, sizeof(SYSCONF_t));
							if(crc32 == header.crc32){
								SYSCONF_TRACE("Load sysconf success! verion: %d.%d.%d.%s, build:%d-%d-%d %d:%d:%d",
									_sysconf_sw_ver_maj, _sysconf_sw_ver_min, _sysconf_sw_ver_rev, _sysconf_sw_ver_ext,
									_sysconf_build_year, _sysconf_build_month, _sysconf_build_mday,
									_sysconf_build_hour, _sysconf_build_min, _sysconf_build_sec);
								success = true;
							}else{
								SYSCONF_TRACE("CRC32 error %d/%d!", header.crc32, crc32);
							}
						}else{
							SYSCONF_TRACE("Load error %d/%d!", header.size, sizeof(SYSCONF_t));
						}
					}else{
						SYSCONF_TRACE("Sysconf size error %d/%d!", header.size, sizeof(SYSCONF_t));
					}
				}else{
					SYSCONF_TRACE("Magic error %s/%s", header.magic, SYSCONF_HEADER_MAGIC);
				}
			}else{
				SYSCONF_TRACE("Load error!");
			}
			fclose(fid);
			fid = NULL;
		}else{
			SYSCONF_TRACE("Open \"%s\" error!", _sysconf_storage);
		}
	}else{
		SYSCONF_TRACE("Storage error!");
	}
	if(true != success){
		sysconf_reset(sysconf);	
	}else{
		sysconf_set_info(sysconf);
	}
	sysconf_get_dev_sn(sysconf);
	//sysconf_reset(sysconf);	
	SYSCONF_save(sysconf);
}

SYSCONF_t* SYSCONF_open()
{
	SYSCONF_t* const sysconf = calloc(sizeof(SYSCONF_t), 1);
	sysconf_load(sysconf);
	return sysconf;
}

void SYSCONF_close(SYSCONF_t* sysconf)
{
	if(sysconf){
		free(sysconf);
	}
}

int SYSCONF_save(SYSCONF_t* sysconf)
{
	bool success = false;
	int ret = 0;
	if(NULL != _sysconf_storage){
		FILE* fid = SYS_NULL;
		fid = fopen(_sysconf_storage, "wb");
		if(NULL != fid){
			ssize_t write_size = sizeof(SYSCONF_HEADER_t) + sizeof(SYSCONF_t);
			SYSCONF_HEADER_t* const header = alloca(write_size);
			SYSCONF_t* const body = (SYSCONF_t*)(header + 1);

			memset(header, 0, write_size);
			// make header
			strcpy(header->magic, SYSCONF_HEADER_MAGIC);
			header->version = 0x100;
			header->size = sizeof(SYSCONF_t);
			memcpy(body, sysconf, sizeof(SYSCONF_t));
			header->crc32 = sysconf_do_crc32(body, header->size);
			SYSCONF_TRACE("Build CRC32 %08x", header->crc32);
			
			// write header
			ret = fwrite(header, 1, write_size, fid);
			if(ret == write_size){
				success = true;
			}else{
				SYSCONF_TRACE("Sysconf write error %d/%d!", ret, write_size);
			}
			fclose(fid);
		}else{
			SYSCONF_TRACE("Open \"%s\" error!", _sysconf_storage);
		}
	}else{
		SYSCONF_TRACE("Storage error!");
	}
	return success ? 0 : -1;
}

static SYS_BOOL_t _sysconf_match(const SYS_CHR_t* str1,const SYS_CHR_t* str2)
{
	if(0 == strcasecmp(str1, str2) && strlen(str1) == strlen(str2)){
		return SYS_TRUE;
	}
	return SYS_FALSE;
}

static SYS_BOOL_t sysconf_match(const SYS_CHR_t* soc, const SYS_CHR_t* model, const SYS_CHR_t* soc2, const SYS_CHR_t* model2)
{
	return _sysconf_match(soc, soc2) && _sysconf_match(model, model2);
}

extern int SYSCONF_HI3507_INCEPTION_reset(SYSCONF_t* sysconf);
extern int SYSCONF_HI3518A_INCEPTION_reset(SYSCONF_t* sysconf);
extern int SYSCONF_HI3518C_INCEPTION_reset(SYSCONF_t* sysconf);
extern int SYSCONF_HI3516C_INCEPTION_reset(SYSCONF_t* sysconf);



int sysconf_init(const SYS_CHR_t* soc, const SYS_CHR_t* model)
{
	/*
	 *_sysconf_reset_func = SYSCONF_HI3518A_INCEPTION_reset;
	 *        return 0;
	 */


	SYSCONF_TRACE("SOC = %s model = %s", soc, model);
	if(sysconf_match("HI3507", "hi3507-inception", soc, model)){
		_sysconf_reset_func = SYSCONF_HI3507_INCEPTION_reset;
		sprintf(_sysconf_serial_code_head, "C1");
		return 0;
	}else if(sysconf_match("HI3518A", "hi3518a-inception", soc, model)){
		_sysconf_reset_func = SYSCONF_HI3518A_INCEPTION_reset;
		sprintf(_sysconf_serial_code_head, "C1");
		return 0;
	}else if(sysconf_match("HI3518C", "hi3518c-inception", soc, model)){
		_sysconf_reset_func = SYSCONF_HI3518C_INCEPTION_reset;
		sprintf(_sysconf_serial_code_head, "C2");
		return 0;
	}
	else if(sysconf_match("HI3516C", "hi3516c-inception", soc, model)){
		_sysconf_reset_func = SYSCONF_HI3516C_INCEPTION_reset;
		sprintf(_sysconf_serial_code_head, "C3");
		return 0;
	}
	SYSCONF_ASSERT(0, "SoC(%s) or model(%s) not support", soc, model);
	return -1;
}

void SYSCONF_init(const SYS_CHR_t* soc, const SYS_CHR_t* model, const SYS_CHR_t* storage, int ver_maj, int ver_min, int ver_rev, const char* ver_ext)
{
	_sysconf_sw_ver_maj = ver_maj;
	_sysconf_sw_ver_min = ver_min;
	_sysconf_sw_ver_rev = ver_rev;
	strncpy(_sysconf_sw_ver_ext, ver_ext, sizeof(_sysconf_sw_ver_ext));
	
	if(!_sysconf_shadow){
		sysconf_get_build_time();
		if(0 == sysconf_init(soc, model)){
			_sysconf_storage = strdup(storage);
			_sysconf_shadow = SYSCONF_open();
		}
	}
}

void SYSCONF_destroy()
{
	if(_sysconf_shadow){
		SYSCONF_close(_sysconf_shadow);
		_sysconf_shadow = SYS_NULL;
		free(_sysconf_storage);
		_sysconf_storage = SYS_NULL;
	}
}

SYSCONF_t* SYSCONF_dup()
{
	return _sysconf_shadow;
}

void SYSCONF_default_factory()
{
		SYSCONF_t* const sysconf = calloc(sizeof(SYSCONF_t), 1);
		sysconf_reset(sysconf);
		SYSCONF_save(sysconf);
		if(sysconf){
			free(sysconf);
		}
}

