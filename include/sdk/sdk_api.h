
#include <assert.h>

// all the interfaces
#include "sdk_api_def.h"
#include "sdk_graphic.h"
#include "sdk_sys.h"
#include "sdk_audio.h"
#include "sdk_vin.h"
//#include "sdk_isp.h" // for ipcam solution onlys
#include "sdk_enc.h"
//#include "sdk_md.h"
//#include "sdk_overlay.h"

#ifndef SDK_API_H_
#define SDK_API_H_

#define SDK_ALIGNED_LITTLE_ENDIAN(__val, __align) \
	((__val) /= (__align), (__val) *= (__align), __val)
#define SDK_ALIGNED_BIG_ENDIAN(__val, __align) \
	((__val) += (__align) - 1, SDK_ALIGNED_LITTLE_ENDIAN((__val), (__align)))

#define SDK_ZERO(ptr, sz) do{memset(ptr, 0, sz);}while(0)

#define SDK_ZERO_VAL(val) do{memset(&(val), 0, sizeof(val));}while(0)
#define SDK_ZERO_PTR(ptr, len) do{memset(ptr, 0, len);}while(0)

#endif //__SDK_API_H_

