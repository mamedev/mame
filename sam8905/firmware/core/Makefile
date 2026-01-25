# SAM8905 Core Module Makefile
#
# Uses SDAS8051 assembler from SDCC toolchain

AS = sdas8051
ASFLAGS = -plosgff

# Output directory
BUILD_DIR = ../build

# Source files
ASM_SRCS = sam_dram.asm sam_aram.asm

# Object files
OBJS = $(ASM_SRCS:%.asm=$(BUILD_DIR)/%.rel)

# Default target
all: $(BUILD_DIR) $(OBJS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble .asm to .rel
$(BUILD_DIR)/%.rel: %.asm sam_defs.inc
	$(AS) $(ASFLAGS) -o $@ $<

# Clean
clean:
	rm -rf $(BUILD_DIR)

# List symbols from object files
symbols: $(OBJS)
	@for f in $(OBJS); do \
		echo "=== $$f ==="; \
		grep -E "^S " $$f || true; \
	done

.PHONY: all clean symbols
