TARGET_EXEC := firmware.elf
TARGET_BIN := firmware.bin
TARGET_MEM := firmware.mem

BUILD_DIR := ./build

SRCS := ./src/disk_format.c \
		./src/main.c \
		./src/numeric.c \
		./src/puter.c \
		./src/sd_card.c \
		./src/spi.c \
		./src/startup.s \
		./src/vga.c 

LINKER := ./data/linker.ld

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := ./src
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

override CFLAGS += $(INC_FLAGS) -march=rv32i_zicsr -mabi=ilp32 \
		  -std=c23 -nostdlib -nostartfiles -ffreestanding -fomit-frame-pointer \
		  -g -Oz -ffunction-sections -fdata-sections -fno-pie \
		  -Wall -Wextra -Wpedantic

LDFLAGS := --no-warn-rwx-segments,--gc-sections

CC := riscv32-none-elf-gcc
OBJCOPY := riscv32-none-elf-objcopy
XXD := xxd

BEAR := bear
CDB := compile_commands.json

.PHONY: all compdb clean

all: $(BUILD_DIR)/$(TARGET_MEM)

compdb:
	mkdir -p $(BUILD_DIR)
	$(BEAR) --output $(BUILD_DIR)/$(CDB) -- $(MAKE) -B $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/%.mem: $(BUILD_DIR)/%.bin
	$(XXD) -p -c4 -e $< | awk '{print $$2}' > $@

$(BUILD_DIR)/$(TARGET_BIN): $(BUILD_DIR)/$(TARGET_EXEC)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) $(LINKER)
	$(CC) $(CFLAGS) -Wl,$(LDFLAGS) -T $(LINKER) $(OBJS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
