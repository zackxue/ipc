
#include "mpeg2ts.h"
#include "mpegts.h"
#include "hls_debug.h"



#define MP2TS_SYS_CLOCK (27000000)

//
// PCR_base(i) = ((sys_clock x t(i)) div 300) % 2^33
// PCR_ext(i)	= ((sys_clock x t(i)) div 1) % 300
//

uint32_t mp2ts_pcr_base_us(uint64_t ti_us)
{
	uint32_t pcr_base = (27 * ti_us) / 300;
	return pcr_base;
}

uint32_t mp2ts_pcr_ext_us(uint64_t ti_us)
{
	uint32_t pcr_ext = (27 * ti_us) % 300;
	return pcr_ext;
}

uint32_t mp2ts_pcr_base_s(uint32_t ti_s)
{
	uint32_t pcr_base = (27000000 / 300) * ti_s;
	return pcr_base;
}

uint32_t mp2ts_pcr_ext_s(uint32_t ti_s)
{
	uint32_t pcr_ext = ti_s % 300;
	return pcr_ext;
}




