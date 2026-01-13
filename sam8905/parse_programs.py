#!/usr/bin/env python3
"""
nKeyfox10 Program and Sound Structure Parser
Parses sound program data and D-RAM templates from the ROM

Program structure analysis based on SAM_LOAD_SND_PROGRAM disassembly.
Sound/algorithm analysis based on SAM_INIT_AND_PLAY_SOUND disassembly.
"""

import struct
import sys

ROM_PATH = "/home/jeff/bastel/roms/hohner/keyfox10/kf10_ic27_v2.bin"

# CODE space tables
PROGRAM_PTR_TABLE = 0x6285
NUM_PROGRAMS = 97

# DATA space tables (add 0x10000 for ROM offset)
SOUND_PTR_TABLE = 0xFFA0  # DATA:FFA0
NUM_SOUNDS = 64


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


def get_word(rom, addr):
    """Read big-endian 16-bit word from CODE space"""
    return (rom[addr] << 8) | rom[addr + 1]


def get_data(rom, addr):
    """Read byte from DATA space (addr is DATA address)"""
    rom_offset = 0x10000 + addr
    return rom[rom_offset] if rom_offset < len(rom) else 0


def get_data_word(rom, addr):
    """Read big-endian 16-bit word from DATA space"""
    return (get_data(rom, addr) << 8) | get_data(rom, addr + 1)


def decode_template_ptr(ptr):
    """Decode the template pointer to understand its meaning"""
    high_nibble = (ptr >> 12) & 0xF
    low_byte = ptr & 0xFF

    type_map = {
        0x2: "TYPE_A",  # 0x2Axx
        0x6: "TYPE_B",  # 0x6Axx
        0xA: "TYPE_C",  # 0xAAxx
        0xE: "TYPE_D",  # 0xEAxx
    }

    prog_type = type_map.get(high_nibble, f"TYPE_{high_nibble:X}")
    return prog_type, low_byte


def decode_dram_ctrl(ctrl):
    """Decode D-RAM control byte (byte 2 of 3-byte entry)"""
    slot_nibble = (ctrl >> 3) & 0x0F
    mixr = ctrl & 0x07
    alg = (ctrl >> 6) & 0x03
    bit7 = (ctrl >> 7) & 1
    return {
        'slot_nibble': slot_nibble,
        'alg': alg,
        'mixr': mixr,
        'bit7': bit7,
        'raw': ctrl
    }


def parse_dram_entry(b0, b1, b2):
    """Parse a 3-byte D-RAM initialization entry"""
    ctrl = decode_dram_ctrl(b2)
    return {
        'value_lo': b0,
        'value_mid': b1,
        'ctrl': ctrl,
        'raw': f"{b0:02X} {b1:02X} {b2:02X}"
    }


def parse_sound_definition(rom, addr):
    """Parse a sound definition from DATA space"""
    sound = {'addr': addr, 'valid': True}
    pos = addr

    # Read algorithm count
    algo_count = get_data(rom, pos)
    if algo_count == 0 or algo_count > 8:
        sound['valid'] = False
        sound['algo_count'] = algo_count
        sound['algo_ptrs'] = []
        sound['voice_section_count'] = 0
        sound['voice_sections'] = []
        sound['size'] = 0
        return sound

    sound['algo_count'] = algo_count
    pos += 1

    # Read algorithm pointers
    sound['algo_ptrs'] = []
    for i in range(algo_count):
        algo_ptr = get_data_word(rom, pos)
        sound['algo_ptrs'].append(algo_ptr)
        pos += 2

    # Read voice section count
    voice_section_count = get_data(rom, pos)
    if voice_section_count > 16:
        sound['valid'] = False
        sound['voice_section_count'] = voice_section_count
        sound['voice_sections'] = []
        sound['size'] = pos - addr
        return sound

    sound['voice_section_count'] = voice_section_count
    pos += 1

    # Parse voice sections
    sound['voice_sections'] = []
    for vs in range(voice_section_count):
        section = {}

        # Read template pointer (2 bytes)
        template_ptr_raw = get_data_word(rom, pos)
        template_ptr = (template_ptr_raw + 0x80) & 0xFFFF
        section['template_ptr_raw'] = template_ptr_raw
        section['template_ptr'] = template_ptr
        pos += 2

        # Read slot count from template
        slot_count = get_data(rom, template_ptr)
        section['slot_count'] = slot_count

        # Parse template entries (3 bytes each)
        section['template_entries'] = []
        tpos = template_ptr + 1
        for s in range(min(slot_count, 16)):  # Limit to prevent runaway
            b0 = get_data(rom, tpos)
            b1 = get_data(rom, tpos + 1)
            b2 = get_data(rom, tpos + 2)
            section['template_entries'].append(parse_dram_entry(b0, b1, b2))
            tpos += 3

        # Read inline count
        inline_count = get_data(rom, pos)
        section['inline_count'] = inline_count
        pos += 1

        # Parse inline entries (3 bytes each)
        section['inline_entries'] = []
        for i in range(min(inline_count, 16)):  # Limit to prevent runaway
            b0 = get_data(rom, pos)
            b1 = get_data(rom, pos + 1)
            b2 = get_data(rom, pos + 2)
            section['inline_entries'].append(parse_dram_entry(b0, b1, b2))
            pos += 3

        sound['voice_sections'].append(section)

    sound['end_addr'] = pos
    sound['size'] = pos - addr
    return sound


def parse_program(rom, addr, next_addr=None):
    """Parse a program structure starting at addr"""
    prog = {}

    # Header
    prog['addr'] = addr
    prog['name'] = rom[addr:addr+8].decode('ascii', errors='replace').rstrip('\x00 ')
    prog['null_term'] = rom[addr + 8]
    prog['flags'] = rom[addr + 9]
    prog['template_ptr'] = get_word(rom, addr + 10)
    prog['prog_type'], prog['type_param'] = decode_template_ptr(prog['template_ptr'])

    # Calculate size
    if next_addr and next_addr > addr:
        prog['size'] = next_addr - addr
    else:
        prog['size'] = None

    # Analyze the data section
    prog['raw_header'] = rom[addr:addr+16].hex(' ')

    # Check for 0xFF terminator
    terminator_pos = None
    max_search = min(150, prog['size']) if prog['size'] else 150
    for i in range(12, max_search):
        if rom[addr + i] == 0xFF:
            terminator_pos = i
            break
    prog['terminator_offset'] = terminator_pos

    # Decode flags
    prog['flag_bit0'] = bool(prog['flags'] & 0x01)
    prog['flag_bit4'] = bool(prog['flags'] & 0x10)

    # Parse voice indices (pattern: idx, 0x05, idx, 0x05, ...)
    prog['voice_indices'] = []
    if prog['size'] and prog['size'] > 20:
        data = rom[addr:addr + prog['size']]
        pos = 15  # First index at offset 15
        while pos < len(data) - 1 and data[pos + 1] == 0x05:
            prog['voice_indices'].append(data[pos])
            pos += 2

    return prog


def print_sound_analysis(rom):
    """Analyze and print sound definitions"""
    print("=" * 70)
    print("SOUND DEFINITION ANALYSIS (DATA:FFA0 table)")
    print("=" * 70)

    sounds = []
    for i in range(NUM_SOUNDS):
        ptr_addr = SOUND_PTR_TABLE + i * 2
        ptr = get_data_word(rom, ptr_addr)
        if ptr != 0xFFFF and ptr > 0 and ptr < 0xF000:
            sound = parse_sound_definition(rom, ptr)
            sound['index'] = i
            if sound['valid']:
                sounds.append(sound)

    print(f"\nFound {len(sounds)} valid sound definitions\n")

    # Summary table
    print(f"{'Idx':<4} {'Addr':<6} {'Size':<5} {'Algos':<6} {'VoiceSec':<10} {'Algorithm Ptrs'}")
    print("-" * 70)

    for sound in sounds[:16]:  # First 16
        algo_str = ' '.join(f"{p:04X}" for p in sound['algo_ptrs'][:4])
        print(f"{sound['index']:<4} {sound['addr']:04X}  {sound['size']:<5} "
              f"{sound['algo_count']:<6} {sound['voice_section_count']:<10} {algo_str}")

    if len(sounds) > 16:
        print(f"... and {len(sounds) - 16} more sounds")

    # Unique algorithms
    print("\n" + "=" * 70)
    print("UNIQUE ALGORITHM POINTERS")
    print("-" * 50)

    all_algos = set()
    for sound in sounds:
        all_algos.update(sound['algo_ptrs'])

    for algo in sorted(all_algos):
        rom_offset = 0x10000 + algo
        print(f"  DATA:{algo:04X} (ROM 0x{rom_offset:05X})")

    # Detailed dump of first few sounds
    print("\n" + "=" * 70)
    print("DETAILED SOUND DUMPS")
    print("-" * 50)

    for sound in sounds[:3]:
        print(f"\nSound {sound['index']}: DATA:{sound['addr']:04X}")
        print(f"  algo_count: {sound['algo_count']}")
        for i, ptr in enumerate(sound['algo_ptrs']):
            template_at = (ptr + 0x80) & 0xFFFF
            print(f"  algo_ptr[{i}]: DATA:{ptr:04X} (template at DATA:{template_at:04X})")
        print(f"  voice_section_count: {sound['voice_section_count']}")

        for vs_idx, vs in enumerate(sound['voice_sections'][:2]):  # First 2 sections
            print(f"\n  Voice Section {vs_idx}:")
            print(f"    template_ptr: {vs['template_ptr_raw']:04X} -> DATA:{vs['template_ptr']:04X}")
            print(f"    slot_count: {vs['slot_count']}")
            for i, entry in enumerate(vs['template_entries'][:4]):
                ctrl = entry['ctrl']
                print(f"      Entry {i}: {entry['raw']} -> slot={ctrl['slot_nibble']:X}, "
                      f"ALG={ctrl['alg']}, MIXR={ctrl['mixr']}")
            if vs['slot_count'] > 4:
                print(f"      ... ({vs['slot_count'] - 4} more)")

            print(f"    inline_count: {vs['inline_count']}")
            for i, entry in enumerate(vs['inline_entries'][:4]):
                ctrl = entry['ctrl']
                print(f"      Inline {i}: {entry['raw']} -> slot={ctrl['slot_nibble']:X}, "
                      f"ALG={ctrl['alg']}, MIXR={ctrl['mixr']}")

        if sound['voice_section_count'] > 2:
            print(f"\n  ... ({sound['voice_section_count'] - 2} more voice sections)")


def print_program_analysis(rom):
    """Analyze and print program definitions"""
    print("\n" + "=" * 70)
    print("PROGRAM STRUCTURE ANALYSIS (CODE:6285 table)")
    print("=" * 70)

    programs = []
    addrs = []

    for i in range(NUM_PROGRAMS):
        ptr_addr = PROGRAM_PTR_TABLE + (i * 2)
        prog_addr = get_word(rom, ptr_addr)
        addrs.append(prog_addr)

    # Parse each program
    for i, addr in enumerate(addrs):
        next_addr = addrs[i + 1] if i + 1 < len(addrs) else None

        # Skip invalid entries
        if addr == 0xFFFF or addr < 0x1000:
            continue

        prog = parse_program(rom, addr, next_addr)
        programs.append((i, prog))

    # Print summary
    print(f"\n{'Idx':<4} {'Addr':<6} {'Size':<5} {'Name':<10} {'Flags':<8} {'TmplPtr':<8} {'Voices'}")
    print("-" * 70)

    for idx, prog in programs[:20]:  # First 20
        size_str = str(prog['size']) if prog['size'] else '?'
        voices = len(prog['voice_indices'])
        print(f"{idx:<4} 0x{prog['addr']:04X} {size_str:<5} {prog['name']:<10} "
              f"0x{prog['flags']:02X}    0x{prog['template_ptr']:04X}   {voices}")

    if len(programs) > 20:
        print(f"... and {len(programs) - 20} more programs")

    # Analyze unique flag values
    print("\n" + "=" * 70)
    print("FLAG ANALYSIS (offset 9)")
    print("-" * 50)

    flag_counts = {}
    for idx, prog in programs:
        flag = prog['flags']
        if flag not in flag_counts:
            flag_counts[flag] = []
        flag_counts[flag].append(prog['name'])

    for flag, names in sorted(flag_counts.items()):
        print(f"Flag 0x{flag:02X}: {len(names)} programs")
        if len(names) <= 5:
            print(f"  Examples: {', '.join(names)}")

    # Analyze template pointers
    print("\n" + "=" * 70)
    print("TEMPLATE POINTER ANALYSIS (offset 10-11)")
    print("-" * 50)

    ptr_counts = {}
    for idx, prog in programs:
        ptr = prog['template_ptr']
        if ptr not in ptr_counts:
            ptr_counts[ptr] = []
        ptr_counts[ptr].append(prog['name'])

    for ptr, names in sorted(ptr_counts.items()):
        prog_type, _ = decode_template_ptr(ptr)
        print(f"Ptr 0x{ptr:04X} ({prog_type}): {len(names)} programs")

    # Voice index analysis
    print("\n" + "=" * 70)
    print("VOICE INDEX ANALYSIS")
    print("-" * 50)
    print("\nVoice indices point to EXTMEM:1200 + index (12-byte slots)\n")

    for idx, prog in programs[:8]:
        if prog['voice_indices']:
            indices_str = ', '.join(f'0x{i:02X}' for i in prog['voice_indices'])
            print(f"  {prog['name']}: {len(prog['voice_indices'])} voices [{indices_str}]")


def print_dram_format():
    """Print D-RAM format documentation"""
    print("\n" + "=" * 70)
    print("D-RAM INITIALIZATION FORMAT")
    print("-" * 50)
    print("""
Each D-RAM entry is 3 bytes:
  Byte 0: value_lo    - D-RAM word LSB
  Byte 1: value_mid   - D-RAM word NSB
  Byte 2: ctrl_flags  - Control byte
          Bit 7:      Conditional offset flag
          Bits 6-3:   Slot address lower nibble
          Bits 2-0:   MIXR (right channel, 0=mute, 7=full)

D-RAM Word 15 (control word) format:
  Bits 8-6: ALG  (algorithm number 0-7)
  Bits 5-3: MIXL (left channel, 0=mute, 7=0dB)
  Bits 2-0: MIXR (right channel, 0=mute, 7=0dB)

EXTMEM buffer (5 bytes at 0x0000-0x0004):
  [0]: D-RAM address = (voice_section << 4) | (ctrl >> 3 & 0x0F)
  [1]: value_lo (+ offset if bit7 set)
  [2]: value_mid (+ offset if bit7 set)
  [3]: ctrl_byte
  [4]: sam_ctrl_flags (from INTMEM:69)
""")


def main():
    rom = read_rom()

    print_sound_analysis(rom)
    print_program_analysis(rom)
    print_dram_format()


if __name__ == '__main__':
    main()
