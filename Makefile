##############################################
#make CMD=yes ATP=yes
#API_TYPE
##############################################

LDFLAGS = 
#LIBS_S = -L/home/dev/Yocto/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/lib
#LIBS_S += -I/home/dev/Yocto/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/include


MKDIR = mkdir -p




CFLAGS =  -I./ 

CFLAGS += -O2 -Wall -c -fmessage-length=0 -pthread -mno-unaligned-access 
LINK_CFLAGS = -I./ 
LINK_CFLAGS += -O2 -Wall -fmessage-length=0 -pthread -mno-unaligned-access -lpng -lz

 
OBJ_DIR = ./obj
SRC_DIR = ./
EXTS := *.c
DIRS := ${shell find ${SRC_DIR} -type d -print}
SRCS := $(foreach dir,$(DIRS),$(wildcard $(addprefix $(dir)/,$(EXTS))))

OBJS := $(patsubst %c, $(OBJ_DIR)/%o, $(SRCS))


TARGET = init_logo

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LINK_CFLAGS) $(LDFLAGS) $(LIBS_S) -o $@ $^ 
$(OBJ_DIR)/%o: %c
	@$(MKDIR) $(dir $@ hacks)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS_S) -o $@ $<
install:
	
clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(OBJ_DIR)

print:
	@echo SRCS: ${SRCS}
	@echo OBJS: ${OBJS}


