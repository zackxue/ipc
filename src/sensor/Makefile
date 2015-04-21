
SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))
DEP := $(patsubst %.o,%.d,$(OBJ))
INC := 

ECHO := @echo
RM := @rm -Rf

CROSS ?=
CC ?= $(CROSS)gcc
AR ?= $(CROSS)ar
CFLAGS ?= -DTRUE="(1==1)" -DFALSE="(1==0)" -D_GNU_SOURCE -g3 -O0 -Wall $(INC)
LDFLAGS ?= -lpthread -lm

###########################################################
###########################################################

TARGET := lib$(shell basename $(CURDIR)).a

.PHONY : 

%.o : %.c
	$(ECHO) -e "\033[33mMaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(TARGET) : $(OBJ)
	$(AR) -r $@ $^

# dependency
sinclude $(DEP)

clean:
	$(RM) $(TARGET) $(OBJ) $(DEP)

