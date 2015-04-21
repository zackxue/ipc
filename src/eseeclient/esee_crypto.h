
#ifndef __ESEE_CRYPTO_H__
#define __ESEE_CRYPTO_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern int ESEE_encrypt_ucode(void* ucode_origin, void* ucode_crypto);
extern int ESEE_encrypt_id(void* id_origin, void* id_crypto);

#ifdef __cplusplus
};
#endif
#endif //__ESEE_CRYPTO_H__

