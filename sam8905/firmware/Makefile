# Makefile for SAM8905 test firmware
# Requires: sdcc, srecord (for srec_cat)

CC = sdcc
CFLAGS = --model-small -mmcs51 --std-c99

# ROM size for Keyfox10 (128KB = 0x20000)
# Note: Original ROM is 128KB, containing program + data
ROM_SIZE = 131072

TARGET = test_sam
SOURCES = test_sam.c

.PHONY: all clean test

all: $(TARGET).bin $(TARGET)_padded.bin

# Compile to Intel HEX
$(TARGET).ihx: $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

# Convert Intel HEX to binary
$(TARGET).bin: $(TARGET).ihx
	objcopy -I ihex -O binary $< $@
	@echo "Binary size: $$(stat -c%s $@) bytes"

# Pad to ROM size (fill with 0xFF like blank EPROM)
$(TARGET)_padded.bin: $(TARGET).bin
	dd if=/dev/zero bs=$(ROM_SIZE) count=1 2>/dev/null | tr '\000' '\377' > $@
	dd if=$< of=$@ conv=notrunc 2>/dev/null
	@echo "Padded ROM: $(ROM_SIZE) bytes"

# Test in MAME (requires MAME to be built)
test: $(TARGET)_padded.bin
	@echo "To test in MAME:"
	@echo "  1. Backup original ROM: cp /path/to/roms/keyfox10/kf10_ic27_v2.bin kf10_ic27_v2.bin.orig"
	@echo "  2. Copy test ROM: cp $(TARGET)_padded.bin /path/to/roms/keyfox10/kf10_ic27_v2.bin"
	@echo "  3. Run: ./mamemuse keyfox10 -rompath /path/to/roms -oslog"

clean:
	rm -f $(TARGET).ihx $(TARGET).bin $(TARGET)_padded.bin
	rm -f $(TARGET).asm $(TARGET).lst $(TARGET).map $(TARGET).mem
	rm -f $(TARGET).rel $(TARGET).rst $(TARGET).sym $(TARGET).lk
