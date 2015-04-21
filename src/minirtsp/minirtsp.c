#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "minirtsp.h"
#include "rtsplib.h"
#include "rtspclient.h"
#include "rtspserver.h"


static RtspPlayList_t g_RtspPlayList={.entries = 1,};
static int g_toggle = true;

void RTSP_PLAYLIST_init()
{
	g_RtspPlayList.entries = 0;
}

void RTSP_PLAYLIST_destroy()
{
	g_RtspPlayList.entries = 0;
}


int RTSP_PLAYLIST_add(char *url,char *user,char *pwd,int transport,int buffer_time,int channel)
{
	if(strlen(url) > RTSP_PLAYITEM_MAX_CHAR){
		VLOG(VLOG_ERROR,"strlen(url) exceed the buffer size");
		return RTSP_RET_FAIL;
	}
	if(g_RtspPlayList.entries >=RTSP_PLAYLIST_MAX_ITEM){
		VLOG(VLOG_ERROR,"the entries of playlist exceed the buffer size");
		return RTSP_RET_FAIL;
	}
	strcpy(g_RtspPlayList.items[g_RtspPlayList.entries].url,url);
	g_RtspPlayList.items[g_RtspPlayList.entries].transport = transport;
	g_RtspPlayList.items[g_RtspPlayList.entries].buffertime = buffer_time;
	g_RtspPlayList.items[g_RtspPlayList.entries].rtsp = NULL;
	g_RtspPlayList.items[g_RtspPlayList.entries].channel = channel;
	g_RtspPlayList.items[g_RtspPlayList.entries].toggle = false;
	if(user) strcpy(g_RtspPlayList.items[g_RtspPlayList.entries].user,user);
	if(pwd) strcpy(g_RtspPlayList.items[g_RtspPlayList.entries].pwd,pwd);
	g_RtspPlayList.entries++;
	
	return RTSP_RET_OK;
}

int RTSP_PLAYLIST_remove(char *url)
{
	int i,flag=false;
	for(i=0;i<g_RtspPlayList.entries;i++){
		if(strcmp(g_RtspPlayList.items[i].url,url) == 0){
			flag=true;
		}
	}
	if(flag == true){
		for(;i<g_RtspPlayList.entries;i++){
			memcpy(&g_RtspPlayList.items[i],&g_RtspPlayList.items[i+1],sizeof(RtspPlayItem_t));
		}
		g_RtspPlayList.entries--;
	}
	return RTSP_RET_OK;
}



void *RTSP_PLAYER_PROC(void *arg)
{
	int i;
	int ret;
	Rtsp_t *r;
	RtspPlayItem_t *pitem=NULL;
	while(g_toggle){
		for(i=0;i<g_RtspPlayList.entries;i++){
			pitem = &g_RtspPlayList.items[i];
			if(pitem->toggle == false){
				pitem->toggle = true;
				RTSPC_daemon((Rtsp_t **)&pitem->rtsp,pitem->url,pitem->user,pitem->pwd,pitem->transport,pitem->buffertime,pitem->channel,&pitem->toggle);
			}
		}
		MSLEEP(5000);
	}
	return NULL;
}

int RTSP_PLAYER_start()
{
	ThreadId_t rtsp_thread;

	THREAD_create(rtsp_thread,RTSP_PLAYER_PROC,NULL);
	return 0;
}

int RTSP_PLAYER_stop()
{
	int i;
	int ret;
	g_toggle = false;
	for(i=0;i<g_RtspPlayList.entries;i++){
		if(g_RtspPlayList.items[i].rtsp != NULL && (g_RtspPlayList.items[i].toggle == true)){
			//RTSP_destroy(g_RtspPlayList.items[i].rtsp);
			g_RtspPlayList.items[i].toggle = false;
		}
	}
	return 0;
}

static void signal_handle(int sign_no) 
{
    VLOG(VLOG_DEBUG,"rtsp player kill!");
	RTSP_PLAYER_stop();
	MSLEEP(1000);
	exit(0);
}

int RTSP_PLAYER_test()
{
	const char *url1="rtsp://211.139.194.251:554/live/2/13E6330A31193128/5iLd2iNl5nQ2s8r8.sdp";//mpeg
	const char *url2="rtsp://192.168.2.10:554/user=admin&password=tlJwpbo6&channel=1&stream=0.sdp?real_stream";
	const char *url3="rtsp://192.168.2.86:554/ch0_0.264";// juan dvr
	const char *url4="rtsp://192.168.2.220:80/720p.264"; // juan old rtsp ,live555
	const char *url5="rtsp://180.168.116.75:554/user=admin&password=&channel=1&stream=0.sdp";//h264, 720p
	const char *url6="rtsp://192.168.2.80:554/test.264"; // vlc streaming
	const char *url7="rtsp://192.168.2.86:554/ch0_0.264";// juan ipc
	const char *url8="rtsp://192.168.2.166:554";// hk ipc
	
	signal(SIGINT,signal_handle);
	RTSP_PLAYLIST_init();
	//RTSP_PLAYLIST_add(url1,NULL,NULL,RTSP_RTP_OVER_UDP,1500,0);
	//RTSP_PLAYLIST_add(url2,NULL,NULL,RTSP_RTP_OVER_UDP,300,0);
	//RTSP_PLAYLIST_add(url3,NULL,NULL,RTSP_RTP_OVER_UDP,300,1);
	//RTSP_PLAYLIST_add(url7,NULL,NULL,RTSP_RTP_OVER_RTSP,500,2);
	//RTSP_PLAYLIST_add(url7,NULL,NULL,RTSP_RTP_OVER_UDP,500,3);
	//RTSP_PLAYLIST_add(url7,NULL,NULL,RTSP_RTP_OVER_UDP,500,4);
	//RTSP_PLAYLIST_add(url5,NULL,NULL,RTSP_RTP_OVER_RTSP,1500,5);
	
	//RTSP_PLAYLIST_add(url2,NULL,NULL,RTSP_RTP_OVER_RTSP,500,6);
	//RTSP_PLAYLIST_add(url3,NULL,NULL,RTSP_RTP_OVER_RTSP,500,7);
	RTSP_PLAYLIST_add(url8,"admin","12345",RTSP_RTP_OVER_UDP,500,5);
	RTSP_PLAYLIST_add(url8,"admin","12345",RTSP_RTP_OVER_UDP,500,6);
	RTSP_PLAYLIST_add(url8,"admin","12345",RTSP_RTP_OVER_RTSP,500,7);
	RTSP_PLAYER_start();
	
	return 0;
}



#ifndef NOCROSS
#include "rtspdef.h"
#include "rtsplib.h"

static int caculate_average(int a[],int n)
{
	int j, k;  
	int flag,tmp;  
	int sum=0;
	k = n;  
	flag = true;  
	while (flag)  
	{	
		flag = false;  
		for (j = 1; j < k; j++){
			if (a[j - 1] > a[j])  
			{ 
				tmp=a[j-1];
				a[j-1]=a[j];
				a[j]=tmp;
				flag = true;	
			}
		}
		k--;  
	}
	for(j=0;j<n;j++) printf("%d,",a[j]);
	printf("\r\n");
	for(j=0;j< n/2;j++)
		sum+=a[j];
	return sum/(n/2);
}


SPOOK_SESSION_PROBE_t MINIRTSP_probe(const void* msg, ssize_t msg_sz)
{
	if((memcmp(msg,"OPTIONS",strlen("OPTIONS")) == 0)
		|| (memcmp(msg,"DESCRIBE",strlen("DESCRIBE")) == 0)
		|| (memcmp(msg,"SET_PARAMETER",strlen("SET_PARAMETER")) == 0)
		|| (memcmp(msg,"GET_PARAMETER",strlen("GET_PARAMETER")) == 0)){
		return SPOOK_PROBE_MATCH;
	}
	return SPOOK_PROBE_MISMATCH;
}

SPOOK_SESSION_LOOP_t MINIRTSP_loop(uint32_t* trigger, int sock, time_t* read_pts, const void* msg, ssize_t msg_sz)
{
	ThreadArgs_t args;
	args.data = NULL;
	args.LParam = (void *)trigger;
	args.RParam = sock;
	RTSPS_proc(&args);
	
	return SPOOK_LOOP_SUCCESS;
}

#else
int main(int argc,char **argv)
{
	const char *usage="./minirtsp [-c | -s ]\r\n";
	if(argc < 2){
		printf(usage);
		return 0;
	}
	if(strcmp(argv[1],"-c")==0){
		VLOG(VLOG_CRIT,"run as player...");
		RTSPC_test(argc,argv);
	}else if(strcmp(argv[1],"-s")==0){
		VLOG(VLOG_CRIT,"run as server...");
		RTSPS_test(argc,argv);
	}else{
		printf(usage);
	}
	return 0;
}
#endif

