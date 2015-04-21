
#include "hichip_debug.h"
#include "hichip.h"
#include "hichip_discover.h"
#include "hichip_http_cgi.h"
#include "ifconf.h"
#include "sysconf.h"
#include "esee_client.h"
#include "md5sum.h"
#include "ucode.h"
#include "generic.h"

#include "app_debug.h"
#include "http_util.h"

typedef struct HICHIP_SERVER {
	stHICHIP_CONF_FUNC conf_func;
	pthread_t discover_tid;
	bool discover_trigger;
}stHICHIP_SERVER, *lpHICHIP_SERVER;

static stHICHIP_SERVER _hichip = {
	.discover_tid = (pthread_t)NULL,
	.discover_trigger = false,
};
static lpHICHIP_SERVER _p_hichip = NULL;

static void *hichip_discover_listener()
{
	int ret = 0;
	char buf[2048];
//	ssize_t recv_sz = 0;
	struct sockaddr_in from_addr, peer_addr;
//	struct ip_mreq mreq;
	ifconf_interface_t irf;

	ifconf_get_interface("eth0", &irf);
	socklen_t addr_len = sizeof(from_addr);
	int sock = HICHIP_DISCOVER_sock_create(ifconf_ipv4_ntoa(irf.ipaddr));

	char route_cmd[256] = {""};

	// add route
	snprintf(route_cmd, sizeof(route_cmd),
		"route add -net "HICHIP_MULTICAST_NET_SEGMENT" netmask "HICHIP_MULTICAST_NET_SEGMENT" %s", _p_hichip->conf_func.ether_lan);
	system(route_cmd);

	// peer address
	peer_addr = HICHIP_DISCOVER_multicast_addr();
	
	while(_p_hichip->discover_trigger){
		fd_set read_fds;
		struct timeval poll_wait;
	
		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);
		poll_wait.tv_sec = 0;
		poll_wait.tv_usec = 500000;
		ret = select(sock + 1, &read_fds, NULL, NULL, &poll_wait);
		if(ret < 0){
			perror("select");
			break;
		}else if(0 == ret){
			continue; // to next loop
		}else{
			ret = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &addr_len);
			if(ret < 0){
				if(errno == EAGAIN){
					// try next time
					continue;
				}
				perror("recvfrom");
				break;
			}
#ifndef MAKE_IMAGE
			if(0 != strncmp(inet_ntoa(from_addr.sin_addr), "192.168.2.36", sizeof("192.168.2.36"))){
				continue;
			}
#endif

			if(0 == strcmp("HDS", buf)){
				// broadcast response from others, ignore
				
			}else{
				size_t const header_size = HTTP_UTIL_check_header(buf, sizeof(buf));
				lpHTTP_HEADER http_request = HTTP_UTIL_parse_request_header(buf, sizeof(buf));
				const char *http_content = (char*)buf + header_size;			

				// endsym
				buf[ret] = '\0';
				
				if(NULL != http_request){
					http_request->dump(http_request);
					
					if(0 == strcmp("SEARCH", http_request->method)){
						APP_TRACE("HICHP search by %s", inet_ntoa(from_addr.sin_addr));
						HICHIP_DISCOVER_process_search(&_p_hichip->conf_func, sock, http_request, http_content);
					}else if(0 == strcmp("CMD", http_request->method)){
						const char *device_id = http_request->read_tag(http_request, "Device-ID");
						if(NULL != device_id && NULL != _p_hichip->conf_func.device_id){
							// check the match device id
							if(0 == strcmp(_p_hichip->conf_func.device_id(), device_id)
								&& strlen(_p_hichip->conf_func.device_id()) == strlen(device_id)){
								// only operate only the device id match one
								HICHIP_DISCOVER_process_cmd(&_p_hichip->conf_func, sock, http_request, http_content);
							}
						}
					}else if(0 == strcmp("GBCMD", http_request->method)){
						printf("%s\r\n", buf);
						const char *device_id = http_request->read_tag(http_request, "Device-ID");
						if(NULL != device_id && NULL != _p_hichip->conf_func.device_id){
							// check the match device id
							if(0 == strcmp(_p_hichip->conf_func.device_id(), device_id)
								&& strlen(_p_hichip->conf_func.device_id()) == strlen(device_id)){
								// only operate only the device id match one
								HICHIP_DISCOVER_process_gb28181(&_p_hichip->conf_func, sock, http_request, http_content);
							}
						}
						//						
					}else{
						// other
						//printf("%s\r\n", buf);
					}

					http_request->free(http_request);
					http_request = NULL;
				}
			}
		}
	}

	// remove route
	snprintf(route_cmd, sizeof(route_cmd),
		"route del -net "HICHIP_MULTICAST_NET_SEGMENT" netmask "HICHIP_MULTICAST_NET_SEGMENT" %s", _p_hichip->conf_func.ether_lan);
	system(route_cmd);

	// close multicast socket
	HICHIP_DISCOVER_sock_release(sock);
	pthread_exit(NULL);
}

static void hichip_discover_start()
{
	if(!_p_hichip->discover_tid){
		int ret = 0;
		_p_hichip->discover_trigger = true;
		ret = pthread_create(&_p_hichip->discover_tid, NULL, hichip_discover_listener, NULL);
		assert(0 == ret);
	}
}

static void hichip_discover_stop()
{
	if(_p_hichip->discover_tid){
		_p_hichip->discover_trigger = false;
		pthread_join(_p_hichip->discover_tid, NULL);
		_p_hichip->discover_tid = (pthread_t)NULL;
	}
}

int HICHIP_init(stHICHIP_CONF_FUNC conf_func)
{
	HICHIP_http_init();
	if(!_p_hichip){
		_p_hichip = &_hichip;
		// init elements
		_p_hichip->conf_func = conf_func;
		_p_hichip->discover_trigger = false;
		_p_hichip->discover_tid = (pthread_t)NULL;
		//
		hichip_discover_start();
		return 0;
	}
	return -1;
}

void HICHIP_destroy()
{
	HICHIP_http_destroy();
	if(_p_hichip){
		hichip_discover_stop();
		_p_hichip = NULL;
	}
}


