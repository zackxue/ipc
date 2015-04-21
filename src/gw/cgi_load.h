#ifndef __CGI_LOAD_H__
#define __CGI_LOAD_H__

#include <stdlib.h>
#include <stdint.h>

#include "aval.h"

#define HTTPD_CGI_NAME_SZ (32)
#define HTTPD_CGI_MAX_COLLECTION (16)

#define MAX_PARAM_IN_URL	10
#define URL_PARAM_SEPARATOR	'&'
#define URL_KEY_SEPARATOR	'='
typedef struct _URL_PARAM
{
	AVal key;
	AVal value;
}_URL_PARAM_t;

extern int   CGI_parse_url(const AVal _query_string, _URL_PARAM_t _param[]);
extern _URL_PARAM_t* CGI_find_param(_URL_PARAM_t* _params, int _params_count, char* _key);

#endif
