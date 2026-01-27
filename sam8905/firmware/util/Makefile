# MS4 Utility Module Makefile
# For SDAS8051 (SDCC toolchain)

AS = sdas8051
ASFLAGS = -plogff

# Build directory
BUILD = ../build

# Source files
UTIL_SRCS = util.asm

# Object files
UTIL_OBJS = $(addprefix $(BUILD)/, $(UTIL_SRCS:.asm=.rel))

.PHONY: all clean

all: $(BUILD) $(UTIL_OBJS)

$(BUILD):
	mkdir -p $(BUILD)

# Pattern rule for assembling
$(BUILD)/%.rel: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(BUILD)/util*.rel $(BUILD)/util*.lst $(BUILD)/util*.sym

