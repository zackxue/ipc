
#ifndef __AVAL_H__
#define __AVAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

typedef struct AVal
{
	char* av_val;
	int av_len;
} AVal;

#define AVC(str) {str,sizeof(str)-1}
#define AVMATCH(a1,a2) ((a1)->av_len == (a2)->av_len && !memcmp((a1)->av_val,(a2)->av_val,(a1)->av_len))
#define AVCASEMATCH(a1,a2) ((a1)->av_len == (a2)->av_len && !strncasecmp((a1)->av_val,(a2)->av_val,(a1)->av_len))

#define AVSTRMATCH(a,str) ((a)->av_len == strlen(str) && !memcmp((a)->av_val,str,(a)->av_len))
#define AVSTRCASEMATCH(a,str) ((a)->av_len == strlen(str) && !strncasecmp((a)->av_val,str,(a)->av_len))

#define AV2STR(__av, __buf) (strncpy((__buf), (__av).av_val, (__av).av_len), (__buf)[(__av).av_len] = 0)
#define AVTRACE(__av) {char __buf[64];AV2STR((__av), __buf);printf("AVTRACE:%s, val=\"%s\", len=%d\n", #__av, __buf, (__av).av_len);}

#define AVAL_STRDUPA(__av) strndupa((__av).av_val, (__av).av_len)
#define AVAL_STRDUP(__av) strndup((__av).av_val, (__av).av_len)


extern inline AVal AVV(const char* av_val, int av_len);


#endif //__AVAL_H__

