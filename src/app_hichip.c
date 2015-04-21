
#include "app_hichip.h"
#include "hichip.h"
#include "md5sum.h"
#include "ucode.h"
#include "app_debug.h"
#include "gb28181.h"

#define UCODE_SN_SIZE (14)
#define HICHIP_WITH_JUAN_SPEC "IPCAMabcdefghijklmnopqrstuvwxyzj"
#define DEVID_RANDOM_SIZE (7)

static const char *hichip_device_id()
{
	static char device_id[36] = {""};

	if(0 == strlen(device_id)){
		// setup once
		char sn_code[128] = {0};
		char sn[32];
		char *p_md5 = NULL;
		memset(sn, 0, sizeof(sn));

		// test ucode rw
		//int ret = UCODE_write(UCODE_SN_MTD, -1, "H1250100001747", strlen("H1250100001747"));


		if(!UCODE_check(UCODE_SN_MTD, -1)){
			UCODE_read(UCODE_SN_MTD, -1, sn);
			p_md5 = md5sum_buffer(sn, UCODE_SN_SIZE);
			snprintf(sn_code, sizeof(sn_code), "IP_camera%s", p_md5);
			memcpy(device_id, sn_code, 32);
		}else{
			memcpy(device_id, HICHIP_WITH_JUAN_SPEC, strlen(HICHIP_WITH_JUAN_SPEC));
		}
		FILE *fp = fopen("/tmp/devidram", "rb");
		int i, ret = 0;
		char random_str[32] = {0};
		if(fp){
			ret = fread(device_id, 32, 1, fp);	
			fclose(fp);
		}else{
			if(!UCODE_check(UCODE_SN_MTD, -1)){
				UCODE_read(UCODE_SN_MTD, -1, sn);
				p_md5 = md5sum_buffer(sn, UCODE_SN_SIZE);
				snprintf(sn_code, sizeof(sn_code), "IP_camera%s", p_md5);
				memcpy(device_id, sn_code, 32);
			}else{
				memcpy(device_id, HICHIP_WITH_JUAN_SPEC, strlen(HICHIP_WITH_JUAN_SPEC));
			}
			fp = fopen("/tmp/devidram", "wb+");
			struct timespec timetic;
			clock_gettime(CLOCK_MONOTONIC, &timetic);
			srand((unsigned) timetic.tv_nsec);
			for(i = 32 - DEVID_RANDOM_SIZE; i < 32; ++i){
				device_id[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
			}
			fwrite(device_id, 32, 1, fp);
			fclose(fp);
		}
		printf("device_id:%s\r\n", device_id);
#if 0

		int i = 0;
		struct timeval tv;
		// random seeds
		gettimeofday(&tv, NULL);
		srand(tv.tv_sec ^tv.tv_usec);
		memset(device_id, 0, sizeof(device_id));
		for(i = 0; i < 32; ++i){
			device_id[i] = rand() % 26 + ((0 == rand() % 2) ? 'A' : 'a');
		}
#endif
		printf("generate device id: %s\r\n", device_id);
	}

	return device_id;
}

static const char *hichip_device_model()
{
	return "IPCAM";
}

static const char *hichip_device_name()
{
	return "IPCAM";
}

static const char *hichip_ether_lan()
{
	return "eth0";
}

static const char *hichip_ether_vlan()
{
	return "eth0:1";
}

int APP_HICHIP_init()
{
	stHICHIP_CONF_FUNC conf_func;
	memset(&conf_func, 0, sizeof(conf_func));
	// init callback interface
	conf_func.device_id = hichip_device_id;
	conf_func.device_model = hichip_device_model;
	conf_func.device_name = hichip_device_name;
	conf_func.ether_lan = hichip_ether_lan;
	conf_func.ether_vlan = hichip_ether_vlan;
	conf_func.gb28181_conf = GB28181_configure;
	return HICHIP_init(conf_func);
}

void APP_HICHIP_destroy()
{
	HICHIP_destroy();
}

