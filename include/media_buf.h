/*
*	IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
*
*	By downloading, copying, installing or using the software you agree to this license.
*	If you do not agree to this license, do not download, install,
*	Copy or use the software.
*
*	Copyright (C) 2012, JUAN, Co, All Rights Reserved.
*
*	Project Name:MediaBuf
*	File Name:
*
*	Writed by Frank Law at 2013 - 05 - 10 JUAN,Guangzhou,Guangdong,China
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef MEDIA_BUF_H_
#define MEDIA_BUF_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MEDIABUF_USER {
	uint32_t forbidden_zero;
}stMEDIABUF_USER, *lpMEDIABUF_USER;

extern int MEDIABUF_init();
extern void MEDIABUF_destroy();

extern int MEDIABUF_new(size_t mem_size, const char *preferred_name, const char *alternate_name,
	int entry_available, int key_entry_available, int user_available);

typedef void (*fMEDIA_BUF_ON_ENTRY_IN)(const char *name, const void *entry_data, size_t entry_size, bool key_flag);
typedef void (*fMEDIA_BUF_ON_ENTRY_OUT)(const char *name, const void *entry_data, size_t entry_size, bool key_flag);
typedef void (*fMEDIA_BUF_ON_ENTRY_CLEAR)(const char *name, const void *entry_data, size_t entry_size, bool key_flag);
typedef void (*fMEDIA_BUF_ON_USER_CHANGED)(const char *name, uint32_t user_number, uint32_t user_max);
typedef void (*fMEDIA_BUF_ON_POOL_EXHAUSTED)(const char *name);

extern void MEDIABUF_hock_entry_in(int id, fMEDIA_BUF_ON_ENTRY_IN on_entry_in);
extern void MEDIABUF_hock_entry_out(int id, fMEDIA_BUF_ON_ENTRY_OUT on_entry_out);
extern void MEDIABUF_hock_entry_clear(int id, fMEDIA_BUF_ON_ENTRY_CLEAR on_entry_clear);
extern void MEDIABUF_hock_user_changed(int id, fMEDIA_BUF_ON_USER_CHANGED on_user_changed);
extern void MEDIABUF_hock_pool_exhausted(int id, fMEDIA_BUF_ON_POOL_EXHAUSTED on_pool_exhausted);

extern lpMEDIABUF_USER MEDIABUF_attach(int id);
extern void MEDIABUF_detach(lpMEDIABUF_USER const user);
extern int MEDIABUF_sync(lpMEDIABUF_USER const user);

extern int MEDIABUF_in(int id, const void *data, size_t data_size, uint32_t key_flag);
extern int MEDIABUF_in_request(int id, size_t data_size, uint32_t key_flag);
extern int MEDIABUF_in_append(int id, const void *data_piece, size_t data_piece_size);
extern int MEDIABUF_in_commit(int id);

extern int MEDIABUF_empty(int id);

extern int MEDIABUF_out(lpMEDIABUF_USER user, void **ret_data_ptr, void *ret_data, size_t *ret_size);
extern int MEDIABUF_out_lock(lpMEDIABUF_USER user);
extern void MEDIABUF_out_unlock(lpMEDIABUF_USER user);

extern int MEDIABUF_lookup_byname(const char *name);
extern uint32_t MEDIABUF_in_speed(int id);

#ifdef __cplusplus
};
#endif
#endif //MEDIA_BUF_H_

