# SAM8905 Firmware Emulator - Makefile
#
# Build the emulator with ALSA MIDI and PortAudio audio output.
# Includes ported firmware MIDI subsystem.
#
# Dependencies (Debian/Ubuntu):
#   sudo apt install libportaudio2 libportaudio-dev libasound2-dev
#
# Dependencies (Arch):
#   sudo pacman -S portaudio alsa-lib

CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -I.. -DSAM_DEVICE_MS4 -DSAM_HW_PLATFORM
LDFLAGS = -lportaudio -lasound -lm

# Emulator sources
EMU_SRCS = main_emu.c sam8905_emu.c audio_portaudio.c midi_alsa_raw.c sam_hw_emu.c sam_rom_emu.c

# Firmware sources (from parent directory)
FW_SRCS = ../sam_midi.c ../sam_hw.c ../sam_voice.c ../sam_dram_config.c ../sam_init.c ../sam_utils.c

# All sources and objects
SRCS = $(EMU_SRCS) $(FW_SRCS)
OBJS = $(EMU_SRCS:.c=.o) sam_midi.o sam_hw.o sam_voice.o sam_dram_config.o sam_init.o sam_utils.o

# Output
TARGET = sam_emu

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Firmware objects from parent directory
sam_midi.o: ../sam_midi.c ../sam_midi.h ../sam_firmware.h ../sam_types.h
	$(CC) $(CFLAGS) -c -o $@ $<

sam_hw.o: ../sam_hw.c ../sam_hw.h ../sam_firmware.h
	$(CC) $(CFLAGS) -c -o $@ $<

sam_voice.o: ../sam_voice.c ../sam_voice.h ../sam_firmware.h
	$(CC) $(CFLAGS) -c -o $@ $<

sam_dram_config.o: ../sam_dram_config.c ../sam_dram_config.h ../sam_firmware.h
	$(CC) $(CFLAGS) -c -o $@ $<

sam_init.o: ../sam_init.c ../sam_firmware.h
	$(CC) $(CFLAGS) -c -o $@ $<

sam_utils.o: ../sam_utils.c ../sam_utils.h ../sam_firmware.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

# Dependencies
main_emu.o: main_emu.c sam8905_emu.h sam8905.h audio_portaudio.h midi_alsa_raw.h ../sam_firmware.h
sam8905_emu.o: sam8905_emu.c sam8905_emu.h sam8905.h
audio_portaudio.o: audio_portaudio.c audio_portaudio.h
midi_alsa_raw.o: midi_alsa_raw.c midi_alsa_raw.h
sam_hw_emu.o: sam_hw_emu.c sam8905_emu.h ../sam_hw.h
