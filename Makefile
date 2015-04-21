
#DEVICE_MODEL ?= hi3507_inc
#DEVICE_MODEL ?= hi3518a_inc
DEVICE_MODEL ?= hi3518c_inc
#DEVICE_MODEL ?= hi3516c_inc

INC_PATH := $(CURDIR)/include
LIB_PATH := $(CURDIR)/lib
BIN_PATH := $(CURDIR)/bin
DBG_PATH := $(CURDIR)/debug

# Hisilicon Hi3507 inception
ifeq ($(DEVICE_MODEL),hi3507_inc)
PRODUCT_CLASS := ipcam
PRODUCT_MODEL := hi3507-inception
SOC_MODEL := HI3507
CROSS := arm-hismall-linux-
endif
# Hisilicon Hi3518a inception
ifeq ($(DEVICE_MODEL),hi3518a_inc)
PRODUCT_CLASS := ipcam
PRODUCT_MODEL := hi3518a-inception
SOC_MODEL := HI3518A
CROSS := arm-hisiv100nptl-linux-
SERISE_CODE := C1
endif

ifeq ($(DEVICE_MODEL),hi3518c_inc)
PRODUCT_CLASS := ipcam
PRODUCT_MODEL := hi3518c-inception
SOC_MODEL := HI3518C
CROSS := arm-hisiv100nptl-linux-
SERISE_CODE := C2
endif

ifeq ($(DEVICE_MODEL),hi3516c_inc)
PRODUCT_CLASS := ipcam
PRODUCT_MODEL := hi3516c-inception
SOC_MODEL := HI3516C
CROSS := arm-hisiv100nptl-linux-
SERISE_CODE := C3
endif

MAKE := @make

.PHONY: all src sdk api sdk_clean tarball image
	
#info:
#	@echo $(PRODUCT_CLASS)
#	@echo $(PRODUCT_MODEL)
#	@echo $(SOC_MODEL)
#


src: srclib
	$(MAKE) -C src CROSS="@$(CROSS)" \
		PRODUCT_CLASS="$(PRODUCT_CLASS)" \
		PRODUCT_MODEL="$(PRODUCT_MODEL)" \
		SOC_MODEL="$(SOC_MODEL)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \
		SERISE_CODE="$(SERISE_CODE)" \

srclib:
	rm -f $(BIN_PATH)/ipcam_app
	$(MAKE) -C src srclib CROSS="@$(CROSS)" \
		PRODUCT_CLASS="$(PRODUCT_CLASS)" \
		PRODUCT_MODEL="$(PRODUCT_MODEL)" \
		SOC_MODEL="$(SOC_MODEL)" \
		INC_PATH="$(INC_PATH)" \
		LIB_PATH="$(LIB_PATH)" \
		BIN_PATH="$(BIN_PATH)" \
		DBG_PATH="$(DBG_PATH)" \
		SERISE_CODE="$(SERISE_CODE)" \
	
sdk:
	$(MAKE) --directory=sdk SOC_MODEL="$(SOC_MODEL)"

api:
	$(MAKE) --directory=api CROSS=$(CROSS)

	
tarball:
	$(MAKE) --directory=sdk clean

image: src
	#cd ../release
	$(MAKE) -C ../release DEVICE_MODEL="$(DEVICE_MODEL)"

all: clean sdk srclib src image

clean:
	rm -f $(shell find $(CURDIR) -name *.o)
	rm -f $(shell find $(CURDIR) -name *.d)
