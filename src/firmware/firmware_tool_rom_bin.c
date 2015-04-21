
#include "firmware.h"

static void usage(const char* cmd)
{
	fprintf(stderr, "%s conf.ini imgpath outpath\r\n", cmd);
}

int main(int argc, char** argv)//argv[1]:inifile ,  argv[2]:image path,  argv[3]:output path
{
	char fw_file[64];
	if(argc < 4){
		usage(argv[0]);
		exit(1);
	}
	
	if(FIRMWARE_init(argv[1], argv[2]) < 0){
		fprintf(stderr, "%s not found!\r\n", argv[1]);
		exit(1);
	}
	//////////////////////////////////////////////////////////////////////////
	FIRMWARE_make_rom(argv[3], "rom", fw_file);
	FIRMWARE_dump_rom(fw_file);
	FIRMWARE_make_bin(argv[3], "bin", fw_file);
	//////////////////////////////////////////////////////////////////////////
	FIRMWARE_destroy();
	exit(0);
}


