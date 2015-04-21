
#include "firmware.h"

extern int FIRMWARE_make_init(const char * info_file, const char* img_path);
extern void FIRMWARE_make_destroy();

extern int FIRMWARE_import_init();
extern void FIRMWARE_import_destroy();

extern int FIRMWARE_upgrade_init();
extern void FIRMWARE_upgrade_destroy();

int FIRMWARE_init(const char* info_file, const char* img_path)
{
	if(0 == FIRMWARE_make_init(info_file, img_path)){
		FIRMWARE_import_init();
		FIRMWARE_upgrade_init();
		return 0;
	}
	return -1;
}

void FIRMWARE_destroy()
{
	FIRMWARE_upgrade_destroy();
	FIRMWARE_import_destroy();
	FIRMWARE_make_destroy();
}

