
include ./Makefile.in

SRC := $(SOURCES) ../../api/src/md5sum.c ../../api/src/inifile.c
INC := $(patsubst %,-I%,$(INCLUDES))

LINK := -lpthread

TOOL_CC := gcc -g3 -Wall -DLINUX -D_GNU_SOURCE $(INC) $(SRC)

TOOL_ROM_BIN := mk_rom_bin
TOOL_UBOOTENV := mk_uboot_env

.PHONY : all clean

all : $(TOOL_UBOOTENV) $(TOOL_ROM_BIN)

$(TOOL_ROM_BIN) : $(SRC)
	$(TOOL_CC) firmware_tool_rom_bin.c -o $@ $(LINK)

$(TOOL_UBOOTENV) : $(SRC)
	$(TOOL_CC) firmware_tool_uboot_env.c -o $@ $(LINK)

clean:
	rm -Rf $(TOOL_ROM_BIN) $(TOOL_UBOOTENV)
	
