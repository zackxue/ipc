

include Makefile.in

SRC := $(SOURCES) ../generic/base64.c ../generic/generic.c
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.o,%.d,$(OBJ))
INC := -I../../include

ECHO := @echo
RM := @rm -Rf

CROSS := arm-hismall-linux-
CC := $(CROSS)gcc
AR := $(CROSS)ar
CFLAGS ?= -DLINUX=2 -D_GNU_SOURCE -D_TEST_SMTP -g3 -O0 -Wall $(INC)
LDFLAGS := -L../../lib -lssl -lcrypto -lz -lpthread -lm -ldl

###########################################################
###########################################################

TARGET := smtp_test

.PHONY : clean test

%.o : %.c
	$(ECHO) -e "\033[33mMaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<


$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


# dependency
sinclude $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)

