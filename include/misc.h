
#ifndef __MISC_H__
#define __MISC_H__
#ifdef __cplusplus
extern "C" {
#endif

#define FIXED_1K (1024)
#define FIXED_1M (FIXED_1K * FIXED_1K)
#define FIXED_1G (FIXED_1K * FIXED_1M)

#define ALIGIN_LITTLE_ENDIAN(value, align) ((value) = (((value) / (align)) * (align)))
#define ALIGIN_BIG_ENDIAN(value, align) ((value) = ((((value) + (align) - 1) / (align)) * (align)))

#ifdef __cplusplus
};
#endif
#endif //__MISC_H__

