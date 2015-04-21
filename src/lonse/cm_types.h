#ifndef cm_types_H
#define cm_types_H

//#ifndef WIN32
#ifdef linux
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/timeb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>
#include <getopt.h>
#include <linux/rtc.h>
#include <termios.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#include <linux/videodev.h>
#include <linux/soundcard.h>
#include <linux/fb.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <mntent.h>
#include <sys/statfs.h>


#ifndef __cplusplus
#include <stdbool.h>
#endif

#include <stdarg.h>

#else

#include <Windows.h>

#ifndef __cplusplus
#define bool unsigned char
#define true  1
#define false 0
#endif
#endif

#ifdef linux
#define Byte unsigned char
#define byte Byte
#define BYTE Byte
#define word unsigned short
#define WORD word
#define DWORD unsigned int
#define uint  unsigned int
#define dword DWORD
#define int16 short
#define uint16 word
#define uint32_t DWORD
#define int64 long long
#define Int64 int64
#define uint64 unsigned long long
#define pchar char*
#define pChar pchar
#else
#define Byte byte
#define word unsigned short
#define DWORD unsigned int
#define uint  unsigned int
#define int16 short
#define uint16 word
#define uint32_t DWORD
#define int64 DWORD64
#define Int64 int64
#define uint64 DWORD64
#define pchar char*
#define pChar pchar
#define time_t int
#endif

#define TDateTime double
typedef char char2[2];
typedef char char4[4];
typedef char char8[8];
typedef char char16[16];
typedef char char20[20];
typedef char char32[32];
typedef char char40[40];
typedef char char48[48];
typedef char char50[50];
typedef char char64[64];
typedef char char80[80];
typedef char char100[100];
typedef char char256[256];
typedef char char1024[1024];
typedef char char4096[1024*4];
typedef char char8192[1024*8];
typedef char8192 char8k;
typedef char16 TIPAddress;

#define NULLHANDLE           -1

#ifdef linux
typedef struct TGUID {
  DWORD Data1;
  WORD  Data2; 
  WORD  Data3; 
  Byte  Data4[8];
} TGUID;
#define GUID TGUID
//-----------------------------------------------------------------------------
typedef struct TRect1 {                        //sizeof 16 Byte
  long left;                                    //左
  long top;                                     //上
  long right;                                   //右
  long bottom;                                  //下
}TRect1;
#define RECT1 TRect1
#endif
//-----------------------------------------------------------------------------
typedef struct bit {                             //字符位操作
  char b0:1, b1:1, b2:1, b3:1, b4:1, b5:1, b6:1, b7:1;
}bit;

typedef struct TNetTime { //sizeof 8
  Byte Year;//Byte; 09 + 2000
  Byte Month;//1..12;
  Byte Day;//1..31;
  Byte Hour;//0..23;
  Byte Minute;//0..59;
  Byte Second;//0..59;
  WORD MilliSecond;//0..999;
}TNetTime;

//-----------------------------------------------------------------------------
typedef struct TVersion {
  union {
    struct {WORD Major,Minor,Release,Build;};
    WORD v[4];
    uint64 f;
  };
}TVersion;

typedef struct TMacAddress {
  union {
    struct {DWORD m4321,m8765;};
    struct {Byte m0,m1,m2,m3,m4,m5,m6,m7;};
  };
}TMacAddress;

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define Bit(S, Bits, Value) ( (Value==1) ? (S|(1<<Bits)) : (S&(~(1<<Bits))) )//同下
//#define Bit(S, Bits, Value) ( (Value==1) ? (S|(1<<Bits)) : (S&(1<<Bits)^0xffffffff) )
#define BitXOR(S, Bits)     ( S ^ (1 << Bits) )
#define BitValue(S, Bits)   ( (S & (1 << Bits)) != 0 )

#endif

