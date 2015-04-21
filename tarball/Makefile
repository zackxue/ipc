
MAKE := make
MKDIR := mkdir -p
RM := rm -Rf
CP := cp -pf
EXTRACT := tar -xvf
ECHO := echo

OPENSSL_TARBALL := $(CURDIR)/openssl-1.0.1c.tar.gz
ZLIB_TARBALL := $(CURDIR)/zlib-1.2.7.tar.tar
UUID_TARBALL := $(CURDIR)/e2fsprogs-1.42.6.tar.gz

INC_PATH ?= ./include
LIB_PATH ?= ./lib

#CROSS ?= @arm-hismall-linux-
#CROSS ?= arm-hisiv100nptl-linux-
CROSS ?=
CC := $(CROSS)gcc
CXX := $(CROSS)g++
CPP := $(CROSS)g++
AR := $(CROSS)ar
STRIP := $(CROSS)strip

###########################################################
###########################################################

.PHONY: all clean openssl zlib uuid

all: uuid openssl zlib
	@$(ECHO) $(INC_PATH)
	@$(ECHO) $(LIB_PATH)

openssl: $(OPENSSL_TARBALL)
	$(RM) $(CURDIR)/$@
	$(MKDIR) $(CURDIR)/$@
	$(EXTRACT) $< -C $(CURDIR)/$@ --strip 1
	$(CURDIR)/$@/config no-asm
	$(MAKE) -C $(CURDIR)/$@ clean
	$(MAKE) -C $(CURDIR)/$@ CC=$(CC) CXX="$(CXX)" CPP="$(CXX)" AR="$(AR) -r"
	$(MKDIR) $(INC_PATH)/openssl/ 
	$(CP) $(CURDIR)/$@/include/openssl/*.h $(INC_PATH)/openssl/
	$(STRIP) -g $(CURDIR)/$@/lib*.a
	$(CP) $(CURDIR)/$@/lib*.a $(LIB_PATH)
	$(RM) $(CURDIR)/$@

zlib: $(ZLIB_TARBALL)
	$(RM) $(CURDIR)/$@
	$(MKDIR) $(CURDIR)/$@
	$(EXTRACT) $< -C $(CURDIR)/$@ --strip 1
	cd $(CURDIR)/$@; ./configure --static
	$(MAKE) -C $(CURDIR)/$@ clean 
	$(MAKE) -C $(CURDIR)/$@ CC="$(CC)" CPP="$(CC)" AR="$(AR)"
	$(CP) $(CURDIR)/$@/zlib.h $(CURDIR)/$@/zconf.h $(INC_PATH)/
	$(STRIP) -g $(CURDIR)/$@/libz.a
	$(CP) $(CURDIR)/$@/libz.a $(LIB_PATH)
	$(RM) $(CURDIR)/$@

uuid: $(UUID_TARBALL)
	$(RM) $(CURDIR)/$@
	$(MKDIR) $(CURDIR)/$@
	$(EXTRACT) $< -C $(CURDIR)/$@ --strip 1
	cd $(CURDIR)/$@; ./configure --disable-tls --disable-nls
	$(MAKE) -C $(CURDIR)/$@ clean 
	$(MAKE) -C $(CURDIR)/$@ # make e2fsprogs first
	$(MAKE) -C $(CURDIR)/$@/lib/uuid clean
	$(MAKE) -C $(CURDIR)/$@/lib/uuid CC="$(CC)" CPP="$(CC)" AR="$(AR)"
	$(CP) $(CURDIR)/$@/lib/libuuid.a $(LIB_PATH)
	$(MKDIR) $(INC_PATH)/uuid/
	$(CP) $(CURDIR)/$@/lib/uuid/*.h $(INC_PATH)/uuid/
	$(RM) $(CURDIR)/$@


clean:


