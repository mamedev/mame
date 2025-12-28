#!/usr/bin/env python3
"""
Extract SAM8905 test data from Keyfox10 ROM
Extracts A-RAM algorithms and D-RAM configurations for standalone testing
"""

import os
import struct

ROM_PATH = "/home/jeff/bastel/roms/hohner/keyfox10/kf10_ic27_v2.bin"
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "test_data")

# Algorithm addresses in DATA space (add 0x10000 for ROM offset)
ALGO_ADDRESSES = {
    "algo_e000": 0xE000,  # FX Algorithm 0
    "algo_e090": 0xE090,  # FX Algorithm 1
    "algo_e114": 0xE114,  # FX Algorithm 2
    "algo_e1ad": 0xE1AD,  # FX Algorithm 3
    "algo_e231": 0xE231,  # FX Algorithm 4
    "algo_e2c1": 0xE2C1,  # FX Algorithm 5
    "algo_0580": 0x0580,  # SND Algorithm 1
    "algo_0880": 0x0880,  # SND Algorithm 2
}

# 22.05kHz mode: 64 instructions x 2 bytes = 128 bytes per algorithm
ALGO_SIZE = 128


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


def extract_algorithm(rom, data_addr, output_name):
    """Extract 128 bytes of algorithm data from DATA space"""
    rom_offset = 0x10000 + data_addr
    algo_data = rom[rom_offset:rom_offset + ALGO_SIZE]

    out_path = os.path.join(OUTPUT_DIR, f"{output_name}.bin")
    with open(out_path, 'wb') as f:
        f.write(algo_data)

    print(f"Extracted {output_name}: DATA:{data_addr:04X} (ROM 0x{rom_offset:05X}) -> {out_path}")

    # Print first 16 instructions as hex
    print(f"  First 16 words:", end="")
    for i in range(0, 32, 2):
        word = (algo_data[i] << 8) | algo_data[i + 1]
        print(f" {word:04X}", end="")
    print()

    return algo_data


def create_simple_dram(slot, algo, freq, amp, mix_l=7, mix_r=7):
    """
    Create a simple D-RAM configuration for one slot

    D-RAM layout per slot (16 words x 19 bits):
      Word 0: PHI (phase accumulator) - upper 12 bits
      Word 1: DPHI (phase increment / frequency) - 19 bits
      Word 2: AMP/MIX (amplitude + pan)
      Word 15: Control (I=idle, ALG, M=mask)

    D-RAM Word 15 format:
      | 18-12 (unused) | 11 (I) | 10-8 (ALG) | 7 (M) | 6-0 (unused) |
    """
    dram = bytearray(256 * 3)  # 256 words x 3 bytes (19-bit packed)

    def write_word(addr, value):
        """Write 19-bit word as 3 bytes (little-endian)"""
        offset = addr * 3
        dram[offset] = value & 0xFF
        dram[offset + 1] = (value >> 8) & 0xFF
        dram[offset + 2] = (value >> 16) & 0x07

    base = slot << 4

    # Word 0: Initial phase = 0
    write_word(base + 0, 0)

    # Word 1: DPHI (frequency)
    # f = 0.042057 * DPHI for 22.05kHz
    # For 440Hz: DPHI = 440 / 0.042057 = 10462
    write_word(base + 1, freq)

    # Word 2: Amplitude + MIX
    # Format: | AMP (12 bits) | X | MIXL (3) | MIXR (3) |
    amp_mix = ((amp & 0xFFF) << 7) | ((mix_l & 7) << 3) | (mix_r & 7)
    write_word(base + 2, amp_mix)

    # Word 3: Waveform select
    # WWF reads (bus >> 9) & 0x1FF, so internal sine 0x100 at bits 17-9
    write_word(base + 3, 0x100 << 9)  # = 0x20000

    # Word 15: Control
    # Bits 11: I (0=active), Bits 10-8: ALG, Bit 7: M (0=interrupt enabled)
    ctrl = (algo & 0x7) << 8
    write_word(base + 15, ctrl)

    # Mark other slots as idle
    for s in range(16):
        if s != slot:
            write_word((s << 4) + 15, 0x800)  # I bit = 1 (idle)

    return dram


def create_test_dram_configs():
    """Create various D-RAM test configurations"""

    # Test 1: Simple sine at 440Hz, slot 0, algo 0
    # DPHI for 440Hz at 22.05kHz: 440 / 0.042057 ≈ 10462
    dram = create_simple_dram(slot=0, algo=0, freq=10462, amp=0x7FF, mix_l=7, mix_r=7)
    out_path = os.path.join(OUTPUT_DIR, "dram_sine_440hz.bin")
    with open(out_path, 'wb') as f:
        f.write(dram)
    print(f"Created: {out_path} (440Hz sine, slot 0, algo 0)")

    # Test 2: Lower frequency sine (110Hz) at reduced volume
    dram = create_simple_dram(slot=0, algo=0, freq=2615, amp=0x400, mix_l=5, mix_r=5)
    out_path = os.path.join(OUTPUT_DIR, "dram_sine_110hz.bin")
    with open(out_path, 'wb') as f:
        f.write(dram)
    print(f"Created: {out_path} (110Hz sine, slot 0, algo 0, -12dB)")

    # Test 3: Two simultaneous slots (chord)
    dram = bytearray(256 * 3)

    def write_word(addr, value):
        offset = addr * 3
        dram[offset] = value & 0xFF
        dram[offset + 1] = (value >> 8) & 0xFF
        dram[offset + 2] = (value >> 16) & 0x07

    # Slot 0: 440Hz (A4)
    write_word(0 * 16 + 1, 10462)  # DPHI
    write_word(0 * 16 + 2, (0x7FF << 7) | (6 << 3) | 6)  # AMP + MIX
    write_word(0 * 16 + 3, 0x100)  # Internal sine
    write_word(0 * 16 + 15, 0 << 8)  # ALG=0, active

    # Slot 1: 554Hz (C#5)
    write_word(1 * 16 + 1, 13175)  # DPHI
    write_word(1 * 16 + 2, (0x7FF << 7) | (6 << 3) | 6)  # AMP + MIX
    write_word(1 * 16 + 3, 0x100)  # Internal sine
    write_word(1 * 16 + 15, 0 << 8)  # ALG=0, active

    # Mark remaining slots as idle
    for s in range(2, 16):
        write_word((s << 4) + 15, 0x800)

    out_path = os.path.join(OUTPUT_DIR, "dram_chord.bin")
    with open(out_path, 'wb') as f:
        f.write(dram)
    print(f"Created: {out_path} (A4 + C#5 chord)")


def create_minimal_algo():
    """
    Create a minimal algorithm that just outputs internal sine

    Micro-instruction format (15 bits):
    | I14-I12 | I11-I10 | I9  | I8 | I7 | I6 | I5 | I4 | I3 | I2 | I1 | I0 |
    |   MAD   |  CMD    | WSP | WA | WB | WM |WPHI|WXY |clrB|WWF |WACC|

    CMD: 00=RM, 01=RADD, 10=RP, 11=RSP
    Active low receivers: WA, WB, WM, WPHI, WXY, clrB, WWF, WACC
    """
    algo = bytearray(128)

    def write_inst(idx, word):
        algo[idx * 2] = (word >> 8) & 0xFF
        algo[idx * 2 + 1] = word & 0xFF

    # Instruction 0: Read D-RAM[1] (DPHI), write to A and B
    # MAD=1, CMD=RM, WSP=0, WA=0, WB=0, rest=1
    # 0001 00 0 0 0 1 1 1 1 1 1 = 0x083F
    write_inst(0, 0x083F)

    # Instruction 1: Add A+B (accumulate phase), write to A
    # MAD=0, CMD=RADD, WSP=0, WA=0, rest=1
    # 0000 01 0 0 1 1 1 1 1 1 1 = 0x047F
    write_inst(1, 0x047F)

    # Instruction 2: Write A to D-RAM[0] (PHI), also WPHI
    # MAD=0, CMD=RADD, WSP=0, WA=1, WB=1, WM=0, WPHI=0, rest=1
    # 0000 01 0 1 1 0 0 1 1 1 1 = 0x059F
    write_inst(2, 0x059F)

    # Instruction 3: Read D-RAM[2] (AMP/MIX), WXY with WSP to load mix
    # MAD=2, CMD=RM, WSP=1, WA=1, WB=1, WM=1, WPHI=1, WXY=0, clrB=1, WWF=1, WACC=1
    # 0010 00 1 1 1 1 1 0 1 1 1 = 0x11F7
    write_inst(3, 0x11F7)

    # Instruction 4: Accumulate - output to mix
    # MAD=0, CMD=RP, WSP=0, all receivers off except WACC
    # 0000 10 0 1 1 1 1 1 1 1 0 = 0x09FE
    write_inst(4, 0x09FE)

    # Fill remaining with NOP (all receivers disabled)
    for i in range(5, 64):
        write_inst(i, 0x7FFF)

    out_path = os.path.join(OUTPUT_DIR, "algo_minimal.bin")
    with open(out_path, 'wb') as f:
        f.write(algo)
    print(f"Created: {out_path} (minimal sine generator algorithm)")

    # Print instructions
    print("  Minimal algorithm instructions:")
    for i in range(6):
        word = (algo[i * 2] << 8) | algo[i * 2 + 1]
        print(f"    {i:02d}: {word:04X}")


def main():
    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    print(f"Output directory: {OUTPUT_DIR}\n")

    # Read ROM
    rom = read_rom()
    print(f"ROM size: {len(rom)} bytes\n")

    # Extract algorithms from ROM
    print("=== Extracting Algorithms from ROM ===")
    for name, addr in ALGO_ADDRESSES.items():
        extract_algorithm(rom, addr, name)

    print()

    # Create D-RAM configurations
    print("=== Creating D-RAM Test Configurations ===")
    create_test_dram_configs()

    print()

    # Create minimal algorithm
    print("=== Creating Minimal Test Algorithm ===")
    create_minimal_algo()

    print("\nDone!")


if __name__ == '__main__':
    main()
