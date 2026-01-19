# Makefile for SAM8905 test firmware
# Requires: sdcc

CC = sdcc
CFLAGS = --model-small -mmcs51 --std-c99

# ROM size for Keyfox10 (128KB = 0x20000)
# Note: Original ROM is 128KB, containing program + data
ROM_SIZE = 131072

# Targets
TARGET1 = test_sam
TARGET2 = test_sam_uart

.PHONY: all clean test uart

all: $(TARGET1)_padded.bin $(TARGET2)_padded.bin

uart: $(TARGET2)_padded.bin

# Pattern rules for building firmware

# Compile C to Intel HEX
%.ihx: %.c
	$(CC) $(CFLAGS) $< -o $@

# Convert Intel HEX to binary
%.bin: %.ihx
	objcopy -I ihex -O binary $< $@
	@echo "Binary size: $$(stat -c%s $@) bytes"

# Pad to ROM size (fill with 0xFF like blank EPROM)
%_padded.bin: %.bin
	dd if=/dev/zero bs=$(ROM_SIZE) count=1 2>/dev/null | tr '\000' '\377' > $@
	dd if=$< of=$@ conv=notrunc 2>/dev/null
	@echo "Padded ROM: $(ROM_SIZE) bytes"

# Test in MAME (requires MAME to be built)
test: $(TARGET2)_padded.bin
	@echo "To test in MAME:"
	@echo "  1. Backup original ROM: cp /path/to/roms/keyfox10/kf10_ic27_v2.bin kf10_ic27_v2.bin.orig"
	@echo "  2. Copy test ROM: cp $(TARGET2)_padded.bin /path/to/roms/keyfox10/kf10_ic27_v2.bin"
	@echo "  3. Run: ./mamemuse keyfox10 -rompath /path/to/roms -oslog"

clean:
	rm -f *.ihx *.bin *.asm *.lst *.map *.mem *.rel *.rst *.sym *.lk
