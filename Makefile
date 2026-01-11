BUILD_DIR := ./build

SRC_DIR := ./src
TB_DIR := ./tb

SRCS = $(shell find $(SRC_DIR) -name '*.v')
TBS = $(shell find $(TB_DIR) -name '*.v')

TARGETS := $(patsubst $(TB_DIR)/%.v,$(BUILD_DIR)/%.out,$(TBS))
VCD_DUMPS := $(patsubst $(TB_DIR)/%.v,$(BUILD_DIR)/%.vcd,$(TBS))

INC_DIRS := $(shell find ./include -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

override IVERILOG_FLAGS += -DIVERILOG -Wall -Wno-sensitivity-entire-array

FONT_SRC = ./data/unscii-16.hex
FONT_TARGET = unscii-16.mem

FIRMWARE_MEM := ./firmware/build/firmware.mem

.PHONY: all clean run wave

all: $(TARGETS)

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.out: $(TB_DIR)/%.v $(SRCS) $(FIRMWARE_MEM) $(BUILD_DIR)/$(FONT_TARGET)
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
