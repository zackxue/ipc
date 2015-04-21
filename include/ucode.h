
#ifndef __UCODE_H__
#define __UCODE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


#define UCODE_SN_MTD "/dev/mtdblock0"


extern int UCODE_write(const char* mtd, off_t off_bottom, const char* ucode, ssize_t sz);
extern int UCODE_read(const char* mtd, off_t off_bottom, char* ucode);
extern int UCODE_check(const char* mtd, off_t off_bottom);

#ifdef __cplusplus
};
#endif
#endif //__UCODEC_H__

