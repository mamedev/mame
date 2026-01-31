#!/usr/bin/env python3
"""
MAX1 Program Structure Parser

Parses program data from the MAX1 (Hohner) ROM.
Uses shared parsing code from parse_programs.py.

MAX1 specifics:
- ROM: max1/ic6_00.bin (32KB)
- Program pointer table at 0x72D9, 100 entries (big-endian)
- 27 algorithms starting at 0x2BC3 (each 64 bytes / 32 words)
- 3 drumsets at 0x1F4E, 0x1FC3, 0x204E
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

ROM_PATH = "/home/jeff/bastel/roms/hohner/max1/ic6_00.bin"

# Program pointer table
PROGRAM_PTR_TABLE = 0x72D9
NUM_PROGRAMS = 100
UNDEFINED_ADDR = 0x0000  # Empty/undefined program marker (TBD)

# Algorithm table
ALGORITHM_BASE = 0x2BC3
NUM_ALGORITHMS = 27
ALGORITHM_SIZE = 64  # bytes (32 words)

# Generate all algorithm addresses
# Format: (rom_addr, aram_slot, description)
# A-RAM slots assigned sequentially for now - actual mapping TBD
ALL_ALGORITHMS = []
for i in range(NUM_ALGORITHMS):
    addr = ALGORITHM_BASE + (i * ALGORITHM_SIZE)
    # A-RAM slot mapping unknown - using index for now
    ALL_ALGORITHMS.append((addr, i, f"algorithm {i}"))

# Drumset addresses
DRUMSET_ADDRS = [0x1F4E, 0x1FC3, 0x204E]


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


def parse_drumsets(rom):
    """
    Parse drumset entries from ROM.

    Each drumset has:
    - 8-byte ASCII name ("DRUMSET ")
    - 1-byte null terminator
    - 1-byte flags (0x82)
    - 2-byte algorithm pointer (little-endian)
    - Header data
    - Mapping table

    Returns list of drumset dicts.
    """
    drumsets = []

    for addr in DRUMSET_ADDRS:
        if addr + 12 >= len(rom):
            continue

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

        drumset = {
            'addr': addr,
            'name': name,
            'flags': flags,
            'aram_ptr': aram_ptr,
            'mappings': []
        }

        # Mapping table starts around offset 38
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

            # Stop if we hit the next drumset or invalid data
            if entry_addr >= DRUMSET_ADDRS[1] if addr == DRUMSET_ADDRS[0] else \
               entry_addr >= DRUMSET_ADDRS[2] if addr == DRUMSET_ADDRS[1] else \
               entry_addr >= addr + 0x200:
                break

            mapping = {
                'index': i,
                'ptr': ptr,
                'flags': entry_flags,
            }
            drumset['mappings'].append(mapping)

        drumsets.append(drumset)

    return drumsets


def print_program_summary(programs):
    """Print summary table of all programs"""
    print("=" * 110)
    print(f"MAX1 PROGRAM STRUCTURE ANALYSIS (0x{PROGRAM_PTR_TABLE:04X} table, {NUM_PROGRAMS} entries)")
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
    print(f"A-RAM ALGORITHM ANALYSIS ({NUM_ALGORITHMS} algorithms at 0x{ALGORITHM_BASE:04X})")
    print("=" * 100)

    # Find unique A-RAM pointers from programs
    aram_ptrs = {}
    for idx, prog in programs:
        ptr = prog['aram_ptr']
        if ptr not in aram_ptrs:
            aram_ptrs[ptr] = []
        aram_ptrs[ptr].append((idx, prog['name']))

    # Print algorithm table
    print(f"\n{'ROM Addr':<10} {'Index':<7} {'Programs':<8} {'Example'}")
    print("-" * 80)

    for rom_addr, algo_idx, desc in ALL_ALGORITHMS:
        prog_list = aram_ptrs.get(rom_addr, [])
        prog_count = len(prog_list)
        prog_str = str(prog_count) if prog_count > 0 else "-"
        example = prog_list[0][1] if prog_list else ""
        print(f"0x{rom_addr:04X}    {algo_idx:<7} {prog_str:<8} {example}")

    # Show algorithms not in table but referenced by programs
    known_addrs = set(addr for addr, _, _ in ALL_ALGORITHMS)
    unknown_refs = [(ptr, progs) for ptr, progs in aram_ptrs.items() if ptr not in known_addrs]
    if unknown_refs:
        print("\nAlgorithms referenced but not in table:")
        for ptr, progs in sorted(unknown_refs):
            print(f"  0x{ptr:04X}: {len(progs)} programs - {progs[0][1]}")


def print_drumset_summary(drumsets):
    """Print summary of drumsets"""
    print("\n" + "=" * 100)
    print("DRUMSET ANALYSIS")
    print("=" * 100)

    for ds in drumsets:
        print(f"\nDrumset: \"{ds['name']}\" at 0x{ds['addr']:04X}")
        print(f"  Flags: 0x{ds['flags']:02X}, Algorithm: 0x{ds['aram_ptr']:04X}")
        print(f"  Mappings: {len(ds['mappings'])} entries")

        # Show first few mappings
        print(f"\n  {'Idx':<4} {'Ptr':<7} {'Flags':<6}")
        print(f"  " + "-" * 20)
        for m in ds['mappings'][:20]:
            ptr_str = f"0x{m['ptr']:04X}" if m['ptr'] else "  ---"
            print(f"  {m['index']:<4} {ptr_str} 0x{m['flags']:02X}")
        if len(ds['mappings']) > 20:
            print(f"  ... ({len(ds['mappings']) - 20} more)")


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


def export_programs_python(programs, rom, output_path, drumsets=None):
    """Export programs grouped by algorithm to a Python file."""
    # Collect all A-RAM pointers (from programs + table)
    all_aram_ptrs = set(addr for addr, _, _ in ALL_ALGORITHMS)
    for _, prog in programs:
        all_aram_ptrs.add(prog['aram_ptr'])

    if drumsets is None:
        drumsets = []

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
        f.write('MAX1 Program Data Export\n')
        f.write('Generated by parse_programs_max1.py\n')
        f.write('\n')
        f.write('Contains A-RAM algorithm data and D-RAM configurations\n')
        f.write('for MAX1 programs, grouped by algorithm.\n')
        f.write('\n')
        f.write(f'{NUM_ALGORITHMS} algorithms starting at 0x{ALGORITHM_BASE:04X}\n')
        f.write('"""\n\n')

        # Export all algorithms
        f.write('# A-RAM algorithm data (32 x 15-bit instruction words)\n')
        f.write('ALGORITHMS = {\n')
        for rom_addr, algo_idx, desc in ALL_ALGORITHMS:
            ptr_key = f"{rom_addr:04X}"
            aram = parse_aram_data(rom, rom_addr)
            if aram:
                f.write(f"    '{ptr_key}': [  # {desc}\n")
                for i in range(0, 32, 8):
                    chunk = aram[i:i+8]
                    hex_vals = ', '.join(f'0x{v:04X}' for v in chunk)
                    f.write(f"        {hex_vals},\n")
                f.write("    ],\n")
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

                f.write('    },\n')

            f.write(']\n\n')

        f.write('# List of all algorithm keys\n')
        f.write(f"ALGORITHM_KEYS = {sorted(by_algorithm.keys())}\n\n")

        f.write('# Mapping from algorithm key to program list\n')
        f.write('PROGRAMS_BY_ALGORITHM = {\n')
        for ptr_key in sorted(by_algorithm.keys()):
            f.write(f"    '{ptr_key}': PROGRAMS_{ptr_key},\n")
        f.write('}\n')

    total_exported = sum(len(v['programs']) for v in by_algorithm.values())
    print(f"Exported {total_exported} programs to {output_path}")


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

    # Parse drumsets
    drumsets = parse_drumsets(rom)
    print(f"Drumsets: {len(drumsets)}")
    for ds in drumsets:
        print(f"  \"{ds['name']}\" at 0x{ds['addr']:04X} ({len(ds['mappings'])} mappings)")
    print()

    # Check for --export flag
    if '--export' in sys.argv:
        output_path = os.path.join(os.path.dirname(__file__), 'max1_programs.py')
        export_programs_python(programs, rom, output_path, drumsets)
        return

    print_program_summary(programs)
    print_aram_analysis(programs, rom)
    print_drumset_summary(drumsets)

    if '--detailed' in sys.argv:
        print_detailed_programs(programs)


if __name__ == '__main__':
    main()
