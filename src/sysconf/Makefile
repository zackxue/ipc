
include ./Makefile.in

SRC := $(SOURCES) sysconf_main.c
INC := $(patsubst %,-I%,$(INCLUDES))

all:
	gcc -g3 -Wall -D_GNU_SOURCE $(INC) $(SRC) -o a.out

clean:
	rm -Rf mtdblock_sysconf.bin a.out
	
