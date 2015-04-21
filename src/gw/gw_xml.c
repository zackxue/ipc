#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "spook/httpd.h"
#include "gw_xml.h"
#include "gw_debug.h"
#include "aval.h"
#include "gw_debug.h"
#include "cgi_load.h"

const char *strnchr(const char *str, size_t len, int character) {
    const char *end = str + len;
    char c = (char)character;
    do {
        if (*str == c) {
            return str;
        }
    } while (++str < end);
    return NULL;
}

#define NON_NUM '0' 

char Char2Num(char ch){ 
	if(ch>='0' && ch<='9') return (char)(ch-'0'); 
	if(ch>='a' && ch<='f') return (char)(ch-'a'+10); 
	if(ch>='A' && ch<='F') return (char)(ch-'A'+10); 
	return NON_NUM; 
} 


static int URLEncode(const char* str, const int strSize, char* result, const int resultSize) 
{ 
	int i; 
	int j = 0; /* for result index */ 
	char ch; 

	if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) { 
		return 0; 
	} 

	for (i=0; (i<strSize) && (j<resultSize); i++) { 
		ch = str[i]; 
		if ((ch >= 'A') && (ch <= 'Z')) { 
			result[j++] = ch; 
		} else if ((ch >= 'a') && (ch <= 'z')) { 
			result[j++] = ch; 
		} else if ((ch >= '0') && (ch <= '9')) { 
			result[j++] = ch; 
		} else if(ch == ' '){ 
			result[j++] = '+'; 
		} else { 
			if (j + 3 < resultSize) { 
				sprintf(result+j, "%%%02X", (unsigned char)ch); 
				j += 3; 
			} else { 
				return 0; 
			} 
		} 
	} 

	result[j] = '\0'; 
	return j; 
} 


static int URLDecode(const char* str, const int strSize, char* result, const int resultSize) 
{ 
	char ch, ch1, ch2; 
	int i; 
	int j = 0; /* for result index */ 

	if ((str == NULL) || (result == NULL) || (strSize <= 0) || (resultSize <= 0)) { 
		return 0; 
	} 

	for (i=0; (i<strSize) && (j<resultSize); i++) { 
		ch = str[i]; 
		switch (ch) { 
			case '+': 
			result[j++] = ' '; 
			break; 

			case '%': 
			if (i+2 < strSize) { 
				ch1 = Char2Num(str[i+1]); 
				ch2 = Char2Num(str[i+2]); 
				if ((ch1 != NON_NUM) && (ch2 != NON_NUM)) { 
					result[j++] = (char)((ch1<<4) | ch2); 

					i += 2; 
					break; 
				} 
			} 

			/* goto default */ 
			default: 
			result[j++] = ch; 
			break; 
		} 
	} 

	result[j] = '\0'; 
	return j; 
} 



int parse_path_info(void *_pSession, char *_xml, const int _xml_size)
{
	GW_ASSERT(NULL!=_pSession && NULL!=_xml, "Parameter error.");
	GW_ASSERT(NULL!=_pSession && NULL!=_xml, "Parameter error at parse_path_info().");

	struct HttpdSession *pSession = (struct HttpdSession *)_pSession;

	if (0 == pSession->uri_query_string.av_len) {
		GW_TRACE("query length:%d. No query_string.", pSession->uri_query_string.av_len);
		return -1;
	}

	struct _URL_PARAM param[MAX_PARAM_IN_URL];
	memset(&param, 0, sizeof(struct _URL_PARAM)*MAX_PARAM_IN_URL);

	int nParam_count = CGI_parse_url(pSession->uri_query_string,param);
	GW_TRACE("Get %d parameters int the url.", nParam_count);
	_URL_PARAM_t *param_xml=CGI_find_param(param,nParam_count,"xml");
	GW_ASSERT(param_xml,"cgi_gw find cmd of 'xml' failed");
	int nRet=URLDecode(param_xml->value.av_val,param_xml->value.av_len,_xml,_xml_size);
	GW_ASSERT(nRet == strlen(_xml), "xml result error.");
	
	return 0;
}
