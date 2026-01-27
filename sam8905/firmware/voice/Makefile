# MS4 Voice Module Makefile
# For SDAS8051 (SDCC toolchain)

AS = sdas8051
ASFLAGS = -plogff

# Build directory
BUILD = ../build

# Source files
VOICE_SRCS = voice_util.asm voice_mgmt.asm voice_init.asm

# Object files
VOICE_OBJS = $(addprefix $(BUILD)/, $(VOICE_SRCS:.asm=.rel))

# Include paths
INCLUDES = -I. -I../core

.PHONY: all clean

all: $(BUILD) $(VOICE_OBJS)

$(BUILD):
	mkdir -p $(BUILD)

# Pattern rule for assembling
$(BUILD)/%.rel: %.asm voice_defs.inc ../core/sam_defs.inc
	$(AS) $(ASFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f $(BUILD)/voice_*.rel $(BUILD)/voice_*.lst $(BUILD)/voice_*.sym

