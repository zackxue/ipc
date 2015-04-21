CROSS := @
#CROSS := @arm-hisiv100nptl-linux-
CC := $(CROSS)gcc
AR := $(CROSS)ar
STRIP := $(CROSS)strip
MKDIR := @mkdir -p
ECHO := @echo -e
RM := @rm -Rf
MAKE := @$(MAKE)
LS := @ls
CP := @cp -Rf
MV := @mv

TARGET := shellagent 
OBJ_DIR	= ./tmp

API_SRC = ../api/src
API_INC	= ../api/include

SRC +=
SRC += vlog.c
#SRC += base64.c
#SRC += _md5.c
#SRC += authentication.c
SRC += _base64.c
SRC += _md5.c
SRC += authentication.c
SRC += sock.c
SRC += 
SRC += jastlib.c
SRC += jastsession.c
SRC += shell_agent.c
SRC += jastserver.c

OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRC))

INC := -I. -I../api/include -I../api/include/http_auth
CFLAGS := -g3 -O0 -Wall -DNOCROSS -DFALSE=0 -DTRUE=1 -DLINUX=2 -Dtrue=1 -Dfalse=0 $(INC)

LDFLAGS := -L. -lm -lpthread 

.PHONY: clean all

all : $(TARGET) tracetest

$(OBJ_DIR)/%.o : %.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(OBJ_DIR)/%.o : ../api/src/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(OBJ_DIR)/%.o : ../api/src/http_auth/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(TARGET) : $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) 

sessiontest :
	rm -f session_test
	gcc $(CFLAGS) -D__TEST jastsession.c -o session_test
	chmod a+x session_test
	rm -f jastsession.o

tracetest :
	rm -f trace_test 
	$(CC) -g trace_test.c -o trace_test
	chmod a+x trace_test
	rm -f trace_test.o

agenttest :
	rm -f agent_test 
	$(CC) -g agent_test.c -o agent_test
	chmod a+x agent_test
	rm -f agent_test.o

sinclude $(DEP)

clean:
	rm -f session_test trace_test agent_test
	$(RM) $(OBJ) $(DEP) $(TARGET)
