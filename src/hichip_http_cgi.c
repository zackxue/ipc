#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"
#include "http_common.h"
#include "sensor.h"
#include "sysconf.h"
#include "hichip_debug.h"
#include "hichip.h"
#include "timertask.h"
#include "sdk/sdk_vin.h"
#include "app_motion_detect.h"
#include "sdk_isp.h" // in hi_isp_tmp
#include "app_overlay.h"
#include "cgi_hash.h"
#include "hichip_http_cgi.h"
#include <string.h>

#define HICHIP_SERVER		"JuanServer"
#include <linux/tcp.h>
#define HICHIP_PARAM_TO_SET (true)
#define HICHIP_PARAM_TO_GET (false)
#define HICHIP_DEFAULT_WIDTH	1280.0
#define HICHIP_DEFAULT_HEIGHT	720.0

#define HICHIP_PARAM_SUFFIX(src, dst, set_get)					\
	do{															\
		char *ptr = NULL;										\
		if(ptr = strstr(src, "get")){							\
			dst = strdupa(src + 3);								\
			set_get = HICHIP_PARAM_TO_GET;						\
		}														\
		else if(ptr = strstr(src, "set")){						\										
			dst = strdupa(src + 3);								\
			set_get = HICHIP_PARAM_TO_SET;						\
		}														\
		else{													\
			dst = src;											\
			set_get = HICHIP_PARAM_TO_GET;						\
		}														\
	}while(0)													\


struct HICHIP_RTP_HDR
{
	// byte 0 
	unsigned short cc :4; // CSRC count 
	unsigned short x :1; // header extension flag 
	unsigned short p :1; // padding flag 
	unsigned short version :2; // protocol version 
	// byte 1 
	unsigned short pt :7; // payload type 
	unsigned short marker :1; // marker bit 
	// bytes 2, 3 
	unsigned short seqno :16; // sequence number 
	// bytes 4-7 
	unsigned int ts; // timestamp in ms 
	// bytes 8-11 
	unsigned int ssrc; // synchronization source 
};

struct HICHIP_RTSP_ITLEAVED_HDR
{
	unsigned char dollar; //8, $:dollar sign(24 decimal)
	unsigned char channelid; //8, channel id
	unsigned short resv; //16, reseved
	unsigned int payloadLen; //32, payload length, the sum with the frame size and the 'rtpHead'
	struct HICHIP_RTP_HDR rtpHead; //rtp head
};



static bool g_bHashInit = false;
static CGI_hashtable *g_htHiChip = NULL;
extern lpSDK_VIN_API sdk_vin;

static char * hichip_uri_suffix2cgi(AVal suffix, AVal param){
	char *strCgi = NULL;
	const char *strQueryParam = AVAL_STRDUPA(param);
	const char *strUriSuff = AVAL_STRDUPA(suffix);
	if(NULL != strstr(strUriSuff, "param") && NULL != (strCgi = strstr(strQueryParam, "cmd="))){
		char *tmp = strstr(strQueryParam, "&");
		int len = 0;
		if(tmp){
			len = tmp - strCgi - strlen("cmd=");
		}
		else{
			len = strlen(strCgi) - strlen("cmd=");
		}
		if(len > 0){
			tmp = calloc(sizeof(char), len  + strlen(".cgi") + 1);
			strncat(tmp, strCgi + strlen("cmd="), len);
			strcat(tmp, ".cgi");
			return tmp;
		}
	}
	else if(NULL != (strCgi = strstr(strUriSuff, "/cgi-bin/hi3510/"))){
		char * tmp = strdup(strCgi + strlen("/cgi-bin/hi3510/"));
		return tmp;
	}
	else if(NULL != (strCgi = strstr(strUriSuff, "/livestream"))){
		char *tmp = strdup(strCgi);
		return tmp;
	}
	else{
		return NULL;
	}
}



static void param_success(char* strContent, ssize_t const nLength, const char* text)
{
	snprintf(strContent, nLength, "[Success] %s"CRLF, text ? text : "");
}

static void param_error(char* strContent, ssize_t const nLength, const char* text)
{
	snprintf(strContent, nLength, "[Error] %s"CRLF, text ? text : "");
}
static int hichip_read_query2param(const char* strQuery, const char* key, AVal *ret)
{
	if(strQuery && key && strlen(key) > 0){
		char* key_with_equal = alloca(strlen(key) + 2);
		char* str_ptr = NULL;
		char *str_ch = NULL;
		ret->av_val = NULL;
		ret->av_len = 0;
		sprintf(key_with_equal, "%s=", key);
		str_ptr = strcasestr(strQuery, key_with_equal);
		if(str_ptr){
			ret->av_val = str_ptr + strlen(key_with_equal);
			str_ch = strchr(str_ptr, '&');
			if(str_ch){
				ret->av_len = str_ch - str_ptr - strlen(key_with_equal);
			}
			else{
				ret->av_len = strlen(ret->av_val);
			}

			return 0;
		}
	}
	return -1;
}


typedef struct 
{
	int max;
	int val;
} RatioValue_t;

static void hichip_ConvertParam(char *str, RatioValue_t *val)
{
	char value[128];
	char *max_value = NULL;
	strcpy(value,str);
	max_value = strstr(value, "/");
	if(max_value)
	{
		*max_value = 0;
		max_value += strlen("/");
		val->max = atoi(max_value);
		char *tmp = strstr(value, ",");
		if(tmp){
			val->val = atoi(tmp + strlen(","));
		}
	}
	else{
		val->val = -1;
		val->val = -1;
	}
}


int hichip_livemedia_cgi(const char* strQuery, char* strContent, ssize_t const nLength, int w, int h){
	uint32_t const session_id = rand();
	uint32_t const ssrc = session_id;
	snprintf(strContent, nLength,
		"Session: %d"CRLF
		"Cseq: 1"CRLF
		"m=video 96 H264/90000/%d/%d"CRLF
		"m=audio 97 G726/8000/1"CRLF
		"Transport: RTP/AVP/TCP;unicast;hisiinterleaved=0-1;ssrc=%x"CRLF
		CRLF,
		1840064387,
		w, h,
		1840064387);
	return 0;
}
int hichip_livemedia11_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	return hichip_livemedia_cgi(strQuery, strContent, nLength, 1280, 720);
}

int hichip_livemedia12_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	return hichip_livemedia_cgi(strQuery, strContent, nLength, 640, 360);
}

int hichip_identify_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{	
	SYSCONF_t *pConf = SYSCONF_dup();

	snprintf(strContent, nLength, "var productid=\"C1F0S9Z3N0P0L0\";"CRLF
										"var vendorid=\"Juan\";"CRLF);
    


	return 0;
}
int hichip_vdisplayattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){

	snprintf(strContent, nLength, "var powerfreq=\"50\";"CRLF);
	
	return 0;
}




// GET /cgi-bin/hi3510/ptzctrl.cgi?-step=0&-act=up&-speed=34 HTTP/1.1
int hichip_ptzctrl_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	AVal av_step, av_act, av_speed;
	int n_step = -1, n_act = -1, n_speed = -1;
	if(0 == hichip_read_query2param(strQuery, "step", &av_step)){
		n_step = atoi(AVAL_STRDUPA(av_step));
	}
	if(0 == hichip_read_query2param(strQuery, "act", &av_act)){
		n_act = atoi(AVAL_STRDUPA(av_act));
	}
	if(0 == hichip_read_query2param(strQuery, "speed", &av_speed)){
		n_speed = atoi(AVAL_STRDUPA(av_speed));
	}
	HICHIP_TRACE("PTZ control step=%d act=%d speed=%d",
		n_step, n_act, n_speed);

	char content[512] = {0};
	param_success(strContent, nLength, "ptz ok");
    

	
	return 0;
}

// GET /cgi-bin/hi3510/param.cgi?cmd=getosd&-chn=1&-region=1&cmd=getosd&-chn=1&-region=0& HTTP/1.1
int hichip_osd_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	int ret = 0;

	ret = snprintf(strContent, nLength, 
		"var x_1=\"%d\";" CRLF
		"var y_1=\"%d\";" CRLF
		"var w_1=\"%d\";" CRLF
		"var h_1=\"%d\";" CRLF
		"var show_1=\"%d\";" CRLF
		"name=\"%s\";" CRLF
		"var x_0=\"%d\";" CRLF
		"var y_0=\"%d\";" CRLF
		"var w_0=\"%d\";" CRLF
		"var h_0=\"%d\";" CRLF
		"var show_0=\"%d\";" CRLF,
		0, 0, 64, 32, 1, "CAM1",
		976, 0, 304, 32, 1);
    
	
	return 0;
}

int hichip_servertime_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	int ret = 0;
	AVal server_time;

	if(bOption){
		if(0 == hichip_read_query2param(strQuery, "time", &server_time)){
			//		int year, month, mday, hour, min, sec;
			struct tm cur_time;
			const char* const str_server_time = AVAL_STRDUPA(server_time);
			ret = sscanf(str_server_time, "%04d.%02d.%02d.%02d.%02d.%02d",
				&cur_time.tm_year, &cur_time.tm_mon, &cur_time.tm_mday, &cur_time.tm_hour, &cur_time.tm_min, &cur_time.tm_sec);

			time_t timet;
			struct timeval tv;
			cur_time.tm_year -= 1900;
			cur_time.tm_mon -= 1;
			timet = mktime(&cur_time);
			tv.tv_sec = timet;
			tv.tv_usec = 0;
			settimeofday(&tv,NULL);
			param_success(strContent, nLength, "set server time"CRLF);

			TTASK_syn_time(tv.tv_sec);
		}
	}
	else{
		time_t tNow;
		struct tm *tmNow;
		time(&tNow);
		SYSCONF_t *pConf = SYSCONF_dup();
		
		tmNow = localtime(&tNow);
		snprintf(strContent, nLength, "var time=\"%04d.%02d.%02d.%02d.%02d.%02d\";"CRLF
			"var timeZone=\"%d\";"CRLF
			"var dstmode=\"off\";"CRLF, tmNow->tm_year+1900, tmNow->tm_mon+1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, 
			tmNow->tm_sec, pConf->ipcam.date_time.time_zone.val);
		
	}
    
	return -1;
}



// param.cgi?cmd=getimageattr
int hichip_imageattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	RatioValue_t hue, brightness, saturation, contrast;
	AVal avHue, avBrightness, avSaturation, avContrast;
	AVal av_scene; // auto, indorr, outdoor
	AVal av_flip, av_mirror; // on, off
	char strQuery_decode[512] = {0};

	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));

	hichip_read_query2param(strQuery_decode, "hue", &avHue);
	hichip_ConvertParam(AVAL_STRDUPA(avHue), &hue);
	hichip_read_query2param(strQuery_decode, "brightness", &avBrightness);
	hichip_ConvertParam(AVAL_STRDUPA(avBrightness), &brightness);
	hichip_read_query2param(strQuery_decode, "saturation", &avSaturation);
	hichip_ConvertParam(AVAL_STRDUPA(avSaturation), &saturation);
	hichip_read_query2param(strQuery_decode, "contrast", &avContrast);
	hichip_ConvertParam(AVAL_STRDUPA(avContrast), &contrast);
	
	hichip_read_query2param(strQuery, "scene", &av_scene);
	hichip_read_query2param(strQuery, "flip", &av_flip);
	hichip_read_query2param(strQuery, "mirror", &av_mirror);
	
	

	SYSCONF_t* sysconf = SYSCONF_dup();
	if(bOption){
		// to set
		int n_hue = -1, n_brightness = -1, n_saturation = -1, n_contrast = -1, n_flip = -1, n_mirror = -1;
#if 1
		if(hue.val >= 0 && hue.val <= hue.max){
			HICHIP_TRACE("hue.val = %d, hue.max = %d, sysmax=%d\n", hue.val, hue.max, sysconf->ipcam.vin[0].hue.max);
			n_hue = hue.val* sysconf->ipcam.vin[0].hue.max / hue.max;
			if(n_hue <= sysconf->ipcam.vin[0].hue.max){
				sysconf->ipcam.vin[0].hue.val = (SYS_U16_t)n_hue;
				SENSOR_hue_set(n_hue);
			}
		}
		//brightness Maybe overflow!!!!  so get it a limit! 
		if(brightness.val >= 0 && brightness.val <= brightness.max && brightness.val <65535){
			n_brightness = brightness.val * sysconf->ipcam.vin[0].brightness.max / brightness.max;
			if(n_brightness <= sysconf->ipcam.vin[0].brightness.max){
				sysconf->ipcam.vin[0].brightness.val = n_brightness;
				//				printf("brightness:%d\r\n", n_brightness);
				//HICHIP_TRACE("ipcam n_brightness:%d\n", n_brightness);
				SENSOR_brightness_set(n_brightness);
			}
		}
		if(saturation.val >= 0 && saturation.val <= saturation.max){
			/*printf("saturation:%d/%d--%d/%d\r\n", 
			saturation.val, saturation.max, 
			sysconf->ipcam.vin[0].saturation.val, 
			sysconf->ipcam.vin[0].saturation.max);		*/	
			n_saturation = saturation.val * sysconf->ipcam.vin[0].saturation.max / saturation.max;
			if(n_saturation <= sysconf->ipcam.vin[0].saturation.max){
				sysconf->ipcam.vin[0].saturation.val = n_saturation;
				//HICHIP_TRACE("ipcam n_saturation:%d\n", n_saturation);
				SENSOR_saturation_set(n_saturation);
			}
		}
		if(contrast.val >= 0 && contrast.val <= contrast.max){
			/*printf("contrast:%d/%d--%d/%d\r\n", 
			contrast.val, contrast.max, 
			sysconf->ipcam.vin[0].contrast.val, 
			sysconf->ipcam.vin[0].contrast.max);*/
			n_contrast = contrast.val * sysconf->ipcam.vin[0].contrast.max /contrast.max;
			if(n_contrast <= sysconf->ipcam.vin[0].contrast.max){
				sysconf->ipcam.vin[0].contrast.val = n_contrast;
				//HICHIP_TRACE("ipcam n_contrast:%d\n", n_contrast);
				SENSOR_contrast_set(n_contrast);
			}
		}
		if(av_flip.av_len){
			n_flip = (0 == strcasecmp(AVAL_STRDUPA(av_flip), "on"));
			sysconf->ipcam.vin[0].flip = n_flip;
			if(!n_flip){
				//				SENSOR_mirror_flip(MODE_UNFLIP);
			}else{
				//				SENSOR_mirror_flip(MODE_FLIP);
			}
		}
		if(av_mirror.av_len){
			n_mirror = (0 == strcasecmp(AVAL_STRDUPA(av_mirror), "on"));
			sysconf->ipcam.vin[0].mirror = n_mirror;
			if(!n_flip){
				//				SENSOR_mirror_flip(MODE_UNMIRROR);
			}else{
				//				SENSOR_mirror_flip(MODE_MIRROR);
			}
		}
		if(n_hue != -1 && n_brightness != -1 && n_saturation != -1 
			&& n_contrast != -1 && n_flip != -1 && n_mirror != -1
			&& n_flip != -1 && n_mirror != -1){
				SYSCONF_save(sysconf);
		}
		param_success(strContent, nLength, "set image attr ok");
#endif

	}else{
		// to get

		snprintf(strContent, nLength, "var hue=\"%d\",\"%d\"/\"%d\";"CRLF
												"var brightness=\"%d\",\"%d\"/\"%d\";"CRLF
												"var saturation=\"%d\",\"%d\"/\"%d\";"CRLF
												"var contrast=\"%d\",\"%d\"/\"%d\";"CRLF
												"var scene=\"%s\";"CRLF
												"var flip=\"%s\";"CRLF
												"var mirror=\"%s\";"CRLF, 
												sysconf->ipcam.vin[0].hue.val,
												sysconf->ipcam.vin[0].hue.val,
												sysconf->ipcam.vin[0].hue.max,
												sysconf->ipcam.vin[0].brightness.val*256/sysconf->ipcam.vin[0].brightness.max,
												sysconf->ipcam.vin[0].brightness.val,
												sysconf->ipcam.vin[0].brightness.max,
												sysconf->ipcam.vin[0].saturation.val*256/sysconf->ipcam.vin[0].saturation.max,
												sysconf->ipcam.vin[0].saturation.val,
												sysconf->ipcam.vin[0].saturation.max,
												sysconf->ipcam.vin[0].contrast.val*8/sysconf->ipcam.vin[0].contrast.max,
												sysconf->ipcam.vin[0].contrast.val,
												sysconf->ipcam.vin[0].contrast.max,
												"auto",
												sysconf->ipcam.vin[0].flip ? "on" : "off",
												sysconf->ipcam.vin[0].mirror ? "on" : "off");
	}
    
	return 0;
}



int hichip_videoattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal videomode, vinorm;
	SYSCONF_t *pSysConf = SYSCONF_dup();
	int nVideoMode;

	
	if(bOption){
		hichip_read_query2param(strQuery, "videomode", &videomode);
		hichip_read_query2param(strQuery, "vinorm", &vinorm);
		int main_stream;
		int sub_stream;
		if(videomode.av_len){
			switch(atoi(AVAL_STRDUPA(videomode))){
				case 18:
					main_stream = SYS_VIN_SIZE_VGA; sub_stream = SYS_VIN_SIZE_QVGA;
					break;
				case 19:
					main_stream = SYS_VIN_SIZE_D1; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				case 21:
					main_stream = sub_stream = SYS_VIN_SIZE_QVGA;
					break;
				case 22:
					main_stream = SYS_VIN_SIZE_CIF; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				case 24:
					main_stream = SYS_VIN_SIZE_QCIF;  sub_stream = SYS_VIN_SIZE_CIF;
					break;
				case 25:
					main_stream = SYS_VIN_SIZE_QCIF; sub_stream = SYS_VIN_SIZE_QCIF;
					break;
				default:
					main_stream = SYS_VIN_SIZE_720P; sub_stream = SYS_VIN_SIZE_720P;
					break;
			}
			pSysConf->ipcam.vin[0].enc_h264[0].stream[0].size.val = main_stream;
			pSysConf->ipcam.vin[0].enc_h264[0].stream[1].size.val = sub_stream;
			SYSCONF_save(pSysConf);
			param_success(strContent, nLength, "set video attr ok");
		}
	}
	else{
		switch (pSysConf->ipcam.vin[0].enc_h264[0].stream[0].size.val | pSysConf->ipcam.vin[0].enc_h264[0].stream[1].size.val)
		{
		case SYS_VIN_SIZE_VGA | SYS_VIN_SIZE_QVGA:
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_D1:
			nVideoMode = 18;
			break;
		case SYS_VIN_SIZE_D1 | SYS_VIN_SIZE_QCIF:
			nVideoMode = 19;
			break;
		case SYS_VIN_SIZE_QVGA | SYS_VIN_SIZE_QVGA:
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_CIF:
			nVideoMode = 21;
			break;
		case SYS_VIN_SIZE_CIF | SYS_VIN_SIZE_QCIF:
			nVideoMode = 22;
			break;
			// 		case SYS_VIN_SIZE_QCIF | SYS_VIN_SIZE_CIF:
			// 			nVideoMode = 24;
			// 			break;
		case SYS_VIN_SIZE_QCIF | SYS_VIN_SIZE_QCIF:
			nVideoMode = 25;
			break;
		default:
			nVideoMode = 31;
		}
		snprintf(strContent, nLength, "var videomode=\"%d\";"CRLF, nVideoMode);
			
	}
    
	return 0;
}

int hichip_vencattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal chn, bps, fps, brmode, imagegrade, gop;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	hichip_read_query2param(strQuery, "chn", &chn);
	int stream_ch = atoi(AVAL_STRDUPA(chn)) - 11;

	if(bOption){
		hichip_read_query2param(strQuery, "bps", &bps);
		hichip_read_query2param(strQuery, "fps", &fps);
		hichip_read_query2param(strQuery, "brmode", &brmode);
		hichip_read_query2param(strQuery, "imagegrade", &imagegrade);
		hichip_read_query2param(strQuery, "gop", &gop);
		if(bps.av_len){
			pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].bps = atoi(AVAL_STRDUPA(bps));
		}
		if(fps.av_len){
			pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].fps = atoi(AVAL_STRDUPA(fps));
		}
		if(brmode.av_len){
			int mode;
			switch(atoi(AVAL_STRDUPA(brmode))){
				case 0:
					mode = SYS_VENC_H264_MODE_CBR;
					break;
				case 1:
					mode = SYS_VENC_H264_MODE_VBR;
					break;
				default:
					mode = SYS_VENC_H264_MODE_VBR;
					break;
			}
			pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].mode.val = mode;
		}
		if(imagegrade.av_len){
			pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].quality.val = atoi(AVAL_STRDUPA(imagegrade));
		}
		if(gop.av_len){
			pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].gop = atoi(AVAL_STRDUPA(gop));
		}
		SYSCONF_save(pSysConf);
		param_success(strContent, nLength, "set venc attr success");		
	
	}
	else{
		int mode;
		int width, height;
		HICHIP_TRACE("stream_ch:%d\n", stream_ch);
		SYS_VIN_SIZE_WIDTH_HEIGHT(pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].size, width, height) ;
		switch(pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].mode.val){
			case SYS_VENC_H264_MODE_CBR:
				mode = 0;
				break;
			case SYS_VENC_H264_MODE_VBR:
				mode = 1;
				break;
			default:
				break;
		}
		snprintf(strContent, nLength, "var bps_%d=\"%d\";"CRLF
															"var fps_%d=\"%d\";"CRLF
															"var gop_%d=\"%d\";"CRLF
															"var brmode_%d=\"%d\";"CRLF
															"var imagegrade_%d=\"%d\";"CRLF
															"var width_%d=\"%d\";"CRLF
															"var height_%d=\"%d\";"CRLF, stream_ch+1, 
															pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].bps, 
															stream_ch+1, pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].fps, 
															stream_ch+1, pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].gop, 
															stream_ch+1, mode, 
															stream_ch+1, 
															1 //pSysConf->ipcam.vin[0].enc_h264[stream_ch].stream[0].quality.val
															, stream_ch+1, width
															, stream_ch+1, height);
// 		sprintf(strContent, "HTTP/1.0 200 OK\r\n"
// 			"Server: HiIpcam\r\n"
// 			"Cache-Control: no-cache\r\n"
// 			"Content-Type:text/html\r\n"
// 			"Connection: close\r\n"
// 			"Content-Length: %d\r\n\r\n", strlen(item));

	}
	
	return 0;
}

int hichip_aencattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] aencattr ok");
	
	return 0;
}

int hichip_audioinvolume_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] audioinvolume ok");
	
	return 0;
}

int hichip_overlayattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption)
{
	AVal av_region, av_show, av_name;
	int n_region = 0;
	int n_show = 0;
	char* str_name = NULL;
	SYSCONF_t* sysconf = SYSCONF_dup();

	if(bOption){
		if(0 == hichip_read_query2param(strQuery, "region", &av_region)){
			n_region = atoi(AVAL_STRDUPA(av_region));
		}
		if(0 == hichip_read_query2param(strQuery, "show", &av_show)){
			n_show = atoi(AVAL_STRDUPA(av_show));
		}
		if(0 == n_region){
			APP_OVERLAY_show_clock(0, 0, n_show);
		}
		else{
			APP_OVERLAY_show_title(0, 0, n_show);
		}
		if(0 == hichip_read_query2param(strQuery, "name", &av_name)){
			str_name = AVAL_STRDUPA(av_name);HICHIP_TRACE("setoverlay_channel\n");
			if(strlen(str_name) <= sizeof(sysconf->ipcam.vin[0].channel_name)){
				strcpy(sysconf->ipcam.vin[0].channel_name, str_name);
				APP_OVERLAY_set_title(0, str_name);
				APP_OVERLAY_update_title(0, 0, NULL, NULL);
				SYSCONF_save(sysconf);
			}
		}
		param_success(strContent, nLength, " set overlay attr ok");
	}
	else{
		snprintf(strContent, nLength, "var show=\"1\";"CRLF"name=\"%s\";"CRLF, sysconf->ipcam.vin[0].channel_name);
	}
    
	return 0;
}


int hichip_netattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pSysConf = SYSCONF_dup();
	AVal dhcpflag, ip, netmask, gateway, fdnsip, sdnsip;

	if(bOption){
		hichip_read_query2param(strQuery, "dhcp", &dhcpflag);
		hichip_read_query2param(strQuery, "ipaddr", &ip);
		hichip_read_query2param(strQuery, "netmask", &netmask);
		hichip_read_query2param(strQuery, "gateway", &gateway);
		hichip_read_query2param(strQuery, "fdnsip", &fdnsip);
		hichip_read_query2param(strQuery, "sdnsip", &sdnsip);
		if(dhcpflag.av_len){
			pSysConf->ipcam.network.lan.dhcp = (0 == strcasecmp(AVAL_STRDUPA(dhcpflag), "on"));
		}
		if(ip.av_len){
			pSysConf->ipcam.network.lan.static_ip.s_addr = inet_addr(AVAL_STRDUPA(ip));
		}
		if(netmask.av_len){
			pSysConf->ipcam.network.lan.static_netmask.s_addr = inet_addr(AVAL_STRDUPA(netmask));
		}
		if(gateway.av_len){
			pSysConf->ipcam.network.lan.static_gateway.s_addr = inet_addr(AVAL_STRDUPA(gateway));
		}
		if(fdnsip.av_len){
			pSysConf->ipcam.network.lan.static_preferred_dns.s_addr = inet_addr(AVAL_STRDUPA(fdnsip));
		}
		if(sdnsip.av_len){
			pSysConf->ipcam.network.lan.static_alternate_dns.s_addr = inet_addr(AVAL_STRDUPA(sdnsip));
		}
		SYSCONF_save(pSysConf);
		param_success(strContent, nLength, "set net attr success");
	}
	else{
		char mac[18] = {0};
		sprintf(mac, "%x:%x:%x:%x:%x:%x", pSysConf->ipcam.network.mac.s[0], 
											pSysConf->ipcam.network.mac.s[1],
											pSysConf->ipcam.network.mac.s[2],
											pSysConf->ipcam.network.mac.s[3],
											pSysConf->ipcam.network.mac.s[4],
											pSysConf->ipcam.network.mac.s[5]);
		char strIP[16] = {0}; snprintf(strIP, sizeof(strIP), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_ip.in_addr));
		char strNetmask[16] = {0}; snprintf(strNetmask, sizeof(strNetmask), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_netmask.in_addr));
		char strGateway[16] = {0}; snprintf(strGateway, sizeof(strGateway), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_gateway.in_addr));
		char strFdns[16] = {0}; snprintf(strFdns, sizeof(strFdns), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_preferred_dns.in_addr));
		char strSdns[16] = {0}; snprintf(strSdns, sizeof(strSdns), "%s", inet_ntoa(pSysConf->ipcam.network.lan.static_alternate_dns.in_addr));
		
		snprintf(strContent, nLength, "var dhcpflag=\"%s\";"CRLF
										"var ip=\"%s\";"CRLF
										"var netmask=\"%s\";"CRLF
										"var gateway=\"%s\";"CRLF
										"var fdnsip=\"%s\";"CRLF
										"var sdnsip=\"%s\";"CRLF
										"var macaddress=\"%s\";"CRLF, 
										pSysConf->ipcam.network.lan.dhcp ? "on" : "off",
										strIP,
										strNetmask,
										strGateway,
										strFdns,
										strSdns, mac);
	}
    
	return 0;
}

int hichip_httpport_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal httpport;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "httpport", &httpport);
		pSysConf->ipcam.network.lan.port[0].value = atoi(AVAL_STRDUPA(httpport));
		SYSCONF_save(pSysConf);
		exit(0);
		param_success(strContent, nLength, "set httpport ok");
	}
	else{
		snprintf(strContent, nLength, "var httpport=\"%d\";"CRLF, pSysConf->ipcam.network.lan.port[0].value);
	}
    
	return 0;
}

int hichip_rtspport_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal rtspport;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "rtspport", &rtspport);
		pSysConf->ipcam.network.lan.port[0].value = atoi(AVAL_STRDUPA(rtspport));
		SYSCONF_save(pSysConf);
		exit(0);
		param_success(strContent, nLength, "set rtspport ok");
	}
	else{
		snprintf(strContent, nLength, "var rtspport=\"%d\";"CRLF, pSysConf->ipcam.network.lan.port[0].value);
	}
    
	return 0;
}

int hichip_infrared_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal infrared;
	SYSCONF_t *pConf = SYSCONF_dup();


	if(bOption){
		hichip_read_query2param(strQuery, "infraredstat", &infrared);
		int mode = 0;
		if(AVSTRMATCH(&infrared, "auto")){
			mode = ISP_IRCUT_MODE_AUTO;
		}
		else if(AVSTRCASEMATCH(&infrared, "open")){
			mode = ISP_IRCUT_MODE_NIGHT;
		}
		else if(AVSTRCASEMATCH(&infrared, "close")){
			mode = ISP_IRCUT_MODE_DAYLIGHT;
		}
		pConf->ipcam.isp.day_night_mode.ircut_mode = mode;
		SENSOR_ircut_mode_set(mode);
		SYSCONF_save(pConf);
		param_success(strContent, nLength, "set infrared ok!");
	}
	else{
		char mode[8] = {0};
		switch(pConf->ipcam.isp.day_night_mode.ircut_mode){
			case ISP_IRCUT_MODE_AUTO:
				strcat(mode, "auto"); break;
			case ISP_IRCUT_MODE_NIGHT:
				strcat(mode, "open"); break;
			case ISP_IRCUT_MODE_DAYLIGHT:
				strcat(mode, "close"); break;
			default:
				break;
		}
		snprintf(strContent, nLength, "var infraredstat=\"%s\";"CRLF, mode);
	}
    
    return 0;
}

int hichip_upnpattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal upnp;
	SYSCONF_t *pSysConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "upm_enable", &upnp);
		pSysConf->ipcam.network.lan.upnp = atoi(AVAL_STRDUPA(upnp));
		SYSCONF_save(pSysConf);
		param_success(strContent, nLength, "set upnp_enable ok");
	}
	else{
		snprintf(strContent, nLength, "var upm_enable=\"%d\";"CRLF, pSysConf->ipcam.network.lan.upnp);
	}
    
	return 0;
}

int hichip_3thddnsattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal enable, service, uname, passwd, domain;
	SYSCONF_t *pConf = SYSCONF_dup();

	if(bOption){
		hichip_read_query2param(strQuery, "d3th_enable", &enable);
		hichip_read_query2param(strQuery, "d3th_service", &service);
		hichip_read_query2param(strQuery, "d3th_uname", &uname);
		hichip_read_query2param(strQuery, "d3th_passwd", &passwd);
		hichip_read_query2param(strQuery, "d3th_domain", &domain);
		pConf->ipcam.network.ddns.enable = atoi(AVAL_STRDUPA(enable));
		snprintf(pConf->ipcam.network.ddns.username, sizeof(pConf->ipcam.network.ddns.username), AVAL_STRDUPA(uname));
		snprintf(pConf->ipcam.network.ddns.password, sizeof(pConf->ipcam.network.ddns.password), AVAL_STRDUPA(passwd));
		snprintf(pConf->ipcam.network.ddns.url, sizeof(pConf->ipcam.network.ddns.url), AVAL_STRDUPA(domain));
		SYSCONF_save(pConf);
		param_success(strContent, nLength, "set ddns ok");
	}
	else{
		snprintf(strContent, nLength, "var d3th_enable=\"%d\";"CRLF
											"var d3th_service=\"%d\";"CRLF
											"var d3th_uname=\"%s\";"CRLF
											"var d3th_passwd=\"%s\";"CRLF
											"var d3th_domain=\"%s\";"CRLF, 
											pConf->ipcam.network.ddns.enable,
											pConf->ipcam.network.ddns.provider,
											pConf->ipcam.network.ddns.username,
											pConf->ipcam.network.ddns.password,
											pConf->ipcam.network.ddns.url);
	}
    
	return 0;
}

int hichip_serverinfo_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){


	SYSCONF_t *pConf = SYSCONF_dup();
	snprintf(strContent, nLength, "var model=\"%s\";"CRLF
										"var hardVersion=\"%s\";"CRLF
										"var softVersion=\"%s\";"CRLF
										"var name=\"%s\";"CRLF,
										pConf->ipcam.info.device_model,
										pConf->ipcam.info.hardware_version,
										pConf->ipcam.info.software_version,
										pConf->ipcam.info.device_name);
    
	return 0;
}

int hichip_mdattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avSensitivity, avName, avX, avY, avWidth, avHeight;
	SYSCONF_t *pConf = SYSCONF_dup();

	char strQuery_decode[512] = {0};
	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));
	if(bOption){
		hichip_read_query2param(strQuery_decode, "enable", &avEnable);
		hichip_read_query2param(strQuery_decode, "s", &avSensitivity);
		hichip_read_query2param(strQuery_decode, "name", &avName);
		hichip_read_query2param(strQuery_decode, "x", &avX);
		hichip_read_query2param(strQuery_decode, "y", &avY);
		hichip_read_query2param(strQuery_decode, "w", &avWidth);
		hichip_read_query2param(strQuery_decode, "h", &avHeight);
		RatioValue_t ratioX, ratioY, ratioWdith, ratioHeight;
		hichip_ConvertParam(AVAL_STRDUPA(avX), &ratioX);
		hichip_ConvertParam(AVAL_STRDUPA(avY), &ratioY);
		hichip_ConvertParam(AVAL_STRDUPA(avWidth), &ratioWdith);
		hichip_ConvertParam(AVAL_STRDUPA(avHeight), &ratioHeight);

		if(1 == atoi(AVAL_STRDUPA(avEnable))){
			float x = (float)ratioX.val / 1280.0;
			float y = (float)ratioY.val / 720.0;
			float Width = (float)ratioWdith.val / 1280.0;
			float Height = (float)ratioHeight.val / 720.0;
			float Threshold = 1.0 / (float)atoi(AVAL_STRDUPA(avSensitivity));

			HICHIP_TRACE("x:%f, y:%f, w:%f, h:%f, s:%d\n", x, y, Width, Height, atoi(AVAL_STRDUPA(avSensitivity)));
			//APP_MD_clear_mask(0);
			//APP_MD_add_rect_mask(0, x, y, Width, Height);
			//if(0 == sdk_vin->check_md_rect(0, AVAL_STRDUPA(avName), NULL, NULL, NULL, NULL, NULL)){
				if(0 != sdk_vin->add_md_rect(0, AVAL_STRDUPA(avName), x, y, Width, Height, Threshold)){
					param_error(strContent, nLength, "set mdattr error");
				}
			//}

		}
		param_success(strContent, nLength, "set mdattr ok");
	}
	else{
		char item[256] = {0};
		int i;
		for(i = 0; i < 4; ++i){
			char strName[5] = {0};
			snprintf(strName, sizeof(strName), "%d", i + 1);
			float xRatio, yRatio, wRatio, hRatio, Threshold;
			int nX, nY, nWidth, nHeight;
			int nSensitivity;
			if(1 == sdk_vin->check_md_rect(0, strName, &xRatio, &yRatio, &wRatio, &hRatio, &Threshold)){
				nSensitivity = 1.0 / Threshold;
				nX = xRatio * 1280;
				nY = yRatio * 720;
				nWidth = wRatio * 1280;
				nHeight = hRatio * 720;
				memset(item, 0, sizeof(item));
				snprintf(item, sizeof(item), "var m%d_enable=\"%d\";"CRLF
											"var m%d_x=\"%d,%d/1280\";"CRLF
											"var m%d_y=\"%d,%d/720\";"CRLF
											"var m%d_w=\"%d,%d/1280\";"CRLF
											"var m%d_h=\"%d,%d/720\";"CRLF
											"var m%d_sensitivity=\"%d\";"CRLF,
											i + 1, 1,
											i + 1, nX, nX,
											i + 1, nY, nY,
											i + 1, nWidth, nWidth, 
											i + 1, nHeight, nHeight,
											i + 1, nSensitivity);
				strncat(strContent, item, nLength - strlen(strContent));					
				HICHIP_TRACE("name:%s, item:%s:\n", strName, item);
			}
			else{
				HICHIP_TRACE("check md failed\n");
			}
		}
		if(0 == strlen(strContent)){
			strncat(strContent, "no motion detect", nLength);
		}
	}
	
    
    return 0;
}
int hichip_ioattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avFlag;

	if(bOption){
		hichip_read_query2param(strQuery, "io_enable", &avEnable);
		hichip_read_query2param(strQuery, "io_flag", &avFlag);

		param_success(strContent, nLength, "set ioattr ok");
	}
	else{
		param_success(strContent, nLength, "get ioattr ok");
		/*snprintf(content, , "var io_enable=\"%d\";"CRLF
											"var io_flag=\"%d\";"CRLF,
											, , );
*/
	}
	
	return 0;
}
int hichip_mdalarm_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avAname, avEmailSwitch, avEmailSnap, avSnapSwitch, avRecordSwitch, avFtpSwitch, avRelaySwitch;

	if(bOption){
		hichip_read_query2param(strQuery, "aname", &avAname);
		hichip_read_query2param(strQuery, "md_email_switch", &avEmailSwitch);
		hichip_read_query2param(strQuery, "md_emailsnap_switch", &avEmailSnap);
		hichip_read_query2param(strQuery, "md_snap_switch", &avSnapSwitch);
		hichip_read_query2param(strQuery, "md_record_switch", &avRecordSwitch);
		hichip_read_query2param(strQuery, "md_ftprec_switch", &avFtpSwitch);
		hichip_read_query2param(strQuery, "md_relay_switch", &avRelaySwitch);

		param_success(strContent , nLength, "set mdalarm ok");
	} 
	else{
		snprintf(strContent, nLength, "get mdalarm ok");
	}
	
	return 0;
}
int hichip_schedule_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "no schedule");
	
	return 0;
}
int hichip_devtype_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	uint32_t type = 0x0007 << 8;
	snprintf(strContent, nLength, "var devtype=\"%04x\";", type);
	
	return 0;
}
int hichip_coverattr_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	AVal avEnable, avName, avX, avY, avWidth, avHeight, avColor;
	SYSCONF_t *pConf = SYSCONF_dup();
	char strQuery_decode[512] = {0};
	http_url_decode(strQuery, strlen(strQuery), strQuery_decode, sizeof(strQuery_decode));
	if(bOption){
		hichip_read_query2param(strQuery_decode, "enable", &avEnable);
		hichip_read_query2param(strQuery_decode, "name", &avName);
		hichip_read_query2param(strQuery_decode, "x", &avX);
		hichip_read_query2param(strQuery_decode, "y", &avY);
		hichip_read_query2param(strQuery_decode, "w", &avWidth);
		hichip_read_query2param(strQuery_decode, "h", &avHeight);
		hichip_read_query2param(strQuery_decode, "color", &avColor);
		RatioValue_t ratioX, ratioY, ratioWdith, ratioHeight;
		hichip_ConvertParam(AVAL_STRDUPA(avX), &ratioX);
		hichip_ConvertParam(AVAL_STRDUPA(avY), &ratioY);
		hichip_ConvertParam(AVAL_STRDUPA(avWidth), &ratioWdith);
		hichip_ConvertParam(AVAL_STRDUPA(avHeight), &ratioHeight);
		if(1 == atoi(AVAL_STRDUPA(avEnable))){
			float x = (float)ratioX.val / 1280.0;
			float y = (float)ratioY.val / 720.0;
			float Width = (float)ratioWdith.val / 1280.0;
			float Height = (float)ratioHeight.val / 720.0;
			//char *pColor = strlwr(AVAL_STRDUPA(avColor));
			uint32_t nColor = 0;
			sscanf(AVAL_STRDUPA(avColor), "%06x", &nColor);
			HICHIP_TRACE("%f, %f, %f, %f, color#%s:%06x\n", x, y, Width, Height, AVAL_STRDUPA(avColor), nColor);
			int nRet  = 0;
			if(0 == sdk_vin->check_cover(0, AVAL_STRDUPA(avName), NULL, NULL, NULL, NULL, NULL)){
				nRet = sdk_vin->add_cover(0, AVAL_STRDUPA(avName), x, y, Width, Height, nColor, 0);
			}
			else{
				nRet = sdk_vin->update_cover(0, AVAL_STRDUPA(avName), x, y, Width, Height, nColor, kSDK_VIN_COVER_UPDATE_FLAG_ALL);
			}
			if(0 != nRet){
				param_error(strContent, nLength, "set coverattr error");
			}

		}
		param_success(strContent, nLength, "set coverattr ok");
	}
	else{
		char item[256] = {0};
		int i;
		for(i = 0; i < 4; ++i){
			char strName[5] = {0};
			snprintf(strName, sizeof(strName), "%d", i + 1);
			float xRatio, yRatio, wRatio, hRatio;
			int nX, nY, nWidth, nHeight;
			uint32_t nColor;
			if(1 == sdk_vin->check_cover(0, strName, &xRatio, &yRatio, &wRatio, &hRatio, &nColor)){
				nX = xRatio * 1280;
				nY = yRatio * 720;
				nWidth = wRatio * 1280;
				nHeight = hRatio * 720;
				memset(item, 0, sizeof(item));
				snprintf(item, sizeof(item), "var c%d_enable=\"%d\";"CRLF
											"var c%d_x=\"%d,%d/1280\";"CRLF
											"var c%d_y=\"%d,%d/720\";"CRLF
											"var c%d_w=\"%d,%d/1280\";"CRLF
											"var c%d_h=\"%d,%d/720\";"CRLF
											"var c%d_color=\"%06x\";"CRLF,
											i + 1, 1,
											i + 1, nX, nX,
											i + 1, nY, nY,
											i + 1, nWidth, nWidth, 
											i + 1, nHeight, nHeight,
											i + 1, nColor);
				strncat(strContent, item, nLength - strlen(strContent));					
				HICHIP_TRACE("name:%s, item:%s:\n", strName, item);
			}
		}
		if(0 == strlen(strContent)){
			strncat(strContent, "no cover", nLength);
		}
	}
    
    return 0;
}
int hichip_preset_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] preset ok");
	
	return 0;
}
int hichip_ptzcomattr(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzcomattr ok");
	
    return 0;
}
int hichip_ptzup_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzup ok");
	
    return 0;
}
int hichip_ptzdown_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzdown ok");
	
    return 0;
}
int hichip_ptzleft_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
 	snprintf(strContent, nLength, "[Success] ptzleft ok");
	
    return 0;
}
int hichip_ptzright_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzright ok");
	
    return 0;
}
int hichip_ptzzoomin_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzzoomin ok");
	
    return 0;
}
int hichip_ptzzoomout_cgi(const char *strQuery, char *strContent, int nLength, bool bOption){
	snprintf(strContent, nLength, "[Success] ptzzoomout ok");
	
    return 0;
}


void HICHIP_http_init(){
	g_htHiChip = CGI_hashAdd(g_htHiChip, "identify.cgi", hichip_identify_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "vdisplayattr.cgi", hichip_vdisplayattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "/livestream/11", hichip_livemedia11_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "/livestream/12", hichip_livemedia12_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzctrl.cgi", hichip_ptzctrl_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "servertime.cgi", hichip_servertime_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "imageattr.cgi", hichip_imageattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "videoattr.cgi", hichip_videoattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "vencattr.cgi", hichip_vencattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "aencattr.cgi", hichip_aencattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "audioinvolume.cgi", hichip_audioinvolume_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "overlayattr.cgi", hichip_overlayattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "netattr.cgi", hichip_netattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "httpport.cgi", hichip_httpport_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "rtspport.cgi", hichip_rtspport_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "infrared.cgi", hichip_infrared_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "upnpattr.cgi", hichip_upnpattr_cgi);	
	g_htHiChip = CGI_hashAdd(g_htHiChip, "3thddnsattr.cgi", hichip_3thddnsattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "serverinfo.cgi", hichip_serverinfo_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "mdattr.cgi", hichip_mdattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ioattr.cgi", hichip_ioattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "mdalarm.cgi", hichip_mdalarm_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "schedule.cgi", hichip_schedule_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "devtype.cgi", hichip_devtype_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "coverattr.cgi", hichip_coverattr_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "preset.cgi", hichip_preset_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzcomattr.cgi", hichip_ptzcomattr);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzup.cgi", hichip_ptzup_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzdown.cgi", hichip_ptzdown_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzleft.cgi", hichip_ptzleft_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzright.cgi", hichip_ptzright_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzzoomin.cgi", hichip_ptzzoomin_cgi);
	g_htHiChip = CGI_hashAdd(g_htHiChip, "ptzzoomout.cgi", hichip_ptzzoomout_cgi);
}

void HICHIP_http_destroy(){
	CGI_hashDestroy(g_htHiChip);
}
int HICHIP_http_cgi(HTTPD_SESSION_t* session)
{
	session->keep_alive = 0;
	int ret = 0;
	char body[2048] = {0};
	char *dstCmd = NULL;
	char *strCgi = hichip_uri_suffix2cgi(session->request_line.uri_suffix, session->request_line.uri_query_string);
	HICHIP_TRACE("suffix:%s, param:%s\n", AVAL_STRDUPA(session->request_line.uri_suffix),AVAL_STRDUPA(session->request_line.uri_query_string));
	if(strCgi){
		int set_get = 2;
		HICHIP_PARAM_SUFFIX(strCgi, dstCmd, set_get);
		if(0 == strcmp(strCgi, "preset.cgi")){
			dstCmd = strCgi;
		}
		CGI_hashtable *pHashItem = CGI_hashFind(g_htHiChip, dstCmd);
		if(pHashItem){
			pHashItem->pCgiFunc(AVAL_STRDUPA(session->request_line.uri_query_string), body, ARRAY_ITEM(body), set_get);
		}
		else{
			HICHIP_TRACE("cgi not found!\n");
		}
		
	}
	else{
		HICHIP_TRACE("get cgi failed\n");
	}

	// send the response content
	if(strCgi && strlen(body) > 0){
		char content[2048*2] = {0};
		int bLiveMedia = (NULL != strstr(dstCmd, "/livestream")) ? 1 : 0; 
		HTTP_HEADER_t* http_header = NULL;
		SYSCONF_t *pConf = SYSCONF_dup();
		if(1 == bLiveMedia){
			http_header = http_response_header_new("1.1", 200, NULL);                      
			http_header->add_tag_text(http_header, "Host", inet_ntoa(pConf->ipcam.network.lan.static_ip.in_addr));
			http_header->add_tag_text(http_header, "Connection", "Keep-Alive");		           
			http_header->add_tag_text(http_header, "Server", HICHIP_SERVER);
			http_header->add_tag_text(http_header, "Cache-Control", "no-cache");
			http_header->add_tag_text(http_header, "Accept-Ranges", "Bytes");
			http_header->add_tag_text(http_header, "Content-Type", "application/octet-stream");
			http_header->add_tag_text(http_header, "Connection", "close");
		}
		else{
			http_header = http_response_header_new("1.1", 200, NULL);                      
			http_header->add_tag_text(http_header, "Server", HICHIP_SERVER);
			http_header->add_tag_text(http_header, "Cache-Control", "no-cache");  
			http_header->add_tag_text(http_header, "Content-Type", "text/html");
			http_header->add_tag_text(http_header, "Connection", "close");      
			http_header->add_tag_int(http_header, "Content-Length", strlen(body));     
		}
		http_header->to_text(http_header, content, sizeof(content));                  	   
		http_response_header_free(http_header);                                        
		http_header = NULL;                                                            
		strncat(content, body, strlen(body));                     		   		
		ret = send(session->sock, content, strlen(content), 0);
		if(ret < 0){
			HICHIP_TRACE("Response  error : %s", strerror(errno));
			return -1;
		}
		if(1 == bLiveMedia){
			const char h264file[10] = {0};
			if(strstr(dstCmd, "11")){
				snprintf(h264file, sizeof(h264file), "720p.264");
			}
			else{
				snprintf(h264file, sizeof(h264file), "360p.264");
			}
			int mediabufID = 0;
			int mediabufSpeed = 0;
			uint32_t seqno = 0;
			lpMEDIABUF_USER user = NULL;
			mediabufID = MEDIABUF_lookup_byname(h264file);
			if(mediabufID < 0){
				HICHIP_TRACE("mediabuf not found\n");
				*session->trigger = false;
				return -1;
			}
			mediabufSpeed = MEDIABUF_in_speed(mediabufID);
			user = MEDIABUF_attach(mediabufID);
			if(user){
				unsigned int ssrc = rand();
				unsigned int seqno = rand();
				int nCount = 0;
				int flags = 1;
				/*ret = setsockopt(session->sock, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags));
				if(ret < 0){
					//fixed me
				}*/
				while(*session->trigger){
					bool mediabuf_wait = true;
					if(0 == MEDIABUF_out_lock(user)){
						void* data_ptr = 0;
						ssize_t data_size = 0;
						if(0 == MEDIABUF_out(user, &data_ptr, NULL, &data_size)){
							struct HICHIP_RTSP_ITLEAVED_HDR frame_header;
							struct HICHIP_RTP_HDR* const rtp_header = &frame_header.rtpHead;
							const lpSDK_ENC_BUF_ATTR const attr = (lpSDK_ENC_BUF_ATTR)data_ptr;
							const uint8_t* const frame = (uint8_t*)(attr + 1);
							ssize_t const frame_size = attr->data_sz;
							STRUCT_ZERO(frame_header);
							frame_header.dollar = '$';
							frame_header.channelid = 0;
							frame_header.resv = 0;
							frame_header.payloadLen = htonl(frame_size + sizeof(frame_header.rtpHead));
							rtp_header->cc = 0;
							rtp_header->x = 0;
							rtp_header->p = 0;
							rtp_header->version = 2;
							rtp_header->pt = 96;
							rtp_header->marker = 0;
							rtp_header->seqno = seqno++;
							rtp_header->ts = htonl(attr->timestamp_us);
							rtp_header->ssrc = ssrc;
							//ret = send(session->sock, &frame_header, sizeof(frame_header), 0);
							//ret = send(session->sock, frame, frame_size, 0);
							char head[] = { 0x00, 0x00, 0x00, 0x01, 0x67};
							 char headSub[] = { 0x00, 0x00, 0x00, 0x01 };
							 char *pCur = frame;
							 char *pNext = NULL;
							 int nSingleSliceLen = 0;
							 int nFrameLast = frame_size;
							 for(; NULL != pCur;){
							 	pNext = memmem(pCur, nFrameLast, headSub, sizeof(headSub));
							 	if(pNext == pCur){
							 		pNext = memmem(pCur + sizeof(headSub), nFrameLast - sizeof(headSub), headSub, sizeof(headSub));
							 		if(NULL != pNext){
							 			nSingleSliceLen = pNext - pCur;
							 			frame_header.payloadLen = htonl(nSingleSliceLen + sizeof(frame_header.rtpHead));rtp_header->seqno = seqno++;
							 			ret = send(session->sock, &frame_header, sizeof(frame_header), 0);
							 			ret = send(session->sock, pCur, nSingleSliceLen, 0);
							 			pCur = pNext;
							 			nFrameLast -= nSingleSliceLen;
							 		}
							 		else{
							 			frame_header.payloadLen = htonl(nFrameLast+ sizeof(frame_header.rtpHead));rtp_header->seqno = seqno++;
							 			ret = send(session->sock, &frame_header, sizeof(frame_header), 0);
							 			ret = send(session->sock, pCur, nFrameLast, 0);
							 			pCur = NULL;
							 		}
							 	}
							}


							if(ret < 0){
								HICHIP_TRACE("ret = %d err = %s", ret, strerror(errno));
								HICHIP_TRACE("sockfd:%d, frame_size:%d\n", session->sock, frame_size);
								break;
							}
							mediabuf_wait = false;
						}
						MEDIABUF_out_unlock(user);
					}
					if(mediabuf_wait){
						usleep(mediabufSpeed);
					}
				}
				MEDIABUF_detach(user);
				user = NULL;
			}

		}
		*session->trigger = false;
		free(strCgi);
		strCgi = NULL;
		return 0;
	}
	return -1;
}




