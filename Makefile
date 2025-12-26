BUILD_DIR := ./build

SRC_DIR := ./src
TB_DIR := ./tb

# Firmware variables ====================================================================

PROJECT_NAME := firmware
FW_TARGET_EXEC := $(PROJECT_NAME).elf
FW_TARGET_BIN := $(PROJECT_NAME).bin
FW_TARGET_MEM := $(PROJECT_NAME).mem

FW_BASE := ./firmware

FW_SRC_DIRS := $(FW_BASE)/src

FW_SRCS := $(FW_BASE)/src/keyboard.c \
		   $(FW_BASE)/src/main.c \
		   $(FW_BASE)/src/numeric.c \
		   $(FW_BASE)/src/puter.c \
		   $(FW_BASE)/src/riscv.c \
		   $(FW_BASE)/src/riscv.s \
		   $(FW_BASE)/src/rtc.c \
		   $(FW_BASE)/src/startup.s \
		   $(FW_BASE)/src/sd_card.c \
		   $(FW_BASE)/src/syscalls.c \
		   $(FW_BASE)/src/uart.c \
		   $(FW_BASE)/src/vga.c

FW_OBJS := $(FW_SRCS:%=$(BUILD_DIR)/%.o)
FW_LINKER := $(FW_BASE)/data/linker.ld

FW_INC_DIRS := $(FW_SRC_DIRS)
FW_INC_FLAGS := $(addprefix -I,$(FW_INC_DIRS))

override CFLAGS += $(FW_INC_FLAGS) -march=rv32i_zicsr -mabi=ilp32 \
		  -std=c23 -specs=nano.specs -nostartfiles -ffreestanding \
		  -g -Oz -ffunction-sections -fdata-sections \
		  -Wall -Wextra -Wpedantic

LDFLAGS := --no-warn-rwx-segments,--gc-sections

CC := riscv32-none-elf-gcc
OBJCOPY := riscv32-none-elf-objcopy
XXD := xxd

BEAR := bear
CDB := compile_commands.json


# Verilog variables ===========================================================

SRCS = $(shell find $(SRC_DIR) -name '*.v')
TBS = $(shell find $(TB_DIR) -name '*.v')

TARGETS := $(patsubst $(TB_DIR)/%.v,$(BUILD_DIR)/%.out,$(TBS))
VCD_DUMPS := $(patsubst $(TB_DIR)/%.v,$(BUILD_DIR)/%.vcd,$(TBS))

INC_DIRS := $(shell find ./include -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

override IVERILOG_FLAGS += -DIVERILOG -Wall

FONT_SRC = ./data/unscii-16.hex
FONT_TARGET = unscii-16.mem

.PHONY: all clean run wave compdb firmware

all: $(TARGETS)

clean:
	rm -rf $(BUILD_DIR)


# Firmware ====================================================================

firmware: $(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_MEM)

compdb:
	mkdir -p $(BUILD_DIR)
	$(BEAR) --output $(BUILD_DIR)/$(CDB) -- make -B $(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_EXEC)

$(BUILD_DIR)/%.mem: $(BUILD_DIR)/%.bin
	$(XXD) -p -c4 -e $< | awk '{print $$2}' > $@

$(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_BIN): $(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_EXEC)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_EXEC): $(FW_OBJS) $(FW_LINKER)
	$(CC) $(CFLAGS) -T $(FW_LINKER) -o $@ $(FW_OBJS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


# Verilog =====================================================================

$(BUILD_DIR)/%.out: $(TB_DIR)/%.v $(SRCS) $(BUILD_DIR)/$(FW_BASE)/$(FW_TARGET_MEM) $(BUILD_DIR)/$(FONT_TARGET)
	mkdir -p $(dir $@)
	iverilog $(INC_FLAGS) $(IVERILOG_FLAGS) -o $@ $< $(SRCS) 

$(BUILD_DIR)/%.vcd: $(BUILD_DIR)/%.out
	mkdir -p $(dir $@)
	vvp $(VVP_FLAGS) $<
	mv dump.vcd $@

$(BUILD_DIR)/$(FONT_TARGET): $(FONT_SRC)
	mkdir -p $(dir $@)
	./tools/extract_font.sh $< > $@

run: $(BUILD_DIR)/$(TB).out
	mkdir -p $(dir $(BUILD_DIR)/$(TB))
	vvp $(VVP_FLAGS) $<
	mv dump.vcd $(BUILD_DIR)/$(TB).vcd

wave: $(BUILD_DIR)/$(TB).vcd
	gtkwave $<
