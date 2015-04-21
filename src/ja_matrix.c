
#include "ja_matrix.h"

#define DECLARE_MATRIX_SET_ELEMENT(__func_name, __matrix_ptr, __element_typedef) \
int __func_name(__matrix_ptr const matrix, int row, int column, __element_typedef element){\
	if(matrix){\
		if(row < matrix->row && column < matrix->column){\
			__element_typedef *const element_offset = (__element_typedef *const)(matrix + 1);\
			element_offset[row * matrix->column + column] = element;\
			return 0;\
		}\
	}\
	return -1;\
}

#define DECLARE_MATRIX_GET_ELEMENT(__func_name, __matrix_ptr, __element_typedef) \
int __func_name(__matrix_ptr const matrix, int row, int column, __element_typedef *element){\
	if(matrix){\
		if(row < matrix->row && column < matrix->column){\
			__element_typedef *const element_offset = (__element_typedef *)(matrix + 1);\
			if(element){\
				*element = element_offset[row * matrix->column + column];\
				return 0;\
			}\
		}\
	}\
	return -1;\
}

#define DECLARE_MATRIX_FREE(__func_name, __matrix_ptr) \
void __func_name(__matrix_ptr const matrix){\
	if(matrix){\
		free(matrix);\
	}\
}

#define DECLARE_MATRIX_DUMP(__func_name, __matrix_ptr, __element_typedef, __element_dump_fmt) \
void __func_name(__matrix_ptr const matrix){\
	int i = 0, ii = 0;\
	printf("Matrix [%dx%d]\r\n", matrix->row, matrix->column);\
	for(i = 0; i < matrix->row; ++i){\
		for(ii = 0; ii < matrix->column; ++ii){\
			__element_typedef *const element_offset = (__element_typedef *)(matrix + 1);\
			char dump_text[64] = {""};\
			__element_typedef val = element_offset[i * matrix->column + ii];\
			const char *hex_sign = val >= 0 ? "" : "-";\
			int hex_val = (val > 0 ? 1 : -1) * (int)val;\
			sprintf(dump_text, __element_dump_fmt "(%s0x%x)", val, hex_sign, hex_val);\
			printf("%20s ", dump_text);\
		}\
		printf("\r\n");\
	}\
}

#define DECLARE_MATRIX_INTERFACE(__matrix_ptr, __element_typedef, __element_dump_fmt,\
	__set_element_func_name,\
	__get_element_func_name,\
	__dump_func_name,\
	__free_func_name)\
static DECLARE_MATRIX_SET_ELEMENT(__set_element_func_name, __matrix_ptr, __element_typedef);\
static DECLARE_MATRIX_GET_ELEMENT(__get_element_func_name, __matrix_ptr, __element_typedef);\
static DECLARE_MATRIX_DUMP(__dump_func_name, __matrix_ptr, __element_typedef, __element_dump_fmt);\
static DECLARE_MATRIX_FREE(__free_func_name, __matrix_ptr);\

#define DECLARE_MATRIX_CREATE(__func_name, __matrix_ptr, __element_typedef, __api_template) \
__matrix_ptr __func_name(size_t row, size_t column){\
	if(row > 0 && column > 0){\
		size_t const element_size = row * column * sizeof(__element_typedef);\
		__matrix_ptr matrix = NULL;\
		matrix = calloc(sizeof(matrix[0]) + element_size, 1);\
		memcpy(matrix, &__api_template, sizeof(__api_template));\
		matrix->row = row; matrix->column = column;\
		return matrix;\
	}\
	return NULL;\
}

static int matrix_set_element(lpJA_MATRIX const matrix, int row, int column, int32_t element);
static int matrix_get_element(lpJA_MATRIX const matrix, int row, int column, int32_t *element);
static void matrix_dump(lpJA_MATRIX const matrix);
static void matrix_free(lpJA_MATRIX const matrix);

DECLARE_MATRIX_INTERFACE(lpJA_MATRIX, int32_t, "%d",\
	matrix_set_element, matrix_get_element, matrix_dump, matrix_free);

static stJA_MATRIX _fop_matrix = {
	.set_element = matrix_set_element,
	.get_element = matrix_get_element,
	.dump = matrix_dump,
	.free = matrix_free,
};

lpJA_MATRIX ja_matrix_create(size_t row, size_t column);
DECLARE_MATRIX_CREATE(ja_matrix_create, lpJA_MATRIX, int32_t, _fop_matrix);

static int matrix_set_element_float(lpJA_MATRIX_FLOAT const matrix, int row, int column, double element);
static int matrix_get_element_float(lpJA_MATRIX_FLOAT const matrix, int row, int column, double *element);
static void matrix_dump_float(lpJA_MATRIX_FLOAT const matrix);
static void matrix_free_float(lpJA_MATRIX_FLOAT const matrix);

DECLARE_MATRIX_INTERFACE(lpJA_MATRIX_FLOAT, double, "%.2f",\
	matrix_set_element_float, matrix_get_element_float, matrix_dump_float, matrix_free_float);


static stJA_MATRIX_FLOAT _fop_matrix_float = {
	.set_element = matrix_set_element_float,
	.get_element = matrix_get_element_float,
	.dump = matrix_dump_float,
	.free = matrix_free_float,
};

lpJA_MATRIX_FLOAT ja_matrix_create_float(size_t row, size_t column);
DECLARE_MATRIX_CREATE(ja_matrix_create_float, lpJA_MATRIX_FLOAT, double, _fop_matrix_float);


