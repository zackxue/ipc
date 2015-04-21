
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ja_matrix.h"

#ifndef JAINI_H_
#define JAINI_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef const char* tJA_INI_ITEXT; // input text type define
typedef char* tJA_INI_OTEXT; // output text type define

typedef struct JA_INI_FILE {
	
	int (*read_text)(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key, tJA_INI_OTEXT text);
	int (*read_ntext)(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key, tJA_INI_OTEXT text, size_t size);

	lpJA_MATRIX (*read_matrix)(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key);

}stJA_INI_FILE, *lpJA_INI_FILE;

extern lpJA_INI_FILE JAINI_open(const char *file_path);
extern void JAINI_close(lpJA_INI_FILE ini);

#ifdef __cplusplus
};
#endif
#endif //JAINI_H_

