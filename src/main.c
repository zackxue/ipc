#include <stdio.h>
#include <stdlib.h>

#include "ipcam.h"

int main(int argc, char** argv)
{
	setlinebuf(stdout);
	setlinebuf(stdin);
	setvbuf(stderr,(char *)NULL,_IONBF,0);
	//
	return IPCAM_main(argc, argv);
}

