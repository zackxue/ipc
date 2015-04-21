#include "unistruct_debug.h"
#include "sysconf.h"
#include "generic.h"
#include "unistruct_gfun.h"

//#include <time.h>
//#include <sys/time.h>



typedef uint32_t SYSSTATE_TYPE_t;
typedef const char* const SYSSTATE_MAPTBL_t;


typedef struct SYSSTATE_UNISTRUCT
{
	char* xpath;
	SYSSTATE_TYPE_t type; // the type of value
	void* val_ptr; // the value offset to SYSSTATE value
	void* map_enu;
	ssize_t map_enu_size;
	
	UNISTRUCT_DO_CHECK do_check;
	UNISTRUCT_DO_POLICY do_policy;
}SYSSTATE_UNISTRUCT_t;



static SYSSTATE_MAPTBL_t system_operation[] =
{
	[SYS_OPERATION_REBOOT] = "reboot",
	[SYS_OPERATION_DEFAULT_FACTORY] = "default factory",
};


#define SYSSTATE_TYPE_TIME_T (0x1000)
#define SYSSTATE_TYPE_MAP (0x2000)

//SYSSTATE_t* p_SYSSTATE;
#define SYSSTATE_UNISTRUCT_TABEL(p_sysstate) \
	{\
		/* time */\
		{ "time@value", SYSSTATE_TYPE_TIME_T, &p_sysstate->ipcam.time, NULL, 0, NULL, do_policy_time },\
		/* system operation */\
		{ "system@operation", SYSSTATE_TYPE_MAP, &p_sysstate->ipcam.operation, (char**)system_operation, ARRAY_ITEM(system_operation), NULL, do_policy_system_operation},\
		/* ptz */\
		/* SDcard */\
	}

static const char* sysstate_map(int val, const char** map, ssize_t map_size)
{
	if(val < map_size){
		return map[val];
	}
	return SYS_NULL;
}

static int sysstate_umap(const char* text, const char** map, ssize_t map_size)
{
	int i = 0;
	for(i = 0; i < map_size; ++i){
		if(0 == strcmp(text, map[i])){
			return i;
		}
	}
	return -1;
}


static SYSSTATE_UNISTRUCT_t* unistruct_find(SYSSTATE_UNISTRUCT_t* table, ssize_t table_size, const char* xpath)
{
	int i = 0;
	for(i = 0; i < table_size; ++i){
		if(0 == strcmp(xpath, table[i].xpath)){\
			return table + i;
		}
	}
	return NULL;
}

static int unistruct_item_rw(SYSSTATE_UNISTRUCT_t* unistruct, char* io_text, SYS_BOOL_t opt_rw)
{
	int ret = -1;
	switch(unistruct->type)
	{
	case SYSSTATE_TYPE_TIME_T:
		{
			//SYS_ENUM_t* const val_ptr = (SYS_ENUM_t*)unistruct->val_ptr;
			time_t timet;
			if(opt_rw){
				timet = time(NULL);
				char time_num[64] = {0};				
				struct tm cur_tm;
				//printf("before timet:%d\r\n", timet);
				//localtime_r(&timet, &cur_tm);
				//printf("set time:%04d/%02d/%02d %02d:%02d:%02d\r\n", cur_tm.tm_year + 1900, cur_tm.tm_mon + 1, cur_tm.tm_mday,cur_tm.tm_hour, cur_tm.tm_min, cur_tm.tm_sec);
				//timet = timegm(&cur_tm);
				sprintf(time_num, "%d", timet);
				//printf("after timet:%d\r\n", timet);
				memcpy(io_text, time_num, strlen(time_num));	
				ret = 0;
			}else{
				timet = (time_t)atoi(io_text);
				memcpy(unistruct->val_ptr, &timet, sizeof(time_t));
				ret = 0;
			}
		}
		break;
	case SYSSTATE_TYPE_MAP:
		{
			//SYS_MAP_t* const map_ptr = (SYS_MAP_t*)unistruct->val_ptr;
			UNISTRUCT_ASSERT(NULL != unistruct->map_enu, "no map found! please check unistruct table");
			SYS_MAP_t map_ptr;
			memcpy(&map_ptr, unistruct->val_ptr, sizeof(SYS_MAP_t));
			if(opt_rw){
				const char** map = unistruct->map_enu;
				strcpy(io_text, map[map_ptr.val]);
				ret = 0;
			}else{
				int map_val = sysstate_umap(io_text, unistruct->map_enu, unistruct->map_enu_size);
				if(map_val <= map_ptr.max){
					map_ptr.val = map_val;
					ret = 0;
				}
				map_ptr.max = unistruct->map_enu_size;
				map_ptr.val = map_val;
				memcpy(unistruct->val_ptr, &map_ptr, sizeof(SYS_MAP_t));					
			}
		}
		break;
	}
	if(!opt_rw){
		//if(unistruct->do_check!= NULL && !unistruct->do_check(NULL, unistruct->val_ptr)){
			if(unistruct->do_policy){
				unistruct->do_policy(NULL, unistruct->val_ptr);
			}
		//}
	}
	return ret;
}


static int unistruct_rw(SYSSTATE_t* const sysstate, const char* xpath, char* io_text, SYS_BOOL_t opt_rw)
{
	SYSSTATE_UNISTRUCT_t* unistruct = NULL;
	// declare the unistruct
	SYSSTATE_UNISTRUCT_t uni_tbl[] = SYSSTATE_UNISTRUCT_TABEL(sysstate);
	// find the relevant struct
	unistruct = unistruct_find(uni_tbl, ARRAY_ITEM(uni_tbl), xpath);
	if(!unistruct){
		return -1;
	}
	return unistruct_item_rw(unistruct, io_text, opt_rw);
}


int SYSSTATE_UNISTRUCT_set(SYSSTATE_t* const sysstate, const char* xpath, const char* text)
{
	int ret = 0;
	ret = unistruct_rw(sysstate, xpath, text, SYS_FALSE);
	return ret;
}

int SYSSTATE_UNISTRUCT_get(SYSSTATE_t* const sysstate, const char* xpath, const char* text)
{
	int ret = 0;
	ret = unistruct_rw(sysstate, xpath, text, SYS_TRUE);
	return ret;
}




