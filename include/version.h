#ifndef __VERSION_H__JDKSDFSLDFJIEFLSDF
#define __VERSION_H__JDKSDFSLDFJIEFLSDF
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static inline char* swver_ext();
static inline int build_year();
static inline int build_month();
static inline int build_day();
static inline int build_hour();
static inline int build_min();
static inline int build_sec();

#define HWVER_MAJ (1)
#define HWVER_MIN (0)
#define HWVER_REV (0)
#define HWVER_EXT "06"

#define SWVER_MAJ (1)
#define SWVER_MIN (0)
#define SWVER_REV (0)
#define SWVER_EXT swver_ext()

#define BUILD_YEAR build_year()
#define BUILD_MONTH build_month()
#define BUILD_DAY build_day()
#define BUILD_HOUR build_hour()
#define BUILD_MIN build_min()
#define BUILD_SEC build_sec()


static inline char* swver_ext()
{
	static char tmp_arr[64];
	char* tmp_s = "$Revision: 1383 $";
	tmp_s = strchr(tmp_s, ':');
	int tmp_i = 0;
	if(tmp_s != NULL)
	{
		tmp_i = atoi(tmp_s+1);
	}
	sprintf(tmp_arr, "%dTest", tmp_i);
	return tmp_arr;
}

static inline int build_year()
{
	char* tmp_s = strchr(__DATE__, ' ');
	tmp_s = strchr(tmp_s+1, ' ');
	int tmp_i = atoi(tmp_s+1);
	return tmp_i;
}

static inline int build_month()
{
	int tmp_i = 0;
	if(strncasecmp(__DATE__, "Dec", 3) == 0)
	{
		tmp_i = 12;
	}
	return tmp_i;
}

static inline int build_day()
{
	char* tmp_s = strchr(__DATE__, ' ');
	int tmp_i = atoi(tmp_s+1);
	return tmp_i;
}

static inline int build_hour()
{
	int tmp_i = atoi(__TIME__);
	return tmp_i;
}

static inline int build_min()
{
	char* tmp_s = strchr(__TIME__, ':');
	int tmp_i = atoi(tmp_s+1);
	return tmp_i;
}

static inline int build_sec()
{
	char* tmp_s = strchr(__TIME__, ':');
	tmp_s = strchr(tmp_s+1, ':');
	int tmp_i = atoi(tmp_s+1);
	return tmp_i;
}


#endif
