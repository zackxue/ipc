
INC_PATH ?= $(CURDIR)/
LIB_PATH ?= $(CURDIR)/
BIN_PATH ?= $(CURDIR)/
DBG_PATH ?= $(CURDIR)/

SDK_NAME := JaSdk
SDK_LIB_A := $(CURDIR)/lib$(SDK_NAME).a

API_NAME := JaApi
API_LIB_A := $(CURDIR)/lib$(API_NAME).a

SRC_LIB := \
		eseeclient timertask ddns smtp sensor firmware upnp \
		bubble owsp httpd \
		usrm \
		gw \
		unistruct \
		hls	\
		bsp \
		rudpa \
		generic \
		sysconf \
		regRW	\
		onvifnvt \
		minirtsp	\
		gb28181	\
		

L_SRC_LIB := -L$(LIB_PATH) $(patsubst %,-l%,$(SRC_LIB))

#SRC := $(wildcard *.c)
SRC := $(wildcard *.c) $(wildcard hi_isp_tmp/*.c)
#INC := $(addprefix -I, $(INC_PATH)) -I$(CURDIR)/hi_isp_tmp
INC := $(addprefix -I, $(INC_PATH)) -I$(CURDIR)/hi_isp_tmp -I$(CURDIR)/../sdk/HI3518A/include # FIXME
OBJ := $(patsubst %.c,$(DBG_PATH)/%.o,$(SRC))
DEP := $(patsubst %.c,$(DBG_PATH)/%.d,$(SRC))
LIB := $(patsubst %,$(LIB_PATH)/%.a,$(SRC_LIB))

SRC_CFLAGS := -D_GNU_SOURCE -DLINUX=2 -g3 -O0 -Wall \
	-DPRODUCT_CLASS=\"$(PRODUCT_CLASS)\" -DPRODUCT_MODEL=\"$(PRODUCT_MODEL)\" -DSOC_MODEL=\"$(SOC_MODEL)\" -DSERISE_CODE=\"$(SERISE_CODE)\"
	
#CROSS ?= arm-hismall-linux-
CROSS ?= arm-hisiv100nptl-linux-
CC := $(CROSS)gcc
CXX := $(CROSS)g++
AR := $(CROSS)ar
STRIP := $(CROSS)strip
CFLAGS := $(SRC_CFLAGS) $(INC)
#LDFLAGS := $(L_SRC_LIB) -lvsip-3531 -lantsserver -lpublic -ltinyxml -lssl -lcrypto -lz  -lpthread -lm -ldl -lrt -lstdc++ -luuid
LDFLAGS := $(L_SRC_LIB) $(SDK_LIB_A) $(API_LIB_A) -lvsip-3531 -lantsserver -lpublic -ltinyxml -lssl -lcrypto -lz  -lpthread -lm -ldl -lrt -lstdc++ -luuid

MK_SRC_LIB := make -f $(CURDIR)/Makefile.srclib \
	CC="$(CC)" CXX="$(CXX)" AR="$(AR)" CFLAGS="$(CFLAGS)" \
	INC_PATH="$(INC_PATH)" LIB_PATH="$(LIB_PATH)" BIN_PATH="$(BIN_PATH)" DBG_PATH="$(DBG_PATH)" \
	--directory=$(CURDIR)/

	
###########################################################
###########################################################

TARGET := $(BIN_PATH)/ipcam_app

.PHONY : clean distclean

$(DBG_PATH)/%.o : %.c
	@mkdir -p $(shell dirname $@)
	@echo -e "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<


$(TARGET): $(SDK_LIB_A) $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
	
srclib:
	@for i in $(SRC_LIB) ; do if test -d `dirname $$i` ; \
		then $(MK_SRC_LIB)$$i || exit $$? ; fi ; done

$(SDK_LIB_A):
	make --directory=$(CURDIR)/../sdk SOC_MODEL=HI3518A

all: srclib $(TARGET)
sinclude $(DEP)

clean:
	@rm -Rf $(DBG_PATH)/* $(TARGET)

distclean: clean
	@rm -Rf $(LIB_OUT)


