TARGET_EXEC := kernel.elf
TARGET_BIN := kernel.bin

BUILD_DIR := ./build

SRCS := ./src/control.c \
		./src/io.c \
		./src/keyboard.c \
		./src/libc.c \
		./src/libc.s \
		./src/main.c \
		./src/numeric.c \
		./src/puter.c \
		./src/riscv.c \
		./src/riscv.s \
		./src/rtc.c \
		./src/startup.s \
		./src/uart.c \
		./src/vga.c

LINKER := ./data/linker.ld

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := ./src
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

override CFLAGS += $(INC_FLAGS) -march=rv32i_zicsr -mabi=ilp32 \
		  -std=c23 -nostdlib -nostartfiles -ffreestanding -fomit-frame-pointer \
		  -g -Oz -ffunction-sections -fdata-sections \
		  -Wall -Wextra -Wpedantic

LDFLAGS := --no-warn-rwx-segments,--gc-sections

CC := riscv32-none-elf-gcc
OBJCOPY := riscv32-none-elf-objcopy

BEAR := bear
CDB := compile_commands.json

UDISKSCTL := udisksctl

.PHONY: all compdb upload clean

all: $(BUILD_DIR)/$(TARGET_BIN)

compdb:
	mkdir -p $(BUILD_DIR)
	$(BEAR) --output $(BUILD_DIR)/$(CDB) -- $(MAKE) -B $(BUILD_DIR)/$(TARGET_EXEC)

upload: $(BUILD_DIR)/$(TARGET_BIN)
	$(UDISKSCTL) mount -b $(DEV); \
	MNT_DIR=$$($(UDISKSCTL) info -b $(DEV) | awk -F: '/MountPoints/ {print $$2}' | xargs -n1 echo); \
	cp $< $$MNT_DIR;

	$(UDISKSCTL) unmount -b $(DEV)

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
