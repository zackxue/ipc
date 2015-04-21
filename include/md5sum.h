
#ifndef __MD5SUM_H__
#define __MD5SUM_H__

#define MD5SUM_DIGEST_STR_LEN (32)
#define MD5SUM_DIGEST_INT_LEN (16)
extern char* md5sum_file(const char* filename);
extern char* md5sum_buffer(const void* buffer, int size);
extern char* md5sum_to_upper(void* buffer, int size);

#endif //__MD5SUM_H__

