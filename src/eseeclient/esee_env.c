
#include "esee_env.h"
#include "esee_debug.h"

#include "generic.h"

static ESEE_ENV_ITEM_t _esee_env[] =
{
	{ "server_domain", NULL },
	{ "server_port", NULL },
	{ "sn", NULL },
	{ "sn_crypto", NULL },
	{ "id", NULL },
	{ "id_crypto", NULL },
	{ "pwd", NULL },
	{ "status", NULL },
	{ "channel", NULL },
	{ "errcmd", NULL },
	{ "errinfo", NULL },
	{ "ecode", NULL },
	{ "interip", NULL },
	{ "exterip", NULL },
	{ "interport", NULL },
	{ "exterport", NULL },
	{ "port", NULL },
	{ "dataport", NULL },
	{ "phoneport", NULL },
	{ "url", NULL },
	{ "version", NULL },
	{ "vendor", NULL }
};

#define ESEE_ENV_ITEMS ((int)(sizeof(_esee_env) / sizeof(_esee_env[0])))

void ESEE_env_clear_item(int index)
{
	if(index < ESEE_ENV_ITEMS){
		free(_esee_env[index].value);
		_esee_env[index].value = NULL; // very important
	}
}

void ESEE_env_clear_item_byname(const char* name)
{
	int i = 0;
	for(i = 0; i < ESEE_ENV_ITEMS; ++i){
		if(STR_THE_SAME(name, _esee_env[i].name)){
			 ESEE_env_clear_item(i);
		}
	}
}

void ESEE_env_clear_item_all()
{
	int i = 0;
	for(i = 0; i < ESEE_ENV_ITEMS; ++i){
		ESEE_env_clear_item(i);
	}
}

void ESEE_env_dump()
{
	int i = 0;
	ESEE_SYNTAX_ON();
	for(i = 0; i < ESEE_ENV_ITEMS; ++i){
		if(_esee_env[i].value){
			//printf("%s=%s\r\n", _esee_env[i].name, _esee_env[i].value);
		}
	}
	ESEE_SYNTAX_OFF();
}

int ESEE_env_set_item(int index, const char* value)
{
	if(index < ESEE_ENV_ITEMS){
		ESEE_env_clear_item(index);
		_esee_env[index].value = strdup(value);
		return index;
	}
	return -1;
}

int ESEE_env_set_item_byname(const char* name, const char* value)
{
	int i = 0;
	for(i = 0; i < ESEE_ENV_ITEMS; ++i){
		if(STR_THE_SAME(name, _esee_env[i].name)){
			return ESEE_env_set_item(i, value);
		}
	}
	return -1;
}

const char* ESEE_env_find_item(int index)
{
	if(index < ESEE_ENV_ITEMS){
		if(_esee_env[index].value){
			return _esee_env[index].value;
		}
		return "";
	}
	return NULL;
}

const char* ESEE_env_find_item_byname(const char* name)
{
	int i = 0;
	for(i = 0; i < ESEE_ENV_ITEMS; ++i){
		if(STR_THE_SAME(name, _esee_env[i].name)){
			return ESEE_env_find_item(i);
		}
	}
	return NULL;
}

