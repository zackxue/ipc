#include "cgi_load.h"
#include <string.h>
//#include "cgi_flv.h"
#include "gw.h"

#include "httpd.h"

static char* strnchr(const char *str, size_t len, int character) {
    const char *end = str + len;
    char c = (char)character;
    do {
        if (*str == c) {
            return str;
        }
    } while (++str < end);
    return NULL;
}

static AVal _get_key(const AVal _kv)
{
	AVal ret = {0};
	char *begin, *end;
	begin = _kv.av_val;		//the first char after '&'
	end = strnchr(_kv.av_val, _kv.av_len, URL_KEY_SEPARATOR);	//'='
	if (NULL!=begin && NULL!=end) {
		ret.av_val = begin;
		ret.av_len = end - begin;
	}
	return ret;
}

static AVal _get_value(const AVal _kv)
{
	AVal ret = {0};
	char *begin = strnchr(_kv.av_val, _kv.av_len, URL_KEY_SEPARATOR);	//'='
	char *end = strnchr(_kv.av_val, _kv.av_len, URL_PARAM_SEPARATOR);	//'&'

	if (NULL != begin) {
		ret.av_val = begin + 1;	//'='
		if (NULL == end) {	//only one k-v pairs
			ret.av_len = _kv.av_len - (ret.av_val - _kv.av_val);
		} else {	//more that one k-v pairs
			ret.av_len = end - ret.av_val;
		}
	}
	return ret;
}

static int _parse_url(const AVal _query_string, _URL_PARAM_t _param[])
{

	uint nPos = 0;
	uint param_num = 0;
	AVal temp_aval_kv = {0};	//key and value

	//char *p = strnchr(_query_string.av_val + nPos, _query_string.av_len - nPos, URL_PARAM_SEPARATOR);
	//nPos = p - _query_string.av_val;
	char *p = _query_string.av_val;
	while (NULL != p) {	//at least 1 key left
		if (param_num >= MAX_PARAM_IN_URL) {
			printf("too many parameters in this url!");
			break;
		}
		//get first key
		temp_aval_kv.av_val = p;
		nPos = temp_aval_kv.av_val - _query_string.av_val;
		temp_aval_kv.av_len = _query_string.av_len - nPos;

		_param[param_num].key	= _get_key(temp_aval_kv);
		_param[param_num].value	= _get_value(temp_aval_kv);
		param_num++;

		p = strnchr(temp_aval_kv.av_val, temp_aval_kv.av_len, URL_PARAM_SEPARATOR);
		if (NULL != p)
			p += 1;	//'&'
		else
			break;
	}

	return param_num;
}
int CGI_parse_url(const AVal _query_string, _URL_PARAM_t _param[])
{
	return _parse_url(_query_string, _param);
}

static _URL_PARAM_t* _find_param(_URL_PARAM_t* _params, int _params_count, char* _key)
{
	int i;
	for(i = 0; i < _params_count; i++)
	{
		AVal tmp = AVV(_key, strlen(_key));
		if(AVCASEMATCH(&tmp, &_params[i].key))
		{
			return &_params[i];
		}
	}
	return NULL;
}

_URL_PARAM_t* CGI_find_param(_URL_PARAM_t* _params, int _params_count, char* _key)
{
	return _find_param(_params, _params_count, _key);
}

