
#ifndef __SMTP_EML_H__
#define __SMTP_EML_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

extern ssize_t SMTP_eml_estimate(const char* subject, const char* body, const char* attach_file);

extern const char* SMTP_eml_attachment_mime(const char* attach_file);
extern int SMTP_eml_make(char* const buf, ssize_t buf_size, const char* subject, const char* body,
	const char* attach_file, const char* file_name);

#ifdef __cplusplus
};
#endif
#endif //__SMTP_EML_H__

