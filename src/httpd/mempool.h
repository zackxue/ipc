/*
 * mempool.h
 *
 *  Created on: 2011-6-30
 *      Author: root
 */

#ifndef MEMPOOL_H_
#define MEMPOOL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#if !defined(MEMPOOL_MAX_LIST_ITEM)
#define MEMPOOL_MAX_LIST_ITEM (1024)
#endif

#if !defined(MEMPOOL_MAX_ALLOC_SIZE)
#define MEMPOOL_MAX_ALLOC_SIZE (1024*1024*10)
#endif

#if !defined(MEMPOOL_INIT_STATIC_MEMORY) && !defined(MEMPOOL_INIT_SHARED_MEMORY)
#define MEMPOOL_INIT_STATIC_MEMORY
#endif

typedef struct _MemPool MemPool;

typedef struct
{
	int pid;
	void* begin;
	void* end;
	int size;
	int real_size;
}MemPoolItem;

#if defined(MEMPOOL_INIT_STATIC_MEMORY)
extern MemPool* MemPool_init();
extern void MemPool_destory(MemPool* _pool);
#endif

#if defined(MEMPOOL_INIT_SHARED_MEMORY)
#define MEMPOOL_SHM_BASE ((void*)0x60000000)
#define MEMPOOL_SHM_PATH "/tmp"
extern int         MemPool_shm_is_init(char* _path);
extern int         MemPool_shm_at_count(char* _path);
extern MemPool*    MemPool_shm_init(void* _base, int _size, char* _path);
extern MemPool*    MemPool_shm_at(void* _base, char* _path);
extern void        MemPool_shm_dt(MemPool* _pool);
extern void        MemPool_shm_destory(MemPool* _pool);
#endif

extern void* MemPool_malloc(MemPool* _pool, int _size);
extern void MemPool_free(MemPool* _pool, void* _addr);
extern void MemPool_print(MemPool* _pool);
extern MemPoolItem* MemPool_get_mempoolitem(MemPool* _pool, void* _addr);
extern int MemPool_get_total_size(MemPool* _pool);
extern int MemPool_get_free_size(MemPool* _pool, int* _malloc_size, int* _real_size);
extern int MemPool_get_used_size(MemPool* _pool, int* _malloc_size, int* _real_size);
extern int MemPool_get_total_count(MemPool* _pool);
extern int MemPool_get_free_count(MemPool* _pool);
extern int MemPool_get_used_count(MemPool* _pool);


extern MemPool* pool;


#ifdef MEMPOOL_REPLACE_SYSFUNC

extern void* malloc (size_t __size);
extern void  free (void *__ptr);
extern void* calloc(size_t __nmemb, size_t __size);
extern void* realloc(void* __ptr, size_t __size);

#endif

#if defined(MEMPOOL_USED)
	#define MALLOC(size) MemPool_malloc(pool, size)
	#define FREE(addr) MemPool_free(pool, addr)

	#if defined(MEMPOOL_INIT_STATIC_MEMORY)
		#define MEMINIT() {pool = MemPool_init();}
		#define MEMEXIT() {MemPool_destory(pool);pool = NULL;}
		#define MEMDESTORY() MEMEXIT()
	#elif defined(MEMPOOL_INIT_SHARED_MEMORY)
		#define MEMSHMISINIT() MemPool_shm_is_init(MEMPOOL_SHM_PATH)
		#define MEMSHMATCOUNT() MemPool_shm_at_count(MEMPOOL_SHM_PATH)
		#define MEMSHMINIT(size) {pool = MemPool_shm_init(MEMPOOL_SHM_BASE,size,MEMPOOL_SHM_PATH);}
		#define MEMSHMAT() {pool = MemPool_shm_at(MEMPOOL_SHM_BASE,MEMPOOL_SHM_PATH);}
		#define MEMSHMDT() {MemPool_shm_dt(pool);pool = NULL;}
		#define MEMSHMDESTORY() {MemPool_shm_destory(pool);pool = NULL;}

		#define MEMINIT() {if(MEMSHMISINIT()==1){MEMSHMAT();}else{MEMSHMINIT(1024*1024*10)}}
		#define MEMEXIT() MEMSHMDT()
		#define MEMDESTORY() MEMSHMDESTORY()
	#else
		#error "define MEMPOOL_INIT_STATIC_MEMORY or MEMPOOL_INIT_SHARED_MEMORY"
	#endif
#else
	#define MEMINIT()
	#define MALLOC(size) malloc(size)
	#define FREE(addr) free(addr)
	#define MEMEXIT()
	#define MEMDESTORY()
#endif

#ifdef __cplusplus
}
#endif
#endif /* MEMPOOL_H_ */
