
#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__
#ifdef __cplusplus
extern	"C"	{
#endif

//#define NDEBUG
#include <assert.h>
#include <sys/types.h>
#include <libgen.h>
#include <linux/limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>
#include <errno.h>

#include <pthread.h>
#include <libgen.h>

typedef struct FwVersion
{
	int major, minor, revision;
	char extend[32];
}FwVersion_t;

typedef struct FwVersionLimit
{
	FwVersion_t low_version;
	FwVersion_t high_version;
}FwVersionLimit_t;

typedef struct FwTime
{
	int year, mon, mday;
	int hour, min, sec;
}FwTime_t;

typedef struct FwBlock
{
	char name[32];
	char flash[32];
	off_t flash_offset;
	// in firmware file
	ssize_t data_size;
	off_t data_offset;
}FwBlock_t;

#define FIRMWARE_HEADER_SIZE (256 * 1024)
typedef struct FwHeader
{
	union
	{
		uint8_t pad[FIRMWARE_HEADER_SIZE];
		struct
		{
			uint8_t pad1[64]; // 128 bytes forbidden zeros
			char magic[64]; // the magic string
			uint8_t pas2[64];
			FwVersion_t version;
			FwVersionLimit_t version_limit;
			FwTime_t release_time;
			int block_cnt;
			FwBlock_t block[16]; // reseved 16 block, generally use 6 in dvr
			// sum check
			// make file -> md5 sum -> put md5 to file -> get md5 from file -> clear md5 sum -> md5 match
			char md5_sum[64]; // clear this bytes when sum check the file
		};
	};	
}FwHeader_t;


#define FIRMWARE_IMPORT_FILE "/tmp/firmware.rom"

extern void FIRMWARE_dump_rom(const char* rom_file);

extern ssize_t FIRMWARE_max_bin_size();
extern ssize_t FIRMWARE_max_rom_size();

extern int FIRMWARE_make_rom(const char* path, const char* ext_name, char* ret_file);
extern int FIRMWARE_make_bin(const char* path, const char* ext_name, char* ret_file);

extern void FIRMWARE_import_clear();
extern int FIRMWARE_import_get_rate();

typedef int (*FIRMWARE_IMPORT_MATCH)(const char* filename);
extern int FIRMWARE_import_from_folder(const char* folder, FIRMWARE_IMPORT_MATCH match);
extern int FIRMWARE_import_from_ftp(const char* ftp_addr, const char* user, const char* password);
extern int FIRMWARE_import_from_http(const char* addr, uint16_t port);

extern uint32_t FIRMWARE_import_check();

extern int FIRMWARE_upgrade_get_rate();
extern int FIRMWARE_upgrade_start();
extern int FIRMWARE_upgrade_wait();
extern int FIRMWARE_upgrade_cancel();

extern int FIRMWARE_init(const char* info_file, const char* img_path);
extern void FIRMWARE_destroy();

#ifdef __cplusplus
}
#endif
#endif	//__FIRMWARE_

