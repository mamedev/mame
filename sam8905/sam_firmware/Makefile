# SAM8905 Controller Firmware - Makefile
#
# Targets:
#   make test     - Build for host testing (gcc)
#   make sdcc     - Build for 8051 (SDCC)
#   make clean    - Remove build artifacts

# Device selection (default: MS4)
DEVICE ?= MS4

# Compiler selection
CC_HOST = gcc
CC_8051 = sdcc

# Flags
CFLAGS_COMMON = -Wall -Wextra -DSAM_DEVICE_$(DEVICE)
CFLAGS_HOST = $(CFLAGS_COMMON) -g -O2
CFLAGS_8051 = $(CFLAGS_COMMON) --model-large -mmcs51

# Sources
SRCS = sam_init.c sam_hw.c sam_utils.c sam_midi.c
HDRS = sam_types.h sam_hw.h sam_config.h sam_rom.h sam_firmware.h sam_utils.h sam_math.h sam_pitch_tables.h sam_midi.h

# Output
BUILD_HOST = build/host
BUILD_8051 = build/8051

#------------------------------------------------------------------------------
# Host build (for testing)
#------------------------------------------------------------------------------

.PHONY: test
test: $(BUILD_HOST)/sam_firmware_test

$(BUILD_HOST)/sam_firmware_test: $(SRCS) $(HDRS) test_main.c | $(BUILD_HOST)
	$(CC_HOST) $(CFLAGS_HOST) -o $@ $(SRCS) test_main.c

$(BUILD_HOST):
	mkdir -p $(BUILD_HOST)

#------------------------------------------------------------------------------
# 8051 build (SDCC)
#------------------------------------------------------------------------------

.PHONY: sdcc
sdcc: $(BUILD_8051)/sam_firmware.ihx

$(BUILD_8051)/sam_firmware.ihx: $(SRCS) $(HDRS) | $(BUILD_8051)
	$(CC_8051) $(CFLAGS_8051) -o $@ $(SRCS)

$(BUILD_8051):
	mkdir -p $(BUILD_8051)

#------------------------------------------------------------------------------
# Clean
#------------------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf build/

#------------------------------------------------------------------------------
# Help
#------------------------------------------------------------------------------

.PHONY: help
help:
	@echo "SAM8905 Controller Firmware Build"
	@echo ""
	@echo "Targets:"
	@echo "  make test        Build for host testing (gcc)"
	@echo "  make sdcc        Build for 8051 (SDCC)"
	@echo "  make clean       Remove build artifacts"
	@echo ""
	@echo "Device selection:"
	@echo "  make DEVICE=MS4        Solton/Hohner MS4 (default)"
	@echo "  make DEVICE=KEYFOX10   Wersi Keyfox 10"
	@echo "  make DEVICE=XE9        Hohner XE9/XE9L"
