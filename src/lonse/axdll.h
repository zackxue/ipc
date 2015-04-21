#ifndef axdll_H
#define axdll_H

#include "Ax_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "App.txt"

#define PROC_TOTAL_DVS_EXE       10 
#define __DEVICETYPE              dt_DevA4
#define FLASH_ADDR_SN 0x00db0000
#define VIDEOCHLCOUNT             1
#define AUDIOCHLCOUNT             1
#define DICHLCOUNT                1
#define DOCHLCOUNT                1
#define RS485DEVCOUNT             1

typedef struct TDevCfg {
    struct TNetCfgPkt NetCfgPkt;                            //设备网络配置包
    struct TWiFiCfgPkt WiFiCfgPkt;
    struct TDevInfoPkt DevInfoPkt;                          //设备信息包
    struct TUserCfgPkt UserCfgPkt;								//用户配置包
    struct TAlmCfgPkt AlmCfgPkt;                            //警报配置包
    struct TRS485CfgPkt RS485CfgPkt;                        //485通信包
    struct TDiDoCfgPkt DiDoCfgPkt;                          //DIDO配置包
    struct TVideoCfgPkt VideoCfgPkt[VIDEOCHLCOUNT];         //视频配置包--单通道
    struct TAudioCfgPkt AudioCfgPkt[AUDIOCHLCOUNT];         //音频配置包--单通道
    struct TMDCfgPkt MDCfgPkt[VIDEOCHLCOUNT];               //移动侦测包--单通道
    struct THideAreaCfgPkt HideAreaCfgPkt[VIDEOCHLCOUNT];   //隐藏录影区域包--单通道
    struct TDiskCfgPkt DiskCfgPkt;													//磁盘配置包
    struct TRecCfgPkt RecCfgPkt[VIDEOCHLCOUNT];             //录影配置包
    struct TFTPCfgPkt FTPCfgPkt;
    struct TSMTPCfgPkt SMTPCfgPkt;
    int Flag;
    int Flag1;
  }TDevCfg;
//-----------------------------------------------------------------------------
  typedef struct TAlmState {//未使用
    int NightDetection;
    int VideoBlind;
    int MotionDetection;
    int VideoLost;
  }TAlmState;

  //-----------------------------------------------------------------------------
  typedef struct TPIDShmPkt { //未使用  	 //各进程PID管理
    char AppName[64];
    int PID;
    dword time;
    int Flag;
  }TPIDShmPkt;
//-----------------------------------------------------------------------------
  typedef enum TCollectGrab{ //sizeof 4 BYTE
    CollectGrabNULL     = 0,
    OV7640              = 1,
    OV7660              = 2,
    MT9V111             = 3,
    OV9655              = 4,
    SONY_ICX408AK_CCD   = 5,
    SONY_ICX409AK_CCD   = 6,
    S7113_Composite     = 7,
    S7113_S_Video       = 8,
    IPTV_7113_S_Video   = 9,
    IPTV_7113_Tuner     =10,
    IPTV_7113_Composite =11,
    OV7720_OV7725       =12,
    S7137_SVideo        =13,
    S7137_Tuner         =14,
    S7137_Composite     =15,
    MT9M112_x           =16,
    OV7710_x            =17,
    Sony_IT1            =18,
    MT9M131             =19,								//使用此项
    TW9910_SVideo_1     =20,
    TW9910_Composite_1  =21,
    TW2835_module_1     =22,
    TW2835_module_2     =23,
    TW9910_SVideo_2     =24,
    TW9910_Composite_2  =25,
    MT9P031             =26,
    TW2815_CH0          =27,
    TW2815_CH1          =28,
    TW2815_CH2          =29,
    TW2815_CH3          =30,
    OV3642              =31,
    OV9710              =32,
    MT9V131             =33,
    MT9V136             =34,
    MT9D112             =35,
    MT9M131_RAW         =36,
    SONY3172            =37,
    OV2650              =38,
    CollectGrabMAX      =39
  }TCollectGrab;

//-----------------------------------------------------------------------------

  typedef struct TRes {//未使用		  //日志常量
    struct {
      char* Login;//用户登录
      char* PlayLive;//开始播放现场
      char* DOControl;//DO控制
      char* Alarm;//警报
      char* ClearAlarm;//关闭警报
      char* SetTime;//设置时间
      char* SetDevInfo;//设置设备信息
      char* SetNetCfg;//设置网络配置
      char* SetAlmCfg;//设置Alarm配置
      char* SetRecCfg;//设置录影配置
      char* SetVideoCfg;//设置视频配置
      char* SetAudioCfg;//设置音频配置
      char* SetMDCfg;//移动侦测配置
      char* SetHideArea;//秘录
      char* SetRS485Cfg;
      char* SetDiDoCfg;
      char* SetDevReboot;//重启设备
      char* SetDevDefault;//系统回到缺省配置 Pkt.Value 不恢复IP; Pkt.Value 恢复IP
      char* SetUserCfg;//设置用户列表
      char* UpgradeImage;//升级影像
      char* UploadFile;//上传文件
      char* DeleteFile;//删除文件
    }Log;
    struct {
      char* None;
      char* MotionDetection;//位移报警Motion Detection
      char* DigitalInput;//DI报警
      char* VideoLost;//视频丢失
    }Alm;
  }TRes;

  //-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif
