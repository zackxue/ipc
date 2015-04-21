

#include "hichip_discover.h"
#include "http_util.h"
#include "sysconf.h"
#include "ifconf.h"
#include "esee_client.h"
#include "app_debug.h"

struct sockaddr_in HICHIP_DISCOVER_multicast_addr()
{
	int ret = 0;
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(HICHIP_MULTICAST_PORT);
	ret = inet_aton(HICHIP_MULTICAST_IPADDR, &sock_addr.sin_addr);
	APP_ASSERT(ret > 0, "HICHIP get multicast address error!");
	return sock_addr;
}

int HICHIP_DISCOVER_sock_create(const char *local_ip)
{
	int sock = 0;
	int ret = 0, flag = 0;
	
	struct sockaddr_in my_addr, to_addr;
	struct ip_mreq mreq;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	memset(&to_addr, 0, sizeof(struct sockaddr_in));
	memset(&mreq, 0, sizeof(struct ip_mreq));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(HICHIP_MULTICAST_PORT);
	inet_aton(HICHIP_MULTICAST_IPADDR, &my_addr.sin_addr);

	// create udp socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	APP_ASSERT(sock > 0, "HICHIP create udp socket failed!");

	ret = bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
	APP_ASSERT(0 == ret, "HICHIP udp socket bind failed!");

	inet_aton(HICHIP_MULTICAST_IPADDR, &mreq.imr_multiaddr);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(sock, SOL_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	APP_ASSERT(0 == ret, "HICHP add member failed!");

	flag = 0; // not backrush 
	ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));
	APP_ASSERT(0 == ret, "HICHIP clear multicast loop failed!");

	ret = fcntl(sock, F_SETFL,O_NONBLOCK);
	assert(0 == ret);

	return sock;
}

void HICHIP_DISCOVER_sock_release(int sock)
{
	close(sock);
}

int HICHIP_DISCOVER_sock_send(int sock, const void *buf, size_t buf_size)
{
	struct sockaddr_in peer_addr = HICHIP_DISCOVER_multicast_addr();
	return sendto(sock, buf, buf_size, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)); 
}

int HICHIP_DISCOVER_process_cmd(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content)
{
	int ret = 0;

	const char *cseq = NULL;
	const char *client_id = NULL, *device_id = NULL;
	const char *authorization = NULL, *modify_type = NULL;
	char network_para[1024] = {""};

	bool ifc_flag = false;
	bool dhcp_flag = true;
	bool dns_flag = false;
	bool dns_stat_flag = false;

	SYSCONF_t* sysconf = SYSCONF_dup();

	const char *set_item = NULL;
	bool const modify_soft = (NULL != modify_type && 0 == strcasecmp("soft", modify_type)) ? true : false;

	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");
	device_id = request_header->read_tag(request_header, "Device-ID");
	authorization = request_header->read_tag(request_header, "Authorization");
	modify_type = request_header->read_tag(request_header, "X-Modify-Type");

	// scanf the first network parameter
	sscanf(request_content, "%[^" kCRLF "]", network_para);
	
	if(strlen(network_para) > 0){
		// part 1.
		//netconf set -ipaddr 192.168.1.7 -netmask 255.255.255.0 -gateway 192.168.1.1 -dhcp off -fdnsip 192.168.1.2 -sdnsip 211.23.12.13 -dnsstat 0 -hwaddr 00:01:89:11:11:07.
		// part 2.
		//httpport set -httpport 8000.
		if(0 == strncasecmp(network_para, "netconf set", strlen("netconf set"))){
			
			ifconf_interface_t ifc;
			ifconf_dns_t dns;

			memset(&ifc, 0, sizeof(ifc));
			memset(&dns, 0, sizeof(dns));

			// check ip address
			set_item = strstr(network_para, "-ipaddr");
			if(NULL != set_item){
				set_item += strlen("-ipaddr") + 1; // offset to value
				if(modify_soft){//soft for dnvr
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan_vlan.static_ip);
				}else{//soft for dnvr
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan.static_ip);
				}
				APP_TRACE("setup ipaddr");
				ifc_flag = true;
			}
			// check net mask address
			set_item = strstr(network_para, "-netmask");
			if(NULL != set_item){
				set_item += strlen("-netmask") + 1; // offset to value
				if(modify_soft){//soft for dnvr
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan_vlan.static_netmask);
				}else{
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan.static_netmask);
				}
				APP_TRACE("setup netmask");
				ifc_flag = true;
			}
			// check gateway
			set_item = strstr(network_para, "-gateway");
			if(NULL != set_item){
				set_item += strlen("-gateway") + 1;
				if(modify_soft){//soft for dnvr
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan_vlan.static_gateway);
				}else{
					SYS_INET_ATON(set_item, &sysconf->ipcam.network.lan.static_gateway);
				}
				APP_TRACE("setup gateway");
				ifc_flag = true;
			}
			// check dhcp
			set_item = strstr(network_para, "-dhcp");
			if(NULL != set_item){
				set_item += strlen("-dhcp") + 1;
				if(0 == strcasecmp("off", set_item)){
					dhcp_flag = false;
				}else{
					dhcp_flag = true;
				}
				// FIXME: ignore this dhchp temprory
			}
			// check preferred dns address
			set_item = strstr(network_para, "-fdnsip");
			if(NULL != set_item){
				set_item += strlen("-fdnsip") + 1;
				dns.preferred = ifconf_ipv4_aton(set_item);
				APP_TRACE("setup fdnsip %s", ifconf_ipv4_ntoa(dns.preferred));
				dns_flag = true;
			}
			// check dns
			set_item = strstr(network_para, "-sdnsip");
			if(NULL != set_item){
				set_item += strlen("-sdnsip") + 1;
				dns.alternate = ifconf_ipv4_aton(set_item);
				APP_TRACE("setup sdnsip %s", ifconf_ipv4_ntoa(dns.alternate));
				dns_flag = true;
			}
			set_item = strstr(network_para, "-dnsstat");
			if(NULL != set_item){
				set_item += strlen("-dnsstat") + 1;
				if(0 == strcasecmp("off", set_item)){
					dns_stat_flag = false;
				}else{
					dns_stat_flag = true;
				}
			}
			// check MAC address
			set_item = strstr(network_para, "-hwaddr");
			if(NULL != set_item){
				set_item += strlen("-hwaddr") + 1;
				if(modify_soft){
					// ignore the MAC configuration when DNVR connected
					
				}else{
					ifc.hwaddr = ifconf_hw_aton(set_item);
					memcpy(sysconf->ipcam.network.mac.s, ifc.hwaddr.s_b, sizeof(SYS_MAC_ADDR_t));
					APP_TRACE("setup hwaddr %02x:%02x:%02x:%02x:%02x:%02x:",
						sysconf->ipcam.network.mac.s1,
						sysconf->ipcam.network.mac.s2,
						sysconf->ipcam.network.mac.s3,
						sysconf->ipcam.network.mac.s4,
						sysconf->ipcam.network.mac.s5,
						sysconf->ipcam.network.mac.s6);
					ifc_flag = true;
				}
			}
			// active
			if(dhcp_flag){
				// FIXME:
				ifc_flag = true;
			}
			if(dns_flag){
				// FIXME:
				//ifconf_set_dns(&dns);
				ifc_flag = true;
			}
			if(dns_stat_flag){
				// FIXME:
				ifc_flag = true;
			}
		}
		
		if(0 == strncasecmp(network_para, "httpport set", strlen("httpport set"))){
			set_item = strstr(network_para, "-httpport");
			if(NULL != set_item){
				set_item += strlen("-httpport") + 1;
				sysconf->ipcam.network.lan.port[0].value = atoi(set_item);
				APP_TRACE("setup httpport = %d", sysconf->ipcam.network.lan.port[0].value);
				ifc_flag = true;
			}
		}
	}

	if(ifc_flag){
		lpHTTP_HEADER response_header = HTTP_UTIL_new_response_header("MCTP", "1.0", 200, NULL);
		if(NULL != response_header){
			char response_buf[1024] = {""};
			const char *response_content = "Segment-Num:1" kCRLF
				"Segment-Seq:1" kCRLF
				"Data-Length:35" kCRLF
				kCRLF
				"[Success] set net information OK!" kCRLF;
			
			response_header->add_tag_int(response_header, "CSeq", atoi(request_header->read_tag(request_header, "CSeq")), true);
			response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
			response_header->add_tag_text(response_header, "Device-ID", (char*)device_id, true);
			response_header->add_tag_text(response_header, "Content-Type", "text/HDP", true);
			response_header->add_tag_int(response_header, "Content-Length", strlen(response_content), true);
			response_header->to_text(response_header, response_buf, sizeof(response_buf));
			//response_header->dump(response_header);
			response_header->free(response_header);
			response_header = NULL;
			// add content
			strcat(response_buf, response_content);
			//APP_TRACE("%s", response_buf);

			// response
			ret = HICHIP_DISCOVER_sock_send(sock, response_buf, strlen(response_buf));
			if(ret < 0){
				return -1;
			}

			// save this parameter
			SYSCONF_save(sysconf);
			if(modify_soft){
				char str_set_vlan[256];
				memset(str_set_vlan, 0, sizeof(str_set_vlan));
				
				sprintf(str_set_vlan, "ifconfig eth0:1 %d.%d.%d.%d netmask %s", 
					sysconf->ipcam.network.lan_vlan.static_ip.s1, 
					sysconf->ipcam.network.lan_vlan.static_ip.s2, 
					sysconf->ipcam.network.lan_vlan.static_ip.s3, 
					sysconf->ipcam.network.lan_vlan.static_ip.s4, 
					inet_ntoa(sysconf->ipcam.network.lan_vlan.static_netmask.in_addr));
				APP_TRACE("hichipcmd:%s", str_set_vlan);
				system(str_set_vlan);				
			}else{
				exit(0);
			}
		}
	}

	return 0;
}

int HICHIP_DISCOVER_process_search(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content)
{
	int ret = 0;
	char str_seg[2048] = {0};
	char str_seg1_data[512] = {0};
	char str_seg1[1024] = {0};
	char str_seg2_data[512] = {0};
	char str_seg2[1024] = {0};

	// some parameters
	ESEE_CLIENT_INFO_t ret_esee_info;

	// network parameter
	//Sysenv_t* const sysenv = SYSENV_dup();
	SYSCONF_t* const sysconf = SYSCONF_dup();
	ifconf_ipv4_addr_t preferred_dns;
	ifconf_ipv4_addr_t alternate_dns;

	ifconf_interface_t ifconf_irf;
	const char *eth_name = NULL;

	const char *cseq = NULL, *client_id = NULL, *search_type = NULL;
	bool search_from_dnvr = false;
	char response_buf[2048] = {""};
	lpHTTP_HEADER response_header = NULL;

	const char *device_id = "", *device_model = "", *device_name = "";
	
	if(conf_func->device_id){
		device_id = conf_func->device_id();
	}
	if(conf_func->device_model){
		device_model = conf_func->device_model();
	}
	if(conf_func->device_name){
		device_name = conf_func->device_name();
	}

	//preferred_dns.s_addr = sysenv->network.preferred_dns;
	//alternate_dns.s_addr = sysenv->network.alternate_dns;
	preferred_dns.s_addr = 0;
	alternate_dns.s_addr = 0;
	
	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");
	search_type = request_header->read_tag(request_header, "X-Search-Type");
	//APP_TRACE("X-Search-Type: %s", search_type);

	search_from_dnvr = (NULL != search_type && 0 == strcmp(search_type, "DNVR")) ? true : false;

	//PARTY3RD_TRACE("cseq:%s client:%s device:%s", str_cseq, str_client_id, s_hichipd_para->device_id);
	ESEE_CLIENT_get_info(&ret_esee_info);
	
	if(search_from_dnvr){
		// dnvr
		eth_name = conf_func->ether_vlan();
	}else{
		// nvr
		eth_name = conf_func->ether_lan();
	}
	APP_ASSERT(NULL != eth_name, "HICHIP eth failed!");
	ret = ifconf_get_interface(eth_name, &ifconf_irf);
	APP_ASSERT(0 == ret, "HICHIP ifconf %s failed!", eth_name);
	
	sprintf(str_seg1_data,
		"Device-ID=%s" kCRLF // device_id
		"Device-Model=%s" kCRLF
		"Device-Name=%s" kCRLF
		"Esee-ID=%s" kCRLF
		"Channel-Cnt=1" kCRLF
		"IP=%d.%d.%d.%d" kCRLF // ipaddr
		"MASK=%d.%d.%d.%d" kCRLF //netmask
		"MAC=%02X:%02X:%02X:%02X:%02X:%02X" kCRLF // hwaddr
		"Gateway=%d.%d.%d.%d" kCRLF // gateway
		"Software-Version=%s" kCRLF
		"Http-Port=%d" kCRLF // port
		"Dhcp=%d" kCRLF
		"Ddns=0" kCRLF
		"Fdns=%d.%d.%d.%d" kCRLF // preferred_dns
		"Sdns=%d.%d.%d.%d" kCRLF // alternate_dns
		"DDNS-Enable=0" kCRLF
		"DDNS-User=" kCRLF
		"DDNS-Passwd=" kCRLF
		"DDNS-Host=" kCRLF
		"DDNS-Port=" kCRLF,
		device_id,
		device_model,
		device_name,
		ret_esee_info.id,
		ifconf_irf.ipaddr.s_b1, ifconf_irf.ipaddr.s_b2, ifconf_irf.ipaddr.s_b3, ifconf_irf.ipaddr.s_b4,
		ifconf_irf.netmask.s_b1, ifconf_irf.netmask.s_b2, ifconf_irf.netmask.s_b3, ifconf_irf.netmask.s_b4,
		ifconf_irf.hwaddr.s_b[0], ifconf_irf.hwaddr.s_b[1], ifconf_irf.hwaddr.s_b[2],
		ifconf_irf.hwaddr.s_b[3], ifconf_irf.hwaddr.s_b[4], ifconf_irf.hwaddr.s_b[5],
		ifconf_irf.gateway.s_b1, ifconf_irf.gateway.s_b2, ifconf_irf.gateway.s_b3, ifconf_irf.gateway.s_b4,
		sysconf->ipcam.info.software_version,
		sysconf->ipcam.network.lan.port[0].value,
		sysconf->ipcam.network.lan.dhcp,
		preferred_dns.s_b1, preferred_dns.s_b2, preferred_dns.s_b3, preferred_dns.s_b4,
		alternate_dns.s_b1, alternate_dns.s_b2, alternate_dns.s_b3, alternate_dns.s_b4);

	sprintf(str_seg1,
		"Segment-Seq:1" kCRLF
		"Data-Length:%d" kCRLF
		"" kCRLF
		"%s",
		strlen(str_seg1_data),
		str_seg1_data);

	sprintf(str_seg2_data,
		"[dev-media-info]" kCRLF
		"cam-count=1" kCRLF
		"[cam1]" kCRLF
		"id=1" kCRLF
		"stream-count=2" kCRLF
		"[cam1-stream1]" kCRLF
		"id=11" kCRLF
		"[cam1-stream2]" kCRLF
		"id=12" kCRLF);

	sprintf(str_seg2,
		"Segment-Seq:2" kCRLF
		"Data-Length:%d" kCRLF
		"" kCRLF
		"%s",
		strlen(str_seg2_data),
		str_seg2_data);

	sprintf(str_seg,
		"Segment-Num:2" kCRLF
		"%s"
		"%s",
		str_seg1,
		str_seg2);

	response_header = HTTP_UTIL_new_response_header("HDS", "1.0", 200, NULL);
	if(NULL != response_header){
		response_header->add_tag_int(response_header, "CSeq", atoi(cseq), true);
		response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
		response_header->add_tag_text(response_header, "Content-Type", "text/HDP", true);
		response_header->add_tag_int(response_header, "Content-Length", strlen(str_seg), true);
		response_header->to_text(response_header, response_buf, sizeof(response_buf));
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;

		// catch the content
		strcat(response_buf, str_seg);
		//APP_TRACE("%s", response_buf);

		// response the search request
		ret = HICHIP_DISCOVER_sock_send(sock, response_buf, strlen(response_buf));
		if(ret < 0){
			return -1;
		}

		return 0;
	}
	
	return -1;
}

int HICHIP_DISCOVER_process_gb28181(lpHICHIP_CONF_FUNC conf_func, int sock, lpHTTP_HEADER request_header, const void *request_content)
{
	int ret = 0;
	//SYSCONF_t* const sysconf = SYSCONF_dup();

	const char *cseq = NULL, *client_id = NULL;
	char response_buf[2048] = {""};
	lpHTTP_HEADER response_header = NULL;

	//printf("recv_conf_content:%s\n",(char *)request_content);
	conf_func->gb28181_conf(request_content);
	
	// some tags of header
	cseq = request_header->read_tag(request_header, "CSeq");
	client_id = request_header->read_tag(request_header, "Client-ID");

	response_header = HTTP_UTIL_new_response_header("DMCBG", "1.0", 200, NULL);
	if(NULL != response_header){
		response_header->add_tag_int(response_header, "CSeq", atoi(cseq), true);
		response_header->add_tag_text(response_header, "Client-ID", (char*)client_id, true);
		response_header->to_text(response_header, response_buf, sizeof(response_buf));
		//response_header->dump(response_header);
		response_header->free(response_header);
		response_header = NULL;
		printf("response:%s.\n",response_buf);
		// response the search request
		ret = HICHIP_DISCOVER_sock_send(sock, response_buf, strlen(response_buf));
		if(ret < 0){
			return -1;
		}

		return 0;
	}
	
	return -1;

}


