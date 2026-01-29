#!/usr/bin/env python3
"""
XE9L Program Structure Parser

Parses program data from the XE9L (Hohner) ROM.
Uses shared parsing code from parse_programs.py.

XE9L specifics:
- ROM: xe9l_v141.bin (64KB)
- Program pointer table at 0x33AD, 171 entries (big-endian)
- Undefined program marker: 0x001E
- 4 algorithms at 0x002A, 0x006A, 0x00AA, 0x00EA
- Algorithms loaded via sam_programs_1 initialization
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


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


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
    print("A-RAM ALGORITHM ANALYSIS")
    print("=" * 100)

    # Find unique A-RAM pointers
    aram_ptrs = {}
    for idx, prog in programs:
        ptr = prog['aram_ptr']
        if ptr not in aram_ptrs:
            aram_ptrs[ptr] = []
        aram_ptrs[ptr].append((idx, prog['name']))

    print(f"\n{len(aram_ptrs)} unique A-RAM algorithms found:\n")
    print(f"{'A-RAM Ptr':<10} {'Count':<6} {'Programs'}")
    print("-" * 80)

    for ptr in sorted(aram_ptrs.keys()):
        names = [f"{idx}:{name}" for idx, name in aram_ptrs[ptr]]
        names_str = ', '.join(names[:6])
        if len(names) > 6:
            names_str += f" (+{len(names) - 6} more)"
        print(f"0x{ptr:04X}    {len(names):<6} {names_str}")

    # Decode all unique algorithms
    print("\n" + "-" * 80)
    print("DECODED A-RAM ALGORITHMS")
    print("-" * 80)

    for ptr in sorted(aram_ptrs.keys()):
        aram_data = parse_aram_data(rom, ptr)
        if aram_data:
            users = ', '.join(name for _, name in aram_ptrs[ptr][:5])
            if len(aram_ptrs[ptr]) > 5:
                users += f" (+{len(aram_ptrs[ptr]) - 5} more)"
            print(f"\n{'─' * 60}")
            print(f"A-RAM at 0x{ptr:04X} (used by {len(aram_ptrs[ptr])} programs: {users})")
            print(decode_algorithm(aram_data))
            print("\nD-RAM usage:")
            usage = analyze_dram_usage(aram_data)
            for addr, counts in sorted(usage.items()):
                print(f"  D-RAM[{addr:2d}]: read={counts['read']}, write={counts['write']}")


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


def export_programs_python(programs, rom, output_path):
    """Export programs grouped by algorithm to a Python file."""
    # Collect unique A-RAM pointers
    aram_ptrs = set()
    for _, prog in programs:
        aram_ptrs.add(prog['aram_ptr'])

    # Analyze WWF instructions for each algorithm
    waveform_words = analyze_algorithms_wwf(rom, aram_ptrs)

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
        f.write(f'for XE9L programs, grouped by algorithm.\n')
        f.write('"""\n\n')

        # Export algorithms
        f.write('# A-RAM algorithm data (32 x 15-bit instruction words)\n')
        f.write('ALGORITHMS = {\n')
        for ptr_key in sorted(by_algorithm.keys()):
            aram = by_algorithm[ptr_key]['aram_data']
            if aram:
                f.write(f"    '{ptr_key}': [\n")
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
        f.write('}\n')

    total_exported = sum(len(v['programs']) for v in by_algorithm.values())
    print(f"Exported {total_exported} programs to {output_path}")
    for ptr_key in sorted(by_algorithm.keys()):
        count = len(by_algorithm[ptr_key]['programs'])
        print(f"  Algorithm 0x{ptr_key}: {count} programs")


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

    # Check for --export flag
    if '--export' in sys.argv:
        output_path = os.path.join(os.path.dirname(__file__), 'xe9l_programs.py')
        export_programs_python(programs, rom, output_path)
        return

    print_program_summary(programs)
    print_aram_analysis(programs, rom)

    if '--detailed' in sys.argv:
        print_detailed_programs(programs)


if __name__ == '__main__':
    main()
