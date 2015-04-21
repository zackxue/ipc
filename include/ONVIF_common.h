#ifndef __ONVIF_COMMON_H__
#define __ONVIF_COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif
/*
VSC : VideoSourceConfiguration
ASC : AudioSourceConfiguration
VEC : VideoEncoderConfiguration
AEC : AudioEncoderConfiguration

*/
#include "sysconf.h"
//#include "soapStub.h"
//#define VIDEO_SOURCE_TOKEN

#define ONVIF_NAME_TOKEN_MAX (40)
#define ONVIF_SCOPE_MAX (3)
#define ONVIF_USER_MAX	(3)
#define ONVIF_RELAYOUTPUT_MAX	(3)
#define ONVIF_PROFILE_MAX (3)

typedef struct 
{
	char *item;
	unsigned int definition;  //Fixed = 0, Configurable = 1;
}ONVIF_Scope_t;

typedef struct {

}ONVIF_User_t;

typedef struct {
	char *RelayOutputToken;
	int RelayMode;
	char *DelayTime;
	int RelayIdleState;
}ONVIF_RelayOutput_t;

typedef struct {
	//profile
	char ProfileName[ONVIF_NAME_TOKEN_MAX];
	char ProfileToken[ONVIF_NAME_TOKEN_MAX];
	int Profilefixed;  // Fixed = 0
	//Video Source Config
	char VSCName[ONVIF_NAME_TOKEN_MAX];
	int VSCUseCount;
	char VSCToken[ONVIF_NAME_TOKEN_MAX];
	char VSCSourceToken[ONVIF_NAME_TOKEN_MAX];
	struct tt__IntRectangle Bounds;
	//Audio Source Config
	char ASCName[ONVIF_NAME_TOKEN_MAX];
	char ASCToken[ONVIF_NAME_TOKEN_MAX];
	char ASCSourceToken[ONVIF_NAME_TOKEN_MAX];
	int ASCUseCount;
	//Video Encoder Config
	char VECName[ONVIF_NAME_TOKEN_MAX];
	char VECToken[ONVIF_NAME_TOKEN_MAX];
	int VECUseCount;
	int VECEncoding;   //JPEG = 0, MPEG4 = 1, H264 = 2
	int VECWidth;
	int VECHeight;
	float VECQuality;
	int VECfps;
	int VECbps;
	int VECH264Gov;
	int VECH264Gop;
	int VECH264Profile; //Profile__Baseline = 0, Profile__Main = 1,Profile__Extended = 2, Profile__High = 3
	char VECTimeOut[ONVIF_NAME_TOKEN_MAX];
	//Audio Endoder Config
	char AECName[ONVIF_NAME_TOKEN_MAX];
	char AECToken[ONVIF_NAME_TOKEN_MAX];
	int AECUseCount;
	int AECEncoding; //G711 = 0, G726 = 1, AAC = 2
	int AECBitrate;
	int AECSampleRate;
	char AECTimeOut[ONVIF_NAME_TOKEN_MAX];

}ONVIF_Profile_t;


typedef struct {
	SYSCONF_t *pConf;
	ONVIF_Scope_t 	Scopes[ONVIF_SCOPE_MAX];
	ONVIF_User_t	User[ONVIF_USER_MAX];
	ONVIF_RelayOutput_t RelayOutput[ONVIF_RELAYOUTPUT_MAX];;
	ONVIF_Profile_t	Profile[ONVIF_PROFILE_MAX];
	int nProfileCount;
}ONVIF_Conf_t;

typedef enum{
	ONVIF_CTRL_SET_PROFILE = 0,
	ONVIF_CTRL_SET_SCOPE,
	ONVIF_CTRL_SET_ADDUSER,
	ONVIF_CTRL_SET_RELAYOUTPUT,
	ONVIF_CTRL_SET_SYSCONF
}ONVIF_CTRLDATA_TYPE_t;


/*
1. use ONVIF_init to run onvif
*/
extern void ONVIF_init();

extern void ONVIF_Ctrldata(ONVIF_CTRLDATA_TYPE_t type, void *pData);
extern int ONVIF_listen();
extern void ONVIF_dup();
extern void ONVIF_end();





#ifdef __cplusplus
};
#endif

#endif // __ONVIF_COMMON_H__