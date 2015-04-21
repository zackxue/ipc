
#include "firmware.h"

#define FIRMWARE_UPGRADE_SPEED (128 * 1024)

typedef struct FwUpgrade
{
	ssize_t upgrade_size;
	ssize_t total_size;

	pthread_t upgrade_tid;
	uint32_t upgrade_trigger;
}FwUpgrade_t;
static FwUpgrade_t* _fw_upgrade = NULL;

int FIRMWARE_upgrade_get_rate()
{
	if(0 == _fw_upgrade->upgrade_size){
		return 0;
	}
	return _fw_upgrade->upgrade_size * 100 / _fw_upgrade->total_size;
}

static int firmware_upgrade_flash(int flash_fd, off_t flash_offset, int fw_fd, off_t fw_offset, ssize_t data_size)
{
	int ret = 0;
	uint8_t buf[FIRMWARE_UPGRADE_SPEED];

	assert(flash_fd > 0 && fw_fd > 0);

	ret = lseek(fw_fd, fw_offset, SEEK_SET);
	assert(fw_offset == ret);
	ret = lseek(flash_fd, flash_offset, SEEK_SET);
	assert(flash_offset == ret);

	do
	{
		ssize_t read_sz = data_size >= FIRMWARE_UPGRADE_SPEED ? FIRMWARE_UPGRADE_SPEED : data_size;
		// read from firmware
		ret = read(fw_fd,  buf, read_sz);
		assert(ret == read_sz);
		// write to flash
		ret = write(flash_fd, buf, read_sz);
		assert(ret == read_sz);
//		fdatasync(flash_fd);
		fsync(flash_fd);

		data_size -= read_sz;
		_fw_upgrade->upgrade_size += read_sz;

	}while(data_size > 0);

	return 0;
}

static void* firmware_upgrade(void* arg)
{
	printf("%s start\n", __FUNCTION__);
	int i = 0;
	int ret = 0;
	FwHeader_t fw_header;
	int fid = -1;

//	// detach thread
//	pthread_detach(pthread_self());

	fid = open(FIRMWARE_IMPORT_FILE, O_RDONLY);
	assert(fid > 0);
	ret = read(fid, &fw_header, sizeof(fw_header));
	assert(sizeof(fw_header) == ret);

//	FIRMWARE_dump_rom(FIRMWARE_IMPORT_FILE);

	// get total size to upgrade
	_fw_upgrade->total_size = 0;
	_fw_upgrade->upgrade_size = 0;
	for(i = 0; i < fw_header.block_cnt; ++i){
		_fw_upgrade->total_size += fw_header.block[i].data_size;
	}
//	printf("upgrade total size = %d\r\n", _fw_upgrade->total_size);

	// upgrade
	for(i = 0; i < fw_header.block_cnt && _fw_upgrade->upgrade_trigger; ++i){
		int flash_fd = open(fw_header.block[i].flash, O_RDWR);
//		printf("upgrade %s offset=%d size = %d\r\n", fw_header.block[i].flash, fw_header.block[i].data_offset, fw_header.block[i].data_size);
		if(flash_fd > 0){
			firmware_upgrade_flash(flash_fd, fw_header.block[i].flash_offset, fid, fw_header.block[i].data_offset, fw_header.block[i].data_size);
			close(flash_fd);
			flash_fd = -1;
		}
	}

	close(fid);
	printf("%s end\n", __FUNCTION__);

	system("reboot"); // must reboot
	
	pthread_exit(NULL);
}

int FIRMWARE_upgrade_start()
{
	printf("%s start\n", __FUNCTION__);
	if(!_fw_upgrade->upgrade_tid){
		int ret = 0;
		if(FIRMWARE_import_check()){
			// import file check ok!!
			_fw_upgrade->upgrade_trigger = true;
			ret = pthread_create(&_fw_upgrade->upgrade_tid, NULL, firmware_upgrade, NULL);
			assert(0 == ret);
			return 0;
		}
	}
	return -1;
}

int FIRMWARE_upgrade_wait()
{
	if(_fw_upgrade->upgrade_tid){
		pthread_join(_fw_upgrade->upgrade_tid, NULL);
		_fw_upgrade->upgrade_trigger = false;
		_fw_upgrade->upgrade_tid = (pthread_t)NULL;
		return 0;
	}
	return -1;
}

int FIRMWARE_upgrade_cancel()
{
	if(_fw_upgrade->upgrade_tid){
		_fw_upgrade->upgrade_trigger = false;
		return FIRMWARE_upgrade_wait();
	}
	return -1;
}

int FIRMWARE_upgrade_init()
{
	if(!_fw_upgrade){
		_fw_upgrade = calloc(sizeof(FwUpgrade_t), 1);

		_fw_upgrade->upgrade_size = 0;
		_fw_upgrade->total_size = 0;
		_fw_upgrade->upgrade_tid = (pthread_t)NULL;
		_fw_upgrade->upgrade_trigger = false;
		return 0;
	}
	return -1;
}

void FIRMWARE_upgrade_destroy()
{
	if(_fw_upgrade){
		FIRMWARE_upgrade_cancel();

		free(_fw_upgrade);
		_fw_upgrade = NULL;
	}
}


