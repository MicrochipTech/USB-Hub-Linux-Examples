
# Choose the project
# Generates  i2c_Bridging

PROJECT = i2c_bridging


######################################################
############## For I2C Bridging demo #########################
EXE_OUT = i2cBridging

CPP_SRC = i2c_bridging.cpp

LIB_USB_PATH=$(shell pkg-config --cflags libusb-1.0)

#Include Paths
INC_DIR = /usr/local/include/libusb-1.0 ../../libMPLABConnect
LIB_DIR = /usr/local/lib /usr/lib

#Defnes
DEFINES = -D LIBUSB

#
# DO NOT EDIT BELOW
#

# Temporary directories
OBJ_DIR = ./obj
OUT_DIR = ./out

# Automatic generation of some important lists
INC_PATH = $(foreach TMP,$(INC_DIR),-I$(TMP))
LIB_PATH = $(foreach TMP,$(LIB_DIR),-L$(TMP))

OBJS = $(CPP_SRC:.cpp=.o) $(GCC_SRC:.c=.o)

# Libraries
LIBS = -lusb-1.0 -lpthread -lm -ldl

# Flags
CCFLAGS = -g -Wall $(LIB_USB_PATH) $(shell pkg-config --libs libusb-1.0)
LDFLAGS = -g libMPLABConnect.a

# Compiler
CCC = $(CXX)
GCC = CC
#CCC = g++ -std=c++11
#GCC = gcc

# Default targets
default: $(EXE_OUT) $(LIB_OUT)

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

$(EXE_OUT): $(OBJS)
	@echo "============="
	@mkdir -p ./out
	@echo "Creating $(EXE_OUT)"
	@$(CCC)  $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS))) $(LDFLAGS) $(LIB_PATH) $(LIBS) -o $(addprefix $(OUT_DIR)/, $(EXE_OUT))

$(LIB_OUT): $(OBJS)
	@echo "============="
	@mkdir -p ./out
	@echo "Creating $(LIB_OUT)"
	@ar rcs $(addprefix $(OUT_DIR)/, $(LIB_OUT)) $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS)))

find:
	-exec touch \{\} \;

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(OUT_DIR)
	rm -f Makefile.bak
