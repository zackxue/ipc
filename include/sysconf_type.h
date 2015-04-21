
#ifndef __SYSCONF_TYPE_H__
#define __SYSCONF_TYPE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef uint8_t SYS_U8_t;
typedef int8_t SYS_S8_t;
typedef uint16_t SYS_U16_t;
typedef int16_t SYS_S16_t;
typedef uint32_t SYS_U32_t;
typedef int32_t SYS_S32_t;
typedef uint64_t SYS_U64_t;
typedef int64_t SYS_S64_t;
typedef double SYS_FLOAT_t;
typedef char SYS_CHR_t;
typedef SYS_U8_t SYS_BOOL_t;

#define SYS_TRUE (1==1)
#define SYS_FALSE (1==0)

#define SYS_NULL ((void*)0)

#pragma pack(1)
///////////////////////////////////////////////////////////////////////////
// expression : val / max
typedef struct SYS_RATE8
{
	SYS_U8_t val;
	SYS_U8_t max;
}SYS_RATE8_t;

typedef struct SYS_RATE16
{
	SYS_U16_t val;
	SYS_U16_t max;
}SYS_RATE16_t;

typedef struct SYS_RATE32
{
	SYS_U32_t val;
	SYS_U32_t max;
}SYS_RATE32_t;

typedef struct SYS_RATE64
{
	SYS_U64_t val;
	SYS_U64_t max;
}SYS_RATE64_t;

typedef SYS_RATE8_t SYS_MAP_t;

///////////////////////////////////////////////////////////////////////////
// expression : val % all != 0
typedef struct SYS_ENUM
{
	SYS_U32_t val;
	SYS_U32_t mask;
}SYS_ENUM_t;

///////////////////////////////////////////////////////////////////////////
// expression : min <= val <= max
typedef struct SYS_RANGE_S8
{
	SYS_S8_t val;
	SYS_S8_t min;
	SYS_S8_t max;
}SYS_RANGE_S8_t;

typedef struct SYS_RANGE_U8
{
	SYS_U8_t val;
	SYS_U8_t min;
	SYS_U8_t max;
}SYS_RANGE_U8_t;

typedef struct SYS_RANGE_S16
{
	SYS_S16_t val;
	SYS_S16_t min;
	SYS_S16_t max;
}SYS_RANGE_S16_t;

typedef struct SYS_RANGE_U16
{
	SYS_U16_t val;
	SYS_U16_t min;
	SYS_U16_t max;
}SYS_RANGE_U16_t;

typedef struct SYS_RANGE_S32
{
	SYS_S32_t val;
	SYS_S32_t min;
	SYS_S32_t max;
}SYS_RANGE_S32_t;

typedef struct SYS_RANGE_U32
{
	SYS_U32_t val;
	SYS_U32_t min;
	SYS_U32_t max;
}SYS_RANGE_U32_t;

typedef struct SYS_RANGE_S64
{
	SYS_S64_t val;
	SYS_S64_t min;
	SYS_S64_t max;
}SYS_RANGE_S64_t;

typedef struct SYS_RANGE_U64
{
	SYS_U64_t val;
	SYS_U64_t min;
	SYS_U64_t max;
}SYS_RANGE_U64_t;

typedef struct SYS_RANGE_FLOAT
{
	SYS_FLOAT_t val;
	SYS_FLOAT_t min;
	SYS_FLOAT_t max;
}SYS_RANGE_FLOAT_t;

///////////////////////////////////////////////////////////////////////////
// geometry
typedef struct SYS_PLANE
{
	SYS_U16_t w;
	SYS_U16_t h;
}SYS_PLANE_t;

typedef struct SYS_POINT
{
	SYS_S16_t x;
	SYS_S16_t y;
}SYS_POINT_t;

typedef struct SYS_RECT
{
	SYS_S16_t x;
	SYS_S16_t y;
	SYS_U16_t w;
	SYS_U16_t h;
}SYS_RECT_t;

///////////////////////////////////////////////////////////////////////////
// date time
typedef struct SYS_DATE
{
	SYS_U16_t year;
	SYS_U8_t month;
	SYS_U8_t day;
}SYS_DATE_t;

typedef struct SYS_TIME
{
	SYS_U8_t hour;
	SYS_U8_t min;
	SYS_U8_t sec;
}SYS_TIME_t;

typedef struct SYS_IP_ADDR
{
	union
	{
		struct in_addr in_addr;
		SYS_U8_t s[4];
		struct
		{
			SYS_U8_t s1, s2, s3, s4;
		};
		SYS_U32_t s_addr;
	};
}SYS_IP_ADDR_t;

#define SYS_INET_ATON(str_ip, sys_ip) \
	do{\
		const in_addr_t in_addr = inet_addr(str_ip);\
		memcpy((sys_ip)->s, &in_addr, sizeof(in_addr));\
	}while(0)

	
typedef struct SYS_MAC_ADDR
{
	union
	{
		SYS_U8_t s[6];
		struct
		{
			SYS_U8_t s1, s2, s3, s4, s5, s6;
		};
	};
}SYS_MAC_ADDR_t;
#pragma pack()

///////////////////////////////////////////////////////////////////////////
// generic level
typedef SYS_RATE8_t SYS_LEVEL_t;

#define SYS_LEVEL_HIGHEST (0)
#define SYS_LEVEL_HIGH (1)
#define SYS_LEVEL_MEDIUM (2)
#define SYS_LEVEL_LOW (3)
#define SYS_LEVEL_LOWEST (4)

	
	
#ifdef __cplusplus
};
#endif
#endif //__SYSCONF_TYPE_H__

