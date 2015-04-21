
#include "version.h"
#include "firmware.h"
#include "inifile.h"
#include "md5sum.h"


// block -> flash -> firmware

typedef struct FwBlockCfg
{
	char* name;
	char* file;
	off_t flash_offset;
	uint32_t mk_rom; // making for the rom file, which is for upgrading
	uint32_t mk_bin; // making for the bin file, which is for programmer
}FwBlockCfg_t;

typedef struct FwFlashCfg
{
	char* name;
	ssize_t size;
	int block_cnt;
	FwBlockCfg_t* block_cfg;
}FwFlashCfg_t;

typedef struct FwMakeCfg
{
	char* file_magic;
	char* file_prefix;
	char* dev_name;
	char* dev_model;
	ssize_t flash_size;
	int flash_cnt;
	FwFlashCfg_t* flash_cfg;
	char* uboot_env;
}FwMakeCfg_t;
static FwMakeCfg_t* _fw_make_cfg = NULL;

// such as convert 1K -> 1024
static int firmware_cfg_read_size(const char* val)
{
	int size = atoi(val);
	if(strchr(val, 'K')){
		size *= 1024;
	}else if(strchr(val, 'M')){
		size *= 1024 * 1024;
	}
	return size;
}

extern int FIRMWARE_env_make(const char* env_list, ssize_t env_size, const char* env_file);


int FIRMWARE_make_init(const char* info, const char* img_path)
{
	int i = 0, ii = 0;
	lpINI_PARSER inf = NULL;
	if(!_fw_make_cfg){
		_fw_make_cfg = calloc(sizeof(FwMakeCfg_t), 1);
		// init
		inf = OpenIniFile(info);
		if(inf){
			char ini_buf[1024] = {""};
			_fw_make_cfg->file_magic = strdup(inf->read_text(inf, "FIRMWARE", "magic", "", ini_buf, sizeof(ini_buf)));
			_fw_make_cfg->file_prefix = strdup(inf->read_text(inf, "FIRMWARE", "prefix", "FW", ini_buf, sizeof(ini_buf)));
			_fw_make_cfg->dev_name = strdup(inf->read_text(inf, "FIRMWARE", "name", "NAME", ini_buf, sizeof(ini_buf)));
			_fw_make_cfg->dev_model = strdup(inf->read_text(inf, "FIRMWARE", "model", "MODEL", ini_buf, sizeof(ini_buf)));
			_fw_make_cfg->flash_size = firmware_cfg_read_size(inf->read_text(inf, "FIRMWARE", "flash_size", "", ini_buf, sizeof(ini_buf)));
			_fw_make_cfg->flash_cnt = inf->read_int(inf, "FIRMWARE", "flash", 0);
			//assert(_fw_info->block_cnt > 0);
			// read block
			_fw_make_cfg->flash_cfg = calloc(sizeof(FwFlashCfg_t), _fw_make_cfg->flash_cnt);
			for(i = 0; i < _fw_make_cfg->flash_cnt; ++i){
				FwFlashCfg_t* flash_cfg = _fw_make_cfg->flash_cfg + i;
				char str_section[64];
				sprintf(str_section, "FLASH%d", i);
				// the mtdblock
				flash_cfg->name = strdup(inf->read_text(inf, str_section, "name", "", ini_buf, sizeof(ini_buf)));
				flash_cfg->size = firmware_cfg_read_size(inf->read_text(inf, str_section, "size", "0", ini_buf, sizeof(ini_buf)));

//				printf("flash=%s size=%d block=%d\r\n", flash_cfg->name, flash_cfg->size, flash_cfg->block_cnt);
					
				// flash block
				flash_cfg->block_cnt = inf->read_int(inf, str_section, "block", 0);
				if(flash_cfg->block_cnt > 0){
					flash_cfg->block_cfg = calloc(sizeof(FwBlockCfg_t), flash_cfg->block_cnt);
					// block section
					for(ii =0; ii < flash_cfg->block_cnt; ++ii){
						FwBlockCfg_t* block_cfg = flash_cfg->block_cfg + ii;
						sprintf(str_section, "FLASH%d_BLOCK%d", i, ii);
						block_cfg->name = strdup(inf->read_text(inf, str_section, "name", "", ini_buf, sizeof(ini_buf)));
						if(!img_path){
							block_cfg->file = strdup(inf->read_text(inf, str_section, "file", "", ini_buf, sizeof(ini_buf)));
						}else{
							char path_file[128];
							strcpy(path_file, img_path);
							if('/' != path_file[strlen(path_file) - 1]){
								strcat(path_file, "/");
							}
							strcat(path_file, basename(strdupa(inf->read_text(inf, str_section, "file", "", ini_buf, sizeof(ini_buf)))));
							block_cfg->file = strdup(path_file);
						}
						block_cfg->flash_offset = firmware_cfg_read_size(inf->read_text(inf, str_section, "offset", "0", ini_buf, sizeof(ini_buf)));
						block_cfg->mk_rom = inf->read_bool(inf, str_section, "rom", TRUE);
						block_cfg->mk_bin = inf->read_bool(inf, str_section, "bin", TRUE);

//						printf("\tblock=%s file=%s offset=%u rom=%s bin=%s\r\n", block_cfg->name, block_cfg->file, (int)block_cfg->flash_offset, block_cfg->mk_rom ? "yes" : "no", block_cfg->mk_bin ? "yes" : "no");
					}
				}else{
					_fw_make_cfg->flash_cfg[i].block_cfg = NULL;
				}
			}
			_fw_make_cfg->uboot_env = strdup(inf->read_text(inf, "FIRMWARE", "uboot_env", "", ini_buf, sizeof(ini_buf)));
			CloseIniFile(inf);
			return 0;
		}
	}
	return -1;
}

void FIRMWARE_make_destroy()
{
	int i = 0, ii = 0;
	if(_fw_make_cfg){
		
		free(_fw_make_cfg->uboot_env);

		for(i = 0; i < _fw_make_cfg->flash_cnt; ++i){
			FwFlashCfg_t* flash_cfg = _fw_make_cfg->flash_cfg + i;
			if(flash_cfg->block_cnt > 0){
				// block section
				for(ii =0; ii < flash_cfg->block_cnt; ++ii){
					FwBlockCfg_t* block_cfg = flash_cfg->block_cfg + ii;
					free(block_cfg->name);
					free(block_cfg->file);
				}
				free(flash_cfg->block_cfg);
			}
			free(flash_cfg->name);
		}

		free(_fw_make_cfg->flash_cfg);
		free(_fw_make_cfg->file_magic);
		free(_fw_make_cfg->file_prefix);
		free(_fw_make_cfg->dev_name);
		free(_fw_make_cfg->dev_model);

		free(_fw_make_cfg);
		_fw_make_cfg = NULL;
	}
}

static int firmware_generate_file(const char* file_name, const void* file_data, ssize_t file_sz)
{
	FILE* fid = fopen(file_name, "wb");
	if(fid){
		int ret = fwrite(file_data, 1, file_sz, fid);
		fclose(fid);
		if(ret == file_sz){
			return 0;
		}
	}
	return -1;
}

static const char* firmware_mk_base_header(const char* magic, const char* file_prefix, const char* dev_name, const char* dev_model,
	FwVersion_t version, FwVersionLimit_t version_limit, FwHeader_t* ret_hdr, char* ret_bname)
{
	time_t cur_time;
	struct tm cur_tm;

	if(ret_hdr){
		// mk header
		strncpy(ret_hdr->magic, magic, sizeof(ret_hdr->magic) - 1);
		// mk version
		ret_hdr->version = version;
		ret_hdr->version_limit = version_limit;
		// mk time
		cur_time = time(NULL);
		localtime_r(&cur_time, &cur_tm);
		ret_hdr->release_time.year = cur_tm.tm_year + 1900;
		ret_hdr->release_time.mon = cur_tm.tm_mon + 1;
		ret_hdr->release_time.mday = cur_tm.tm_mday;
		ret_hdr->release_time.hour = cur_tm.tm_hour;
		ret_hdr->release_time.min = cur_tm.tm_min;
		ret_hdr->release_time.sec = cur_tm.tm_sec;
		// others
		ret_hdr->block_cnt = 0;
		memset(ret_hdr->block, 0, sizeof(ret_hdr->block));
		memset(ret_hdr->md5_sum, 0, sizeof(ret_hdr->md5_sum));

		sprintf(ret_bname, "%s" "_" "%04d%02d%02d" "_" "%s_%s" "_" "%d_%d_%d_%s" ,
			file_prefix,
			ret_hdr->release_time.year, ret_hdr->release_time.mon, ret_hdr->release_time.mday,
			dev_name, dev_model,
			ret_hdr->version.major, ret_hdr->version.minor, ret_hdr->version.revision, ret_hdr->version.extend);

		return ret_bname;
	}
	return NULL;
}

int FIRMWARE_make_rom(const char* path, const char* ext_name, char* ret_file)
{
	int i = 0, ii = 0;
	int ret = 0;
	
	void* fw_file = NULL;
	int fw_file_offset = 0;
	FwHeader_t* fw_header = NULL;
	char fw_file_name[128];
	char fw_base_name[128];
	FwVersion_t fw_ver;
	FwVersionLimit_t fw_ver_limit;
	char* fw_md5 = NULL;

	memset(&fw_ver, 0, sizeof(fw_ver));
	memset(&fw_ver_limit, 0, sizeof(fw_ver_limit));
	fw_ver.major = SWVER_MAJ;
	fw_ver.minor = SWVER_MIN;
	fw_ver.revision = SWVER_REV;
	strcpy(fw_ver.extend, SWVER_EXT);

	FILE* fid = NULL;
	
	if(_fw_make_cfg){
		fw_file = calloc(sizeof(fw_header) + _fw_make_cfg->flash_size, 1);
//		firmware_fill_rand(fw_file, fw_sz);
		fw_header = (FwHeader_t*)fw_file; // header at the beginning of the data
		// make the header
		sprintf(fw_file_name, "%s/%s.%s",
			path,
			firmware_mk_base_header(
				_fw_make_cfg->file_magic,
				_fw_make_cfg->file_prefix,
				_fw_make_cfg->dev_name,
				_fw_make_cfg->dev_model,
				fw_ver,
				fw_ver_limit,
				fw_header,fw_base_name),
			ext_name);
		// offset to payload area
		fw_file_offset = sizeof(FwHeader_t);
		
		// foreach all the blocks
		for(i = 0; i < _fw_make_cfg->flash_cnt; ++i){
			FwFlashCfg_t* const flash_cfg = _fw_make_cfg->flash_cfg + i;
//			printf("flash_cfg->block_cnt = %d\r\n", flash_cfg->block_cnt);
			for(ii = 0; ii < flash_cfg->block_cnt; ++ii){
				FwBlockCfg_t* const block_cfg = flash_cfg->block_cfg + ii;

//				printf("block_cfg->mk_rom = %d\r\n", flash_cfg->block_cnt);
				if(block_cfg->mk_rom){
					FwBlock_t* const block = fw_header->block + fw_header->block_cnt;
					
					// need to make rom
					strcpy(block->name, block_cfg->name);
					strcpy(block->flash, flash_cfg->name);
					block->flash_offset = block_cfg->flash_offset;
					block->data_offset = fw_file_offset;
					block->data_size = 0;

//					printf("block=%d name=%s size=%d/%d\r\n", fw_header->block_cnt, block->name, block->data_size, flash_cfg->size);
//					if(0 == strcmp(block_cfg->name, "ubootenv")){
//						// make the u-boot env
//						ret = FIRMWARE_env_make(_fw_make_cfg->uboot_env, flash_cfg->size, block_cfg->file);
//						assert(0 == ret);
//					}

					fid = fopen(block_cfg->file, "rb");
					if(fid){
						ret =  fread(fw_file + block->data_offset, 1, flash_cfg->size, fid);
						fclose(fid);
						block->data_size = ret;
						
						// offset to next block
						ret += 128 * 1024 - 1;
						ret /= 128 * 1024;
						ret *= 128 * 1024; // align to 128k
						fw_file_offset += ret;
						fw_header->block_cnt++;
					}
				}
			}
		}

		// put md5
		fw_md5 = md5sum_buffer(fw_file, fw_file_offset);
		strcpy(fw_header->md5_sum, fw_md5);

		ret = firmware_generate_file(fw_file_name, fw_file, fw_file_offset);
		assert(0 == ret);

		free(fw_file);
		fw_file = NULL;

		if(ret_file){
			strcpy(ret_file, fw_file_name);
		}
		return 0;
	}
	return -1;
}

#define FIRMWARE_DUMP(fmt...) \
	do{ printf("\033[1;32m"); printf(fmt); printf("\033[0m\r\n");}while(0)
		

void FIRMWARE_dump_rom(const char* rom_file)
{
	int i = 0;
	int ret = 0;
	ssize_t fw_sz = 0;
	FILE* fid = fopen(rom_file, "rb");
	FwHeader_t* fw_header = NULL;
	FwVersion_t* fw_ver = NULL;
	FwVersion_t* fw_ver_low = NULL;
	FwVersion_t* fw_ver_high = NULL;
	FwTime_t* fw_time = NULL;
	char* fw_md5 = NULL;
	char* fw_check = NULL;
	if(fid){
		
		// get rom_file size
		ret = fseek(fid, 0, SEEK_END);
		assert(0 == ret);
		fw_sz = ftell(fid);
		ret = fseek(fid, 0, SEEK_SET);
		assert(0 == ret);
		// get the data from rom_file
		fw_header = calloc(fw_sz, 1);
		ret = fread(fw_header, 1, fw_sz, fid);
		fclose(fid);
		// analyse
		FIRMWARE_DUMP("[FIRMWARE]");
		FIRMWARE_DUMP("magic = %s", fw_header->magic);
		fw_ver = &fw_header->version;
		fw_ver_low = &fw_header->version_limit.low_version;
		fw_ver_high = &fw_header->version_limit.high_version;
		FIRMWARE_DUMP("version = %d.%d.%d %s (%d.%d.%d , %d.%d.%d)", fw_ver->major, fw_ver->minor, fw_ver->revision, fw_ver->extend,
			fw_ver_low->major, fw_ver_low->minor, fw_ver_low->revision,
			fw_ver_high->major, fw_ver_high->minor, fw_ver_high->revision);
		fw_time = &fw_header->release_time;
		FIRMWARE_DUMP("release time: %04d/%02d/%02d %02d:%02d:%02d",
			fw_time->year, fw_time->mon, fw_time->mday, fw_time->hour, fw_time->min, fw_time->sec);
		fw_md5 = strdup(fw_header->md5_sum);
		memset(fw_header->md5_sum, 0, sizeof(fw_header->md5_sum));
		fw_check = md5sum_buffer(fw_header, fw_sz);
		FIRMWARE_DUMP("md5 sum: %s", fw_md5);
		FIRMWARE_DUMP("  check: %s %s", fw_check, 0 == memcmp(fw_md5, fw_check, strlen(fw_md5)) ? "match" : "error");
		FIRMWARE_DUMP("block = %d", fw_header->block_cnt);
		FIRMWARE_DUMP("\r\n");

		for(i = 0; i < fw_header->block_cnt; ++i){
			char block_name[32];
			FwBlock_t* fw_block = fw_header->block + i;
			float block_sz = (double)fw_block->data_size;
			sprintf(block_name, "[BLOCK%d]", i);
			FIRMWARE_DUMP("%s", block_name);
			FIRMWARE_DUMP("name = %s", fw_block->name);
			FIRMWARE_DUMP("flash = %s", fw_block->flash);
			FIRMWARE_DUMP("flash offset = %d", (int)fw_block->flash_offset);
			FIRMWARE_DUMP("offset = %d", (int)fw_block->data_offset);
			block_sz /= 1024 * 1024;
			FIRMWARE_DUMP("size = %d (%.3fMB)", fw_block->data_size, block_sz);
			FIRMWARE_DUMP("\r\n");
		}
		fsync(1);

		free(fw_header);
	}
}


int FIRMWARE_make_bin(const char* path, const char* ext_name, char* ret_file)
{
	int i = 0, ii = 0;
	int ret = 0;
	
	void* fw_file = NULL;
	int fw_file_offset = 0;
	
	char fw_file_name[128];
	char fw_file_bname[128];
	FwHeader_t fw_header;
	FwVersion_t fw_ver;
	FwVersionLimit_t fw_ver_limit;
	memset(&fw_ver, 0, sizeof(fw_ver));
	memset(&fw_ver_limit, 0, sizeof(fw_ver_limit));
	fw_ver.major = SWVER_MAJ;
	fw_ver.minor = SWVER_MIN;
	fw_ver.revision = SWVER_REV;
	strcpy(fw_ver.extend, SWVER_EXT);

	char* fw_md5 = NULL;

	FILE* fid = NULL;
	
	if(_fw_make_cfg){
		fw_file = calloc(_fw_make_cfg->flash_size, 1);
		fw_file_offset = 0;

		sprintf(fw_file_name, "%s/%s.%s",
			path,
			firmware_mk_base_header(
				_fw_make_cfg->file_magic,
				_fw_make_cfg->file_prefix,
				_fw_make_cfg->dev_name,
				_fw_make_cfg->dev_model,
				fw_ver, fw_ver_limit,
				&fw_header,
				fw_file_bname),
			ext_name);
		
		// foreach all the blocks
		for(i = 0; i < _fw_make_cfg->flash_cnt; ++i){
			FwFlashCfg_t* const flash_cfg = _fw_make_cfg->flash_cfg + i;
			for(ii = 0; ii < flash_cfg->block_cnt; ++ii){
				FwBlockCfg_t* const block_cfg = flash_cfg->block_cfg + ii;
				if(block_cfg->mk_bin){
					fid = fopen(block_cfg->file, "rb");
					if(fid){
						// ful-fill the firmware file
						ret =  fread(fw_file + fw_file_offset + block_cfg->flash_offset, 1, flash_cfg->size, fid);
						assert(ret > 0);
						printf("bin %s size = %d\r\n", block_cfg->name, ret);
						fclose(fid);
					}
				}
			}
			fw_file_offset += flash_cfg->size;
		}

		ret = firmware_generate_file(fw_file_name, fw_file, _fw_make_cfg->flash_size);
		assert(0 == ret);

		fw_md5 = md5sum_buffer(fw_file, _fw_make_cfg->flash_size);
		printf("md5 buffer\t%s\r\n", fw_md5);
		fw_md5 = md5sum_file(fw_file_name);
		printf("md5 file\t%s\r\n", fw_md5);

		free(fw_file);
		fw_file = NULL;

		if(ret_file){
			strcpy(ret_file, fw_file_name);
		}
		return 0;
	}
	return -1;
}

int FIRMWARE_make_uboot_env(const char* uboot_env_text, const char* uboot_env_bin, const char* uboot_env_sz)
{
	return FIRMWARE_env_make(uboot_env_text, firmware_cfg_read_size(uboot_env_sz), uboot_env_bin);
}

ssize_t FIRMWARE_max_bin_size()
{
	return _fw_make_cfg->flash_size;
}

ssize_t FIRMWARE_max_rom_size()
{
	return FIRMWARE_max_bin_size() + sizeof(FwHeader_t); // with additional header
}

