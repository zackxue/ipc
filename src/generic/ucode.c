
#include "ucode.h"
#include "time.h"

//#define UCODE_BLOCK_MTD "./test_ucode"

#define UCODE_BLOCK_SZ (1 * 1024)
#define UCODE_BLOCK_HEADER_SZ (64) // bytes
#define UCODE_BLOCK_OFFSET_SZ (8)

#define UCODE_BLOCK_PREFIX "JUAN UCODE VER"
#define UCODE_MAX_SZ_V10 (14)

typedef struct UCODE_BLOCK
{
	uint8_t header[UCODE_BLOCK_HEADER_SZ]; // JUAN UCODE VER
	uint8_t offset[UCODE_BLOCK_OFFSET_SZ];
	uint8_t content[UCODE_BLOCK_SZ - UCODE_BLOCK_HEADER_SZ - UCODE_BLOCK_OFFSET_SZ];
}UCODE_BLOCK_t;

static int ucode_rw(const char* mtd, off_t off_bottom, struct UCODE_BLOCK* block, int flag /* 1: write 0: read */)
{
	if(block){
		int fd = -1;
		int ret = 0;
		fd = open(mtd, O_RDWR);
		if(fd > 0){
			if(off_bottom < 0){
				ret = lseek(fd, -1 * UCODE_BLOCK_SZ, SEEK_END);
			}else{
				ret = lseek(fd, off_bottom, SEEK_SET);
				ret = lseek(fd, -1 * UCODE_BLOCK_SZ, SEEK_CUR);
			}
			if(flag){
				ret = write(fd, block, sizeof(struct UCODE_BLOCK));
			}else{
				ret = read(fd, block, sizeof(struct UCODE_BLOCK));
			}
			assert(ret == sizeof(struct UCODE_BLOCK));
			close(fd);
			fd = -1;
			return 0L;
		}
	}
	return -1L;
}

static void ucode_mk_header(uint8_t* ret_header, int ver_maj, int ver_min)
{
	if(ret_header){
		uint8_t header[UCODE_BLOCK_HEADER_SZ];
		memset(header, 0, sizeof(header));
		sprintf(header, "%s%d.%d", UCODE_BLOCK_PREFIX, ver_maj, ver_min);
		strcpy(ret_header, header);
	}
}

static uint8_t _ucode_mask(uint8_t ch)
{
	switch(ch)
	{
	case 0:
		ch = '.';
		break;
	case ' ':
		ch = rand() % 1 ? '{' : '}';
		break;
	default:
		if(ch >= '0' && ch <= '9'){
			ch -= '0'; ch += 3; ch %= 10; ch += '0';
		}else if(ch >= 'a' && ch <= 'z'){
			ch -= 'a'; ch += 6; ch %= 26; ch += 'a';
		}else if(ch >= 'A' && ch <= 'Z'){
			ch -= 'A'; ch += 9; ch %= 26; ch += 'A';
		}
		break;
	}
	return ch;
}

static uint8_t _ucode_umask(uint8_t ch)
{
	switch(ch)
	{
	case '.':
		ch = 0;
		break;
	case '{':
	case '}':
		ch = ' ';
		break;
	default:
		if(ch >= '0' && ch <= '9'){
			ch -= '0'; ch += 10; ch -= 3; ch %= 10; ch += '0';
		}else if(ch >= 'a' && ch <= 'z'){
			ch -= 'a'; ch += 26; ch -= 6; ch %= 26; ch += 'a';
		}else if(ch >= 'A' && ch <= 'Z'){
			ch -= 'A'; ch += 26; ch -= 9; ch %= 26; ch += 'A';
		}
		break;
	}
	return ch;
}

static void ucode_mask(void* block, ssize_t sz)
{
	if(block){
		int i = 0;
		char* str = (char*)block;
		for(i = 0; i < sz; ++i){
			str[i] = _ucode_mask(str[i]);
		}
	}
}

static void ucode_umask(void* block, ssize_t sz)
{
	if(block){
		int i = 0;
		char* str = (char*)block;
		for(i = 0; i < sz; ++i){
			str[i] = _ucode_umask(str[i]);
		}
	}
}

static void ucode_generate_rand_str(char* str, int sz)
{
	int i = 0;
	const char model[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[]{}<>()";
	const int model_sz = strlen(model);
	srand((unsigned int)time(NULL));
	for(i = 0; i < sz; ++i){
		str[i] = model[rand() % model_sz];
        }
}

int UCODE_write(const char* mtd, off_t off_bottom, const char* ucode, ssize_t sz)
{
	struct UCODE_BLOCK ucodeblk;
	uint32_t random_offset;
	memset(&ucodeblk, 0, sizeof(ucodeblk));
	if(sz > UCODE_MAX_SZ_V10 || sz <= 0){
		return -1;
	}
	// make header
	ucode_mk_header(ucodeblk.header, 1, 0);
	// make a random offset
	random_offset = rand() % (sizeof(ucodeblk.content) - UCODE_MAX_SZ_V10);
	sprintf(ucodeblk.offset, "%d", random_offset);
	// fill random string
	ucode_generate_rand_str(ucodeblk.content, sizeof(ucodeblk.content));
	// put the number
	memcpy(ucodeblk.content + random_offset, ucode, sz);
	// mask
	ucode_mask(&ucodeblk, sizeof(ucodeblk));
	// write
	return ucode_rw(mtd, off_bottom, &ucodeblk, 1);
}


int UCODE_read(const char* mtd, off_t off_bottom, char* ucode)
{
	struct UCODE_BLOCK ucodeblk;
	uint32_t offset;
	// read
	if(0 == ucode_rw(mtd, off_bottom, &ucodeblk, 0)){
		// umask
		ucode_umask(&ucodeblk, sizeof(ucodeblk));
		// get offset
		offset = atoi(ucodeblk.offset);
		// get ucode
		memcpy(ucode, ucodeblk.content + offset, UCODE_MAX_SZ_V10);
		ucode[UCODE_MAX_SZ_V10] = '\0';
		return 0L;
	}
	memset(ucode, '0', UCODE_MAX_SZ_V10);
	ucode[UCODE_MAX_SZ_V10] = '\0';
	return -1L;
}

int UCODE_check(const char* mtd, off_t off_bottom)
{
	struct UCODE_BLOCK ucodeblk;
	if(0 == ucode_rw(mtd, off_bottom, &ucodeblk, 0)){
		ucode_umask(ucodeblk.header, sizeof(ucodeblk.header));
		if(0 == memcmp(ucodeblk.header, UCODE_BLOCK_PREFIX, strlen(UCODE_BLOCK_PREFIX))){
			return 0L;
		}
	}
	return -1L;
}


