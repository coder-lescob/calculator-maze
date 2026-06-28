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

PLATFORM := device

ifeq ($(PLATFORM), simulator)
    CC := gcc

    C_FLAGS := -fPIC "-I/usr/local/lib/node_modules/nwlink/dist/eadk" -ggdb -fno-exceptions -Wall 
    LD_FLAGS := -shared -undefined,dynamic_lookup
else
    CC := arm-none-eabi-gcc

    C_FLAGS = $(shell $(NWLINK) eadk-cflags-device) -MMD -MP
    LD_FLAGS = $(shell $(NWLINK) eadk-ldflags-device) -lc -lm --specs=nosys.specs -fdata-sections -ffunction-sections \
  -flto -fno-fat-lto-objects -fwhole-program -fvisibility=internal \
  -Wl,-e,main \
  -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level \
  -Wl,--gc-sections -Wl,--defsym=end=0
endif

.PHONY: build 
build:
ifeq ($(OS),Window_NT)
	-@mkdir $(BUILD_DIR)
else
	-@mkdir -p $(BUILD_DIR)
endif
	@$(NWLINK) png-icon-o $(ICON) $(ICON_O)

ifeq ($(PLATFORM), simulator)
	@$(CC) $(C_FLAGS) $(SRC) $(LD_FLAGS) -o $(TARGET)
else
	@$(CC) $(C_FLAGS) $(SRC) $(ICON_O) $(LD_FLAGS) -o $(TARGET)
endif

run: build
ifeq ($(PLATFORM), simulator)
	@$(SIM) --nwb $(TARGET)
else
	@$(NWLINK) install-nwa $(TARGET)
endif

debug:
ifeq ($(PLATFORM), simulator)
	@gdb --args $(SIM)  --nwb $(TARGET)
endif
	