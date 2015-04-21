
#ifndef __GENERIC_H__
#define __GENERIC_H__
#ifdef _cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


#define CHECK_FILE_EXIST(str_name) (-1 != access(str_name, F_OK))

#define GET_FILE_SIZE(str_name, s32_size) \
	do{\
		struct stat file_stat;\
		if(stat((str_name), &file_stat) < 0){ \
			(s32_size) = -1; \
		} \
		(s32_size) = (ssize_t)(file_stat.st_size); \
	}while(0)


#define STR_THE_SAME(str, str_match) \
	(strlen(str) == strlen(str_match) && 0 == strcmp(str, str_match))

#define STR_CASE_THE_SAME(str, str_match) \
	(strlen(str) == strlen(str_match) && 0 == strcasecmp(str, str_match))


#define GET_HOST_BYNAME(str_domain, u32_host) \
	do{ \
		struct hostent* host_ent = gethostbyname2(str_domain, AF_INET); \
		if(!host_ent){ u32_host = 0; } \
		else{ \
			struct in_addr* const host_addr_list = (struct in_addr*)(host_ent->h_addr_list[0]); \
			struct in_addr host_addr = host_addr_list[0]; \
			u32_host = host_addr.s_addr; \
		} \
	}while(0)


#define STRUCT_ZERO(stru) do{ memset(&stru, 0, sizeof(stru)); }while(0)

#define ARRAY_ZERO(arr) do{ memset(arr, 0, sizeof(arr)); }while(0)
#define ARRAY_SIZE(arr) (sizeof(arr))
#define ARRAY_ITEM(arr) (ARRAY_SIZE(arr) / sizeof(arr[0]))

#define TIMEZONE_SYNC(timezone)\
	do{\
	unsigned char tzstr[32] = {0};\
	sprintf(tzstr, "GMT%c%d", (timezone)>0? '-':'+', abs(timezone));\
	printf("%s:%s\r\n", __FUNCTION__, tzstr);\
	setenv("TZ", tzstr, 1);\
	tzset();\
	}while(0)

#define MAKE_IMAGE   //please add this line when makeing image!!

extern float cpu_get_status();

#ifdef _cplusplus
};
#endif
#endif //__GENERIC_H__

