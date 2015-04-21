CROSS := @
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

TARGET := minirtsp 
OBJ_DIR	= ./tmp

SRC +=
SRC += vlog.c
SRC += _base64.c
SRC += _md5.c
SRC += authentication.c
SRC += sock.c
SRC += netstream.c
SRC += timerange.c
SRC += rtsplib.c
SRC += rtplib.c
SRC += rtcplib.c
SRC += sdplib.c
SRC += dlink.c
SRC += buffer.c
SRC += rtspserver.c
SRC += rtspclient.c
SRC += minirtsp.c

OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRC))

INC := -I. -I../../api/include -I../../api/include/http_auth
CFLAGS := -g3 -O0 -Wall -DNOCROSS -DFALSE=0 -DTRUE=1 -DLINUX=2 $(INC)

LDFLAGS := -L. -lm -lpthread 

.PHONY: clean all

all : $(TARGET)

$(OBJ_DIR)/%.o : %.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(OBJ_DIR)/%.o : ../../api/src/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(OBJ_DIR)/%.o : ../../api/src/http_auth/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(TARGET) : $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) 

sinclude $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(TARGET)
