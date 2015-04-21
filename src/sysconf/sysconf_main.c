
#include "sysconf.h"
#include "sysconf_debug.h"

int main(int argc, char** argv)
{
	SYSCONF_init("hi3507", "ipcam", "mtdblock_sysconf.bin");

	SYSCONF_t* sysconf = SYSCONF_dup();
	const char* xpaths[] =
	{
		"spec@vin","spec@ain","spec@io_sensor","spec@io_alarm","spec@hdd","spec@sd_card",
		/* info */
		"info@device_name","info@device_model","info@device_sn","info@hardware_version","info@software_version","info@build_date","info@build_time",
		/* date time */
		"datetime@date_format","datetime@date_separator","datetime@time_format","datetime@time_zone","datetime@day_saving_time",
		"datetime@ntp_sync","datetime@ntp_user_domain",
		/* generic */
		"generic@key_buzzer","generic@idle_timeout","generic@irda_id","generic@language",
		/* ain */
		"ain0@sample_rate", "ain0@sample_width",
		"ain0/encode0@engine","ain0/encode0@packet",
		/* vin */
		"vin0@shutter","vin0@standard","vin0@hue","vin0@contrast","vin0@brightness","vin0@saturation",
		"vin0/encode_h2640/stream0@name","vin0/encode_h2640/stream0@profile",
		"vin0/encode_h2640/stream0@size","vin0/encode_h2640/stream0@mode",
		"vin0/encode_h2640/stream0@on_demand","vin0/encode_h2640/stream0@fps",
		"vin0/encode_h2640/stream0@gop","vin0/encode_h2640/stream0@ain_bind",
		"vin0/encode_h2640/stream0@quality","vin0/encode_h2640/stream0@bps",
		"vin0/encode_h2641/stream0@name","vin0/encode_h2641/stream0@profile",
		"vin0/encode_h2641/stream0@size","vin0/encode_h2641/stream0@mode",
		"vin0/encode_h2641/stream0@on_demand","vin0/encode_h2641/stream0@fps",
		"vin0/encode_h2641/stream0@gop","vin0/encode_h2641/stream0@ain_bind",
		"vin0/encode_h2641/stream0@quality","vin0/encode_h2641/stream0@bps",
		"vin0/encode_jpeg0@name","vin0/encode_jpeg0@quality","vin0/encode_jpeg0@size",
		/* ptz */
		"ptz0@baudrate", "ptz0@addr", "ptz0@protocol",
		"ptz0/tour@actived",
		"ptz0/tour/point0@preset", "ptz0/tour/point0@time",
		/* network */
		"network@mac",
		/* lan */
		"network/lan@dhcp","network/lan@static_ip","network/lan@static_netmask","network/lan@static_gateway",
		"network/lan@static_preferred_dns","network/lan@static_alternate_dns",
		/* pppoe */
		"network/pppoe@enable","network/pppoe@username","network/pppoe@password",
		/* ddns */
		"network/ddns@enable","network/ddns@provider","network/ddns@url","network/ddns@username","network/ddns@password",
		/* 3g */
		"network/threeg@enable","network/threeg@apn","network/threeg@pin","network/threeg@username","network/threeg@password",
		/* esee */
		"network/esee@enable",
		"network/lan/port0@name",
		"network/lan/port0@value",
	};

	int i = 0;
	for(i = 0; i < sizeof(xpaths) / sizeof(xpaths[0]); ++i){
		char val[128];
		SYSCONF_UNISTRUCT_get(sysconf, xpaths[i], val);
		SYSCONF_TRACE("%s : %s", xpaths[i], val);
	}

	SYSCONF_TRACE("---");
	SYSCONF_UNISTRUCT_set(sysconf, "datetime@time_zone", "-7");
	SYSCONF_UNISTRUCT_set(sysconf, "network/pppoe@enable", "no");

	SYSCONF_destroy();
	return 0;
}


