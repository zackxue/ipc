

include Makefile.in

SRC := $(SOURCES) ezxml.c base64.c
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.o,%.d,$(OBJ))
INC := -I../../include

ECHO := @echo
RM := @rm -Rf

#CROSS := arm-hismall-linux-
CC := @$(CROSS)gcc
AR := $(CROSS)ar
CFLAGS ?= -DLINUX=2 -D_GNU_SOURCE -D_TEST_ESEE -g3 -O0 -Wall $(INC)
LDFLAGS := -L../../lib -lpthread -lm 


###########################################################
###########################################################

TARGET := esee_test

.PHONY : clean test

%.o : %.c
	$(ECHO) -e "\033[33mMaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

all:
	@ln -s ../generic/ezxml.c .
	@ln -s ../generic/base64.c .
	@make $(TARGET)
	$(RM) ezxml.c base64.c

$(TARGET) : $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)


# dependency
sinclude $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)
	$(RM) ezxml.c base64.c

