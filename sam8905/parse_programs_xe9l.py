#!/usr/bin/env python3
"""
XE9L Program Structure Parser

Parses program data from the XE9L (Hohner) ROM.
Uses shared parsing code from parse_programs.py.

XE9L specifics:
- ROM: xe9l_v141.bin (64KB)
- Program pointer table at 0x33AD, 171 entries (big-endian)
- Undefined program marker: 0x001E
- 8 algorithms loaded by load_algo_table_1:
  - 0x871A → A-RAM slot 0 (percussion/rhythm)
  - 0x86DA → A-RAM slot 1 (percussion/rhythm)
  - 0x00EA → A-RAM slot 2 (CLAVINET)
  - 0x00AA → A-RAM slot 3 (accordion variants)
  - 0x006A → A-RAM slot 4 (main instruments)
  - 0x002A → A-RAM slot 5 (bass/piano)
  - 0x3503 → A-RAM slot 6 (percussion/rhythm)
  - 0x9AFF → A-RAM slot 7 (percussion/rhythm)
- Programs reference only 4 algorithms: 0x002A, 0x006A, 0x00AA, 0x00EA
"""

import sys
import os

# Import shared parsing functions
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from parse_programs import (
    get_word, parse_program, parse_aram_data,
    decode_dram_stream, decode_dram_init, analyze_algorithms_wwf,
    format_handler_detail, format_voice_init_detail,
    decode_algorithm, analyze_dram_usage,
)

ROM_PATH = "/home/jeff/bastel/roms/hohner/ms4/xe9l_v141.bin"

# Program pointer table
PROGRAM_PTR_TABLE = 0x33AD
NUM_PROGRAMS = 171
UNDEFINED_ADDR = 0x001E  # Empty/undefined program marker

# All 8 algorithms loaded by load_algo_table_1
# Format: (rom_addr, aram_slot, description)
ALL_ALGORITHMS = [
    (0x871A, 0x00, "percussion/rhythm slot 0"),
    (0x86DA, 0x20, "percussion/rhythm slot 1"),
    (0x00EA, 0x40, "CLAVINET"),
    (0x00AA, 0x60, "accordion variants"),
    (0x006A, 0x80, "main instruments"),
    (0x002A, 0xA0, "bass/piano"),
    (0x3503, 0xC0, "percussion/rhythm slot 6"),
    (0x9AFF, 0xE0, "percussion/rhythm slot 7"),
]

# Algorithms referenced by programs (subset of above)
PROGRAM_ALGORITHMS = {0x002A, 0x006A, 0x00AA, 0x00EA}

# Drum kit and drum sounds
DRUM_KIT_ADDR = 0x8600      # "STA DRUM" kit header
DRUM_SOUNDS_START = 0x8760  # First drum sound entry
DRUM_SOUNDS_END = 0x8D6C    # End of drum sound area
DRUM_ALGORITHM = 0x86DA     # All drum sounds use this algorithm


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


def parse_drum_sounds(rom):
    """
    Parse drum sound entries from ROM.

    Drum sounds are stored at 0x8760-0x8D6B and all use algorithm 0x86DA.
    Each entry has the same format as regular programs:
    - 8-byte ASCII name
    - 1-byte flags
    - 2-byte algorithm pointer (big-endian)
    - D-RAM init data

    Returns list of (index, drum_data) tuples.
    """
    drums = []
    addr = DRUM_SOUNDS_START
    idx = 0

    while addr < DRUM_SOUNDS_END:
        # Check if we have a valid drum entry by looking for algorithm pointer
        if addr + 10 >= len(rom):
            break

        # Read potential name (8 bytes)
        name_bytes = rom[addr:addr + 8]

        # Check if name looks valid (printable ASCII)
        try:
            name = name_bytes.rstrip(b'\x00 ').decode('ascii')
            if not name or not all(0x20 <= b < 0x7F for b in name_bytes[:len(name)]):
                break
        except (UnicodeDecodeError, ValueError):
            break

        # Read flags at offset 9 (offset 8 is null terminator)
        flags = rom[addr + 9]

        # Read algorithm pointer at offset 10-11 (little-endian)
        aram_ptr = rom[addr + 10] | (rom[addr + 11] << 8)

        # Verify this is a drum sound (must use DRUM_ALGORITHM)
        if aram_ptr != DRUM_ALGORITHM:
            # Try next byte in case of alignment issue
            addr += 1
            continue

        # Parse as program using shared function
        # Find next drum address for size calculation
        next_addr = None
        test_addr = addr + 20  # Minimum size
        while test_addr < DRUM_SOUNDS_END and test_addr + 12 < len(rom):
            test_ptr = rom[test_addr + 10] | (rom[test_addr + 11] << 8)
            if test_ptr == DRUM_ALGORITHM:
                # Check if name looks valid
                try:
                    test_name = rom[test_addr:test_addr + 8].rstrip(b'\x00 ').decode('ascii')
                    if test_name and all(0x20 <= rom[test_addr + i] < 0x7F for i in range(len(test_name))):
                        next_addr = test_addr
                        break
                except:
                    pass
            test_addr += 1

        prog = parse_program(rom, addr, next_addr)
        if prog:
            prog['is_drum'] = True
            drums.append((idx, prog))
            idx += 1

            # Move to next entry
            if next_addr:
                addr = next_addr
            else:
                break
        else:
            addr += 1

    return drums


def parse_drum_kit(rom):
    """
    Parse drum kit header and mapping table at 0x8600.

    Structure:
    - 8-byte name ("STA DRUM")
    - 1-byte flags (0x82)
    - 2-byte algorithm pointer (0x86DA)
    - Variable header data
    - Mapping table: 3-byte entries (ptr_hi, ptr_lo, flags)
      - flags=0x80: Direct drum sound pointer
      - flags=0x13: Reference to regular program

    Returns dict with kit info and mappings.
    """
    addr = DRUM_KIT_ADDR

    # Read name
    name_bytes = rom[addr:addr + 8]
    try:
        name = name_bytes.rstrip(b'\x00 ').decode('ascii')
    except:
        name = "???"

    # offset 8 is null terminator, flags at offset 9
    flags = rom[addr + 9]
    # Algorithm pointer at offset 10-11 (little-endian)
    aram_ptr = rom[addr + 10] | (rom[addr + 11] << 8)

    kit = {
        'addr': addr,
        'name': name,
        'flags': flags,
        'aram_ptr': aram_ptr,
        'mappings': []
    }

    # Mapping table starts around offset 38 (based on earlier analysis)
    # Look for start of mapping entries
    map_start = addr + 38

    # Parse mapping entries (3 bytes each)
    for i in range(128):  # Max MIDI notes
        entry_addr = map_start + (i * 3)
        if entry_addr + 3 > len(rom):
            break

        ptr_hi = rom[entry_addr]
        ptr_lo = rom[entry_addr + 1]
        entry_flags = rom[entry_addr + 2]

        ptr = (ptr_hi << 8) | ptr_lo

        # Stop if we hit invalid data
        if ptr == 0 and entry_flags == 0:
            # Check if more valid entries follow
            if entry_addr + 6 < len(rom):
                next_ptr = (rom[entry_addr + 3] << 8) | rom[entry_addr + 4]
                if next_ptr == 0:
                    break

        mapping = {
            'index': i,
            'ptr': ptr,
            'flags': entry_flags,
            'type': 'drum' if entry_flags == 0x80 else 'program' if entry_flags == 0x13 else 'unknown'
        }
        kit['mappings'].append(mapping)

    return kit


def print_program_summary(programs):
    """Print summary table of all programs"""
    print("=" * 110)
    print(f"XE9L PROGRAM STRUCTURE ANALYSIS (0x{PROGRAM_PTR_TABLE:04X} table, {NUM_PROGRAMS} entries)")
    print("=" * 110)

    print(f"\n{'Idx':<4} {'Addr':<7} {'Size':<5} {'Name':<10} {'Flags':<6} "
          f"{'Slots':<6} {'A-RAM':<7} {'DRAMe0':<12} {'VoicePtr':<9} "
          f"{'Stream':<7} {'VoiceData (slot 0)'}")
    print("-" * 110)

    for idx, prog in programs:
        size_str = str(prog['size']) if prog['size'] else '?'
        ci = 'C' if prog['complex_init'] else ' '
        de0 = prog['dram_entry0']['raw']
        vptr = prog['voice_data_ptr']
        vptr_str = f"0x{vptr:04X}" if vptr != 0 else "  --- "
        stream_len = len(prog['dram_stream'])

        if prog['voice_slots']:
            vdata = ' '.join(f"{b:02X}" for b in prog['voice_slots'][0]['data'])
        else:
            vdata = "---"

        print(f"{idx:<4} 0x{prog['addr']:04X} {size_str:<5} {prog['name']:<10} "
              f"0x{prog['flags']:02X}{ci} {prog['slot_count']:<6} "
              f"0x{prog['aram_ptr']:04X} {de0}  {vptr_str}   "
              f"{stream_len:<7} {vdata}")


def print_aram_analysis(programs, rom):
    """Print A-RAM algorithm analysis"""
    print("\n" + "=" * 100)
    print("A-RAM ALGORITHM ANALYSIS (8 algorithms loaded by load_algo_table_1)")
    print("=" * 100)

    # Find unique A-RAM pointers from programs
    aram_ptrs = {}
    for idx, prog in programs:
        ptr = prog['aram_ptr']
        if ptr not in aram_ptrs:
            aram_ptrs[ptr] = []
        aram_ptrs[ptr].append((idx, prog['name']))

    # Print algorithm table
    print(f"\n{'ROM Addr':<10} {'A-RAM':<7} {'Programs':<8} {'Description'}")
    print("-" * 80)

    for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
        prog_count = len(aram_ptrs.get(rom_addr, []))
        prog_str = str(prog_count) if prog_count > 0 else "-"
        print(f"0x{rom_addr:04X}    0x{aram_slot:02X}    {prog_str:<8} {desc}")

    # Decode all 8 algorithms
    print("\n" + "-" * 80)
    print("DECODED A-RAM ALGORITHMS")
    print("-" * 80)

    for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
        aram_data = parse_aram_data(rom, rom_addr)
        if aram_data:
            prog_list = aram_ptrs.get(rom_addr, [])
            if prog_list:
                users = ', '.join(name for _, name in prog_list[:5])
                if len(prog_list) > 5:
                    users += f" (+{len(prog_list) - 5} more)"
                user_info = f"used by {len(prog_list)} programs: {users}"
            else:
                user_info = "not referenced by programs (percussion/rhythm)"
            print(f"\n{'─' * 60}")
            print(f"A-RAM at 0x{rom_addr:04X} → slot 0x{aram_slot:02X} ({user_info})")
            print(decode_algorithm(aram_data))
            print("\nD-RAM usage:")
            usage = analyze_dram_usage(aram_data)
            for addr, counts in sorted(usage.items()):
                print(f"  D-RAM[{addr:2d}]: read={counts['read']}, write={counts['write']}")


def print_drum_summary(drums, kit):
    """Print summary of drum sounds and kit mapping"""
    print("\n" + "=" * 100)
    print("DRUM SOUNDS AND KIT ANALYSIS")
    print("=" * 100)

    # Drum kit info
    print(f"\nDrum Kit: \"{kit['name']}\" at 0x{kit['addr']:04X}")
    print(f"  Flags: 0x{kit['flags']:02X}, Algorithm: 0x{kit['aram_ptr']:04X}")
    print(f"  Mappings: {len(kit['mappings'])} entries")

    # Count mapping types
    drum_refs = [m for m in kit['mappings'] if m['type'] == 'drum']
    prog_refs = [m for m in kit['mappings'] if m['type'] == 'program']
    print(f"    Direct drum pointers (0x80): {len(drum_refs)}")
    print(f"    Program references (0x13): {len(prog_refs)}")

    # Drum sounds table
    print(f"\n{'Idx':<4} {'Addr':<7} {'Size':<5} {'Name':<10} {'Flags':<6} "
          f"{'Slots':<6} {'A-RAM':<7} {'VoiceData (slot 0)'}")
    print("-" * 90)

    for idx, drum in drums:
        size_str = str(drum['size']) if drum['size'] else '?'
        ci = 'C' if drum['complex_init'] else ' '

        if drum['voice_slots']:
            vdata = ' '.join(f"{b:02X}" for b in drum['voice_slots'][0]['data'][:8])
        else:
            vdata = "---"

        print(f"{idx:<4} 0x{drum['addr']:04X} {size_str:<5} {drum['name']:<10} "
              f"0x{drum['flags']:02X}{ci} {drum['slot_count']:<6} "
              f"0x{drum['aram_ptr']:04X} {vdata}")

    # Show some kit mappings
    print(f"\nKit Mapping Table (first 40 entries):")
    print(f"{'Idx':<4} {'Ptr':<7} {'Flags':<6} {'Type':<10}")
    print("-" * 40)
    for m in kit['mappings'][:40]:
        ptr_str = f"0x{m['ptr']:04X}" if m['ptr'] else "  ---"
        print(f"{m['index']:<4} {ptr_str} 0x{m['flags']:02X}   {m['type']}")


def print_detailed_programs(programs):
    """Print detailed per-program data"""
    print("\n" + "=" * 100)
    print("DETAILED PROGRAM DATA")
    print("=" * 100)

    for idx, prog in programs:
        print(f"\n{'─' * 80}")
        print(f"Program {idx}: \"{prog['name']}\" at 0x{prog['addr']:04X} "
              f"(size={prog['size']})")
        print(f"  Raw header: {prog['raw_header']}")
        print(f"  Flags: 0x{prog['flags']:02X} "
              f"(complex={'yes' if prog['complex_init'] else 'no'}, "
              f"slots={prog['slot_count']})")
        print(f"  A-RAM ptr: 0x{prog['aram_ptr']:04X}")

        de0 = prog['dram_entry0']
        ctrl = de0['ctrl']
        print(f"  D-RAM entry0: [{de0['raw']}] "
              f"word=0x{de0['dram_word']:04X}, "
              f"addr_nibble={ctrl['addr_nibble']}, "
              f"mix={ctrl['mix_bits']}")

        if prog['voice_slots']:
            print(f"  Voice slots: {len(prog['voice_slots'])}")
            for i, slot in enumerate(prog['voice_slots']):
                vdata = ' '.join(f"{b:02X}" for b in slot['data'])
                print(f"    Slot {i}: ptr=0x{slot['ptr']:04X} data=[{vdata}]")
                detail = format_voice_init_detail(slot['data'])
                if detail:
                    print(f"             {detail}")
        else:
            print(f"  Voice slots: NONE")

        stream = prog['dram_stream']
        if stream:
            print(f"  D-RAM stream (offset {prog['dram_stream_offset']}, "
                  f"{len(stream)} bytes):")
            entries = decode_dram_stream(stream)
            for entry in entries:
                byte_hex = ' '.join(f"{b:02X}" for b in entry['bytes'])
                print(f"    word {entry['word']:2d} [{entry['offset']:3d}]: "
                      f"{entry['name']:<10s} {byte_hex}")
                detail = format_handler_detail(entry)
                if detail:
                    print(f"{'':>29}{detail}")


def export_programs_python(programs, rom, output_path, drums=None):
    """Export programs grouped by algorithm to a Python file."""
    # Collect all A-RAM pointers (from programs + additional ones)
    all_aram_ptrs = set(addr for addr, _, _ in ALL_ALGORITHMS)
    for _, prog in programs:
        all_aram_ptrs.add(prog['aram_ptr'])

    if drums is None:
        drums = []

    # Analyze WWF instructions for each algorithm
    waveform_words = analyze_algorithms_wwf(rom, all_aram_ptrs)

    # Group programs by A-RAM pointer
    by_algorithm = {}
    for idx, prog in programs:
        ptr = prog['aram_ptr']
        ptr_key = f"{ptr:04X}"
        if ptr_key not in by_algorithm:
            by_algorithm[ptr_key] = {
                'aram_data': prog['aram_data'],
                'programs': []
            }
        by_algorithm[ptr_key]['programs'].append((idx, prog))

    with open(output_path, 'w') as f:
        f.write('"""\n')
        f.write('XE9L Program Data Export\n')
        f.write('Generated by parse_programs_xe9l.py\n')
        f.write('\n')
        f.write('Contains A-RAM algorithm data and D-RAM configurations\n')
        f.write('for XE9L programs, grouped by algorithm.\n')
        f.write('\n')
        f.write('8 algorithms loaded by load_algo_table_1:\n')
        for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
            f.write(f'  0x{rom_addr:04X} -> A-RAM slot 0x{aram_slot:02X} ({desc})\n')
        f.write('"""\n\n')

        # Export ALL 8 algorithms (including those not referenced by programs)
        f.write('# A-RAM algorithm data (32 x 15-bit instruction words)\n')
        f.write('# Includes all 8 algorithms loaded by load_algo_table_1\n')
        f.write('ALGORITHMS = {\n')
        for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
            ptr_key = f"{rom_addr:04X}"
            aram = parse_aram_data(rom, rom_addr)
            if aram:
                f.write(f"    '{ptr_key}': [  # slot 0x{aram_slot:02X} - {desc}\n")
                for i in range(0, 32, 8):
                    chunk = aram[i:i+8]
                    hex_vals = ', '.join(f'0x{v:04X}' for v in chunk)
                    f.write(f"        {hex_vals},\n")
                f.write("    ],\n")
        f.write('}\n\n')

        # Export algorithm slot mapping
        f.write('# Algorithm ROM address to A-RAM slot mapping\n')
        f.write('ALGORITHM_SLOTS = {\n')
        for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
            f.write(f"    '0x{rom_addr:04X}': 0x{aram_slot:02X},  # {desc}\n")
        f.write('}\n\n')

        # Export programs for each algorithm
        for ptr_key in sorted(by_algorithm.keys()):
            progs = by_algorithm[ptr_key]['programs']
            f.write(f'# Programs using algorithm at 0x{ptr_key} ({len(progs)} programs)\n')
            f.write(f'PROGRAMS_{ptr_key} = [\n')

            wf_words = waveform_words.get(ptr_key, [])

            for idx, prog in progs:
                f.write('    {\n')
                f.write(f"        'idx': {idx},\n")
                f.write(f"        'name': '{prog['name']}',\n")
                f.write(f"        'addr': 0x{prog['addr']:04X},\n")
                f.write(f"        'flags': 0x{prog['flags']:02X},\n")
                f.write(f"        'slot_count': {prog['slot_count']},\n")
                f.write(f"        'complex_init': {prog['complex_init']},\n")

                de0 = prog['dram_entry0']
                f.write(f"        'dram_entry0': {{'word': 0x{de0['dram_word']:04X}, "
                        f"'addr_nibble': {de0['ctrl']['addr_nibble']}, "
                        f"'mix_bits': {de0['ctrl']['mix_bits']}}},\n")

                f.write("        'voice_slots': [\n")
                for slot in prog['voice_slots']:
                    data_hex = ', '.join(f'0x{b:02X}' for b in slot['data'])
                    f.write(f"            {{'ptr': 0x{slot['ptr']:04X}, 'data': [{data_hex}]}},\n")
                f.write("        ],\n")

                stream = prog['dram_stream']
                if stream:
                    stream_hex = ', '.join(f'0x{b:02X}' for b in stream)
                    f.write(f"        'dram_stream': [{stream_hex}],\n")
                else:
                    f.write("        'dram_stream': [],\n")

                # Decoded D-RAM init
                if stream:
                    dram_init = decode_dram_init(stream, wf_words)
                    dram_hex = ', '.join(f'0x{v:05X}' for v in dram_init['dram'])
                    f.write(f"        'dram_init': [{dram_hex}],\n")
                    pp = dram_init['pitch']
                    f.write(f"        'pitch_params': {{'note_offset': {pp['note_offset']}, "
                            f"'fine_tune': {pp['fine_tune']}, 'vel_sens': {pp['vel_sens']}}},\n")
                    ap = dram_init['amp']
                    f.write(f"        'amp_params': {{'level': {ap['level']}, 'amp': 0x{ap['amp']:02X}, "
                            f"'vel_sens': {ap['vel_sens']}, 'env_ctrl': 0x{ap['env_ctrl']:02X}}},\n")
                    f.write(f"        'waveform_words': {wf_words},\n")
                else:
                    f.write("        'dram_init': [0] * 16,\n")
                    f.write("        'pitch_params': {'note_offset': 0, 'fine_tune': 0, 'vel_sens': 0},\n")
                    f.write("        'amp_params': {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0x00},\n")
                    f.write("        'waveform_words': [],\n")

                f.write('    },\n')

            f.write(']\n\n')

        f.write('# List of all algorithm keys\n')
        f.write(f"ALGORITHM_KEYS = {sorted(by_algorithm.keys())}\n\n")

        f.write('# Mapping from algorithm key to program list\n')
        f.write('PROGRAMS_BY_ALGORITHM = {\n')
        for ptr_key in sorted(by_algorithm.keys()):
            f.write(f"    '{ptr_key}': PROGRAMS_{ptr_key},\n")
        f.write('}\n\n')

        # Export drum sounds
        if drums:
            wf_words_drum = waveform_words.get(f"{DRUM_ALGORITHM:04X}", [])

            f.write(f'# Drum sounds (all use algorithm 0x{DRUM_ALGORITHM:04X})\n')
            f.write(f'DRUM_SOUNDS = [\n')

            for idx, drum in drums:
                f.write('    {\n')
                f.write(f"        'idx': {idx},\n")
                f.write(f"        'name': '{drum['name']}',\n")
                f.write(f"        'addr': 0x{drum['addr']:04X},\n")
                f.write(f"        'flags': 0x{drum['flags']:02X},\n")
                f.write(f"        'slot_count': {drum['slot_count']},\n")
                f.write(f"        'complex_init': {drum['complex_init']},\n")
                f.write(f"        'aram_ptr': 0x{drum['aram_ptr']:04X},\n")

                de0 = drum['dram_entry0']
                f.write(f"        'dram_entry0': {{'word': 0x{de0['dram_word']:04X}, "
                        f"'addr_nibble': {de0['ctrl']['addr_nibble']}, "
                        f"'mix_bits': {de0['ctrl']['mix_bits']}}},\n")

                f.write("        'voice_slots': [\n")
                for slot in drum['voice_slots']:
                    data_hex = ', '.join(f'0x{b:02X}' for b in slot['data'])
                    f.write(f"            {{'ptr': 0x{slot['ptr']:04X}, 'data': [{data_hex}]}},\n")
                f.write("        ],\n")

                stream = drum['dram_stream']
                if stream:
                    stream_hex = ', '.join(f'0x{b:02X}' for b in stream)
                    f.write(f"        'dram_stream': [{stream_hex}],\n")
                else:
                    f.write("        'dram_stream': [],\n")

                # Decoded D-RAM init
                if stream:
                    dram_init = decode_dram_init(stream, wf_words_drum)
                    dram_hex = ', '.join(f'0x{v:05X}' for v in dram_init['dram'])
                    f.write(f"        'dram_init': [{dram_hex}],\n")
                    pp = dram_init['pitch']
                    f.write(f"        'pitch_params': {{'note_offset': {pp['note_offset']}, "
                            f"'fine_tune': {pp['fine_tune']}, 'vel_sens': {pp['vel_sens']}}},\n")
                    ap = dram_init['amp']
                    f.write(f"        'amp_params': {{'level': {ap['level']}, 'amp': 0x{ap['amp']:02X}, "
                            f"'vel_sens': {ap['vel_sens']}, 'env_ctrl': 0x{ap['env_ctrl']:02X}}},\n")
                    f.write(f"        'waveform_words': {wf_words_drum},\n")
                else:
                    f.write("        'dram_init': [0] * 16,\n")
                    f.write("        'pitch_params': {'note_offset': 0, 'fine_tune': 0, 'vel_sens': 0},\n")
                    f.write("        'amp_params': {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0x00},\n")
                    f.write("        'waveform_words': [],\n")

                f.write('    },\n')

            f.write(']\n\n')

            f.write(f'# Drum algorithm address\n')
            f.write(f'DRUM_ALGORITHM = 0x{DRUM_ALGORITHM:04X}\n')

    total_exported = sum(len(v['programs']) for v in by_algorithm.values())
    print(f"Exported {total_exported} programs to {output_path}")
    if drums:
        print(f"Exported {len(drums)} drum sounds")
    print(f"Algorithms exported: {len(ALL_ALGORITHMS)} (all loaded by load_algo_table_1)")
    for rom_addr, aram_slot, desc in ALL_ALGORITHMS:
        ptr_key = f"{rom_addr:04X}"
        count = len(by_algorithm.get(ptr_key, {}).get('programs', []))
        prog_info = f"{count} programs" if count > 0 else "no programs (rhythm/percussion)"
        print(f"  0x{ptr_key} (slot 0x{aram_slot:02X}): {prog_info}")


def main():
    rom = read_rom()
    print(f"ROM: {ROM_PATH}")
    print(f"ROM size: {len(rom)} bytes (0x{len(rom):X})")

    # Read all program pointers
    addrs = []
    for i in range(NUM_PROGRAMS):
        ptr_addr = PROGRAM_PTR_TABLE + (i * 2)
        prog_addr = get_word(rom, ptr_addr)
        addrs.append(prog_addr)

    # Build sorted address list for size calculation
    valid_addrs = sorted(set(
        a for a in addrs if a != UNDEFINED_ADDR and a < len(rom)
    ))

    addr_to_size = {}
    for i, a in enumerate(valid_addrs):
        if i + 1 < len(valid_addrs):
            addr_to_size[a] = valid_addrs[i + 1] - a
        else:
            addr_to_size[a] = None

    # Parse each program
    programs = []
    undefined_count = 0
    for i, addr in enumerate(addrs):
        if addr == UNDEFINED_ADDR:
            undefined_count += 1
            continue
        if addr >= len(rom):
            continue

        next_addr = addr + addr_to_size[addr] if addr_to_size.get(addr) else None
        prog = parse_program(rom, addr, next_addr)
        if prog:
            programs.append((i, prog))

    print(f"Total entries: {NUM_PROGRAMS}")
    print(f"Undefined (0x{UNDEFINED_ADDR:04X}): {undefined_count}")
    print(f"Valid programs: {len(programs)}\n")

    # Parse drum sounds
    drums = parse_drum_sounds(rom)
    kit = parse_drum_kit(rom)
    print(f"Drum sounds: {len(drums)} (using algorithm 0x{DRUM_ALGORITHM:04X})")
    print(f"Drum kit: \"{kit['name']}\" with {len(kit['mappings'])} mappings\n")

    # Check for --export flag
    if '--export' in sys.argv:
        output_path = os.path.join(os.path.dirname(__file__), 'xe9l_programs.py')
        export_programs_python(programs, rom, output_path, drums)
        return

    print_program_summary(programs)
    print_aram_analysis(programs, rom)
    print_drum_summary(drums, kit)

    if '--detailed' in sys.argv:
        print_detailed_programs(programs)


if __name__ == '__main__':
    main()
