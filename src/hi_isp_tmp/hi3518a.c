
#include "sdk_debug.h"

struct HI3518A_ERR_MAP
{
	uint32_t errno;
	const char* str;
};

static struct HI3518A_ERR_MAP _hi3518a_err_map[] =
{
	// sys
	{ 0xA0028003, "HI_ERR_SYS_ILLEGAL_PARAM", },
	{ 0xA0028006, "HI_ERR_SYS_NULL_PTR", },
	{ 0xA0028009, "HI_ERR_SYS_NOT_PERM", },
	{ 0xA0028010, "HI_ERR_SYS_NOTREADY", },
	{ 0xA0028012, "HI_ERR_SYS_BUSY", },
	{ 0xA002800C, "HI_ERR_SYS_NOMEM", },

	// venc
	{ 0xA0078001, "HI_ERR_VENC_INVALID_DEVID", },
	{ 0xA0078002, "HI_ERR_VENC_INVALID_CHNID", },
	{ 0xA0078003, "HI_ERR_VENC_ILLEGAL_PARAM", },
	{ 0xA0078004, "HI_ERR_VENC_EXIST", },
	{ 0xA0078005, "HI_ERR_VENC_UNEXIST", },
	{ 0xA0078006, "HI_ERR_VENC_NULL_PTR", },
	{ 0xA0078007, "HI_ERR_VENC_NOT_CONFIG", },
	{ 0xA0078008, "HI_ERR_VENC_NOT_SUPPORT", },
	{ 0xA0078009, "HI_ERR_VENC_NOT_PERM", },
	{ 0xA007800C, "HI_ERR_VENC_NOMEM", },
	{ 0xA007800D, "HI_ERR_VENC_NOBUF", },
	{ 0xA007800E, "HI_ERR_VENC_BUF_EMPTY", },
	{ 0xA007800F, "HI_ERR_VENC_BUF_FULL", },
	{ 0xA0078010, "HI_ERR_VENC_SYS_NOTREADY", },

	// vpss
	{ 0xA0088001, "HI_ERR_VPSS_INVALID_DEVID", },
	{ 0xA0088002, "HI_ERR_VPSS_INVALID_CHNID", },
	{ 0xA0088003, "HI_ERR_VPSS_ILLEGAL_PARAM", },
	{ 0xA0088004, "HI_ERR_VPSS_EXIST", },
	{ 0xA0088005, "HI_ERR_VPSS_UNEXIT", },
	{ 0xA0088006, "HI_ERR_VPSS_NULL_PTR", },
	{ 0xA0086008, "HI_ERR_VPSS_NOT_SUPPORT", },
	{ 0xA0088009, "HI_ERR_VPSS_NOT_PERM", },
	{ 0xA008800C, "HI_ERR_VPSS_NOMEM", },
	{ 0xA008800D, "HI_ERR_VPSS_NOBUF" },
	{ 0xA0088010, "HI_ERR_VPSS_NOTREADY", },
	{ 0xA0088012, "HI_ERR_VPSS_BUSY", },
};

const char* SOC_strerror(uint32_t const errno)
{
	int i = 0;
	for(i = 0; i < (int)(sizeof(_hi3518a_err_map) / sizeof(_hi3518a_err_map[0])); ++i){
		if(errno == _hi3518a_err_map[i].errno){
			return _hi3518a_err_map[i].str;
		}
	}
	return "UNKNOWN ERROR!";
}



