
#include "axdll.h"
#include "generic.h"
#include "ifconf.h"
#include "lonse_debug.h"
#include "lonse.h"

#define LONSE_MULTCAST_ETHER "eth0"
#define LONSE_DISCOVER_MULTICAST_ADDR "239.255.255.250"

#define LONSE_DISCOVER_SOURCE_PORT (1899)
#define LONSE_DISCOVER_DEVICE_PORT (1900) // server 1899

#define DEVICE_MODEL_FOR_SEARCH "LA92x1_HM"

typedef struct LONSE_DISCOVER
{
	char eth[16];
	char device_id[36];

	pthread_t doloop_tid;
	uint32_t doloop_trigger;
	
}LONSE_DISCOVER_t;

static LONSE_DISCOVER_t _lonse_discover;
static LONSE_DISCOVER_t* _p_lonse_discover = NULL;

static void discover_route(bool add_del, const char* eth)
{
	int ret = 0;
	char sys_cmd[128] = {""};
	ret = snprintf(sys_cmd, ARRAY_ITEM(sys_cmd),
		"route %s -net 224.0.0.0 netmask 224.0.0.0 %s",
		add_del ? "add" : "del", eth);
	LONSE_TRACE("System \"%s\"", sys_cmd);
	system(sys_cmd);
}

static int discover_sock_create(const char* local_ip, const char* member_ip, uint32_t member_port)
{
	int sock = 0;
	int ret = 0;
	struct sockaddr_in my_addr, to_addr;
	struct ip_mreq mreq;
	int flag = 0;

	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	memset(&to_addr, 0, sizeof(struct sockaddr_in));
	memset( &mreq, 0, sizeof(struct ip_mreq));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(LONSE_DISCOVER_DEVICE_PORT);
	ret = inet_aton(member_ip, &my_addr.sin_addr);
	assert(ret >= 0);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(sock > 0);

	ret = bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
	assert(0 == ret);

	ret = inet_aton(member_ip, &mreq.imr_multiaddr);
	assert(ret >= 0);

	//ret = inet_aton(local_ip, &(mreq.imr_interface)); // FIXME: the ip must follow the current local ip addresss
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(sock, SOL_IP, IP_ADD_MEMBERSHIP, &mreq,sizeof(mreq));
	LONSE_ASSERT(0 == ret, "setsockopt err!");

	flag = 0; // not backrush 
	ret = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));
	LONSE_ASSERT(0 == ret, "setsockopt err!");

	return sock;
}

static void discover_sock_release(int sock)
{
	close(sock);
}

static int discover_response_getinfo(int sock, struct sockaddr_in peer_addr)
{
	int ret = 0;
	TNetCmdPkt info_pkt;
	TDevInfoPkt* const p_dev_info = &info_pkt.CmdPkt.MulticastInfo.DevInfo;
	TNetCfgPkt* const p_net_cfg = &info_pkt.CmdPkt.MulticastInfo.NetCfg;
	TWiFiCfgPkt* const p_wifi_cfg = &info_pkt.CmdPkt.MulticastInfo.WiFiCfg;
	
	ifconf_interface_t irf;
	ifconf_dns_t dns;
	
	STRUCT_ZERO(info_pkt);
	info_pkt.HeadPkt.VerifyCode = Head_CmdPkt;
	info_pkt.HeadPkt.PktSize = sizeof(TCmdPkt);

	
	info_pkt.CmdPkt.MsgID = Msg_GetMulticastInfo;
	info_pkt.CmdPkt.PktHead = Head_CmdPkt;
	info_pkt.CmdPkt.Value = 1;
//	info_pkt.CmdPkt.MulticastInfo.DevInfo = DevCfg->DevInfoPkt;
//	info_pkt.CmdPkt.MulticastInfo.NetCfg = DevCfg->NetCfgPkt;
//	info_pkt.CmdPkt.MulticastInfo.WiFiCfg = DevCfg->WiFiCfgPkt;
	info_pkt.CmdPkt.MulticastInfo.Flag = 1;

	p_dev_info->SN = 11110000;
	p_dev_info->DevType = __DEVICETYPE;
	strncpy(p_dev_info->SoftVersion, "1.0.0", ARRAY_ITEM(p_dev_info->SoftVersion));
	strncpy(p_dev_info->FileVersion, "1.0.0", ARRAY_ITEM(p_dev_info->FileVersion));
	strncpy(p_dev_info->DevName, "JUAN hi3518a", ARRAY_ITEM(p_dev_info->DevName));
	strncpy(p_dev_info->DevDesc, "JUAN hi3518a", ARRAY_ITEM(p_dev_info->DevDesc));
	p_dev_info->VideoChlCount = VIDEOCHLCOUNT;
	p_dev_info->AudioChlCount = AUDIOCHLCOUNT;
	p_dev_info->DiChlCount = DICHLCOUNT;
	p_dev_info->DoChlCount = DOCHLCOUNT;
	p_dev_info->RS485DevCount =RS485DEVCOUNT;
	p_dev_info->Language = cn;
	p_dev_info->OEMType = 1;
	p_dev_info->RebootHM.w = w_close;
	p_dev_info->RebootHM.start_h = 0;
	p_dev_info->RebootHM.start_m = 0;
	p_dev_info->Info.NotExistAudio = false;
	p_dev_info->Info.NotExistRS485 = false;
	p_dev_info->Info.NotExistIO = false;

	p_net_cfg->CmdPort = 80;
	p_net_cfg->rtspPort = 80;
	p_net_cfg->HttpPort = 80;
	p_net_cfg->Lan.IPType = 0;//¾²Ì¬IP
	sprintf(p_net_cfg->Lan.DevIP, "192.168.100.168");
	sprintf(p_net_cfg->Lan.DevMAC, "00:00:00:00:00:00");
	sprintf(p_net_cfg->Lan.SubMask, "255.255.0.0");
	sprintf(p_net_cfg->Lan.Gateway, "192.168.1.1");
	sprintf(p_net_cfg->Lan.DNS1, "192.168.1.1");
	//sprintf(p_net_cfg->Lan.DNS2, "192.168.1.1");
	p_net_cfg->DDNS.Active = false;
	p_net_cfg->DDNS.DDNSType = 0;
	sprintf(p_net_cfg->DDNS.DDNSDomain, "Domain");
	sprintf(p_net_cfg->DDNS.HostAccount, "Account");
	sprintf(p_net_cfg->DDNS.HostPassword, "Password");
	p_net_cfg->PPPOE.AutoStart = false;
	sprintf(p_net_cfg->PPPOE.Account, "Account");
	sprintf(p_net_cfg->PPPOE.Password, "Password");
	p_net_cfg->uPnP.Active = false;

	p_wifi_cfg->Active = false;
	sprintf(p_wifi_cfg->DevIP, "192.168.2.168");
	sprintf(p_wifi_cfg->SubMask, "255.255.0.0");
	sprintf(p_wifi_cfg->Gateway, "192.168.2.1");
	//PPkt->SSID;
	p_wifi_cfg->Channel = 1;//ÆµµÀ1..14 default 1=Auto
	p_wifi_cfg->EncryptType = 0;//(Encrypt_None,Encrypt_WEP,Encrypt_WPA);
	p_wifi_cfg->WEPKeyBit = 0;//(kBit64,kBit128);
	p_wifi_cfg->WEPIndex = 0;//0..3;//=0


	STRUCT_ZERO(irf);
	STRUCT_ZERO(dns);

	ifconf_get_interface("eth0", &irf);
	ifconf_get_dns(&dns);

	do
	{	
		info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.IPType = 0;
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DevIP, ifconf_ipv4_ntoa(irf.ipaddr));
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DevMAC, ifconf_hw_ntoa(irf.hwaddr));
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.SubMask, ifconf_ipv4_ntoa(irf.netmask));
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.Gateway, ifconf_ipv4_ntoa(irf.gateway));
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DNS1, ifconf_ipv4_ntoa(dns.preferred));
		strcpy(info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DNS2, ifconf_ipv4_ntoa(dns.alternate));
		info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.Flag = 0;

		LONSE_TRACE("Lonse is discovering\r\n"
			"MAC\t%s\r\n"
			"IP\t%s\r\n"
			"MASK\t%s\r\n"
			"GATEWAY\t%s\r\n",
			info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DevMAC,
			info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.DevIP,
			info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.SubMask,
			info_pkt.CmdPkt.MulticastInfo.NetCfg.Lan.Gateway);
			
	}while(0);

	ret = sendto(sock, &info_pkt, sizeof(info_pkt), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
	if(ret < 0){
		return -1;
	}
	return 0;
}


static void* discover_doloop()
{
	int ret = 0;
	TNetCmdPkt net_cmd_pkt;
//	ssize_t recv_sz = 0;
	struct sockaddr_in from_addr, peer_addr;
//	struct ip_mreq mreq;

	socklen_t addr_len = sizeof(from_addr);
	int const sock = discover_sock_create("192.168.1.45", LONSE_DISCOVER_MULTICAST_ADDR, LONSE_DISCOVER_DEVICE_PORT);

	// peer address
	STRUCT_ZERO(peer_addr);
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(LONSE_DISCOVER_SOURCE_PORT);
	ret = inet_aton(LONSE_DISCOVER_MULTICAST_ADDR, &peer_addr.sin_addr);
	LONSE_ASSERT(ret > 0, "inet_aton err!");

	ret = fcntl(sock, F_SETFL,O_NONBLOCK);
	LONSE_ASSERT(0 == ret, "fcntl err!");

	LONSE_TRACE("Lonse loop start (%08x, %08x)", getpid(), pthread_self());
	while(_p_lonse_discover->doloop_trigger){
		in_port_t from_port = 0;
		ret = recvfrom(sock, &net_cmd_pkt, sizeof(net_cmd_pkt), 0, (struct sockaddr *)&from_addr, &addr_len);
		if(ret < 0){
			if(errno == EAGAIN){
				// try next time
				usleep(400000);
				continue;
			}
			perror("recvfrom");
			break;
		}
		
		from_port = ntohs(from_addr.sin_port);
		if(LONSE_DISCOVER_SOURCE_PORT == from_port){
			if(Head_CmdPkt == net_cmd_pkt.HeadPkt.VerifyCode){
				if(Msg_GetMulticastInfo == net_cmd_pkt.CmdPkt.MsgID){
					discover_response_getinfo(sock, peer_addr);
				}
			}
		}
	}
	
	discover_sock_release(sock);
	pthread_exit(NULL);
}

static void discover_doloop_start()
{
	if(!_p_lonse_discover->doloop_tid){
		int ret = 0;
		_p_lonse_discover->doloop_trigger = true;
		ret = pthread_create(&_p_lonse_discover->doloop_tid, NULL, discover_doloop, NULL);
		assert(0 == ret);
	}
}

static void discover_doloop_stop()
{
	if(_p_lonse_discover->doloop_tid){
		_p_lonse_discover->doloop_trigger = false;
		pthread_join(_p_lonse_discover->doloop_tid, NULL);
		_p_lonse_discover->doloop_tid = (pthread_t)NULL;
	}
}

int LONSE_init(const char* eth)
{
	if(!_p_lonse_discover){
		STRUCT_ZERO(_lonse_discover);
		_p_lonse_discover = &_lonse_discover;
		
		// init
		strncpy(_p_lonse_discover->eth, eth, ARRAY_ITEM(_p_lonse_discover->eth));
		_p_lonse_discover->doloop_trigger = false;
		_p_lonse_discover->doloop_tid = (pthread_t)NULL;
		
		//
		discover_route(true, _p_lonse_discover->eth);
		discover_doloop_start();
		return 0;
	}
	return -1;
}

void LONSE_destroy()
{
	if(_p_lonse_discover){
		discover_route(false, _p_lonse_discover->eth);
		discover_doloop_start();
		//
		free(_p_lonse_discover);
		_p_lonse_discover = NULL;
	}
}


