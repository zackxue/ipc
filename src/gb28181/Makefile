CROSS :=arm-hisiv100nptl-linux-
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

TARGET := gb28181 
OBJ_DIR	= ./tmp

SRC +=
SRC += siplib.c
SRC += mpeg2ps.c
SRC += gb28181.c
SRC += sipparse.c
SRC += siprtpsession.c
SRC += test.c
SRC += 
SRC += rtplib.c
SRC += rtpbuf.c
SRC += rtplink.c
SRC += sock.c
SRC += vlog.c
SRC += mpeg.c
SRC += sdplib.c
SRC += _base64.c
SRC += _md5.c
SRC += authentication.c
SRC += ezxml.c
SRC += circlebuf.c
SRC += wav.c
SRC += netstream.c
SRC += sysconf.c

OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRC))

INC := -I. -I../../include -I../../include/http_auth -I../../include/rtp -I../../include/gb28181
#CFLAGS := -g3 -O0 -Wall -DFALSE=0 -DTRUE=1 -DLINUX=2 -Dtrue=1 -Dfalse=0 $(INC)
CFLAGS := -g3 -O0 -Wall -DNOCROSS -DFALSE=0 -DTRUE=1 -DLINUX=2 -Dtrue=1 -Dfalse=0 $(INC)

LDFLAGS := -L. -lm -lpthread 

.PHONY: clean all

all : $(TARGET)


$(OBJ_DIR)/%.o : %.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(OBJ_DIR)/%.o : ../../api/src/http_auth/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(OBJ_DIR)/%.o : ../../api/src/rtp/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(OBJ_DIR)/%.o : ../../api/src/gb28181/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(OBJ_DIR)/%.o : ../../api/src/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(OBJ_DIR)/%.o : ../minirtsp/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<

$(OBJ_DIR)/%.o : ../sysconf/%.c
	$(ECHO) "\033[33mmaking $<...\033[0m"
	$(CC) $(CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o $@ -c $<
	
$(TARGET) : $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS) 

sinclude $(DEP)

clean:
	$(RM) $(OBJ) $(DEP) $(TARGET)
