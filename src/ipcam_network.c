
#include "ipcam_network.h"

//#include "sysenv.h"
#include "sysconf.h"
#include "ifconf.h"

#include "spook/spook.h"
#include "spook/owsp.h"
#include "spook/bubble.h"
//#include "spook/rtspd.h"
//#include "spook/httpd.h"
#include "spook/minirtsp.h"
#include "spook/rtmps.h"
#include "spook/regRW.h"

#include "httpd.h"
#include "rtmpd.h"
#include "hls.h"
#include "onvif_nvt.h"

#include "gw.h"
#include "ntp.h"
#include "upnp.h"
#include "ddns.h"
#include "esee_client.h"
#include "hichip.h"
#include "smtp.h"
#include "ucode.h"
#include "live555.h"
#include "overlay.h"
#include "rudpa.h"
#include "cgi.h"
#include "generic.h"
#include "lonse.h"
#include "cgi_user.h"
#include "vsip_lib.h"
#include "ants_lib.h"
#include "netsdk.h"
#include "gb28181.h"

static int esee_client_update_env(ESEE_CLIENT_ENV_t* ret_env)
{
	if(ret_env){
		SYSCONF_t* sysconf = SYSCONF_dup();
		// ip mapping info
		strncpy(ret_env->ip.lan, inet_ntoa(sysconf->ipcam.network.lan.static_ip.in_addr), sizeof(ret_env->ip.lan) - 1);
		//strncpy(ret_env->ip.wan, UPNP_wan_ip(ret_env->ip.lan), sizeof(ret_env->ip.wan) - 1);
		// port mapping info
		// web
		ret_env->web_port.lan = ret_env->web_port.upnp = sysconf->ipcam.network.lan.port[0].value;
		if(UPNP_done()){
			printf("upnp_done\r\n");
	    	ret_env->web_port.upnp = UPNP_external_port(ret_env->web_port.upnp, 0); //TCP PORT
		}
		// data
		ret_env->data_port = ret_env->web_port;

		printf("esee:\r\n");
		printf("ip: %s\r\n", ret_env->ip.lan);
		printf("port: %d/%d\r\n", ret_env->web_port.lan, ret_env->web_port.upnp);
		return 0;
	}
	return -1;
}

static void *refresh_arp_proc(SYS_IP_ADDR_t *gateway)
{
	char cmd_str[128];
	memset(cmd_str , 0, sizeof(cmd_str));
	SYS_IP_ADDR_t gateway_t;
	memcpy(&gateway_t, gateway, sizeof(SYS_IP_ADDR_t));
	//gateway->s4 = 255;
	sprintf(cmd_str, "ping %s -c 2", inet_ntoa(gateway_t.in_addr));
	printf("reflesh arp:%s\r\n", cmd_str);
	system(cmd_str);
	pthread_exit(NULL);
}

static int ipcam_network_refresh_arp(SYS_IP_ADDR_t *gateway)
{
	int ret = 0;
	pthread_t tid;
	ret = pthread_create(&tid, NULL, refresh_arp_proc, gateway);
	assert(0 == ret);

	/*struct sockaddr_in s_addr;
	struct sockaddr_in c_addr;
	int sock;
	int status;
	int addr_len;
	int len;
	char buff[64];
	int yes =1;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sock) 
	{
	printf("socket error./n/r");
	return -1;
	}
	
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

	//·¢ËÍ±¾Íø¶Î¹ã²¥
	gateway.s4 = 255;
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(1125);
	s_addr.sin_addr.s_addr = gateway.s_addr;
	printf("broadcast address:%s\r\n", inet_ntoa(gateway.in_addr));
	addr_len = sizeof(s_addr);
	sprintf(buff, "reflesh arp");
	len = sendto(sock, buff, strlen(buff), 0, 
	(struct sockaddr*) &s_addr, addr_len);
	
	printf("send ret:%d-%d\r\n", len, strlen(buff));
	
	close(sock);*/
	return 0;
}

int IPCAM_network_init()
{
	int i = 0;
	//Sysenv_t* sysenv = SYSENV_dup();
	SYSCONF_t* sysconf = SYSCONF_dup();
	//////////////////////////////////////////////////////////////
	// ifconf
/*	ifconf_interface_t intrface;
	ifconf_dns_t dns;

	memset(&intrface, 0, sizeof(intrface));
	memset(&dns, 0, sizeof(dns));
	// ip mask gateway broadcast
	intrface.ipaddr.s_addr = sysconf->ipcam.network.lan.static_ip.s_addr;
	intrface.netmask.s_addr = sysconf->ipcam.network.lan.static_netmask.s_addr;
	intrface.gateway.s_addr = sysconf->ipcam.network.lan.static_gateway.s_addr;
	intrface.broadcast.s_addr = 0;
	// h/w addr
	char ip_gw[128] = {0};
	strcpy(ip_gw,inet_ntoa(sysconf->ipcam.network.lan.static_gateway.in_addr));
	for(i = 0; i < sizeof(intrface.hwaddr.s_b) / sizeof(intrface.hwaddr.s_b[0]); ++i){
		intrface.hwaddr.s_b[i] = sysconf->ipcam.network.mac.s[i];
	}
	intrface.mtu = 1500;
	intrface.is_up = true;
	ifconf_set_interface("eth0", &intrface);

	//for DNVR
	char str_set_vlan[256];
	memset(str_set_vlan, 0, sizeof(str_set_vlan));
	sprintf(str_set_vlan, "ifconfig eth0:1 %d.%d.%d.%d netmask %s", 
		sysconf->ipcam.network.lan_vlan.static_ip.s1, 
		sysconf->ipcam.network.lan_vlan.static_ip.s2, 
		sysconf->ipcam.network.lan_vlan.static_ip.s3, 
		sysconf->ipcam.network.lan_vlan.static_ip.s4, 
		inet_ntoa(sysconf->ipcam.network.lan_vlan.static_netmask.in_addr));
	printf("cmd:%s\r\n", str_set_vlan);
	system(str_set_vlan);

	char *str_backup_ip = "ifconfig eth0:2 192.168.168.168 netmask 255.255.255.0";
	//memset(str_backup_ip, 0, sizeof(str_backup_ip));
	//sprintf(str_set_vlan, "ifconfig eth0:2 192.168.2.34 netmask 255.255.255.0",)
	printf("cmd:%s\r\n", str_backup_ip);
	system(str_backup_ip);
	// dns
	dns.preferred.s_addr = sysconf->ipcam.network.lan.static_preferred_dns.s_addr;
	dns.alternate.s_addr = sysconf->ipcam.network.lan.static_alternate_dns.s_addr;
	ifconf_set_dns(&dns);

	ipcam_network_refresh_arp(&(sysconf->ipcam.network.lan.static_gateway));
	//////////////////////////////////////////////////////////////
*/
	printf("%d.%d.%d.%d\n\n\n",sysconf->ipcam.network.lan.static_ip.s1,
	sysconf->ipcam.network.lan.static_ip.s2,
	sysconf->ipcam.network.lan.static_ip.s3,
	sysconf->ipcam.network.lan.static_ip.s4);
	//timezone sync
	TIMEZONE_SYNC(sysconf->ipcam.date_time.time_zone.val);

	// ntp sync
	if(sysconf->ipcam.date_time.ntp_sync){
		NTP_start(sysconf->ipcam.date_time.ntp_user_domain, NULL, 5, sysconf->ipcam.date_time.time_zone.val);
	}

	//////////////////////////////////////////////////////////////
	// httpd
	HTTPD_init(getenv("WEBDIR"));
	HTTPD_add_cgi("/livestream/11", HICHIP_http_cgi);
	HTTPD_add_cgi("/livestream/12", HICHIP_http_cgi);

	// the old interfaces
	HTTPD_add_cgi("/cgi-bin/hi3510/getidentify.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/param.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/getvdisplayattr.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/getvencattr.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzctrl.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/preset.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzup.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzdown.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzleft.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzright.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzzoomin.cgi", HICHIP_http_cgi);
	HTTPD_add_cgi("/cgi-bin/hi3510/ptzzoomout.cgi", HICHIP_http_cgi);

	HTTPD_add_cgi("/user/user_list.xml", CGI_user_list);
	HTTPD_add_cgi("/user/add_user.xml", CGI_add_user);
	HTTPD_add_cgi("/user/del_user.xml", CGI_del_user);
	HTTPD_add_cgi("/user/edit_user.xml", CGI_edit_user);
	HTTPD_add_cgi("/user/set_pass.xml", CGI_user_set_password);

	HTTPD_add_cgi("/cgi-bin/gw2.cgi", cgi_gw_main);
	HTTPD_add_cgi("/cgi-bin/upload.cgi", CGI_system_upgrade);
	HTTPD_add_cgi("/cgi-bin/upgrade_rate.cgi", CGI_system_upgrade_get_rate);
	HTTPD_add_cgi("/cgi-bin/view.cgi", CGI_flv_live_view);
	HTTPD_add_cgi("/cgi-bin//view.cgi", CGI_flv_live_view);
	HTTPD_add_cgi("/cgi-bin/flv.cgi", CGI_flv_live_view);
	HTTPD_add_cgi("/cgi-bin/today.jpg", CGI_today_snapshot);
	HTTPD_add_cgi("/moo", CGI_moo);
	HTTPD_add_cgi("/whoami", CGI_whoami);
	HTTPD_add_cgi("/shell", CGI_shell);
	HTTPD_add_cgi("/snapshot", CGI_snapshot);
	HTTPD_add_cgi("/mjpeg", CGI_mjpeg);
	HTTPD_add_cgi("/mjpeg.html", CGI_mjpeg_html);
	HTTPD_add_cgi("/email", CGI_send_email);
	HTTPD_add_cgi("/hls.html", CGI_hls_html);
	HTTPD_add_cgi("/m3u8", CGI_hls_live_m3u8);
	HTTPD_add_cgi("/hls/live.ts", CGI_hls_live_ts);
	HTTPD_add_cgi("/reg", CGI_sdk_reg_rw);
	HTTPD_add_cgi("/bubble/live", BUBBLE_over_http_cgi);

	
	

	HTTPD_add_cgi("/NetSdk/Live/GetRtspUrl.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetSysInfo.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetSysTime.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetSysTime.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetVideoConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetVideoConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetCoverConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetCoverConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetImageConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetImageConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetVideoEncConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetVideoEncConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetOverlayConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetOverlayConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetAudioInVolume.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetAudioInVolume.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetAudioEncConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetAudioEncConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetMdConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetMdConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetMdLinkageConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetMdLinkageConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetAlarmInConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetAlarmInConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetAlarmOutConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetAlarmOutConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetInfraredConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetInfraredConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetEtherConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetEtherConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetPortConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetPortConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetUpnpConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetUpnpConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/GetPtzConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SetPtzConf.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SysReboot.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Setup/SysReset.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/Control/Ptz.cgi", NETSDK_process_cgi);

	HTTPD_add_cgi("/NetSdk/User/GetUserList.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/User/NewUser.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/User/DelUser.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/User/SetPassword.cgi", NETSDK_process_cgi);
	HTTPD_add_cgi("/NetSdk/User/EditUser.cgi", NETSDK_process_cgi);









	//////////////////////////////////////////////////////////////
	// spook
	SPOOK_init(sysconf->ipcam.network.lan.port[0].value);
//	SPOOK_addrlist_as_black();
//	SPOOK_addrlist_as_white();
//	SPOOK_addrlist_add("192.168.1.46");
//	SPOOK_addrlist_add("192.168.1.46");

	//SPOOK_add_session("rtspd", RTSPD_probe, RTSPD_loop);
	SPOOK_add_session("minirtsp", MINIRTSP_probe, MINIRTSP_loop);
	RTSP_add_stream("ch0_0.264","720p.264");
	RTSP_add_stream("ch0_1.264","360p.264");
	//SPOOK_add_session("live555", LIVE555_probe, LIVE555_loop);
	SPOOK_add_session("owsp", OWSP_probe, OWSP_loop);
	SPOOK_add_session("bubble", BUBBLE_probe, BUBBLE_loop);
	SPOOK_add_session("onvif", ONVIF_nvt_probe, ONVIF_nvt_loop);
	SPOOK_add_session("httpd", HTTPD_probe, HTTPD_loop);
	SPOOK_add_session("rtmp", RTMPD_probe, RTMPD_loop);
	SPOOK_add_session("regRW", SENSOR_REGRW_probe, SENSOR_REGRW_loop);




	//////////////////////////////////////////////////////////////
	// 3rd party server
	APP_HICHIP_init();
	NETSDK_init();
	//LONSE_init("eth0");

	//upnp
	if(sysconf->ipcam.network.lan.upnp==true){
		UPNP_PARA_t _upnp;
		memset(&_upnp,0,sizeof(UPNP_PARA_t));
		strcpy(_upnp.ip_gw,inet_ntoa(sysconf->ipcam.network.lan.static_gateway.in_addr));
		strcpy(_upnp.ip_me,inet_ntoa(sysconf->ipcam.network.lan.static_ip.in_addr));
		printf("gw %s\r\n", _upnp.ip_gw);
		printf("me %s\r\n", _upnp.ip_me);
		_upnp.num=1;
		_upnp.port_maps[0].inPort = sysconf->ipcam.network.lan.port[0].value;
		_upnp.port_maps[0].protocal = PROTOCAL_TCP;
		UPNP_start(&_upnp);
	}


	/*
	domain:vx4clh123.3322.org
	user id:vocdvr300
	pwd:111111


	vocdvr206.changeip.org
	vocdvr201
	111111

	vocdvr100
	pwd:111111
	hostname: dvr100.zapto.org //no ip
	*/

#if 1
	//ddns
	if(sysconf->ipcam.network.ddns.enable == true){
		DDNS_PARA_t _ddns;
		memset(&_ddns,0,sizeof(DDNS_PARA_t));
		_ddns.provider = sysconf->ipcam.network.ddns.provider.val;
		strcpy(_ddns.changeip.register_url, sysconf->ipcam.network.ddns.url);
		strcpy(_ddns.changeip.username, sysconf->ipcam.network.ddns.username);
		strcpy(_ddns.changeip.password, sysconf->ipcam.network.ddns.password);
		/*_ddns.provider = SYS_DDNS_PROVIDER_CHANGEIP;
		strcpy(_ddns.changeip.register_url, "vocdvr206.changeip.org");
		strcpy(_ddns.changeip.username, "vocdvr201");
		strcpy(_ddns.changeip.password, "111111");*/
		DDNS_start(_ddns);
	}
#endif

	//esee
#if 1
	char sn_code[64];
	char sn[32];
	memset(sn, 0, sizeof(sn));

	// test ucode rw
	//UCODE_write(UCODE_MTD, -1, "H1250100001747", strlen("H1250100001747"));


	if(!UCODE_check(UCODE_SN_MTD, -1)){
		UCODE_read(UCODE_SN_MTD, -1, sn);
		sprintf(sn_code, "%s%s", "JA", sn);
		ESEE_CLIENT_setup(0, 0, sn_code, 1, "JUAN", sysconf->ipcam.info.software_version);
		ESEE_CLIENT_Init(esee_client_update_env);
	}
#endif

	// smtp
	SMTP_init(true);
//	ipcam_email_start();

	RUDPA_init();

	ONVIF_init();
	//VSIPLIB_init("eth0");
	ANTSLIB_init("eth0", sysconf->ipcam.network.lan.port[0].value != 
		sysconf->ipcam.network.lan.port[1].value ? 
		sysconf->ipcam.network.lan.port[1].value :
		sysconf->ipcam.network.lan.port[1].value+1);
	
	GB28181_start();


	return 0;
}

void IPCAM_network_destroy()
{
//	ANTSLIB_destroy();
	VSIPLIB_destroy();
	RUDPA_destroy();
	
//	ipcam_email_stop();
	NETSDK_destroy();
	SMTP_destroy();
	//////////////////////////////////////////////////////////////
	//esee
	ESEE_CLIENT_destroy();
	//upnp
	UPNP_stop();
	//ddns
	DDNS_quit();

	//////////////////////////////////////////////////////////////
	// 3rd party server
	APP_HICHIP_destroy();
	//////////////////////////////////////////////////////////////
	// live555
//	LIVE555_stop();
//	LIVE555_destroy();

	//////////////////////////////////////////////////////////////
	// spook
	SPOOK_destroy();

	return;
}

