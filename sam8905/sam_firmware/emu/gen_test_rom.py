#!/usr/bin/env python3
"""
Generate a simple test ROM for SAM8905 emulator testing.

Creates a minimal 64KB ROM with:
- Program pointer table at 0x0040
- One simple test program with fixed amplitude (no envelopes)
- A-RAM algorithm from MS4 (dpiano27 algorithm at 0x027A)

Usage:
    python3 gen_test_rom.py [output_file]

Default output: test_rom.bin
"""

import sys
import struct

# ROM size (64KB like MS4)
ROM_SIZE = 0x10000

# Program pointer table location
PROGRAM_PTR_TABLE = 0x0040
NUM_PROGRAMS = 66

# Where to place our test program
PROGRAM_ADDR = 0x1000

# Where to place A-RAM algorithm data
ARAM_ADDR = 0x2000

# Simple sinus oscillator algorithm - modified for MS4 firmware D-RAM layout
# Firmware writes: DRAM[0]=pitch, DRAM[1]=amplitude, DRAM[2]=routing (unused by algo)
# Uses DRAM[2] for phase accumulation (starts at 0, not touched by firmware after init)
#
# Instruction bits (active LOW for receivers):
#   14:11 = MAD (D-RAM address within slot)
#   10:9  = Emitter: 0=RM, 1=RADD, 2=RP, 3=RSP
#   8     = WSP (modifier)
#   7=WA, 6=WB, 5=WM, 4=WPHI, 3=WXY, 2=clearB, 1=WWF, 0=WACC
ALGORITHM_SINUS = [
    0x107F,  # RM 2, WA            - Read accumulated phase from DRAM[2] into A
    0x00BF,  # RM 0, WB            - Read pitch increment from DRAM[0] into B
    0x13CF,  # RADD 2, WM,WPHI,WSP - Write A+B to DRAM[2], set PHI, WSP→internal sine
    0x09F7,  # RM 1, WXY, WSP      - Read amp from DRAM[1], X=waveform(phi), Y=amp, mix_l/r
    0x06FE,  # RSP, WACC           - Accumulate X*Y to output
    # Fill rest with NOP (0x7FFF)
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF,
]

# A-RAM algorithm from MS4 address 0x027A (used by dpiano27)
# 32 x 15-bit instruction words, stored as little-endian word pairs
ALGORITHM_DPIANO = [
    0x08EF, 0x7EFB, 0x50BD, 0x28F7, 0x78FD, 0x4CCF, 0x59F7, 0x11EF,
    0x20FD, 0x086F, 0x3ADF, 0x113F, 0x42DF, 0x18BF, 0x7CF7, 0x43DF,
    0x30BF, 0x2876, 0x41EF, 0x20FD, 0x38EE, 0x2BDF, 0x087F, 0x00BF,
    0x0ADF, 0x48F7, 0x113F, 0x12DF, 0x18BF, 0x13DE, 0x7FFF, 0x7FFF,
]

# Default to simple sinus for testing
ALGORITHM = ALGORITHM_SINUS

# D-RAM stream for simple fixed-amplitude sound
# Handler 0x08 (pitch): 10 bytes
# Handler 0x10 (amplitude): 9 bytes
# Handler 0x20 (output, terminates): 4 bytes
DRAM_STREAM = bytes([
    # Handler 0x08: Pitch (10 bytes)
    0x08,       # dispatch byte: handler 0x08
    0x00,       # velocity sensitivity = 0
    0x00,       # note offset = 0 (use MIDI note directly)
    0x00,       # control flags = 0
    0x02,       # bend range = 2 semitones
    0x00, 0x00, # fine tune = 0
    0x00,       # skip
    0x00,       # portamento rate = 0
    0x00,       # portamento depth = 0

    # Handler 0x10: Amplitude (9 bytes)
    0x10,       # dispatch byte: handler 0x10
    0x3F,       # base level = 63 (max)
    0x7F,       # amplitude = 127 (max)
    0x00,       # envelope control = 0 (no envelope, instant on)
    0x00,       # attack rate = 0 (instant)
    0x00,       # unused
    0x7F,       # sustain level = 127 (max)
    0x00,       # velocity sensitivity = 0
    0x00,       # modulation amount = 0

    # Handler 0x20: Output routing (4 bytes, TERMINATES)
    0x20,       # dispatch byte: handler 0x20
    0x00,       # route byte 1
    0x00,       # route byte 2
    0x20,       # route byte 3: bit 5 = voice active
])

# Voice init data (7 bytes per slot)
# Format from MS4: [env_attack, env_decay, env_flags, lfo_rate, lfo_depth, lfo_delay, lfo_flags]
VOICE_INIT_DATA = bytes([
    0x00,       # envelope attack = 0 (instant)
    0x00,       # envelope decay = 0
    0x00,       # envelope flags = 0 (no envelope processing)
    0x00,       # LFO rate = 0 (no LFO)
    0x00,       # LFO depth = 0
    0x00,       # LFO delay = 0
    0x00,       # LFO flags = 0
])


def build_program(name: str, slot_count: int, aram_ptr: int, dram_stream: bytes, voice_init: bytes) -> bytes:
    """
    Build MS4-format program structure.

    MS4 format (based on firmware analysis):
        0-7:   Name (8 bytes, space-padded)
        8:     Null terminator
        9:     Flags (bit7=complex, bits3:0=slot_count)
        10-11: A-RAM pointer (little-endian)
        12-14: D-RAM entry0 (3 bytes)
        15-16: First voice_ptr (little-endian) - envelope/LFO block
        17-18: Second voice_ptr OR 0x0000 terminator
        ...    More voice_ptrs until 0x0000
        N:     D-RAM handler stream (after 0x0000 terminator)
    """
    prog = bytearray()

    # Name (8 bytes, space-padded)
    name_bytes = name.encode('ascii')[:8].ljust(8, b' ')
    prog.extend(name_bytes)

    # Null terminator
    prog.append(0x00)

    # Flags: slot_count in lower 4 bits
    prog.append(slot_count & 0x0F)

    # A-RAM pointer (little-endian)
    prog.extend(struct.pack('<H', aram_ptr))

    # D-RAM entry0 (3 bytes) - using typical values
    prog.extend([0x01, 0xA4, 0x85])  # word=0xA401, addr=8, mix=5

    # Calculate where voice init data will be placed
    # Layout: header (17 bytes) + voice_ptr (2) + terminator (2) + dram_stream + voice_init
    voice_ptr = PROGRAM_ADDR + 17 + 2 + 2 + len(dram_stream)

    # First voice_ptr (little-endian) - points to envelope/LFO init data
    prog.extend(struct.pack('<H', voice_ptr))

    # Slot pointer terminator (0x0000) - indicates no more envelope blocks
    prog.extend([0x00, 0x00])

    # D-RAM handler stream (after terminator)
    prog.extend(dram_stream)

    # Voice init data
    prog.extend(voice_init)

    # Padding for program_init_copy (8 bytes at offset 22 from program start)
    # This is read by voice_init_slots, pad with zeros
    while len(prog) < 30:
        prog.append(0x00)

    return bytes(prog)


def build_aram_data(algorithm: list) -> bytes:
    """Convert algorithm words to ROM format (little-endian byte pairs)."""
    data = bytearray()
    for word in algorithm:
        data.extend(struct.pack('<H', word))
    return bytes(data)


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Generate SAM8905 test ROM')
    parser.add_argument('output', nargs='?', default='test_rom.bin', help='Output ROM file')
    parser.add_argument('--algo', choices=['sinus', 'dpiano'], default='sinus',
                        help='Algorithm to use (default: sinus)')
    args = parser.parse_args()

    output_file = args.output
    algorithm = ALGORITHM_SINUS if args.algo == 'sinus' else ALGORITHM_DPIANO

    # Create empty ROM
    rom = bytearray(ROM_SIZE)
    rom[:] = [0xFF] * ROM_SIZE  # Fill with 0xFF (unprogrammed flash)

    # Build program pointer table at 0x0040
    # All entries point to our single test program (big-endian pointers)
    for i in range(NUM_PROGRAMS):
        ptr_addr = PROGRAM_PTR_TABLE + (i * 2)
        # Big-endian pointer
        rom[ptr_addr] = (PROGRAM_ADDR >> 8) & 0xFF
        rom[ptr_addr + 1] = PROGRAM_ADDR & 0xFF

    # Build A-RAM algorithm data
    aram_data = build_aram_data(algorithm)
    rom[ARAM_ADDR:ARAM_ADDR + len(aram_data)] = aram_data

    # Build test program
    program = build_program(
        name='TestSnd',
        slot_count=1,
        aram_ptr=ARAM_ADDR,
        dram_stream=DRAM_STREAM,
        voice_init=VOICE_INIT_DATA
    )
    rom[PROGRAM_ADDR:PROGRAM_ADDR + len(program)] = program

    # Write ROM file
    with open(output_file, 'wb') as f:
        f.write(rom)

    print(f"Generated test ROM: {output_file}")
    print(f"  ROM size: {ROM_SIZE} bytes (0x{ROM_SIZE:X})")
    print(f"  Algorithm: {args.algo}")
    print(f"  Program table: 0x{PROGRAM_PTR_TABLE:04X} ({NUM_PROGRAMS} entries)")
    print(f"  Program addr: 0x{PROGRAM_ADDR:04X} ({len(program)} bytes)")
    print(f"  A-RAM addr: 0x{ARAM_ADDR:04X} ({len(aram_data)} bytes)")
    print(f"  D-RAM stream: {len(DRAM_STREAM)} bytes")
    print()
    print("Usage:")
    print(f"  ./sam_emu -r {output_file} -p 0")
    print()
    print("Program structure:")
    print(f"  Name: 'TestSnd'")
    print(f"  Slots: 1")
    print(f"  Pitch: note_offset=0, vel_sens=0, fine_tune=0")
    print(f"  Amplitude: level=63, amp=127, no envelope")


if __name__ == '__main__':
    main()
