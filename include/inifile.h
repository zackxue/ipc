
#ifndef INIFILE_H
#define INIFILE_H

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#ifdef LINUX /* Remove CR, on unix systems. */
#define INI_REMOVE_CR
#define DONT_HAVE_STRUPR
#endif

#ifndef CCHR_H
#define CCHR_H
typedef const char cchr;
#endif

typedef char* INI_STR_t;
typedef const char* INI_CSTR_t;
typedef void* INI_BIN_t;
typedef const void* INI_CBIN_t;

#ifndef __cplusplus
#ifdef LINUX
#include <stdbool.h>
#define TRUE true
#define FALSE false
#else
typedef int bool;
#define true  1
#define TRUE  1
#define false 0
#define FALSE 0
#endif
#endif

#define tpNULL       0
#define tpSECTION    1
#define tpKEYVALUE   2
#define tpCOMMENT    3


struct ENTRY
{
   char   Type;
   char  *Text;
   struct ENTRY *pPrev;
   struct ENTRY *pNext;
} ENTRY;

typedef struct
{
   struct ENTRY *pSec;
   struct ENTRY *pKey;
   char          KeyText [128];
   char          ValText [128];
   char          Comment [255];
} EFIND;

typedef struct INI_PARSER
{
	struct ENTRY *entry;
	struct ENTRY *cur_entry;
	FILE *inifile;

	void (*write_text)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, INI_CSTR_t value);
	void (*write_binary)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, INI_CBIN_t bin, size_t bin_len);
	void (*write_bool)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, bool value);
	void (*write_int)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, int value);
	void (*write_double)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, double value);
	
	bool (*delete_key)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key);

	INI_CSTR_t (*read_text)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, INI_CSTR_t def_val, INI_STR_t buf, size_t stack_len);
	ssize_t (*read_binary)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, INI_BIN_t buf, size_t stack_len);
	bool (*read_bool)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, bool def_val);
	int (*read_int)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, int def_val);
	double (*read_float)(struct INI_PARSER *thiz,
		INI_CSTR_t section, INI_CSTR_t key, double def_val);

}stINI_PARSER, *lpINI_PARSER;

/* Macros */
//#define ArePtrValid(Sec,Key,Val) ((Sec!=NULL)&&(Key!=NULL)&&(Val!=NULL))
#define ArePtrValid(Sec,Key,Val) ((Sec!=NULL)&&(Key!=NULL))

/* Connectors of this file (Prototypes) */

extern stINI_PARSER* OpenIniFile (INI_CSTR_t FileName);
extern void CloseIniFile (stINI_PARSER* thiz);
extern bool WriteIniFile (stINI_PARSER* thiz, const char *FileName);


#endif


