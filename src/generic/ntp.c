

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <time.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>

#include "bsp/rtc.h"

#define NTP_SERVER_PORT  (123)
#define NTP_CLIENT_PORT (NTP_SERVER_PORT)

typedef struct NtpPack
{
	uint8_t li_vn_mode;
	uint8_t stratum;
	uint8_t poll;
	uint8_t precision;
	uint32_t root_delay;
	uint32_t root_dispersion;
	char ref_id[4];
	uint32_t reftimestamphigh;
	uint32_t reftimestamplow;
	uint32_t oritimestamphigh;
	uint32_t oritimestamplow;
	uint32_t recvtimestamphigh;
	uint32_t recvtimestamplow;
	uint32_t trantimestamphigh;  
	uint32_t trantimestamplow;
}NtpPack_t;

typedef struct NTP
{
	uint32_t ntp_trigger;
	pthread_t ntp_tid;
	char *server_domain;
	char *server_ip;
	int timeout;
	char timezone;
}NTP_t;
static NTP_t* _ntp = NULL;


static int64_t time_sec_from_1970_to_1900()
{
	int year_leap = 0;
	int year_origin = 0;
	int i = 0;
	for(i = 1900; i < 1970; ++i){
		if(((i % 4 == 0) && (i % 100 != 0)) || (i % 400 == 0)){
			++year_leap;
		}else{
			++year_origin;
		}
	}
	return (year_leap * 366 + year_origin * 365) * 24 * 60 * 60;
}

static const char* ntp_server_ip_list[] =
{
	// list from 'http://tf.nist.gov/tf-cgi/servers.cgi'
	// IP -- Address Name -- Location -- Status
	"64.90.182.55", // nist1-ny.ustiming.org		New York City, NY	All services available
	"96.47.67.105", // nist1-nj.ustiming.org		Bridgewater, NJ	All services available
	"206.246.122.250", // nist1-pa.ustiming.org		Hatfield, PA	All services available
	"129.6.15.28", // time-a.nist.gov		NIST, Gaithersburg, Maryland	ntp ok, time,daytime busy, not recommended
	"129.6.15.29", // time-b.nist.gov		NIST, Gaithersburg, Maryland	ntp ok, time,daytime busy, not recommended
	"64.236.96.53", // nist1.aol-va.symmetricom.com		Reston, Virginia	All services available
	"216.119.63.113", // nist1.columbiacountyga.gov		Columbia County, Georgia	All services available
	"64.250.177.145", // nist1-atl.ustiming.org		Atlanta, Georgia	All services available
	"208.66.175.36", // nist1-chi.ustiming.org		Chicago, Illinois	Temporary Failure
	"38.106.177.10", //nist-chicago (No DNS)		Chicago, Illinois	All services available
	"96.226.123.117", // nist.time.nosc.us		Carrollton, Texas	All services available
	"50.77.217.185", //nist.expertsmi.com		Monroe, Michigan	All services available
	"64.113.32.5", // nist.netservicesgroup.com		Southfield, Michigan	NTP ok, daytime and time busy
	"66.219.116.140", // nisttime.carsoncity.k12.mi.us		Carson City, Michigan	All services available
	"216.229.0.179", // nist1-lnk.binary.net		Lincoln, Nebraska	All services available
	"24.56.178.140", // wwv.nist.gov		WWV, Fort Collins, Colorado	All services available
	"132.163.4.101", // time-a.timefreq.bldrdoc.gov		NIST, Boulder, Colorado	ntp ok, time, daytime busy, not recommended
	"132.163.4.102", // time-b.timefreq.bldrdoc.gov		NIST, Boulder, Colorado	All services busy, not recommended
	"132.163.4.103", // time-c.timefreq.bldrdoc.gov		NIST, Boulder, Colorado	ntp ok, time daytime busy, not recommended
	//time.nist.gov	global address for all servers	Multiple locations	All services available
	"128.138.140.44", // utcnist.colorado.edu		University of Colorado, Boulder	All services available
	"128.138.141.172", // utcnist2.colorado.edu		University of Colorado, Boulder	All services available
	"64.111.21.121", // nist.colorado-networks.com		Colorado Springs, CO	Network problems
	"198.60.73.8", // ntp-nist.ldsbc.edu		LDSBC, Salt Lake City, Utah	All services available
	"64.250.229.100", // nist1-lv.ustiming.org		Las Vegas, Nevada	All services available
	"131.107.13.100", // time-nw.nist.gov		Microsoft, Redmond, Washington	ntp, time ok, daytime busy, not recommended
	"216.228.192.69", // nist-time-server.eoni.com		La Grande, Oregon	All services available
	"207.200.81.113", // nist1.aol-ca.symmetricom.com		Mountain View, California	All services available
	"69.25.96.13", // nist1.symmetricom.com		San Jose, California	All services available
	"216.171.124.36", // nist1-sj.ustiming.org		San Jose, California	All services available
	"64.147.116.229", // nist1-la.ustiming.org		Los Angeles, California	All services available
};

static void ntp_pack_ntohl(NtpPack_t* pack)
{
	pack->root_delay = ntohl(pack->root_delay);
	pack->root_dispersion = ntohl(pack->root_dispersion);
	pack->reftimestamphigh =ntohl(pack->reftimestamphigh);
	pack->reftimestamplow = ntohl(pack->reftimestamplow);
	pack->oritimestamphigh = ntohl(pack->oritimestamphigh);
	pack->oritimestamplow = ntohl(pack->oritimestamplow);
	pack->recvtimestamphigh = ntohl(pack->recvtimestamphigh);
	pack->recvtimestamplow = ntohl(pack->recvtimestamplow);
	pack->trantimestamphigh = ntohl(pack->trantimestamphigh);
	pack->trantimestamplow = ntohl(pack->trantimestamplow);
}

static int ntp_sock_create()
{
	int ret = 0;
	int sock = -1;
	struct sockaddr_in my_addr;
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock > 0);

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(NTP_CLIENT_PORT);
//	ret = bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
//	assert(0 == ret);
	
	ret = fcntl(sock, F_SETFL,O_NONBLOCK);
	assert(0 == ret);
	
	return sock;
}

static int ntp_sock_send(int sock, const char* server_domain, const char* server_addr, uint16_t server_port, uint64_t send_pts)
{
	struct sockaddr_in addr;
	NtpPack_t ntp_pack;
	struct hostent* host_ent = NULL;

	if(server_domain){
		host_ent = gethostbyname2(server_domain, AF_INET);
		if(!host_ent){
			// no host name
			printf("domain %s not found!\r\n", server_domain);
			return -1;
		}
		// reset the server ip
		server_addr = inet_ntoa(*(struct in_addr*)(host_ent->h_addr)); //inet_ntoa(*host_ip);
		printf("host: %s -- ip: %s\r\n", server_domain, server_addr);
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(server_port);
	addr.sin_addr.s_addr = inet_addr(server_addr);

	// request
	memset(&ntp_pack, 0, sizeof(ntp_pack));
	ntp_pack.li_vn_mode = 0x1b; //0|(3<<2)|(3<<5);
  	ntp_pack.oritimestamphigh = htonl(send_pts);

	// needless to check send result
	return sendto(sock, &ntp_pack, sizeof(ntp_pack), 0,(struct sockaddr *)&addr, sizeof(struct sockaddr));
}

static int ntp_sock_recv(int sock, int wait_sec, NtpPack_t* ret_pack)
{
	int ret = 0;
	struct sockaddr_in addr;
	socklen_t addr_len = 0;
	// wait response
	while(wait_sec-- > 0)
	{
		ret = recvfrom(sock, ret_pack, sizeof(NtpPack_t), 0, (struct sockaddr*)&addr, &addr_len);
		if(ret < 0){
			if(errno == EAGAIN){
				sleep(1);
				continue;
			}
			return -1;
		}
		break;
	}
	if(wait_sec > 0){
		printf("recv from %s\r\n", inet_ntoa(addr.sin_addr));
		ntp_pack_ntohl(ret_pack);
		return 0;
	}
	return -1;
}

static void ntp_sock_release(int sock)
{
	close(sock);
}

int NTP_sync()
{
	int i = 0;
	int ret = 0;
	int  sock;
	struct timeval tv;
	struct timezone tz;
//	struct sockaddr_in addr;
//	socklen_t addr_len = sizeof(struct sockaddr);

//	int retry_time = timeout;
	time_t ntp_time = 0;
	
	NtpPack_t ntp_pack;

	uint64_t send_pts = 0, recv_pts = 0;
	uint64_t diff_time = 0, diff_delay = 0;

	int64_t const second_1900_1970 = time_sec_from_1970_to_1900();

	///////////////////////////////////////////////////////
	// temporary to setup chinese time zone
	// ////////////////////////////////////////////////////
	// default to GMT+8
	tz.tz_minuteswest = -1 * _ntp->timezone * 60;
	tz.tz_dsttime = 0;
	settimeofday(NULL, &tz);

	// create the sock for ntp transmit
	sock = ntp_sock_create();

	// mark down the send timestamp
	send_pts = second_1900_1970 + time(NULL);
	if((_ntp->server_domain && strlen(_ntp->server_domain) != 0) || _ntp->server_ip){
		ntp_sock_send(sock, _ntp->server_domain, _ntp->server_ip, NTP_SERVER_PORT, send_pts);
	}else{
		for(i = 0; i < sizeof(ntp_server_ip_list) / sizeof(ntp_server_ip_list[0]); ++i){
			ntp_sock_send(sock, NULL, ntp_server_ip_list[i], NTP_SERVER_PORT, send_pts);
		}
	}

//	sleep(1);
	// wait response
	ret = ntp_sock_recv(sock, _ntp->timeout, &ntp_pack);
	// mark down recv timestamp
	recv_pts = time(NULL) + second_1900_1970;
	ntp_sock_release(sock);

	if(0 == ret){
		 // get the diff of time
		diff_time = ((ntp_pack.recvtimestamphigh - send_pts) + (ntp_pack.trantimestamphigh - recv_pts)) / 2;
		diff_delay =((ntp_pack.recvtimestamphigh - send_pts) - (ntp_pack.trantimestamphigh - recv_pts)) / 2;

		gettimeofday(&tv, &tz);

		// get the ntp time stamp
		//ntp_time = time(NULL) + diff_time + diff_delay + (-1 * tz.tz_minuteswest * 60);
		ntp_time = time(NULL) + diff_time + diff_delay;
		tv.tv_sec = ntp_time;
		tv.tv_usec = 0;
 
		settimeofday(&tv,NULL);
		RTC_settime(ntp_time);
//		printf("diff time: %llu\r\n", diff_time);
		system("date");
		return 0;
	}

	return -1;
}

int NTP_start(const char* server_domain, const char* server_ip, int timeout, char timezone)
{
	if(!_ntp){
		_ntp = calloc(sizeof(NTP_t), 1);
		assert(_ntp);
		_ntp->server_domain = server_domain;
		_ntp->server_ip = server_ip;
		_ntp->timeout = timeout;
		_ntp->timezone = timezone;
		_ntp->ntp_trigger = 0;
		_ntp->ntp_tid = (pthread_t)NULL;
		
		int ret = 0;
		_ntp->ntp_trigger = 1;
		ret = pthread_create(&_ntp->ntp_tid, NULL, NTP_sync, NULL);
		assert(0 == ret);
	}
	
	
	return 0;
}

