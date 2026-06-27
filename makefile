include sim_path.mak

BUILD_DIR := build
TARGET := $(BUILD_DIR)/maze.nwa
SIM_TARGET := $(BUILD_DIR)/maze
O          := $(BUILD_DIR)/maze.o

NWLINK := npx --yes nwlink

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.c)

ICON := $(SRC_DIR)/icon.png
ICON_O := $(BUILD_DIR)/icon.o

PLATFORM := "device"

ifeq ($(PLATFORM), "simulator")
    CC := gcc

    C_FLAGS := -fPIC "-I/usr/local/lib/node_modules/nwlink/dist/eadk"
    LD_FLAGS := -shared
else
    CC := arm-none-eabi-gcc

    C_FLAGS := -Os -mthumb -mfloat-abi=hard -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -DPLATFORM_DEVICE=1 "-I/usr/local/lib/node_modules/nwlink/dist/eadk"
    LD_FLAGS := -Wl,--relocatable -nostartfiles
endif

.PHONY: build 
build:
	@mkdir -p $(BUILD_DIR)
	@$(CC) -c $(C_FLAGS) $(SRC) -o $(O)
	@$(NWLINK) png-icon-o $(ICON) $(ICON_O)
	@$(CC) $(C_FLAGS) $(O) $(ICON_O) $(LD_FLAGS) -o $(TARGET)

send: build
	@$(NWLINK) install-nwa $(TARGET)

sim: build
	@$(NWLINK) nwa-elf $(TARGET) $(SIM_TARGET)
	@$(SIM) --nwb $(SIM_TARGET)
	