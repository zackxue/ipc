
SRC_PATH := .
#KERN_PATH := /Hi3511_VSSDK_V1.1.2.3_SPC003/pub/kbuild-LESS_REL/
KERN_PATH :=/root/nfs/gm_ipc/Hi3518_SDK_V1.0.3.0/osdrv/kernel/linux-3.0.y

SRC := $(wildcard $(SRC_PATH)/*.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.o,%.d,$(OBJ))
INC := -Iinclude -I$(SRC_PATH) -I/root/nfs/gm_ipc/app_rebulid/include

ECHO := @echo
RM := @rm -Rf

#CROSS ?= arm-linux-gcc
CROSS := arm-hisiv100nptl-linux-
CC := $(CROSS)gcc
AR := $(CROSS)ar
#EXTRA_CFLAGS += $(INC) -DTRUE="(1==1)" -DFALSE="(1==0)" -D_GNU_SOURCE -g3 -O0 -Wall 
EXTRA_CFLAGS += $(INC)
LDFLAGS ?= -lpthread -lm

###########################################################
###########################################################
# module
obj-m := i2cm_18a.o rtc_18a.o watchdog.o
###########################################################
###########################################################

TARGET := lib$(shell basename $(CURDIR)).a

.PHONY : all clean module

%.o : %.c
	$(CC) $(EXTRA_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<


$(TARGET) : $(OBJ)
	$(AR) -r $@ $^

module :
	$(MAKE) -C $(KERN_PATH) M=$(PWD) modules

rtc	: $(OBJ_RTC)

	$(CC) $(EXTRA_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	

# dependency
sinclude $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)
	$(MAKE) -C $(KERN_PATH) M=$(PWD) clean
	$(RM) $(patsubst %.o,%.mod.c,$(obj-m)) $(patsubst %.o,%.mod.o,$(obj-m))

