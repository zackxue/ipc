#include "cgi_hash.h"

CGI_hashtable* CGI_hashAdd(CGI_hashtable *hashtable, const char *name, CGI_func_t func){
	CGI_hashtable *item = NULL;
	if(NULL == name || NULL != (item = CGI_hashFind(hashtable, name))){
		return hashtable;
	}
	else{
		item = malloc(sizeof(CGI_hashtable));
		if(NULL != item){
			item->strCgiName = strdup(name);
			item->pCgiFunc = func;
			HASH_ADD_KEYPTR(hh, hashtable, item->strCgiName, strlen(item->strCgiName), item);
			return hashtable;
		}
	}
}
CGI_hashtable* CGI_hashFind(CGI_hashtable *hashtable, const char *name){
	CGI_hashtable *item = NULL;
	HASH_FIND_STR(hashtable, name, item);
	return item;
}
void CGI_hashDestroy(CGI_hashtable *hashtable){
	CGI_hashtable *item, *tmp;
	HASH_ITER(hh, hashtable, item, tmp){
		HASH_DEL(hashtable, item);
		free(item->strCgiName);
		free(item);
	}
}
