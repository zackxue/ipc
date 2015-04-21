#ifndef HICHIP_HASH_H_
#define HICHIP_HASH_H_

#include "uthash.h"
#include <stdbool.h>

typedef int (* CGI_func_t)(const char *strQuery, char *strContent, int nLength, bool bOption);

typedef struct {
	const char 			*strCgiName;
	CGI_func_t 			pCgiFunc;
	UT_hash_handle		hh;
}CGI_hashtable;



void CGI_hashDestroy(CGI_hashtable *hashtable);
CGI_hashtable* CGI_hashAdd(CGI_hashtable *hashtable, const char *name, CGI_func_t func);
CGI_hashtable* CGI_hashFind(CGI_hashtable *hashtable, const char *name);



#endif