# Project Name = MPLABConnect
# Generates  libMPLABConnect.a

PROJECT = MPLABConnectSDKV1_1

LIB_OUT = libMPLABConnect.a
SHL_OUT = libMPLABConnect.so

CPP_SRC =   MchpUSBInterface.cpp
CPP_SRC1 = SpiFlash.cpp
CPP_SRC2 = USBHubAbstraction.cpp

LIB_USB_PATH=$(shell pkg-config --cflags libusb-1.0)

#Include Paths
INC_DIR = /usr/local/include/libusb-1.0 /usr/local/include/ .

LIB_DIR = /usr/local/lib

#Defnes
DEFINES = -D LIBUSB

#
# DO NOT EDIT BELOW
#

# Temporary directories
OBJ_DIR = .
OUT_DIR = .

# Automatic generation of some important lists
INC_PATH = $(foreach TMP,$(INC_DIR),-I$(TMP))
LIB_PATH = $(foreach TMP,$(LIB_DIR),-L$(TMP))

OBJS = $(CPP_SRC:.cpp=.o) $(GCC_SRC:.c=.o) $(CPP_SRC1:.cpp=.o) $(CPP_SRC2:.cpp=.o)

# Libraries
LIBS = -lusb-1.0 -lpthread -lm -ldl -lrt -lncurses

# Flags
CCFLAGS = -g $(LIB_USB_PATH) $(shell pkg-config --libs libusb-1.0)
LDFLAGS = -g

# Compiler
CCC = g++ -std=c++11 -static -Wall -Wextra
GCC = gcc


all : $(LIB_OUT)



.cpp.o:
	@mkdir -p ./obj
	@echo "============="
	@echo "Compiling $<"
	@$(CCC) $(INC_PATH) $(CCFLAGS) $(DEFINES) -c $< -o $(OBJ_DIR)/$(notdir $@)

.c.o:
	@mkdir -p ./obj
	@echo "============="
	@echo "Compiling $<"
	@$(GCC) $(INC_PATH) $(CCFLAGS) $(DEFINES) -c $< -o $(OBJ_DIR)/$(notdir $@)




$(LIB_OUT): $(OBJS)
	@echo "============="
	@mkdir -p ./out
	@echo "Creating $(LIB_OUT)"
	@ar rcs $(addprefix $(OUT_DIR)/, $(LIB_OUT)) $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS)))

$(SHL_OUT): $(OBJS)
	@echo "============="
	@mkdir -p ./out
	@echo "Creating $(SHL_OUT)"
	@$(CCC) $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS))) $(LIBS) $(LIB_PATH) -shared -o $(addprefix $(OUT_DIR)/, $(SHL_OUT))

find:
	-exec touch \{\} \;
clean:
	rm -f *.o
	rm -f *.a
	rm -f Makefile.bak
