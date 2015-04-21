
#include "ja_ini.h"
#include "inifile.h"

typedef struct JA_INI_FILE_PRIV {
	stINI_PARSER *ini_file; // inifile.h
}stJA_INI_FILE_PRIV, *lpJA_INI_FILE_PRIV;

static const char *ini_trim(const char *ptr)
{
	while('\0' != *ptr){
		if(' ' != *ptr){
			return ptr;
		}
		++ptr;
	}
	return NULL;
}

static int ini_read_int(const char *ptr, int def_val)
{
	ptr = ini_trim(ptr);
	if(NULL != ptr){
		int plus_minus = 1; // +1 or -1
		while('\0' != *ptr){
			if('-' == *ptr){
				plus_minus *= -1;
				++ptr;
				continue;
			}else{
				int val = atoi(ptr);
				// have a double check when zero
				if(0 == val){
					const char *val_off = ini_trim(ptr);
					if(NULL != val_off){
						if(0 == memcmp(val_off, "0x", 2) || 0 == memcmp(val_off, "0X", 2)){
							if(1 == sscanf(val_off + 2, "%x", &val)){
								return plus_minus * val;
							}
						}
					}
					break;
				}else{
					return plus_minus * val;
				}
			}
		}
	}
	return def_val;
}

static int ini_read_matrix_element(const char *text, int *row, int *column)
{
	text = ini_trim(text);
	if(NULL != text){
		if('(' == *text){
			int where_row, where_column;
			if(2 == sscanf(text, "(%d,%d)=", &where_row, &where_column)){
				// success to parse
				text = strchr(text, '=') + 1;
				if(row){
					*row = where_row;
				}
				if(column){
					*column = where_column;
				}
				return ini_read_int(text, 0);
			}
		}else{
			return ini_read_int(text, 0);
		}
	}
	return 0;
}

static int jaini_read_text(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key, tJA_INI_OTEXT text)
{
	if(ini){
		lpJA_INI_FILE_PRIV priv = (lpJA_INI_FILE_PRIV)(ini + 1);
		stINI_PARSER *ini = priv->ini_file;
		char buf[1024] = {""};
		const char* text_ptr = ini->read_text(ini, session, key, NULL, buf, sizeof(buf));
		if(text_ptr){
			strcpy(text, text_ptr);
			return strlen(text);
		}
	}
	return -1;
}

static int jaini_read_ntext(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key, tJA_INI_OTEXT text, size_t size)
{
	if(ini){
		lpJA_INI_FILE_PRIV priv = (lpJA_INI_FILE_PRIV)(ini + 1);
		stINI_PARSER *ini = priv->ini_file;
		char buf[1024] = {""};
		const char* text_ptr = ini->read_text(ini, session, key, NULL, buf, sizeof(buf));
		if(text_ptr){
			snprintf(text, size, "%s", text_ptr);
			return strlen(text);
		}
	}
	return -1;
}

static lpJA_MATRIX jaini_read_matrix(struct JA_INI_FILE *const ini, tJA_INI_ITEXT session, tJA_INI_ITEXT key)
{
	int i = 0, ii = 0;
	if(ini){
		char matrix_text[1024] = {""};
		if(ini->read_ntext(ini, session, key, matrix_text, sizeof(matrix_text)) > 0){
			int matrix_row = 0, matrix_column = 0;
			if(2 == sscanf(matrix_text, "%dx%d[^|\r\n]", &matrix_row, &matrix_column)){
				lpJA_MATRIX matrix = ja_matrix_create(matrix_row, matrix_column);
				if(NULL != matrix){
					// parse elementanant
					char *val_ptr = strchr(matrix_text, '|') + 1;
					for(i = 0; i < matrix_row; ++i){
						for(ii = 0; ii < matrix_column; ++ii){
							int where_row = i, where_column = ii;
							//int const element = 0;
							int const element = ini_read_matrix_element(val_ptr, &where_row, &where_column);
							matrix->set_element(matrix, where_row, where_column, element);
							val_ptr = strchr(val_ptr, ' ');
							if(!val_ptr){
								// format error
								break;
							}
							++val_ptr;
						}
					}
					return matrix;
				}
				// error occur, free the matrix
				matrix->free(matrix);
				return NULL;
			}
		}
	}
	return NULL;
}

lpJA_INI_FILE JAINI_open(const char *file_path)
{
	stINI_PARSER *ini = OpenIniFile(file_path);
	if(ini){
		lpJA_INI_FILE ja_ini = calloc(sizeof(stJA_INI_FILE) + sizeof(stJA_INI_FILE_PRIV), 1);
		lpJA_INI_FILE_PRIV priv = (lpJA_INI_FILE_PRIV)(ja_ini + 1);

		// note the ini handle
		priv->ini_file = ini;
		// interfaces
		ja_ini->read_text = jaini_read_text;
		ja_ini->read_ntext = jaini_read_ntext;
		ja_ini->read_matrix = jaini_read_matrix;
		return ja_ini;
	}
	return NULL;
}

void JAINI_close(lpJA_INI_FILE ini)
{
	if(ini){
		lpJA_INI_FILE_PRIV priv = (lpJA_INI_FILE_PRIV)(ini+ 1);
		// close ini file
		CloseIniFile(priv->ini_file);
		priv->ini_file = NULL;
		// free self
		free(ini);;
	}
}

int test_main()
{
	lpJA_INI_FILE ini = JAINI_open("hi3518_ar0130_isp.ini");
	if(ini){
		char text[1024] = {""};
		lpJA_MATRIX matrix = NULL;

		ini->read_text(ini, "OPTION", "sensor_manufactor", text);
		matrix = ini->read_matrix(ini, "COLOR_CORRECTION", "high_color_matrix");
		matrix->dump(matrix);
		matrix->free(matrix);
		matrix = NULL;
		JAINI_close(ini);
		ini = 0;
	}
	return 0;
}

