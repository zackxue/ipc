
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef JA_MATRIX_H_
#define JA_MATRIX_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct JA_MATRIX {
	size_t row, column;
	
	int (*set_element)(struct JA_MATRIX *const matrix, int row, int column, int element);
	int (*get_element)(struct JA_MATRIX *const matrix, int row, int column, int *element);
	void (*dump)(struct JA_MATRIX *const matrix);
	void (*free)(struct JA_MATRIX *const matrix);
	
}stJA_MATRIX, *lpJA_MATRIX;

typedef struct JA_MATRIX_FLOAT {
	size_t row, column;
	
	int (*set_element)(struct JA_MATRIX_FLOAT *const matrix, int row, int column, double element);
	int (*get_element)(struct JA_MATRIX_FLOAT *const matrix, int row, int column, double *element);
	void (*dump)(struct JA_MATRIX_FLOAT *const matrix);
	void (*free)(struct JA_MATRIX_FLOAT *const matrix);
	
}stJA_MATRIX_FLOAT, *lpJA_MATRIX_FLOAT;

extern lpJA_MATRIX ja_matrix_create(size_t row, size_t column);
extern lpJA_MATRIX_FLOAT ja_matrix_create_float(size_t row, size_t column);

#ifdef __cplusplus
};
#endif
#endif //JA_MATRIX_H_

