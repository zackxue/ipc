
#include "firmware.h"

static void usage(const char* cmd)
{
	fprintf(stderr, "%s in.txt out.bin outsize\r\n", cmd);
}

int main(int argc, char** argv)
{
	if(argc < 3){
		usage(argv[0]);
		exit(1);
	}
	
	FIRMWARE_make_uboot_env(argv[1], argv[2], argv[3]);
	exit(0);
}


