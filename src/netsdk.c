#include <string.h>
#include "netsdk.h"
#include "sdk/sdk_api.h"
#include "media_buf.h"
#include "generic.h"
#include "http_common.h"
#include "sensor.h"
#include "sysconf.h"
#include "timertask.h"
#include "sdk/sdk_vin.h"
#include "app_motion_detect.h"
#include "sdk_isp.h" // in hi_isp_tmp
#include "cgi_hash.h"
#include "app_overlay.h"
#include "hichip_debug.h"
#include "base64.h"
#include "usrm.h"
#include "ja_ini.h"

#define HICHIP_SERVER		"JuanServer"

static CGI_hashtable *g_htNetsdkCGI = NULL;
static char g_strUser[50] = {0};
static char g_strPwd[50] = {0};
static char* strrstr(const char *src, const char pattern){
	if(NULL == src){
		return NULL;
	}
	int nIndex = strlen(src) - 1;
	for( ; -1 != nIndex; --nIndex){
		if(pattern == src[nIndex]){
			return src + nIndex + 1;
		}
	}
	return NULL;
}
/*
this function retrives name of cgi and returns option flag of set
*/
static int suffix2cgi(AVal suffix, AVal *cgi){
	int nRet = 0;
	int nOffset = 0;
	char *strSuffix = AVAL_STRDUPA(suffix);
	char *strCgi = strrstr(strSuffix, '/');
	if(NULL != strCgi){
		if(NULL != strstr(strCgi, "Set")){
			nOffset = 3;
			nRet = 1;
		}
		else if(NULL != strstr(strCgi, "Get")){
			nRet  = 0;
			nOffset = 3;
		}
		else{
			nRet = 1;
			nOffset = 0;
		}
		cgi->av_val = strCgi + nOffset;
		cgi->av_len = strlen(strCgi) - nOffset;
		return nRet;
	}
}

static int getParamValue(const char *query, const char *key, AVal *value){
	int nRet = 0;
	if(NULL == query || NULL == key){
		HICHIP_TRACE("get param value error\n");
		return -1;
	}
	char *pKey = alloca(strlen(key) + 2);
	snprintf(pKey, strlen(key) + 2, "%s=", key);
	char *pTmp = strstr(query, pKey);
	if(NULL != pTmp){
		char *pEnd = strstr(pTmp, "&");
		value->av_val = pTmp + strlen(pKey);
		if(NULL != pEnd){
			value->av_len = pEnd - value->av_val;
		}
		else{
			value->av_len = strlen(pTmp) - strlen(pKey);
		}
		nRet = value->av_len;
	}
	else{
		value->av_val = NULL;
		value->av_len = 0;
		HICHIP_TRACE("key not found\n");
		nRet = -1;
	}
	return nRet;
}

static int string2ratio(const char *str, RatioValue_t *val){
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
// typedef enum {
// 	ITOA_Flag = 0,
// 	ITOA_Switch,
// 	ITOA_VideoEncQuality,
// 	ITOA_Profile,
// 	ITOA_VideoEncMode,
// 	ATOI_Flag,
// 	ATOI_Switch,
// 	ATOI_VideoEncQuality,
// 	ATOI_Profile,
// 	ATOI_VideoEncMode,	
// }
// static int transform(int type, int value, char *text){
// 	int nRet = 0;
// 	switch(type){
// 		case ITOA_Flag:
// 		strcat(text, (0 == value)? "no" : "yes");
// 		break;
// 		case ITOA_Switch:
// 		strcat(text, (0 == value)? "off", "on");
// 		break;
// 		case ITOA_VideoEncMode:
// 		strcat(text, (SYS_VENC_H264_MODE_VBR == value)? "variable": "constant");
// 		break;
// 		case ITOA_VideoEncQuality:
// 		break;
// 		case ITOA_Profile:
// 		break;
// 		case ATOI_Flag:
// 		nRet = 0 == strcmp(text, "yes")? 1 : 0;
// 		break;
// 		case ATOI_Switch:
// 		nRet = 0 == strcmp(text, "on")? 1 : 0;
// 		break;
// 		case ATOI_VideoEncMode:
// 		break;
// 		case ATOI_VideoEncQuality:
// 		break;
// 		case ATOI_Profile:
// 		break;
// 		default:
// 		break;
// 	}
// }
static int RtspUrl(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	AVal avChl, avStream;
	getParamValue(strQuery, "ch", &avChl);
	getParamValue(strQuery, "stream", &avStream);
	int nChl = atoi(AVAL_STRDUPA(avChl));
	int nStream = atoi(AVAL_STRDUPA(avStream));
	nRet = snprintf(strContent, nLength, "var url=\"%s\";"
				"var port=\"%d\";"
				"var stream_name=\"ch0_%d.264\";",
				inet_ntoa(pConf->ipcam.network.lan.static_ip.in_addr),
				pConf->ipcam.network.lan.port[0].value,
				nStream);

	return nRet;
}
static int VideoConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	if(bOption){
		AVal avChl, avShutter, avStandard;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "shutter", &avShutter);
		getParamValue(strQuery, "standard", &avStandard);
		int nChl = atoi(AVAL_STRDUPA(avChl));
		int nStandard = atoi(AVAL_STRDUPA(avStandard));
		if(avShutter.av_len > 0){
			pConf->ipcam.vin[0].digital_shutter.val = atoi(AVAL_STRDUPA(avShutter));			
		}
		SYSCONF_save(pConf);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl;
		getParamValue(strQuery, "ch", &avChl);
		int nChl = atoi(AVAL_STRDUPA(avChl));
		nRet = snprintf(strContent, nLength, "var ch=\"%d\";"
										"var width=\"%d\";"
										"var height=\"%d\";"
										"var shutter=\"%d\";",
										nChl, 1280, 720, pConf->ipcam.vin[0].digital_shutter.val);
		
	}
	return nRet;
}
static int CoverConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	if(bOption){
		AVal avChl, avId, avX, avY, avW, avH, avColor, avEnable;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "id", &avId);
		getParamValue(strQuery, "x", &avX);
		getParamValue(strQuery, "y", &avY);
		getParamValue(strQuery, "width", &avW);
		getParamValue(strQuery, "height", &avH);
		getParamValue(strQuery, "color", &avColor);
		getParamValue(strQuery, "enable", &avEnable);

		if(0 == strcmp("yes", AVAL_STRDUPA(avEnable))){
			float fX = atoi(AVAL_STRDUPA(avX)) / 1280.0;
			float fY = atoi(AVAL_STRDUPA(avY)) / 720.0;
			float fW = atoi(AVAL_STRDUPA(avW)) / 1280.0;
			float fH = atoi(AVAL_STRDUPA(avH)) / 720.0;
			int nColor = 0;
			sscanf(AVAL_STRDUPA(avColor), "%06x", &nColor);
			HICHIP_TRACE(strQuery);
			HICHIP_TRACE("%s, %s, %s, %s\n", AVAL_STRDUPA(avX), AVAL_STRDUPA(avY), AVAL_STRDUPA(avW), AVAL_STRDUPA(avH));
			HICHIP_TRACE("%f,%f,%f,%f,%x\n", fX, fY, fW, fH, nColor);
			if(0 == sdk_vin->check_cover(0, AVAL_STRDUPA(avId), NULL, NULL, NULL, NULL, NULL)){
				nRet = sdk_vin->add_cover(0, AVAL_STRDUPA(avId), fX, fY, fW, fH, nColor, 0);
			}
			else{
				nRet = sdk_vin->update_cover(0, AVAL_STRDUPA(avId), fX, fY, fW, fH, nColor, kSDK_VIN_COVER_UPDATE_FLAG_ALL);
			}
		}
		else{
			sdk_vin->del_cover(0, AVAL_STRDUPA(avId));
		}

		snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl, avId;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "id", &avId);
		int nChl = atoi(AVAL_STRDUPA(avChl));
		float fRatioX, fRatioY, fRatioW, fRatioH;
		int nColor;
		if(1 == sdk_vin->check_cover(nChl, AVAL_STRDUPA(avId), &fRatioX, &fRatioY, &fRatioW, &fRatioH, &nColor)){
			int nX = fRatioX * 1280;
			int nY = fRatioY * 720;
			int nW = fRatioW * 1280;
			int nH = fRatioH * 720;
			nRet = snprintf(strContent, nLength, "var ch=\"%d\";"
												"var id=\"%s\";"
												"var enable=\"yes\";"
												"var x=\"%d\";"
												"var y=\"%d\";"
												"var width=\"%d\";"
												"var height=\"%d\";"
												"var color=\"%x\";",
												nChl, AVAL_STRDUPA(avId),
												nX, nY, nW, nH, nColor);
		}
		else{
			nRet = snprintf(strContent, nLength, "var result=\"failed\";");
		}
	}
	return nRet;
}
static int ImageConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	if(bOption){
		AVal avChl, avBrightness, avSaturation, avContrast, avHue, avFlipH, avFlipV;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "brightness", &avBrightness);
		getParamValue(strQuery, "saturation", &avSaturation);
		getParamValue(strQuery, "contrast", &avContrast);
		getParamValue(strQuery, "hue", &avHue);
		getParamValue(strQuery, "flip_horizontal", &avFlipH);
		getParamValue(strQuery, "flip_vertical", &avFlipV);
		// RatioValue_t rBrightness, rSaturation, rContrast, rHue, rFlipH, rFlipV;
		// string2ratio(AVAL_STRDUPA(avBrightness), &rBrightness);
		// string2ratio(AVAL_STRDUPA(avSaturation), &rSaturation);
		// string2ratio(AVAL_STRDUPA(avContrast), &rContrast);
		// string2ratio(AVAL_STRDUPA(avHue), &rHue);
		// string2ratio(AVAL_STRDUPA(avFlipH), &rFlipH);
		// string2ratio(AVAL_STRDUPA(avFlipV), &rFlipV);
		int nBrightness = atoi(AVAL_STRDUPA(avBrightness));
		int nSaturation = atoi(AVAL_STRDUPA(avSaturation));
		int nContrast = atoi(AVAL_STRDUPA(avContrast));
		int nHue = atoi(AVAL_STRDUPA(avHue));
		if(nBrightness >= 0 && nBrightness <= 100){
			pConf->ipcam.vin[0].brightness.val = nBrightness * pConf->ipcam.vin[0].brightness.max / 100;
			SENSOR_brightness_set(nBrightness);
		}
		if(nSaturation >= 0 && nSaturation <= 100){
			pConf->ipcam.vin[0].saturation.val = pConf->ipcam.vin[0].saturation.max * nSaturation / 100;
			SENSOR_saturation_set(nSaturation);
		}
		if(nContrast >= 0 && nContrast <= 100){
			pConf->ipcam.vin[0].contrast.val = nContrast * pConf->ipcam.vin[0].contrast.max / 100;
			SENSOR_contrast_set(nContrast);
		}
		if(nHue >= 0 && nHue <= 100){
			pConf->ipcam.vin[0].hue.val = nHue * pConf->ipcam.vin[0].hue.max / 100;
			SENSOR_hue_set(nHue);
		}
		pConf->ipcam.vin[0].flip = (0 == strcmp("yes", AVAL_STRDUPA(avFlipH))) ? 1 : 0;
		pConf->ipcam.vin[0].mirror = (0 == strcmp("yes", AVAL_STRDUPA(avFlipV))) ? 1 : 0;
		SYSCONF_save(pConf);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		snprintf(strContent, nLength, "var hue=\"%2.2f\";"
										"var brightness=\"%2.2f\";"
										"var saturation=\"%2.2f\";"
										"var contrast=\"%2.2f\";"
										"var flip_horizontal=\"%s\";"
										"var flip_vertical=\"%s\";", 
										(float)pConf->ipcam.vin[0].hue.val / pConf->ipcam.vin[0].hue.max * 100.0,
										(float)pConf->ipcam.vin[0].brightness.val / pConf->ipcam.vin[0].brightness.max * 100.0,
										(float)pConf->ipcam.vin[0].saturation.val / pConf->ipcam.vin[0].saturation.max * 100.0,
										(float)pConf->ipcam.vin[0].contrast.val / pConf->ipcam.vin[0].contrast.max * 100.0,
										pConf->ipcam.vin[0].flip ? "yes" : "no",
										pConf->ipcam.vin[0].mirror ? "yes" : "no");
	}


	return nRet;
}
static int VideoEncConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	if(bOption){
		AVal avChl, avStream, avProfile, avBps, avFps, avGop, avMode, avQuality, avWith, avHeight;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "stream", &avStream);
		getParamValue(strQuery, "profile", &avProfile);
		getParamValue(strQuery, "bps", &avBps);
		getParamValue(strQuery, "fps", &avFps);
		getParamValue(strQuery, "gop", &avGop);
		getParamValue(strQuery, "mode", &avMode);
		getParamValue(strQuery, "quality", &avQuality);
		getParamValue(strQuery, "width", &avWith);
		getParamValue(strQuery, "height", &avHeight);
		int nStream = atoi(AVAL_STRDUPA(avStream));
		if(avBps.av_len > 0){
			pConf->ipcam.vin[0].enc_h264[nStream].stream[0].bps = atoi(AVAL_STRDUPA(avBps));
		}
		if(avFps.av_len > 0){
			pConf->ipcam.vin[0].enc_h264[nStream].stream[0].fps = atoi(AVAL_STRDUPA(avFps));
		}
		if(avGop.av_len > 0){
			pConf->ipcam.vin[0].enc_h264[nStream].stream[0].gop = atoi(AVAL_STRDUPA(avGop));
		}
		if(avMode.av_len > 0){
			pConf->ipcam.vin[0].enc_h264[nStream].stream[0].mode.val = 0 == strcmp(AVAL_STRDUPA(avMode), "variable") ? SYS_VENC_H264_MODE_VBR : SYS_VENC_H264_MODE_CBR;
		}
		
		SYSCONF_save(pConf);
		snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl, avStream;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "stream", &avStream);
		int nChl = atoi(AVAL_STRDUPA(avChl));
		int nStream = atoi(AVAL_STRDUPA(avStream));
		int width, height;
		SYS_VIN_SIZE_WIDTH_HEIGHT(pConf->ipcam.vin[0].enc_h264[nStream].stream[0].size, width, height) ;
		snprintf(strContent, nLength, "var ch=\"%d\";"
										"var stream=\"%d\";"
										"var profile=\"main\";"
										"var bps=\"%d\";"
										"var fps=\"%d\";"
										"var gop=\"%d\";"
										"var mode=\"%s\";"
										"var width=\"%d\";"
										"var height=\"%d\";", 
										nChl, nStream,
										pConf->ipcam.vin[0].enc_h264[nStream].stream[0].bps, 
										pConf->ipcam.vin[0].enc_h264[nStream].stream[0].fps, 
										pConf->ipcam.vin[0].enc_h264[nStream].stream[0].gop, 
										SYS_VENC_H264_MODE_VBR == pConf->ipcam.vin[0].enc_h264[nStream].stream[0].mode.val ? "variable": "constant", 
										width, height);
	}
	return nRet;
}
static int OverlayConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	if(bOption){
		AVal avChl, avName, avShow, avText;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "name", &avName);
		getParamValue(strQuery, "show", &avShow);
		getParamValue(strQuery, "text", &avText);
		int nShow = 0 == strcmp(AVAL_STRDUPA(avShow), "yes") ? 1 : 0;
		if(0 == strcmp(AVAL_STRDUPA(avName), "title")){
			APP_OVERLAY_show_title(0, 0, nShow);
			if(NULL != AVAL_STRDUPA(avText)){
				strcpy(pConf->ipcam.vin[0].channel_name, AVAL_STRDUPA(avText));
				APP_OVERLAY_set_title(0, pConf->ipcam.vin[0].channel_name);
				APP_OVERLAY_update_title(0, 0, NULL, NULL);
			}
		}
		else{
			APP_OVERLAY_show_clock(0, 0, nShow);
		}
		SYSCONF_save(pConf);
	}
	
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	return nRet;
}
static int AudioInVolume(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	if(bOption){
		AVal avChl, avVolume;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "volume", &avVolume);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl;
		getParamValue(strQuery, "ch", &avChl);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	return nRet;
}
static int AudioEncConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	if(bOption){
		AVal avCodec;
		// getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "codec", &avCodec);
		// getParamValue(strQuery, "enable", &avEnable);
		strncpy(pConf->ipcam.ain[0].enc[0].engine, AVAL_STRDUPA(avCodec), sizeof(pConf->ipcam.ain[0].enc[0].engine));
		SYSCONF_save(pConf);
		snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl;
		getParamValue(strQuery, "ch", &avChl);
		nRet = snprintf(strContent, nLength, "var ch=\"%s\";"
										"var codec=\"%s\";"
										"var enable=\"yes\";",
										AVAL_STRDUPA(avChl), pConf->ipcam.ain[0].enc[0].engine);
	}
	return nRet;
}
static int MdConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;	
	if(bOption){
		AVal avChl, avId, avEnable, avX, avY, avWith, avHeight, avSensitivity;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "id", &avId);
		getParamValue(strQuery, "enable", &avEnable);
		getParamValue(strQuery, "x", &avX);
		getParamValue(strQuery, "y", &avY);
		getParamValue(strQuery, "width", &avWith);
		getParamValue(strQuery, "height", &avHeight);
		getParamValue(strQuery, "sensitivity", &avSensitivity);
		// RatioValue_t rX, rY, rW, rH;
		// string2ratio(AVAL_STRDUPA(avX), &rX);
		// string2ratio(AVAL_STRDUPA(avY), &rY);
		// string2ratio(AVAL_STRDUPA(avWith), &rW);
		// string2ratio(AVAL_STRDUPA(avHeight), &rH);
		if(0 == strcmp("yes", AVAL_STRDUPA(avEnable))){
			float fX = atoi(AVAL_STRDUPA(avX)) / 1280.0;
			float fY = atoi(AVAL_STRDUPA(avY)) / 720.0;
			float fW = atoi(AVAL_STRDUPA(avWith)) / 1280.0;
			float fH = atoi(AVAL_STRDUPA(avHeight)) / 720.0;
			float fThreshold = 1.0 / (float)atoi(AVAL_STRDUPA(avSensitivity));
			if(0 != sdk_vin->add_md_rect(0, AVAL_STRDUPA(avId), fX, fY, fW, fH, fThreshold)){
				nRet = snprintf(strContent, nLength, "var result=\"failed\";");	
				HICHIP_TRACE("add md failed\n");	
			}
		}
		else{
			HICHIP_TRACE("clear md\n");
			sdk_vin->del_md_rect(0, AVAL_STRDUPA(avId));
		}
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		AVal avChl, avId;
		getParamValue(strQuery, "ch", &avChl);
		getParamValue(strQuery, "id", &avId);
		float fX, fY, fW, fH, fThreshold;
		int nX, nY, nW, nH, nSensitivity;
		if(1 == sdk_vin->check_md_rect(0, AVAL_STRDUPA(avId), &fX, &fY, &fW, &fH, &fThreshold)){
			nSensitivity = 1.0 / fThreshold;
			nX = fX * 1280;
			nY = fY * 720;
			nW = fW * 1280;
			nH = fH * 720;
			nRet = snprintf(strContent, nLength, "var ch=\"%s\";"
												"var id=\"%s\";"
												"var enable=\"yes\";"
												"var x=\"%d\";"
												"var y=\"%d\";"
												"var width=\"%d\";"
												"var height=\"%d\";"
												"var sensitivity=\"%d\";", 
												AVAL_STRDUPA(avChl), 
												AVAL_STRDUPA(avId),
												nX, nY, nW, nH, nSensitivity);
		}
		else{
			nRet = snprintf(strContent, nLength, "var result=\"failed\";");
		}

	}
	return nRet;
}
static int MdLinkageConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	if(bOption){
		AVal avChl, avEmailNotice, avEmailText, avEmailSnap, avFtpPush, avFtpText, avFtpSnap, avFtpRecord, avAlarmOut, avAlarmDuration;
		getParamValue(strQuery, "email_notice", &avEmailNotice);
		getParamValue(strQuery, "email_text", &avEmailText);
		getParamValue(strQuery, "email_snapshot", &avEmailSnap);
		getParamValue(strQuery, "ftp_push", &avFtpPush);
		getParamValue(strQuery, "ftp_text", &avFtpText);
		getParamValue(strQuery, "ftp_snapshot", &avFtpSnap);
		getParamValue(strQuery, "ftp_record", &avFtpRecord);
		getParamValue(strQuery, "alarm_out", &avAlarmOut);
		getParamValue(strQuery, "alarm_duration", &avAlarmDuration);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	return nRet;
}
static int AlarmInConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	return nRet;
}
static int AlarmOutConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	return nRet;
}
static int InfraredConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	if(bOption){
		AVal avMode;
		getParamValue(strQuery, "mode", &avMode);
		int nMode = 0;
		if(0 == strcmp("auto", AVAL_STRDUPA(avMode))){
			nMode = ISP_IRCUT_MODE_AUTO;
		}
		else if(0 == strcmp("on", AVAL_STRDUPA(avMode))){
			nMode = ISP_IRCUT_MODE_NIGHT;
		}
		else{
			nMode = ISP_IRCUT_MODE_DAYLIGHT;
		}
		pConf->ipcam.isp.day_night_mode.ircut_mode = nMode;
		SENSOR_ircut_mode_set(nMode);
		SYSCONF_save(pConf);
		nRet = snprintf(strContent, nLength, "var=\"ok\";");
	}
	else{
		char mode[8] = {0};
		switch(pConf->ipcam.isp.day_night_mode.ircut_mode){
			case ISP_IRCUT_MODE_AUTO:
				strcat(mode, "auto"); break;
			case ISP_IRCUT_MODE_NIGHT:
				strcat(mode, "on"); break;
			case ISP_IRCUT_MODE_DAYLIGHT:
				strcat(mode, "off"); break;
			default:
				break;
		}
		snprintf(strContent, nLength, "var mode=\"%s\";", mode);
	}

	return nRet;
}
static int EtherConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	if(bOption){
		AVal avDhcp, avIp, avNetmask, avGateway, avFirDns, avSecDns, avMac;
		getParamValue(strQuery, "dhcp", &avDhcp);
		getParamValue(strQuery, "static_ip", &avIp);
		getParamValue(strQuery, "static_netmask", &avNetmask);
		getParamValue(strQuery, "static_gateway", &avGateway);
		getParamValue(strQuery, "preferred_dns", &avFirDns);
		getParamValue(strQuery, "alternate_dns", &avSecDns);
		getParamValue(strQuery, "mac", &avMac);
		pConf->ipcam.network.lan.dhcp = (0 == strcmp("yes", AVAL_STRDUPA(avDhcp)));
		if(avIp.av_len > 0){
			pConf->ipcam.network.lan.static_ip.s_addr = inet_addr(AVAL_STRDUPA(avIp));
		}
		if(avNetmask.av_len > 0){
			pConf->ipcam.network.lan.static_netmask.s_addr = inet_addr(AVAL_STRDUPA(avNetmask));
		}
		if(avGateway.av_len > 0){
			pConf->ipcam.network.lan.static_gateway.s_addr = inet_addr(AVAL_STRDUPA(avGateway));
		}
		if(avFirDns.av_len > 0){
			pConf->ipcam.network.lan.static_preferred_dns.s_addr = inet_addr(AVAL_STRDUPA(avFirDns));
		}
		if(avSecDns.av_len > 0){
			pConf->ipcam.network.lan.static_alternate_dns.s_addr = inet_addr(AVAL_STRDUPA(avSecDns));		
		}
		SYSCONF_save(pConf);
		nRet = snprintf(strContent, nLength, "var=\"ok\";");	
	}
	else{
		char strDhcp[10] = {0};
		if(1 == pConf->ipcam.network.lan.dhcp){
			strcat(strDhcp, "yes");
		}
		else{
			strcat(strDhcp, "no");
		}
		char mac[18] = {0};
		sprintf(mac, "%x:%x:%x:%x:%x:%x", pConf->ipcam.network.mac.s[0], 
											pConf->ipcam.network.mac.s[1],
											pConf->ipcam.network.mac.s[2],
											pConf->ipcam.network.mac.s[3],
											pConf->ipcam.network.mac.s[4],
											pConf->ipcam.network.mac.s[5]);
		char strIP[16] = {0}; snprintf(strIP, sizeof(strIP), "%s", inet_ntoa(pConf->ipcam.network.lan.static_ip.in_addr));
		char strNetmask[16] = {0}; snprintf(strNetmask, sizeof(strNetmask), "%s", inet_ntoa(pConf->ipcam.network.lan.static_netmask.in_addr));
		char strGateway[16] = {0}; snprintf(strGateway, sizeof(strGateway), "%s", inet_ntoa(pConf->ipcam.network.lan.static_gateway.in_addr));
		char strFdns[16] = {0}; snprintf(strFdns, sizeof(strFdns), "%s", inet_ntoa(pConf->ipcam.network.lan.static_preferred_dns.in_addr));
		char strSdns[16] = {0}; snprintf(strSdns, sizeof(strSdns), "%s", inet_ntoa(pConf->ipcam.network.lan.static_alternate_dns.in_addr));
		nRet = snprintf(strContent, nLength, "var dhcp=\"%s\";"
										"var static_ip=\"%s\";"
										"var static_netmask=\"%s\";"
										"var static_gateway=\"%s\";"
										"var static_preferred_dns=\"%s\";"
										"var static_alternate_dns=\"%s\";"
										"var mac=\"%s\";",
										strDhcp, strIP, strNetmask, strGateway, strFdns, strSdns, mac);
	}
	return nRet;
}
static int PortConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	if(bOption){
		AVal avPort;
		getParamValue(strQuery, "port", &avPort);
		if(avPort.av_len){
			pConf->ipcam.network.lan.port[0].value = atoi(AVAL_STRDUPA(avPort));
		}
		SYSCONF_save(pConf);
		exit(0);
	}
	else{
		nRet = snprintf(strContent, nLength, "var port=\"%d\";", pConf->ipcam.network.lan.port[0].value);

	}
	
	return nRet;
}
static int UpnpConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	SYSCONF_t *pConf = SYSCONF_dup();
	int nRet = 0;
	if(bOption){
		AVal avEnable;
		getParamValue(strQuery, "enable", &avEnable);
		pConf->ipcam.network.lan.upnp = (0 == strcmp("yes", AVAL_STRDUPA(avEnable)))? 1: 0;
		SYSCONF_save(pConf);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		char strUpnp[5] = {0};
		if(1 == pConf->ipcam.network.lan.upnp){
			strcpy(strUpnp, "yes");
		}
		else{
			strcpy(strUpnp, "no");
		}
		nRet = snprintf(strContent, nLength, "var enable=\"%s\";", strUpnp);
	}
	return nRet;
}
static int PtzConf(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	return nRet;
}
static int PtzAction(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	return nRet;
}

static int SysReboot(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	exit(0);
	return nRet;
}

static int SysReset(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	SYSCONF_default_factory();
	exit(0);
	return nRet;
}

static int SysInfo(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	nRet = snprintf(strContent, nLength, "var model=\"%s\";"
											"var hardware_version=\"%s\";"
											"var software_version-\"%s\";"
											"var name=\"%s\";",
											pConf->ipcam.info.device_model,
											pConf->ipcam.info.hardware_version,
											pConf->ipcam.info.software_version,
											pConf->ipcam.info.device_name);
	return nRet;
}

static int SysTime(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	SYSCONF_t *pConf = SYSCONF_dup();
	if(bOption){
		AVal avDate, avTime, avTimeZone, avDst;
		getParamValue(strQuery, "date", &avDate);
		getParamValue(strQuery, "time", &avTime);
		getParamValue(strQuery, "time_zone", &avTimeZone);
		getParamValue(strQuery, "dst", &avDst);
		if(avTimeZone.av_len > 0){
			int nTimezone = atoi(AVAL_STRDUPA(avTimeZone));
			TIMEZONE_SYNC(nTimezone);
			pConf->ipcam.date_time.time_zone.val = nTimezone;			
		}
		if(avDst.av_len > 0){
			int nDst = (0 == strcmp("no", AVAL_STRDUPA(avDst))) ? 0: 1;
			pConf->ipcam.date_time.day_saving_time.val = nDst;			
		}

		struct tm tmSetting;
		sscanf(AVAL_STRDUPA(avDate), "%04d%02d%02d", &tmSetting.tm_year, &tmSetting.tm_mon, &tmSetting.tm_mday);
		sscanf(AVAL_STRDUPA(avTime), "%02d%02d%02d", &tmSetting.tm_hour, &tmSetting.tm_min, &tmSetting.tm_sec);
		tmSetting.tm_year -= 1900;
		tmSetting.tm_mon -= 1;
		time_t tvSeconds = mktime(&tmSetting);
		struct timeval tvSetting = { 	.tv_sec = tvSeconds, 
										.tv_usec = 0 
									};
		settimeofday(&tvSetting, NULL);
		TTASK_syn_time(tvSetting.tv_sec);
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		time_t tNow;
		struct tm *tmNow = NULL;
		time(&tNow);
		tmNow = localtime(&tNow);
		nRet = snprintf(strContent, nLength, "var date=\"%04d%02d%02d\";"
										"var time=\"%02d%02d%02d\";"
										"var time_zone=\"%d\";"
										"var dst=\"%s\";",
										tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, 
										tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec,
										pConf->ipcam.date_time.time_zone.val,
										(0 == pConf->ipcam.date_time.day_saving_time.val)? "yes": "no");
	}
	return nRet;
}

static int UserList(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	lpJA_INI_FILE pIni = JAINI_open("/tmp/usrm.ini");
	char user[10] = {0};
	pIni->read_text(pIni, "OPTION", "user", user);
	int nCount = atoi(user);
	int i;
	for(i = 0; i < nCount; ++i){
		char session[10] = {0};
		char name[50] = {0};
		char admin[10] = {0};
		char item[100] = {0};
		sprintf(session, "USER%d", i);
		pIni->read_text(pIni, session, "name", name);
		pIni->read_text(pIni, session, "admin", admin);
		nRet = snprintf(item, sizeof(item), "var user%d={ name:\"%s\", role:\"%s\"};", i, name, (0 == strcmp("1", admin))? "admin" : "user");
		strcat(strContent, item);
	}

	return nRet;
}

static int NewUser(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	AVal avUser, avPwd, avRole;
	getParamValue(strQuery, "user_name", &avUser);
	getParamValue(strQuery, "user_role", &avRole);
	getParamValue(strQuery, "password", &avPwd);
	int bAdmin = (0 == strcmp(AVAL_STRDUPA(avRole), "admin"))? 1: 0;
	nRet = USRM_add_user(AVAL_STRDUPA(avUser), AVAL_STRDUPA(avPwd), bAdmin, USRM_PERMIT_ALL, 1);
	USRM_store();
	if(USRM_GREAT == nRet){
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		nRet = snprintf(strContent, nLength, "var result=\"failed\";");
	}
	return nRet;
}

static int DelUser(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	AVal avUser, avPwd;
	getParamValue(strQuery, "user_name", &avUser);
	getParamValue(strQuery, "password", &avPwd);
	nRet = USRM_del_user(AVAL_STRDUPA(avUser));
	USRM_store();
	if(USRM_GREAT == nRet){
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");
	}
	else{
		nRet = snprintf(strContent, nLength, "var result=\"failed\";");
	}
	return nRet;
}

static int SetPassword(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	AVal avNewPwd, avRole;

	getParamValue(strQuery, "new_password", &avNewPwd);
	getParamValue(strQuery, "user_role", &avRole);
	int bAdmin = (0 == strcmp(AVAL_STRDUPA(avRole), "admin"))? 1: 0;
	nRet = USRM_edit_user(g_strUser, AVAL_STRDUPA(avNewPwd), bAdmin, USRM_PERMIT_ALL);
	USRM_store();
	if(USRM_GREAT == nRet){
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");	
	}
	else{
		nRet = snprintf(strContent, nLength, "var result=\"failed\";");		
	}
	return nRet;
}

static int EditUser(const char *strQuery, char *strContent, int nLength, bool bOption){
	int nRet = 0;
	AVal avUser, avPwd, avRole;
	getParamValue(strQuery, "user_name", &avUser);
	getParamValue(strQuery, "user_role", &avRole);
	getParamValue(strQuery, "password", &avPwd);
	int bAdmin = (0 == strcmp(AVAL_STRDUPA(avRole), "admin"))? 1: 0;
	HICHIP_TRACE("EditUser:%d\n", bAdmin);
	nRet = USRM_edit_user(AVAL_STRDUPA(avUser), AVAL_STRDUPA(avPwd), bAdmin, USRM_PERMIT_ALL);
	USRM_store();
	if(USRM_GREAT == nRet){
		nRet = snprintf(strContent, nLength, "var result=\"ok\";");	
	}
	else{
		nRet = snprintf(strContent, nLength, "var result=\"failed\";");		
	}

	return nRet;

}

/*


*/
void NETSDK_init(){
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "RtspUrl.cgi", RtspUrl);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "VideoConf.cgi", VideoConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "CoverConf.cgi", CoverConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "ImageConf.cgi", ImageConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "VideoEncConf.cgi", VideoEncConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "OverlayConf.cgi", OverlayConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "AudioInVolume.cgi", AudioInVolume);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "AudioEncConf.cgi", AudioEncConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "MdConf.cgi", MdConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "MdLinkageConf.cgi", MdLinkageConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "AlarmInConf.cgi", AlarmInConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "AlarmOutConf.cgi", AlarmOutConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "InfraredConf.cgi", InfraredConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "EtherConf.cgi", EtherConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "PortConf.cgi", PortConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "UpnpConf.cgi", UpnpConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "PtzConf.cgi", PtzConf);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "PtzAction.cgi", PtzAction);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "SysReboot.cgi", SysReboot);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "SysReset.cgi", SysReset);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "SysInfo.cgi", SysInfo);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "SysTime.cgi", SysTime);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "UserList.cgi", UserList);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "NewUser.cgi", NewUser);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "DelUser.cgi", DelUser);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "Password.cgi", SetPassword);
	g_htNetsdkCGI = CGI_hashAdd(g_htNetsdkCGI, "EditUser.cgi", EditUser);

}

void NETSDK_destroy(){
	CGI_hashDestroy(g_htNetsdkCGI);
}


int doCheckAuth(const char *auth){
	int nRet = 0;
	if(NULL == auth || 0 == strlen(auth)){
		return -1;
	}
	char *strAuthDec = alloca(strlen(auth) * 2 );
	memset(strAuthDec, 0, strlen(auth) * 2);
	int nLenDec = base64_decode(auth + strlen("Basic "), strAuthDec, strlen(auth) - strlen("Basic "));
	int i = 0;
	for( ; i < nLenDec; ++i){
		if(':' == strAuthDec[i]){
			memset(g_strUser, 0, sizeof(g_strUser));
			memset(g_strPwd, 0, sizeof(g_strPwd));
			strncpy(g_strUser, strAuthDec, i);
			strncpy(g_strPwd, strAuthDec + i + 1, nLenDec - i - 1);
			break;
		}
	}
	nRet = USRM_check_user(g_strUser, g_strPwd);
	HICHIP_TRACE("user check:%d, user:%s, pwd:%s\n", nRet, g_strUser, g_strPwd);
	return nRet;
}
int NETSDK_process_cgi(HTTPD_SESSION_t* session){
	int nRet = 0;
	session->keep_alive = 0;
	char content[2048] = {0};
	HTTP_HEADER_t *http_header = NULL;
	
	AVal avAuthorization;
	http_read_header(session->request_buf, "Authorization", &avAuthorization);

	if(USRM_GREAT != doCheckAuth(AVAL_STRDUPA(avAuthorization))){
		http_header = http_response_header_new("1.1", 401, NULL);
		http_header->add_tag_text(http_header, "WWW-Authenticate", "Basic realm=\"TP-LINK Wireless N Router WR840N\"");
		http_header->to_text(http_header, content, ARRAY_ITEM(content));
		http_response_header_free(http_header);
		http_header = NULL;
		nRet = send(session->sock, content, strlen(content), 0);
		return nRet;
	}
	USRM_I_KNOW_U_t *pUsrm = USRM_login(g_strUser, g_strPwd);
	char body[1024] = {0};
	AVal avCgi;
	int nOption = suffix2cgi(session->request_line.uri_suffix, &avCgi);
	char *strCgi = AVAL_STRDUPA(avCgi);
	if(NULL != strCgi){
		CGI_hashtable *pHashItem = CGI_hashFind(g_htNetsdkCGI, strCgi);
		if(NULL != pHashItem){
			HICHIP_TRACE("%s, %d\n", strCgi, nOption);
			pHashItem->pCgiFunc(AVAL_STRDUPA(session->request_line.uri_query_string), body, sizeof(body), nOption);
		}
		else{
			HICHIP_TRACE("cgi not found\n");
		}
		http_header = http_response_header_new("1.1", 200, NULL);                      
		http_header->add_tag_text(http_header, "Server", HICHIP_SERVER);
		http_header->add_tag_text(http_header, "Cache-Control", "no-cache");  
		http_header->add_tag_text(http_header, "Content-Type", "text/html");
		http_header->add_tag_text(http_header, "Connection", "close");      
		http_header->add_tag_int(http_header, "Content-Length", strlen(body));
		http_header->to_text(http_header, content, sizeof(content));
		http_response_header_free(http_header);
		http_header = NULL;
		strncat(content, body, strlen(body));
		nRet = send(session->sock, content, strlen(content), 0);
		if(nRet < 0){
			HICHIP_TRACE("NETSDK send data failed, error:%s\n", strerror(errno));
		}
	}
	else{
		HICHIP_TRACE("no cgi pass\n");
	}
	USRM_logout(pUsrm);
	*session->trigger = false;
	return nRet;
}