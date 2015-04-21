#ifndef Ax_protocol_H
#define Ax_protocol_H 

#include "cm_types.h"

#define Port_Ax_CmdData        3001                        //TCP 缺省命令数据端口 对讲端口
#define Port_Ax_http             80
#define Port_Ax_RTSP            554

#define Port_Ax_Multicast      1900
#define Port_Ax_Search_Local   1899
#define IP_Ax_Multicast  "239.255.255.250"//uPnP IP查询 多播对讲IP
#define REC_FILE_EXT           "ra2"
#define REC_FILE_EXT_DOT       ".ra2"


#pragma option push -b
typedef enum TDevType {// sizeof 4
  dt_None=0,
  dt_MgrSvr=1,
  dt_VideoSvr=2,
  dt_S3=3,
  dt_S4=4,
  dt_S5=5,
  dt_S6=6,
  dt_S7=7,
  dt_S8=8,
  dt_S9=9,
  dt_Clt=10,
  dt_DevA1=11,
  dt_DevC2=12,
  dt_DevA3=13,
  dt_DevA4=14,			//使用此项
  dt_DevC5=15,
  dt_D6=16,
  dt_D7=17,
  dt_D8=18,
  dt_D9=19,
  dt_D10=20,
  dt_DevD1=21,
  dt_DevD4=22,
  dt_DevOther
}TDevType;

//-----------------------------------------------------------------------------
typedef struct TSMTPCfgPkt {//sizeof 500 Byte
  int Active;
  char100 Subject;
  char100 Reply;
  char100 Sender;
  char100 SMTPServer;
  int SMTPPort;
  char40 Account;
  char40 Password;
  int SSL;
  int Flag;
  int Flag1;
}TSMTPCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TFTPCfgPkt {//sizeof 320 Byte
  int Active;
  char100 FTPServer;
  int FTPPort;
  char40 Account;
  char40 Password;
  char40 UploadPath;
  int PASVMode;
  int ProxyType;
  char40 ProxyUserName;
  char40 ProxyPassword;
  int Flag;
}TFTPCfgPkt;

//-----------------------------------------------------------------------------
typedef enum TGroupType{//sizeof 4 Byte
  pt_Cmd,
  pt_PlayLive,
  pt_PlayHistory,
  pt_PlayMedia
}TGroupType;
//-----------------------------------------------------------------------------
typedef enum TFontColor { //未使用			//OSD 字体颜色 sizeof 4 Byte
  cl_Black       =0x00,
  cl_Maroon      =0x01,
  cl_Green       =0x02,
  cl_Olive       =0x03,
  cl_Navy        =0x04,
  cl_Purple      =0x05,
  cl_Teal        =0x06,
  cl_Red         =0x07,
  cl_Lime        =0x08,
  cl_Yellow      =0x09,
  cl_Blue        =0x0a,
  cl_Fuchsia     =0x0b,
  cl_Aqua        =0X0c,
  cl_Gray        =0x0d,
  cl_Silver      =0x0e,
  cl_White       =0x0f,
  cl_Transparent =0xff
}TFontColor;

static int FontColors[16+1] = //未使用
{
  0x000000,//cl_Black=0,//黑
  0x000080,//cl_Maroon=1,//暗红
  0x008000,//cl_Green=2,//深绿
  0x008080,//cl_Olive=3,//土黄
  0x800000,//cl_Navy=4,//深蓝
  0x800080,//cl_Purple=5,//紫
  0x808000,//cl_Teal=6,//深青
  0x0000FF,//cl_Red=7,//红
  0x00FF00,//cl_Lime=8,//绿
  0x00FFFF,//cl_Yellow=9,//黄
  0xFF0000,//cl_Blue=10,//蓝
  0xFF00FF,//cl_Fuchsia=11,//洋红
  0xFFFF00,//cl_Aqua=12,//青
  0x808080,//cl_Gray=13,//深灰
  0xC0C0C0,//cl_Silver=14,//灰
  0xFFFFFF,//cl_White=15,//白
  0xFFFFFF//cl_Transparent=255//透明
};
//-----------------------------------------------------------------------------
typedef enum TResolution {
  D1    = 0,
  HFD1  = 1,
  CIF   = 2,
  QCIF  = 3
}TResolution;
//-----------------------------------------------------------------------------
typedef enum TStandardEx {
  StandardExMin,
  P720x576,//1
  P720x288,
  P704x576,
  P704x288,
  P352x288,
  P176x144,
  N720x480,
  N720x240,
  N704x480,
  N704x240,
  N352x240,
  N176x120,
  V160x120,//  QQVGA
  V320x240,//   QVGA
  V640x480,//    VGA
  V800x600,//   SVGA
  V1024x768,//   XGA  
  V1280x720,//  WXGA yst
  V1280x800,//  WXGA
  //V1280x854,//  WXGA+
  V1280x960,//  _VGA
  V1280x1024,// SXGA
  V1360x768,// WXSGA+
  V1400x1050,// SXGA+
  V1600x1200,// UXGA
  V1680x1050,//WSXGA+
  V1920x1200,//WUXGA
  V2048x1536,// QXGA   
  V2560x1600,//QSXGAW
  V2560x2048,//QSXGA
  V3400x2400,//QUXGAW
  StandardExMax
}TStandardEx;
//-----------------------------------------------------------------------------
typedef enum TVideoType {                        //视频格式sizeof 4 Byte
  MPEG4          =0,
  MJPEG          =1,
  H264           =2,
}TVideoType;
//-----------------------------------------------------------------------------
typedef enum TImgFormat {
  if_RGB           =0,
  if_YUV420        =1,
  if_YUV422        =2,
}TImgFormat;
//-----------------------------------------------------------------------------
typedef enum TStandard {
  NTSC           =0,
  PAL            =1
}TStandard;
//-----------------------------------------------------------------------------
typedef struct TBatchCfgPkt { //批量修改配置 sizeof 16
  int BitRate;                                   
  Byte Standard;                               
  Byte Resolution;                             
  Byte FrameRate;                              
  Byte IPInterval;                             
  int AudioActive;
  int DevTime;
}TBatchCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TVideoFormatEx { //未使用                   //视频格式 sizeof 64 暂未用到
  Byte StandardEx;//TStandardEx
  Byte VideoType;//TVideoType                     //MPEG4=0x00, MJPEG=0x01  H264=0x02
  Byte FrameRate;                                //帧率 1-30 MAX:PAL 25 NTSC 30
  Byte IPInterval;                               //IP帧间隔 1-120 default 30

  Byte Brightness;                               //亮度   0-255
  Byte Contrast;                                 //对比度 0-255
  Byte Hue;                                      //色度   0-255
  Byte Saturation;                               //饱和度 0-255

  char40 Title;                                  //OSD标题 20个汉字

  int  BitRate;                                  //码流 64K 128K 256K 512K 1024K 1536K 2048K 2560K 3072K
  Byte BitRateType;                              //0定码流 1定画质
  bool FlipHorizontal;                           //水平翻转 false or true
  bool FlipVertical;                             //垂直翻转 false or true
  bool ShowOsdInDev;
  
  Byte OsdColor;
  bool IsShowTitle;                            //显示时间标题 false or true
  bool IsShowTime;                               //显示水印 false or true
  bool IsShowBitRate;

  int Flag;
}TVideoFormatEx;
//-----------------------------------------------------------------------------
typedef struct TVideoFormat {                    //视频格式 sizeof 128
  int  Standard;                                 //制式 PAL=1, NTSC=0
  int  Width;                                    //宽
  int  Height;                                   //高 
  TVideoType VideoType;                          //MPEG4=0x00, MJPEG=0x01  H264=0x02
  Byte  Brightness;                               //亮度   0-255
  Byte  Contrast;                                 //对比度 0-255
  Byte  Hue;                                      //色度   0-255
  Byte  Saturation;                               //饱和度 0-255
  int  FrameRate;                                //帧率 1-30 
  int  IPInterval;                               //IP帧间隔 10-120 
  int  BitRateType;                              //0定码流 1定画质
  int  BitRate;                                  //码流 
  int  FlipHorizontal;                           //水平翻转 false or true
  int  FlipVertical;                             //垂直翻转 false or true
  char40 Title;                                  //OSD标题 20个汉字

  Byte TitleFontSize;
  bool ShowTitleInDev;
  bool IsShowTitle;                            //显示时间标题 false or true
  Byte TitleColor;
  short TitleX;
  short TitleY;

  Byte TimeFontSize;
  bool ShowTimeInDev;
  bool IsShowTime;                               //显示水印 false or true
  Byte TimeColor;
  short TimeX;
  short TimeY;
/*
  Byte WaterMarkFontSize;//Reserved
  bool ShowWaterMarkInDev;//Reserved
  bool IsShowWaterMark;//Reserved
  Byte WaterMarkColor;//Reserved
  short WaterMarkX;//Reserved
  short WaterMarkY;//Reserved
*/
  Byte DeInterlaceType;
  bool IsDeInterlace;
  Byte Reserved[6];

  Byte FrameRateFontSize;
  bool ShowFrameRateInDev;
  bool IsShowFrameRate;
  Byte FrameRateColor;
  short FrameRateX;
  short FrameRateY;

  struct { 
    Byte VideoType;//MPEG4=0x0000, MJPEG=0x0001  H264=0x0002
    Byte StandardEx;//TStandardEx
    Byte FrameRate;//帧率 1-30
    Byte BitRateType;//0定码流 1定画质
    int BitRate;//码流
  }Sub;//子码流

  int Flag;
}TVideoFormat;
//-----------------------------------------------------------------------------
typedef struct TVideoCfgPkt {                    //视频设置包 sizeof 148 Byte
  int  Chl;                                      //通道 0 
  int  Active;                                   //是否启动
  Byte InputType;                                //输入类型
  Byte CMOSFreq;                                 
  Byte Reserved[2];
  struct TVideoFormat VideoFormat;               //视频格式
  int Flag;                                     
  int Flag1;
}TVideoCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TAudioType {                        //音频格式sizeof 4 Byte
  AudioTypeNULL         =0x0000,
  PCM                   =0x0001,
  ADPCM                 =0x0002,
  IEEE_FLOAT            =0x0003,
  IBM_CVSD              =0x0005,  
  ALAW                  =0x0006,
  MULAW                 =0x0007,
  OKI_ADPCM             =0x0010,
  DVI_ADPCM             =0x0011,
  MEDIASPACE_ADPCM      =0x0012,  
  SIERRA_ADPCM          =0x0013,  
  G723_ADPCM            =0x0014,
  DIGISTD               =0x0015,  
  DIGIFIX               =0x0016,  
  DIALOGIC_OKI_ADPCM    =0x0017,
  MEDIAVISION_ADPCM     =0x0018,  
  YAMAHA_ADPCM          =0x0020,  
  SONARC                =0x0021,
  DSPGROUP_TRUESPEECH   =0x0022,  
  ECHOSC1               =0x0023,  
  AUDIOFILE_AF36        =0x0024,
  APTX                  =0x0025,  
  AUDIOFILE_AF10        =0x0026,  
  DOLBY_AC2             =0x0030,
  GSM610                =0x0031,
  MSNAUDIO              =0x0032,
  ANTEX_ADPCME          =0x0033,  
  CONTROL_RES_VQLPC     =0x0034,
  DIGIREAL              =0x0035,  
  DIGIADPCM             =0x0036,  
  CONTROL_RES_CR10      =0x0037,
  NMS_VBXADPCM          =0x0038,  
  CS_IMAADPCM           =0x0039,  
  ECHOSC3               =0x003A,
  ROCKWELL_ADPCM        =0x003B,  
  ROCKWELL_DIGITALK     =0x003C,  
  XEBEC                 =0x003D,
  G721_ADPCM            =0x0040,  
  G728_CELP             =0x0041,
  MPEGLAYER2            =0x0050,
  MPEGLAYER3            =0x0055,
  CIRRUS                =0x0060,  
  ESPCM                 =0x0061,  
  VOXWARE_1             =0x0062,
  CANOPUS_ATRAC         =0x0063,  
  G726_ADPCM            =0x0064,  
  G722_ADPCM            =0x0065,
  DSAT                  =0x0066,  
  DSAT_DISPLAY          =0x0067,  
  VOXWARE_2             =0x0075,
  SOFTSOUND             =0x0080,  
  RHETOREX_ADPCM        =0x0100,  
  CREATIVE_ADPCM        =0x0200,
  CREATIVE_FASTSPEECH8  =0x0202,  
  CREATIVE_FASTSPEECH10 =0x0203,  
  QUARTERDECK           =0x0220,
  FM_TOWNS_SND          =0x0300,  
  BTV_DIGITAL           =0x0400,  
  OLIGSM                =0x1000,
  OLIADPCM              =0x1001,  
  OLICELP               =0x1002,  
  OLISBC                =0x1003,
  OLIOPR                =0x1004,  
  LH_CODEC              =0x1100,  
  NORRIS                =0x1400,
}TAudioType;
//-----------------------------------------------------------------------------
typedef struct TAudioFormatEx { //未使用                   //音频格式 = sizeof 8 暂未用到
  Byte AudioType;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
  Byte nChannels;                               //单声道=0 立体声=1
  WORD wBitsPerSample;                          //number of bits per sample of mono data 
  DWORD nSamplesPerSec;                          //采样率 
}TAudioFormatEx;
//-----------------------------------------------------------------------------
typedef struct TAudioFormat {                    //音频格式 = TWaveFormatEx = sizeof 32
  DWORD wFormatTag;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
  DWORD nChannels;                               //单声道=0 立体声=1
  DWORD nSamplesPerSec;                          //采样率 
  DWORD nAvgbytesPerSec;                         //for buffer estimation 
  DWORD nBlockAlign;                             //block size of data 
  DWORD wBitsPerSample;                          //number of bits per sample of mono data 
  DWORD cbSize;                                  //本包大小
  int Flag;                                      
}TAudioFormat;
//-----------------------------------------------------------------------------
typedef struct TAudioCfgPkt {                    //音频设置包 sizeof 48 Byte
  int  Chl;                                      //通道0
  int  Active;                                   //是否启动声音
  struct TAudioFormat AudioFormat;               //音频格式
#define MIC_IN  0
#define LINE_IN 1
  int InputType;                                 //0 MIC输入, 1 LINE输入
  int Flag;                                     
}TAudioCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TPlayCtrl {                         //播放控制sizeof 4 Byte
  PS_None               =0,                 //空
  PS_Play               =1,                 //播放
  PS_Pause              =2,                 //暂停
  PS_Stop               =3,                 //停止
  PS_FastBackward       =4,                 //快退
  PS_FastForward        =5,                 //快进
  PS_StepBackward       =6,                 //步退
  PS_StepForward        =7,                 //步进
  PS_DragPos            =8,                 //拖动
}TPlayCtrl; 
//-----------------------------------------------------------------------------
typedef struct TPlayCtrlPkt {
  TPlayCtrl PlayCtrl;
  DWORD Speed;
  DWORD Pos;
}TPlayCtrlPkt;
//-----------------------------------------------------------------------------
typedef struct TRecFilePkt {                     //播放历史包 SizeOf 100
  char20 DevIP;
  byte Chl;
  byte RecType;                               //0:普通录影 1:警报录影 2媒体文件
  byte Reserved[2];
  int StartTime;                              //开始时间
  int EndTime;                                //结束时间
  char64 FileName;                               //文件名
  int Flag;                                      //保留
}TRecFilePkt;
//-----------------------------------------------------------------------------
//  /sd/20091120/20091120_092749_0.ra2
typedef struct TRecFileIdx {                     //录影文件索引包 sizeof 80
  char64 FileName;
  Byte Chl;
  Byte RecType;
  Byte Reserved;
  Byte Flag;
  time_t StartTime;
  time_t EndTime;
  DWORD FileSize;
}TRecFileIdx;

#define RECFILELSTCOUNT 16
typedef struct TRecFileLst { //sizeof 1288
  int Total;
  int SubTotal;
  TRecFileIdx Lst[RECFILELSTCOUNT];
}TRecFileLst;
//-----------------------------------------------------------------------------
typedef struct TRecFileHead {                    //录影文件头格式 sizeof 256 Byte
  DWORD DevType;                                 //设备类型 = DEV_Ax
  DWORD FileSize;                                //文件大小
  int StartTime;                              //开始时间
  int EndTime;                                //停止时间
  char20 DevName;                                //设备ID
  char20 DevIP;                                  //设备IP
  DWORD VideoChannel;                            //视频通道数统计
  DWORD AudioChannel;                            //音频通道数统计
  struct TVideoFormat VideoFormat;               //视频格式
  struct TAudioFormat AudioFormat;               //音频格式
  int Flag;                                      //保留
  int Flag1;                                      //保留
  int Flag2;                                      //保留
  int Flag3;                                      //保留
  int Flag4;                                      //保留
  int Flag5;                                      //保留
  int Flag6;                                      //保留
  int Flag7;                                      //保留
}TRecFileHead;
//-----------------------------------------------------------------------------
typedef struct TFilePkt {                        //上传文件包 sizeof 272
  int FileType;                                    //1 PTZ协议  2升级镜像 
  DWORD FileSize;
  char256 FileName;
  int Handle;
  int Flag;
  char256 DstFile;
}TFilePkt;
//-----------------------------------------------------------------------------
typedef enum TAlmType {
  Alm_None             =0,//空
  Alm_MotionDetection  =1,//位移报警Motion Detection
  Alm_DigitalInput     =2,//DI报警
  Alm_VideoLost        =3,//视频丢失
  Net_Disconn          =4,//网络断线
  Net_ReConn           =5,//网络重连
  Alm_HddFill          =6,//磁满
  Alm_VideoBlind       =7,//视频遮挡
  Alm_Other2           =8,//其它报警2
  Alm_Other3           =9,//其它报警3
  Alm_Other4           =10,//其它报警4
  Alm_Other5           =11,//其它报警5
  Alm_OtherMax         =12    
}TAlmType;              

typedef struct TAlmSendPkt {                     //警报上传包sizeof 36
  TAlmType AlmType;                              //警报类型
  int AlmTime;                                //警报时间
  int AlmPort;                                   //警报端口
  char20 DevIP;
  int Flag;                                      //MD 区域索引
}TAlmSendPkt;
//-----------------------------------------------------------------------------
typedef struct TDiCfgPkt {                     //DI状态包 sizeof 264
  int UsedCount;
  struct {
    bool Active;                                 //DI是否打开 常闭=0，常开=1
    Byte InputType;                              // 0 低电平　1 高电平 
    Byte Tag;
    Byte Tag1;
  }Lst[64];
  int Flag;
}TDiCfgPkt;

typedef struct TDoCfgPkt {                        //do配置包　sizeof 264
  int UsedCount;
  struct {
    bool Active;                                 //DO是否打开
    Byte OutType;                                // 0 低电平　1 高电平
    Byte Tag;
    Byte Tag1;
  }Lst[64];
  int Flag;
}TDoCfgPkt;

typedef struct TDiDoCfgPkt {                    //sizeof 528
  struct TDiCfgPkt DiCfgPkt;
  struct TDoCfgPkt DoCfgPkt;
}TDiDoCfgPkt;

typedef struct TDoControlPkt {                 //do控制包　sizeof 16
  int Channel;
  int Value;                                   // 0 关　1 开
  int Flag;
  int Flag1;
}TDoControlPkt;
//-----------------------------------------------------------------------------
typedef enum TTaskDayType{w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7,w_Interval} TTaskDayType;
//typedef enum TTaskDayType{w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7} TTaskDayType;
typedef struct TTaskhm {
  //TTaskDayType w;//w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7
  Byte w;//w_close,w_1,w_2,w_3,w_4,w_5,w_6,w_7,w_1_5,w_1_6,w_6_7,w_1_7,w_Interval
  Byte Days;
  Byte Reserved[2];
  Byte start_h;//时 0-23
  Byte start_m;//分 0-59
  Byte stop_h;//时 0-23
  Byte stop_m;//分 0-59
}TTaskhm;

#define MDLSTCOUNT                3
typedef struct TMDCfgPkt {                       //移动侦测包 sizeof 96
  int Chl;                                      
  int Active;                                    //false or true
  struct {
    bool Active;
    Byte Reserved;
    Byte Sensitive;                              //侦测灵敏度 0-255
    Byte Tag;
    RECT1 Rect1;                        //侦测区域 sizeof 8
  }Lst[MDLSTCOUNT];
  struct TTaskhm hm[MDLSTCOUNT];
  int Flag;
}TMDCfgPkt;
//-----------------------------------------------------------------------------
#define ALMCFGLST       16
typedef struct TAlmCfgPkt {                   //警报配置包 sizeof 268
  int AlmOutTimeLen;                    //报警输出时长(秒) 0 ..600 s
  int AutoClearAlarm;
  int Flag;
  struct {
    Byte AlmType;//Byte(TAlmType)
    Byte Channel;
    bool ActiveBuzzer;
    bool IsAlmRec;
    bool NetSend;
    bool ActiveDO;//DI关联DO通道 false close
    Byte DOChannel;// 1-255 do channel
    Byte GotoPTZPoint;// >0为预设位
  }Lst[ALMCFGLST];
  struct TTaskhm hm[ALMCFGLST];
}TAlmCfgPkt;
//-----------------------------------------------------------------------------
#define USER_GUEST     1 
#define USER_OPERATOR  2
#define USER_ADMIN     3
#define GROUP_GUEST    1
#define GROUP_OPERATOR 2
#define GROUP_ADMIN    3

#define MAXUSERCOUNT             20              //最大用户数量
typedef struct TUserCfgPkt {                     //sizeof 1048
  int Count;
  struct {
    int UserGroup;                                 //Guest=1 Operator=2 Administrator=3
    int Authority;                                 //3为admin ,
    char20 UserName;                               //用户名 admin不能更改
    char20 Password;                               //密码
    int Flag;
  }Lst[MAXUSERCOUNT];
  int Flag;
}TUserCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TLogPkt {                         //sizeof 112
  int LogType;                                   //日志类型 所有=0 操作=1 普通=2 警报=3
  int LogID;                                     //日志编号
  int LogTime;                                   //操作时间
  char20 OperatUser;                             //操作用户
  char80 Content;                                //内容
}TLogPkt;
//-----------------------------------------------------------------------------
typedef struct TLogSearchPkt {                   //日志查询包 sizeof 16
  int LogType;                                   //日志类型 所有=0 操作=1 普通=2 警报=3
  int StartTime;                       
  int EndTime;
  int Flag;
}TLogSearchPkt;
//-----------------------------------------------------------------------------
typedef struct TLogLstPkt {                  //日志包列表 sizeof 1352
  int Total;
  int SubTotal;
  TLogPkt Lst[12];
}TLogLstPkt;
//-----------------------------------------------------------------------------
typedef enum TPTZCmd {                           //sizeof 4 Byte
  PTZ_None,
  PTZ_Up,//上
  PTZ_Up_Stop,//上停止
  PTZ_Down,//下
  PTZ_Down_Stop,//下停止
  PTZ_Left,//左
  PTZ_Left_Stop,//左停止
  PTZ_Right,//右
  PTZ_Right_Stop,//右停止

  PTZ_LeftUp,//左上
  PTZ_LeftUp_Stop,//左上停止
  PTZ_RightUp,//右上
  PTZ_RightUp_Stop,//右上停止
  PTZ_LeftDown,//左下
  PTZ_LeftDown_Stop,//左下停止
  PTZ_RightDown,//右下
  PTZ_RightDown_Stop,//右下停止

  PTZ_IrisIn,//光圈小
  PTZ_IrisInStop,//光圈停止
  PTZ_IrisOut,//光圈大
  PTZ_IrisOutStop,//光圈停止

  PTZ_ZoomIn,//倍率小
  PTZ_ZoomInStop,//倍率停止
  PTZ_ZoomOut,//倍率大
  PTZ_ZoomOutStop,//倍率停止

  PTZ_FocusIn,//焦距小
  PTZ_FocusInStop,//焦距停止
  PTZ_FocusOut,//焦距大
  PTZ_FocusOutStop,//焦距停止

  PTZ_LightOn,//灯光小
  PTZ_LightOff,//灯光大
  PTZ_RainBrushOn,//雨刷开
  PTZ_RainBrushOff,//雨刷开
  PTZ_AutoOn,//自动开始  //Rotation
  PTZ_AutoOff,//自动停止

  PTZ_TrackOn,
  PTZ_TrackOff,
  PTZ_IOOn,
  PTZ_IOOff,

  PTZ_ClearPoint,//云台复位
  PTZ_SetPoint,//设定云台定位
  PTZ_GotoPoint,//云台定位
  PTZ_SetPointRotation,
  PTZ_SetPoint_Left,
  PTZ_GotoPoint_Left,
  PTZ_SetPoint_Right,
  PTZ_GotoPoint_Right,
  PTZ_DayNightMode,//白天、夜光模式 0白天 1夜光
  PTZ_Max
}TPTZCmd;
//-----------------------------------------------------------------------------
typedef enum TPTZProtocol {                      //云台协议 sizeof 4
  Pelco_P               =0,
  Pelco_D               =1,
  Protocol_Custom       =2,
}TPTZProtocol;

typedef struct TPTZPkt {                         //PTZ 云台控制  sizeof 108
  TPTZCmd PTZCmd;                                    //=PTZ_None 为透明传输
  union {
    struct {
      TPTZProtocol Protocol;                         //云台协议
      int Address;                                   //云台地址
      int PanSpeed;                                  //云台速度
      int Value;                                     //保留或预设位
    };
    struct {
      char100 TransBuf;
      int TransBufLen;
    };
  };
}TPTZPkt;
//-----------------------------------------------------------------------------
typedef struct TPlayLivePkt {                    //播放现场包//sizeof 20
  DWORD VideoChlMask;//通道掩码 
  DWORD AudioChlMask;
  int Value;                                     //Value=0发送所有帧，Value=1只发送视频I帧
  DWORD SubVideoChlMask;
//11  int IsRecvAlarm;                               //0接收设备警报 1不接收设备警报
  int Flag;                                      //保留 
}TPlayLivePkt;
//-----------------------------------------------------------------------------
typedef struct TPlayBackPkt {                    //sizeof 20
  int Chl;
  int FileType;                                  //0:普通录影 1:警报录影 2媒体文件
  int StartTime;                                 //开始时间
  int EndTime;                                   //结束时间
  int Flag;
}TPlayBackPkt;
//-----------------------------------------------------------------------------
typedef enum TMsgID {
  Msg_None                  = 0,
  Msg_Login                 = 1001,//用户登录
  Msg_PlayLive              = 1002,//开始播放现场
  Msg_StartPlayRecFile      = 1003,//播放录影文件
  Msg_StopPlayRecFile       = 1004,//停止播放录影文件
  Msg_StartRec              = 1007,//开始设备录影
  Msg_StopRec               = 1008,//停止设备录影
  Msg_GetRecFileLst         = 1009,//取得录影文件列表
  Msg_StartDownloadFile     = 1011,//开始下载文件
  Msg_StopDownloadFile      = 1012,//停止下载文件
  Msg_StartUploadFile       = 1013,//开始上传文件
  Msg_AbortUploadFile       = 1014,//取消上传文件
  Msg_StartTalk             = 1015,//开始对讲
  Msg_StopTalk              = 1016,//停止对讲
  Msg_PlayControl           = 1017,//播放控制
  Msg_PTZControl            = 1018,//云台控制
  Msg_Alarm                 = 1020,//警报
  Msg_ClearAlarm            = 1021,//关闭警报
  Msg_SetBrightness         = 1022,//设置亮度
  Msg_SetContrast           = 1023,//设置对比度
  Msg_SetHue                = 1024,//设置色度
  Msg_SetSaturation         = 1025,//设置饱和度
  Msg_GetTime               = 1026,//取得时间
  Msg_SetTime               = 1027,//设置时间
  Msg_GetDevInfo            = 1028,//取得设备信息
  Msg_SetDevInfo            = 1029,//设置设备信息
  Msg_GetNetCfg             = 1030,//取得网络配置
  Msg_SetNetCfg             = 1031,//设置网络配置
  Msg_GetAlmCfg             = 1032,//取得Alarm配置
  Msg_SetAlmCfg             = 1033,//设置Alarm配置
  Msg_GetRecCfg             = 1034,//取得录影配置
  Msg_SetRecCfg             = 1035,//设置录影配置
  Msg_GetVideoCfg           = 1036,//取得视频配置
  Msg_SetVideoCfg           = 1037,//设置视频配置
  Msg_GetAudioCfg           = 1038,//取得音频配置
  Msg_SetAudioCfg           = 1039,//设置音频配置
  Msg_GetMDCfg              = 1040,//移动侦测配置
  Msg_SetMDCfg              = 1041,//移动侦测配置
  Msg_GetHideArea           = 1042,//秘录
  Msg_SetHideArea           = 1043,//秘录
  Msg_DeleteFile            = 1044,//删除文件
  Msg_GetLogLst             = 1045,//取得日志列表
  Msg_DelLog                = 1046,//删除日志
  Msg_SetDevReboot          = 1047,//重启设备
  Msg_SetDevLoadDefault     = 1048,//系统回到缺省配置 Pkt.Value= 0 不恢复IP, Pkt.Value= 1 恢复IP
  Msg_SendSense             = 1051,//1.本地：接收appsense侦测和回复appsense, 
  Msg_GetDevRecFileHead     = 1052,//取得设备文件文件头信息
  Msg_GetDevState           = 1053,//取得系统状态
  Msg_GetUserLst            = 1054,//取得用户列表
  Msg_SetUserLst            = 1055,//设置用户列表
  Msg_GetRS485Cfg           = 1065,
  Msg_SetRS485Cfg           = 1066,
  Msg_GetDiDoCfg            = 1067,
  Msg_SetDiDoCfg            = 1068,
  Msg_GetFTPCfg             = 1069,
  Msg_SetFTPCfg             = 1070,
  Msg_KillUserConn          = 1071,//断开用户连接
  Msg_GetSMTPCfg            = 1072,
  Msg_SetSMTPCfg            = 1073,
  Msg_GetAllCfg             = 1081,//取得所有配置
  Msg_GetWiFiCfg            = 1082,//取得WiFi配置
  Msg_SetWiFiCfg            = 1083,//设置WiFi配置
  Msg_GetDiskCfg            = 1084,//设置Disk配置
  Msg_SetDiskCfg            = 1085,//设置Disk配置
  Msg_DevSnapShot           = 1100,//设备拍照
  Msg_GetColors             = 1101,//取得亮度、对比度、色度、饱和度
  Msg_SetColors             = 1102,//设置亮度、对比度、色度、饱和度
  //Msg_SetColorDefault       = 1103,
  Msg_StartUploadFileEx     = 1113,//开始上传文件tftp
  Msg_GetMulticastInfo      = 2001,
  Msg_SetMulticastInfoOld   = 2002,
  Msg_SetMulticastInfo      = 2003,
  Msg_SetBatchCfg           = 2004,//批量修改配置
  Msg_Sense                 = 2010,
  Msg_Debug                 = 3001,//调试
  Msg_______
}TMsgID;
//-----------------------------------------------------------------------------
#define RECPLANLST 4
typedef struct TPlanRecPkt {                        //排程录影结构 sizeof 224
  struct {
    bool Active;
    Byte start_h;    //时 0-23
    Byte start_m;    //分 0-59
    Byte stop_h;     //时 0-23
    Byte stop_m;     //分 0-59
    bool IsRun;      //当前计划是否启动
    Byte Flag1;
    Byte Flag2;
  }Week[7][RECPLANLST];                                 //日一二三四五六 每天最多4个任务
}TPlanRecPkt;
//-----------------------------------------------------------------------------
typedef enum TRecStyle {
  rs_RecManual,
  rs_RecAuto,
  rs_RecPlan,
  rs_RecAlarm
}TRecStyle;

typedef struct TRecCfgPkt {                      //录影配置包 sizeof 260
  int ID;
  int DevID;//PC端管理软件只用于存储数据库中设备编号　设备端保留
  int Chl;
  bool IsLoseFrameRec;//是否丢帧录影
  byte RecStreamType;//0 主码流 1 次码流
  byte Reserved;
  bool IsRecAudio;//录制音频 暂没有用到
  DWORD Rec_AlmPrevTimeLen;//警前录影时长     5 s
  DWORD Rec_AlmTimeLen;//警后录影时长        10 s
  DWORD Rec_NmlTimeLen;//一般录影分割时长   600 s
  TRecStyle RecStyle;//录影类型
  TPlanRecPkt Plan;
  int bFlag;
}TRecCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TDiskCfgPkt {   //sizeof 888
  int IsFillOverlay;      // 是否覆盖早期文件(false或true,false为不覆盖,true为覆盖,缺省为false)
  char20 CurrentDiskName; // 当前正在录影的磁盘索引 0..7, ReadOnly
  struct {
    char20 DiskName;      // 磁盘 
    int Active;           // 是否做为录影磁盘 false or true
    DWORD DiskSize;       // M ReadOnly
    DWORD FreeSize;       // M
    DWORD MinFreeSize;    // M
  }Disk[24];
}TDiskCfgPkt;
//-----------------------------------------------------------------------------
#define HIDEAREALSTCOUNT          3
typedef struct THideAreaCfgPkt {                 //隐藏录影区域包 sizeof 72
  int Chl;                                       //通道 0..15 对应 1..16通道
  int Active;
  struct {
    int Active;                                    //false or true
    RECT1 Rect1;
  }Lst[HIDEAREALSTCOUNT];
  int Flag;
}THideAreaCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TLanguage {
  cn = 0,
  tw = 1,
  en = 2 
}TLanguage;
static const  char* DevLanguage[3] = {"cn","tw","en"};
//-----------------------------------------------------------------------------
typedef struct TAxInfo {//sizeof 40
  union {
    char40 Reserved;
    struct {
      bool ExistWiFi;
      bool ExistSD;
      bool ExistUSB;
      bool ExistHD;       // 4
      DWORD VideoTypeMask;// 8
      uint64 StandardMask;//16
      DWORD AutioTypeMask;//20暂未用到
      bool NotExistAudio;
      bool NotExistIO;
      bool NotExistRS485;
      byte flag;
      uint64 SubStandardMask;//32
    };
  };
}TAxInfo;

#pragma pack(4)
typedef struct TDevInfoPkt {                     //设备信息包sizeof 180
  char DevModal[12];                             //设备型号 
  DWORD SN;
  int DevType;                                   //设备类型
  char20 SoftVersion;                            //软件版本
  char20 FileVersion;                            //文件版本
  char20 DevName;                                //设备标识
  char40 DevDesc;                                //设备备注
  struct TAxInfo Info;

  int VideoChlCount;
  Byte AudioChlCount;
  Byte DiChlCount;
  Byte DoChlCount;
  Byte RS485DevCount;
  Byte Language;//TLanguage
  Byte MaxUserConn;                               //最大用户连接数 default 10
  Byte OEMType;
  bool DoubleStream;                              //是否双码流 
  struct {
    Byte w;//TTaskDayType;
    Byte start_h;//时 0-23
    Byte start_m;//分 0-59
    Byte Days;
  }RebootHM;
  int Flag;
}TDevInfoPkt;
#pragma pack()
//-----------------------------------------------------------------------------
typedef struct TUserConnInfoPkt {//设备端用户连接信息 sizeof 12
  int LoginTime;
  DWORD ClientIP;
  WORD ClientPort;
  Byte GroupType;//(Byte)TGroupType
  Byte UserGroup;//Guest=1 Operator=2 Administrator=3
}TUserConnInfoPkt;
//-----------------------------------------------------------------------------
typedef struct TDevStatePkt {                    //设备状态信息包 1020
  struct {  //硬盘信息
    Byte DiskType;                                  //1=HD  2=SD 3=USB
    Byte Reserved;
    WORD Tag;
    DWORD TotalSpace;                              //硬盘大小(M)
    DWORD FreeSpace;                               //硬盘剩余空间( M )
  }HD[4];
  int ConnCount;//连接用户数量
  TUserConnInfoPkt UserConnInfoPkt[80];         //连接用户信息
  //int Flag;                                      //保留
  //int Flag1;                                     //保留
  int LastVideoDataTime;
  int LastAudioDataTime;
  int NetworkOK;
  //char20 wIP;
}TDevStatePkt;
//-----------------------------------------------------------------------------
typedef struct TWiFiCfgPkt {                     //无线配置包 sizeof 200
  int Active;
  char20 DevIP;
  char20 SubMask;
  char20 Gateway;
  char32 SSID;
  int Channel;//频道1..14 default 1=Auto
#define Encrypt_None   0
#define Encrypt_WEP    1
#define Encrypt_WPA    2
  int EncryptType;//(Encrypt_None,Encrypt_WEP,Encrypt_WPA);
  char64 Password;
  union {
    struct {
      char32 ValueStr;
    };
    struct {
      int WEPKeyBit;//(kBit64,kBit128);
      int WEPIndex;//0..3;//=0
    };
    struct {
      int WPAKeyType;//(TKIP,AES);
    };
  };
}TWiFiCfgPkt;
//-----------------------------------------------------------------------------
typedef struct TNetCfgPkt {                      //设备网络配置包sizeof 372
  int CmdPort;                                   //命令数据端口
  int rtspPort;                                  //rtsp端口
  int HttpPort;                                  //http网页端口
  /*WORD CmdPort;                                   //命令数据端口
  WORD wCmdPort;
  WORD rtspPort;                                  //rtsp端口
  WORD wrtspPort;
  WORD HttpPort;                                  //http网页端口
  WORD wHttpPort;*/
  struct {
    int IPType;               // 0=静态IP(StaticIP) 1=动态IP(DHCP)
    char20 DevIP;
    char20 DevMAC;
    char20 SubMask;
    char20 Gateway;
    char20 DNS1;
    char20 DNS2;
    //char20 wIP;
    int Flag;
  }Lan;
  struct {
    int Active;
    int DDNSType;                               //0=3322.ORG 1=dynDNS.ORG
    char40 DDNSDomain;                           //或DDNS SERVER IP
    char40 HostAccount;                          //DDNS帐号
    char40 HostPassword;                         //DDNS密码
    int Flag;
  }DDNS;
  struct {
    int AutoStart;
    char40 Account;
    char40 Password;
    int Flag;
  }PPPOE;
  struct {
    int Active;
    int Flag;
  }uPnP;
  int Flag;
}TNetCfgPkt;
//-----------------------------------------------------------------------------
typedef enum TBaudRate{
  BaudRate_1200  =    1200,
  BaudRate_2400  =    2400,
  BaudRate_4800  =    4800,
  BaudRate_9600  =    9600,
  BaudRate_19200  =  19200,
  BaudRate_38400  =  38400,
  BaudRate_57600  =  57600,
  BaudRate_115200 = 115200
}TBaudRate;

typedef enum TDataBit{
  DataBit_5 = 5,
  DataBit_6 = 6,
  DataBit_7 = 7,
  DataBit_8 = 8
}TDataBit;

typedef enum TParityCheck{
  ParityCheck_None  = 0,
  ParityCheck_Odd   = 1,
  ParityCheck_Even  = 2,
  ParityCheck_Mask  = 3,
  ParityCheck_Space = 4
}TParityCheck;

typedef enum TStopBit{
  StopBit_1   = 0,
  StopBit_1_5 = 1,
  StopBit_2   = 2
}TStopBit;

typedef struct TRS485CfgPkt {                       //485通信包 sizeof 280
  int Chl;
  TBaudRate BPS;//波特率
  TDataBit DataBit;//数据位
  TParityCheck ParityCheck;//奇偶校验
  TStopBit StopBit;//停止位
  struct {
    Byte Address;
    Byte PTZProtocol;//云台协议
    Byte PTZSpeed;
    Byte Reserved;
  }Lst[32];//对应相应的视频通道
  
  //char PTZNameLst[128];//暂时未用到 format "Pelco_P\nPelco_D\nProtocol_Custom"

  int PTZCount;
  char20 PTZNameLst[6];
  int Reserved;

  int Flag;
}TRS485CfgPkt;

//-----------------------------------------------------------------------------
typedef struct TColorsPkt {
  int Chl;
  Byte  Brightness;                               //亮度   0-255
  Byte  Contrast;                                 //对比度 0-255
  Byte  Hue;                                      //色度   0-255
  Byte  Saturation;                               //饱和度 0-255
}TColorsPkt;
//-----------------------------------------------------------------------------
typedef struct TMulticastInfoPkt {               //多播发送信息包sizeof 556->588
  TDevInfoPkt DevInfo;
  TNetCfgPkt NetCfg;
  int Flag;
  struct {
    int  Standard;                                 //制式 PAL=1, NTSC=0
    int  Width;                                    //宽 720 360 180 704 352 176 640 320 160
    int  Height;                                   //高 480 240 120 576 288 144 
    TVideoType VideoType;                          //MPEG4=0x00, MJPEG=0x01  H264=0x02
  }v;
  struct {
    DWORD wFormatTag;                              //PCM=0X0001, ADPCM=0x0011, MP2=0x0050, MP3=0X0055, GSM610=0x0031
    DWORD nChannels;                               //单声道=0 立体声=1
    DWORD nSamplesPerSec;                          //采样率 
    DWORD wBitsPerSample;                          //number of bits per sample of mono data 
  }a;
  TWiFiCfgPkt WiFiCfg;
}TMulticastInfoPkt;
//-----------------------------------------------------------------------------
#define Head_CmdPkt           0xAAAAAAAA         //命令包包头
#define Head_VideoPkt         0xBBBBBBBB         //视频包包头
#define Head_AudioPkt         0xCCCCCCCC         //音频包包头
#define Head_TalkPkt          0xDDDDDDDD         //对讲包包头
#define Head_UploadPkt        0xEEEEEEEE         //上传包
#define Head_DownloadPkt      0xFFFFFFFF         //下载包
#define Head_CfgPkt           0x99999999         //配置包
#define Head_SensePkt         0x88888888         //侦测包
//-----------------------------------------------------------------------------

#pragma pack(4)
typedef struct THeadPkt{                         //sizeof 8
  DWORD VerifyCode;                              //校验码 = 0xAAAAAAAA 0XBBBBBBBB 0XCCCCCCCC 0XDDDDDDDD 0XEEEEEEEE
  DWORD PktSize;                                 //本包大小=1460-8(waiting)
}THeadPkt;

#pragma pack()
//-----------------------------------------------------------------------------
typedef struct TFrameInfo { //录影文件数据帧头  16 Byte
  Int64 FrameTime;                               //帧时间，time_t*1000000 +us
  Byte Chl;                                      //通道 0
  bool IsIFrame;                                 //是否I帧
  WORD FrameID;                                  //帧索引,从0 开始,到65535，周而复始
  union {
    DWORD PrevIFramePos;                         //前一个I帧文件指针，用于文件中处理或网络包发送
    int StreamType;                              //0为主码流 1为次码流 
    DWORD DevID;                                 //单连接多设备时用到，暂保留
  };
}TFrameInfo;

typedef struct TDataFrameInfo { //录影文件数据帧头  24 Byte
  THeadPkt Head;
  TFrameInfo Frame;
}TDataFrameInfo;
//-----------------------------------------------------------------------------
//错误代码
#define ERR_FAIL           0
#define ERR_OK             1
#define ERR_MAXUSERCONN    10001//连接用户数超过最大设定
//-----------------------------------------------------------------------------
typedef struct TLoginPkt {                       //用户登录包 sizeof 252->892
  char20 UserName;                               //用户名称
  char20 Password;                               //用户密码
  char20 DevIP;                                  //要连接的设备IP,或 host
  int UserGroup;                                 //Guest=1 Operator=2 Administrator=3  
  int SendSensePkt;                              //是否发送侦测包 0不发送 1发送
  TDevInfoPkt DevInfoPkt;
  TVideoFormat v[4];
  TAudioFormat a[4];
  int Flag;//返回是否在线　0不在线　1在线
}TLoginPkt;
//-----------------------------------------------------------------------------
typedef struct TCmdPkt {                         //sizeof 1460-8
  DWORD PktHead;                                 //包头校验码 =Head_CmdPkt 0xAAAAAAAA
  TMsgID MsgID;                                  //消息
  DWORD Session;                                 //网络用户许可，当发送网络登录包时此值为0，等于返回登录包的Session  //当包为程序内部通讯包时，此值忽略
  DWORD Value;                                   //属性或返回值 0 or 1 or ErrorCode
  union {
    char ValueStr[1460- 4*4 - 8];
    struct TLoginPkt LoginPkt;                   //登录包
    struct TPlayLivePkt LivePkt;                 //播放现场包
    struct TRecFilePkt RecFilePkt;               //回放录影包
    struct TPTZPkt PTZPkt;                       //云台控制台
    struct TRecFileLst RecFileLst;
    struct TPlayCtrlPkt PlayCtrlPkt;             //回放录影控制包

    struct TAlmSendPkt AlmSendPkt;               //警报上传包
    struct TDevInfoPkt DevInfoPkt;               //设备信息包
    struct TLogSearchPkt LogSearchPkt;           //日志查询包
    struct TLogLstPkt LogLst;                    //日志包列表
    struct TNetCfgPkt NetCfgPkt;                 //设备网络配置包
    struct TWiFiCfgPkt WiFiCfgPkt;               //无线网络配置包
    struct TDiskCfgPkt DiskCfgPkt;               //磁盘配置包
    struct TUserCfgPkt UserCfgPkt;               //用户配置包
    struct TRecCfgPkt RecCfgPkt;                 //录影配置包
    struct TMDCfgPkt MDCfgPkt;                   //移动侦测包--单通道
    struct TDiDoCfgPkt DiDoCfgPkt;               //DIDO配置包
    struct TDoControlPkt DoControlPkt;           //DO控制包    
    struct THideAreaCfgPkt HideAreaCfgPkt;       //隐藏录影区域包--单通道
    struct TAlmCfgPkt AlmCfgPkt;                 //警报配置包
    struct TVideoCfgPkt VideoCfgPkt;             //视频配置包--单通道
    struct TAudioCfgPkt AudioCfgPkt;             //音频配置包--单通道    
    struct TRecFileHead FileHead;                //取得设备文件文件头信息
    struct TDevStatePkt DevStatePkt;             //设备状态信息包
    struct TFilePkt FilePkt;                     //上传文件包
    struct TRS485CfgPkt RS485CfgPkt;             //485通信包--单通道
    struct TUserConnInfoPkt UserConnInfoPkt;     //现主要用于断开某些用户连接
    struct TColorsPkt Colors;                    //设置取得亮度、对比度、色度、饱和度

    struct TMulticastInfoPkt MulticastInfo;      //多播信息

    struct TFTPCfgPkt FTPCfgPkt;
    struct TSMTPCfgPkt SMTPCfgPkt;
    struct TBatchCfgPkt BatchCfgPkt;             //批量修改配置
  };
}TCmdPkt;
//-----------------------------------------------------------------------------
typedef struct TNetCmdPkt {                      //网络发送包 sizeof 1460
  struct THeadPkt HeadPkt;
  struct TCmdPkt CmdPkt;
}TNetCmdPkt;

//#pragma pack(pop)
//-----------------------------------------------------------------------------
typedef struct TTalkHeadPkt {                    //对讲包包头  sizeof 32
  DWORD VerifyCode;                              //校验码 = 0XDDDDDDDD
  DWORD PktSize;                                 
  char20 TalkIP;
  DWORD TalkPort;
}TTalkHeadPkt;

//*****************************************************************************
//*****************************************************************************
//**DECODER PROTOCOL***DECODER PROTOCOL***DECODER PROTOCOL***DECODER PROTOCOL**
//*****************************************************************************
//*****************************************************************************

typedef enum TdecMsgID{//解码命令 sizeof 4 Byte
  decmsg_None,               //空
  decmsg_Login,              //登录
  decmsg_Sense,              //侦测包
  decmsg_Reboot,             //重启设备
  decmsg_Default,            //系统回到缺省配置
  decmsg_SearchencDev,       //查询A1, A3, A4播放设备
  decmsg_GetTime,            //取得时间
  decmsg_SetTime,            //设置时间
  decmsg_GetDevCfg,          //取得设备配置
  decmsg_SetDevCfg,          //设置设备配置
  decmsg_GetPlayCfg,         //取得播放配置
  decmsg_SetPlayCfg,         //设置播放配置
  decmsg_PTZControl,         //云台控制
  decmsg_TransSend,          //透明传输命令(保留)
  decmsg_Ping,               //远程ping
  decmsg_SetSplit,           //分割屏幕
  decmsg_SelectSplit,        //选择分割窗口
  decmsg_StartTalk,          //开始对讲
  decmsg_Talking,            //对讲中
  decmsg_StopTalk,           //停止对讲
  decmsg_DownloadFile,       //开始下载文件
  decmsg_AbortDownloadFile,  //取消下载文件
  decmsg_UploadFile,         //开始上传文件
  decmsg_AbortUploadFile,    //取消上传文件
  //decmsg_Play,               //开始播放
  //decmsg_Pause,              //暂停播放
  //decmsg_Stop,               //停止播放
  //decmsg_PlayPrev,           //播放前一任务
  //decmsg_PlayNext,           //播放后一任务
  //decmsg_PlayIndex,          //播放指定任务
  //decmsg_StartInsPlay,       //开始插入播放
  //decmsg_StopInsPlay,        //停止插入播放
  decmsg_None_max
}TdecMsgID;

typedef struct TdecHeadPkt{//解码命令包头 sizeof 16
  DWORD VerifyCode;          //校验码 = 0xAAAAAAAA
  TdecMsgID MsgID;           //消息
  DWORD Value;               //保存一些数据
  int NextSize;              //后续包大小
}TdecHeadPkt;

typedef struct TencDevInfo{//A1 A3 A4设备信息 sizeof 156
  DWORD encDevID;            // = SN ?
  char40 DevHost;            //待播放设备IP或域名
  char40 SvrHost;            //转发服务器IP或域名
  char20 DevName;            //设备描述
  char20 UserName;           //待播放设备帐号
  char20 Password;           //待播放设备密码
  WORD DataPort;             //待播放设备端口
  Byte DevType;              //待播放设备TDevType
  Byte Channel;              //联接通道
  TStandardEx Standard;
  int Reserved;
}TencDevInfo;

typedef struct TPlayItem { //sizeof 36
  DWORD PlayTime;//秒
  struct {
    DWORD encDevID;
    bool IsFullScreen;
    bool IsPlayAudio;
    WORD Reserved;
  }Play[4];
}TPlayItem;

typedef struct TdecPlayCfg {//sizeof 16
  DWORD DevCount;
  struct TencDevInfo* DevLst;
  DWORD ItemsCount;
  struct TPlayItem* Items;
}TdecPlayCfg;

typedef struct TdecDevCfg{ //设备信息包sizeof 1048
  struct {
    DWORD SN;                  //序列号
    Byte DevType;              //TDevType
    Byte ChlCount;             //通道数量
    Byte OEMType;              //OEM版本类型
    Byte Language;             //TLanguage;
    TVersion SoftVersion;      //软件版本
    TVersion FileVersion;      //文件版本
    char20 DevName;            //设备名称
    Byte Reserved[8];
  }Info;
  struct {
    WORD CmdPort;              //命令数据端口
    WORD HttpPort;             //http网页端口
    char20 DevIP;              //设备IP
    char20 DevMAC;             //设备MAC地址
    char20 SubMask;            //设备子网掩码
    char20 Gateway;            //设备网关
    char20 DNS1;               //设备DNS
  }Net;
  struct {
    Byte w;                    //TTaskDayType;
    Byte start_h;              //当w!=w_Interval时值为0-23, 当w==w_Interval时为间隔小时
    Byte start_m;              //分0-59
    Byte tag;                  //保留
  }RebootHM;                 //定时重启
  struct {
    int Count;                 //用户数量 = Lst中数量
    struct {
      char20 UserName;           //用户名 admin用户不能更改
      char20 Password;           //密码
      DWORD Authority;           //Guest=1 Operator=2 Administrator=3
    }Lst[MAXUSERCOUNT];
  }User;
  int Flag;                  //保留
}TdecDevCfg;

typedef struct TdecLoginPkt {//用户登录包 sizeof 48
  char20 UserName;           //用户名称
  char20 Password;           //用户密码
  int Flag;
  int Reserved;
}TdecLoginPkt;

typedef struct TdecPingPkt { //远程PING包  sizeof 48
  char40 RemoteIP;
  DWORD RemotePort;          //为0时为ICMP PING, 大于0时为TCP PING
  DWORD TimeOut;             //超时,单位ms
}TdecPingPkt;

typedef struct TdecSearchPkt {//查询包 sizeof 1064
  TdecHeadPkt Head;
  TdecDevCfg Cfg;
}TdecSearchPkt;

#pragma option pop //end C++Builder enum 4 Byte

#endif //end Ax_protocol_H

