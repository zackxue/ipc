
#include <arpa/inet.h>
#include "aval.h"
#include "generic.h"
#include "http_common.h"
#include "upnp_debug.h"
#include "upnp_util.h"

void upnp_log(const char* file_name, void* xml_buf, ssize_t xml_size)
{
	FILE* fid = NULL;
	fid = fopen(file_name, "w+b");
	if(NULL != fid){
		UPNP_TRACE("Save as \"%s\"", file_name);
		fwrite(xml_buf, 1, xml_size, fid);
		fclose(fid);
		fid = NULL;
	}
}

void upnp_log_append(const char* file_name, void* xml_buf, ssize_t xml_size)
{
	FILE* fid = NULL;
	fid = fopen(file_name, "a+b");
	if(NULL != fid){
		UPNP_TRACE("Append as \"%s\"", file_name);
		fwrite(xml_buf, 1, xml_size, fid);
		fclose(fid);
		fid = NULL;
	}
}


static int broadcast_sock_new(int timeo_s)
{
	int ret = 0;
	int sock = 0;
	struct timeval timeo = {timeo_s, 0};
	struct sockaddr_in local_addr;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		UPNP_TRACE("create udp socket failed.\n");
		return -1;
	}
	
	//set send / recv timeout
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));
	UPNP_ASSERT(0 == ret, "Set send timeout failed.");
	ret = setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO, &timeo, sizeof(timeo));
	UPNP_ASSERT(ret>=0,"Set  recv timeout failed.");

	STRUCT_ZERO(local_addr);
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(53200);
	local_addr.sin_addr.s_addr = INADDR_ANY;
	//bind 
	ret = bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr));
	UPNP_ASSERT(0 == ret,"UDP bind failed: %s", strerror(errno));

	return sock;
}

static void broadcast_sock_close(int sock)
{
	close(sock);
}

int upnp_discovery_server(const char* broadcast_addr, 
	in_port_t broadcast_port,
	UPNP_CONTEXT_t* context,
	char *ip_gw)
{
	int ret = 0;
	char buf[4096] = {""};
	struct sockaddr_in peer_addr;
	int addr_len = 0;
	int sock_udp = broadcast_sock_new(5);

	if(sock_udp > 0){
		memset(buf,0,sizeof(buf));
		ret = snprintf(buf,ARRAY_ITEM(buf),
			"M-SEARCH * HTTP/1.1\r\n"
			"HOST: %s:%d\r\n"
			"ST: %s\r\n"
			"MAN: \"ssdp:discover\"\r\n"
			"MX: %u\r\n"
			"\r\n",
			broadcast_addr, broadcast_port,
			"urn:schemas-upnp-org:device:InternetGatewayDevice:1",
			2);
		
		STRUCT_ZERO(peer_addr);
		peer_addr.sin_family = AF_INET;
		peer_addr.sin_port = htons(broadcast_port);
		peer_addr.sin_addr.s_addr = inet_addr(broadcast_addr);
		ret = sendto(sock_udp, buf, strlen(buf), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
		if(strlen(buf) != ret){
			UPNP_TRACE("Discovery to %s:%d failed", broadcast_addr, (uint32_t)broadcast_port);
			broadcast_sock_close(sock_udp);
			return -1;
		}

		STRUCT_ZERO(peer_addr);
		
		int n=0;
		do{
			n++;
			ret = recvfrom(sock_udp, buf, ARRAY_ITEM(buf), 0, (struct sockaddr *)&peer_addr, &addr_len);
			if(ret > 0){
				AVal av_location = AVC(""), av_st = AVC("");
				
				// success to receive
				buf[ret]=0;
				// log it
				upnp_log("/tmp/discovery.pcap", buf, ret);
				//printf("get dicovery ack:\n%s\n",buf);
				// read the http header
				if(0 == http_read_header(buf, "LOCATION", &av_location)
					&& 0 == http_read_header(buf, "ST", &av_st)){
					if(context){
						sscanf(av_location.av_val,"http://%[^:]:%d%s",
							context->server_addr,&context->server_port,context->location);
						UPNP_TRACE("%d expect:%s ip:%s port:%d loc:%s",n,
							ip_gw,
							context->server_addr,context->server_port,context->location);
						strncpy(context->device_type, AVAL_STRDUPA(av_st), ARRAY_ITEM(context->device_type));
						//UPNP_TRACE("LOCATION: %s | ST: %s", context->location, context->device_type);
						if(strcmp(ip_gw,context->server_addr)==0){
							//strcpy(context->server_addr,sz_ip);
							UPNP_TRACE("->>>>>>>>found upnp in gateway.");
							broadcast_sock_close(sock_udp);
							return 0;
						}
						//broadcast_sock_close(sock_udp);
						//return 0;
					}
				}
			}
		}while(ret >0);
		broadcast_sock_close(sock_udp);
		return -1;
	}
	return -1;
}

static int http_sock_new(const char* server_addr, in_port_t server_port, int timeo_s)
{
	int ret = 0;
	struct timeval send_timeo = { .tv_sec = timeo_s, .tv_usec = 0, };
	struct timeval recv_timeo = { .tv_sec = timeo_s, .tv_usec = 0, };
	int sock = -1;
	struct sockaddr_in peer_addr;

	// setup socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	UPNP_ASSERT(sock > 0, "socket %s", strerror(errno));

	send_timeo.tv_sec = 10;
	send_timeo.tv_usec = 0;
	recv_timeo.tv_sec = 2;
	recv_timeo.tv_usec = 0;
	ret = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &send_timeo, sizeof(send_timeo));
	UPNP_ASSERT(0 == ret, "Set send timeout %s", strerror(errno));
	
	ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeo, sizeof(recv_timeo));
	UPNP_ASSERT(0 == ret, "Set recv timeout %s", strerror(errno));

	// start to send request
	STRUCT_ZERO(peer_addr);
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(server_port);
	peer_addr.sin_addr.s_addr = inet_addr(server_addr);
	
	ret = connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
	if(ret < 0){
		UPNP_TRACE("Connect to %s:%d failed %s",server_addr,server_port, strerror(errno));
		return -1;
	}

	return sock;
}

static void http_sock_close(int sock)
{
	close(sock);
}

static int make_http_get_header(void* buf, ssize_t buf_size, const char* uri, const char* where_addr, in_port_t where_port)
{
	int ret = 0;
	memset(buf,0,sizeof(buf));
	
	ret = snprintf(buf, buf_size,
		"GET %s HTTP/1.1" CRLF
		"Host: %s:%d" CRLF
		"Connection: close" CRLF
		"User-Agent: JA UPnP/1.0" CRLF
		CRLF,
		uri, where_addr, where_port);

	UPNP_TRACE("Make HTTP GET header size=%d\r\n%s",ret, buf);
	return 0;
}

int upnp_http_get_location(UPNP_CONTEXT_t* context)
{
	int ret = 0;
	int sock = -1;

	sock = http_sock_new(context->server_addr, context->server_port, 5);
	if(sock > 0){
		char http_header[1024] = {""};

		// make a get http request
		make_http_get_header(http_header, ARRAY_ITEM(http_header),
			context->location, context->server_addr, context->server_port);

		// send get packet
		ret = send(sock, http_header, strlen(http_header), 0);
		UPNP_ASSERT(strlen(http_header) == ret, "Send error %s", strerror(errno));

		ret = recv(sock, http_header, ARRAY_ITEM(http_header), MSG_PEEK);
		if(ret > 0 && strstr(http_header, CRLF CRLF)){
			// peek msg success
			ssize_t header_size = (strstr(http_header, CRLF CRLF) - http_header) + strlen(CRLF CRLF);
			
			// receive http header
			ret = recv(sock, http_header, header_size, 0);
			if(ret == header_size){
				AVal av_content_len = AVC("0");

				// save the http header
				upnp_log("/tmp/upnp/location.pcap", http_header, header_size);
		
				http_header[header_size] = '\0';
				if(0 == http_read_header(http_header, "Content-Length", &av_content_len)){
					ssize_t const content_len = atoi(AVAL_STRDUPA(av_content_len));
					char* const content_buf = calloc(content_len, 1);
					
					UPNP_TRACE("Content Length = %d", content_len);
					ret = recv(sock, content_buf, content_len, 0);
 					if(ret == content_len){
 						upnp_log_append("/tmp/upnp/location.pcap", content_buf, content_len);
						upnp_log("/tmp/upnp/igd.xml", content_buf, content_len);
						
						http_sock_close(sock);
						free(content_buf);
						return 0;
					}

					free(content_buf);
				}
			}
		}
		http_sock_close(sock);
	}
	return -1;
	
}


