
#ifndef __ESEE_ENV_H__
#define __ESEE_ENV_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct ESEE_ENV_ITEM
{
	char* name;
	char* value;
}ESEE_ENV_ITEM_t;

extern void ESEE_env_dump();

extern void ESEE_env_clear_item(int index);
extern void ESEE_env_clear_item_byname(const char* name);
extern void ESEE_env_clear_item_all();

extern int ESEE_env_set_item(int index, const char* value);
extern int ESEE_env_set_item_byname(const char* name, const char* value);

extern const char* ESEE_env_find_item(int index);
extern const char* ESEE_env_find_item_byname(const char* name);

#ifdef __cplusplus
};
#endif
#endif //__ESEE_ENV_H__

