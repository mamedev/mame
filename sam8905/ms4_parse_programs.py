#!/usr/bin/env python3
"""
MS4 Program Structure Parser
Parses program data and A-RAM algorithms from the MS4 ROM.

MS4 uses a simpler format than Keyfox10:
- Single 64KB code space (no separate DATA space)
- Program pointer table at 0x0040, 66 entries
- A-RAM data pointer directly in program header (offset 10-11, little-endian)
- D-RAM parameter entries starting at offset 12 (3 bytes each)
- Voice init data pointer at offset 15-16 (little-endian)

Program data layout (from firmware disassembly):
  Offset 0-7:   ASCII name (space-padded)
  Offset 8:     Null terminator (0x00)
  Offset 9:     Flags (bit7=complex_init, bits3:0=slot_count)
  Offset 10-11: A-RAM data pointer (LE: [10]=DPL, [11]=DPH)
  Offset 12-14: First D-RAM parameter entry (val_lo, val_mid, ctrl)
  Offset 15-16: Voice init data pointer (LE), 0x0000 = none
  Offset 17+:   Additional D-RAM parameter data
"""

import sys
import os

ROM_PATH = "/home/jeff/bastel/roms/hohner/ms4/ms4_05_r1_0.bin"

# Program pointer table in code space
PROGRAM_PTR_TABLE = 0x0040
NUM_PROGRAMS = 66

# Import A-RAM decoder from same directory
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sam8905_aram_decoder import decode_algorithm, analyze_dram_usage


def read_rom():
    with open(ROM_PATH, 'rb') as f:
        return f.read()


def get_word(rom, addr):
    """Read big-endian 16-bit word from code space"""
    return (rom[addr] << 8) | rom[addr + 1]


def get_word_le(rom, addr):
    """Read little-endian 16-bit word (DPL, DPH order)"""
    return rom[addr] | (rom[addr + 1] << 8)


def decode_dram_ctrl(ctrl):
    """
    Decode D-RAM control byte (byte 2 of 3-byte entry).

    Format:
      Bit 7:     Conditional offset flag
      Bits 6-3:  D-RAM address nibble (slot offset within 16-word section)
      Bits 2-0:  Mix/routing bits
    """
    return {
        'addr_nibble': (ctrl >> 3) & 0x0F,
        'mix_bits': ctrl & 0x07,
        'cond_flag': bool(ctrl & 0x80),
        'raw': ctrl
    }


def parse_dram_entry(val_lo, val_mid, ctrl_byte):
    """Parse a 3-byte D-RAM parameter entry"""
    ctrl = decode_dram_ctrl(ctrl_byte)
    dram_word = (val_mid << 8) | val_lo
    return {
        'val_lo': val_lo,
        'val_mid': val_mid,
        'ctrl': ctrl,
        'dram_word': dram_word,
        'raw': f"{val_lo:02X} {val_mid:02X} {ctrl_byte:02X}"
    }


def parse_aram_data(rom, ptr):
    """
    Parse 32-word A-RAM algorithm data from ROM.

    A-RAM data is stored as 32 x 2 bytes = 64 bytes.
    Each word is stored [low_byte, high_byte], forming a 15-bit instruction.

    Returns:
        List of 32 15-bit instruction words, or None if pointer invalid
    """
    if ptr == 0 or ptr == 0xFFFF or ptr + 64 > len(rom):
        return None

    words = []
    for i in range(32):
        lo = rom[ptr + i * 2]
        hi = rom[ptr + i * 2 + 1]
        words.append((hi << 8) | lo)
    return words


def parse_program(rom, addr, next_addr=None):
    """
    Parse an MS4 program structure starting at addr.

    Data format (from firmware dram_param_processor at CODE:AA9A):
      Offset 0-7:   ASCII name (space-padded)
      Offset 8:     Null terminator
      Offset 9:     Flags (bit7=complex, bits3:0=slot_count)
      Offset 10-11: A-RAM data pointer (LE)
      Offset 12-14: D-RAM entry (val_lo, val_mid, ctrl)
      Offset 15-16: Voice data ptr for slot 0 (LE, 0=none)
      Offset 17+:   If voice_data_ptr != 0: additional 2-byte slot ptrs until 0x0000
                    Then: D-RAM command stream (variable-length, terminates on bit7 set)
    """
    prog = {}

    prog['addr'] = addr
    prog['name'] = rom[addr:addr + 8].decode('ascii', errors='replace').rstrip('\x00 ')
    prog['null_term'] = rom[addr + 8]
    prog['flags'] = rom[addr + 9]

    # A-RAM data pointer (little-endian at offset 10-11)
    prog['aram_ptr'] = get_word_le(rom, addr + 10)

    # First D-RAM parameter entry at offset 12-14
    prog['dram_entry0'] = parse_dram_entry(
        rom[addr + 12], rom[addr + 13], rom[addr + 14]
    )

    # Voice init data pointer at offset 15-16 (little-endian)
    prog['voice_data_ptr'] = get_word_le(rom, addr + 15)

    # Decode flags
    prog['complex_init'] = bool(prog['flags'] & 0x80)
    prog['slot_count'] = prog['flags'] & 0x0F

    # Calculate size from next program address
    if next_addr and next_addr > addr:
        prog['size'] = next_addr - addr
    else:
        prog['size'] = None

    # Parse A-RAM data
    prog['aram_data'] = parse_aram_data(rom, prog['aram_ptr'])

    # Read voice init data (7 bytes from each pointer)
    # First pointer at offset 15-16, additional slot pointers at offset 17+
    prog['voice_slots'] = []
    vptr = prog['voice_data_ptr']
    if vptr != 0 and vptr != 0xFFFF and vptr + 7 <= len(rom):
        prog['voice_slots'].append({
            'ptr': vptr,
            'data': list(rom[vptr:vptr + 7])
        })

    # Parse additional slot pointers (2 bytes each, until 0x0000)
    dram_stream_offset = 17  # default if no voice ptr
    if prog['voice_data_ptr'] != 0:
        pos = addr + 17
        max_pos = addr + (prog['size'] if prog['size'] else 256)
        while pos + 1 < max_pos:
            slot_ptr = get_word_le(rom, pos)
            pos += 2
            if slot_ptr == 0:
                break  # terminator found
            if slot_ptr + 7 <= len(rom):
                prog['voice_slots'].append({
                    'ptr': slot_ptr,
                    'data': list(rom[slot_ptr:slot_ptr + 7])
                })
        dram_stream_offset = pos - addr

    # D-RAM command stream (variable-length encoding)
    # Each byte's bits 5:3 select a handler; bit 7 set = terminator
    prog['dram_stream_offset'] = dram_stream_offset
    if prog['size'] and prog['size'] > dram_stream_offset:
        stream_start = addr + dram_stream_offset
        stream_end = addr + prog['size']
        prog['dram_stream'] = list(rom[stream_start:stream_end])
    else:
        prog['dram_stream'] = []

    # Raw header for debugging (first 22 bytes)
    header_len = min(22, prog['size']) if prog['size'] else 22
    prog['raw_header'] = rom[addr:addr + header_len].hex(' ')

    return prog


def print_program_summary(programs):
    """Print summary table of all programs"""
    print("=" * 110)
    print("MS4 PROGRAM STRUCTURE ANALYSIS (CODE:0040 table, 66 entries)")
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


def print_detailed_programs(programs):
    """Print detailed per-program data"""
    print("\n" + "=" * 100)
    print("DETAILED PROGRAM DATA")
    print("=" * 100)

    for idx, prog in programs:
        print(f"\n{'─' * 80}")
        print(f"Program {idx}: \"{prog['name']}\" at CODE:0x{prog['addr']:04X} "
              f"(size={prog['size']})")
        print(f"  Raw header: {prog['raw_header']}")
        print(f"  Flags: 0x{prog['flags']:02X} "
              f"(complex={'yes' if prog['complex_init'] else 'no'}, "
              f"slots={prog['slot_count']})")
        print(f"  A-RAM ptr: 0x{prog['aram_ptr']:04X}")

        # D-RAM entry 0
        de0 = prog['dram_entry0']
        ctrl = de0['ctrl']
        print(f"  D-RAM entry0: [{de0['raw']}] "
              f"word=0x{de0['dram_word']:04X}, "
              f"addr_nibble={ctrl['addr_nibble']}, "
              f"mix={ctrl['mix_bits']}, "
              f"cond={'Y' if ctrl['cond_flag'] else 'N'}")

        # Voice slots
        if prog['voice_slots']:
            print(f"  Voice slots: {len(prog['voice_slots'])}")
            for i, slot in enumerate(prog['voice_slots']):
                vdata = ' '.join(f"{b:02X}" for b in slot['data'])
                print(f"    Slot {i}: ptr=0x{slot['ptr']:04X} data=[{vdata}]")
        else:
            print(f"  Voice slots: NONE (ptr=0x0000)")

        # D-RAM command stream
        stream = prog['dram_stream']
        if stream:
            stream_hex = ' '.join(f"{b:02X}" for b in stream[:52])
            if len(stream) > 52:
                stream_hex += f" ... (+{len(stream) - 52} bytes)"
            print(f"  D-RAM stream (offset {prog['dram_stream_offset']}, "
                  f"{len(stream)} bytes):")
            print(f"    {stream_hex}")


def print_aram_analysis(programs):
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
        # Get first program using this algorithm
        first_idx, _ = aram_ptrs[ptr][0]
        first_prog = None
        for prog_idx, prog in programs:
            if prog_idx == first_idx:
                first_prog = prog
                break

        if first_prog and first_prog['aram_data']:
            users = ', '.join(name for _, name in aram_ptrs[ptr][:5])
            if len(aram_ptrs[ptr]) > 5:
                users += f" (+{len(aram_ptrs[ptr]) - 5} more)"
            print(f"\n{'─' * 60}")
            print(f"A-RAM at 0x{ptr:04X} (used by {len(aram_ptrs[ptr])} programs: {users})")
            print(decode_algorithm(first_prog['aram_data']))
            print("\nD-RAM usage:")
            usage = analyze_dram_usage(first_prog['aram_data'])
            for addr, counts in sorted(usage.items()):
                print(f"  D-RAM[{addr:2d}]: read={counts['read']}, write={counts['write']}")


def print_flags_analysis(programs):
    """Analyze flag byte patterns"""
    print("\n" + "=" * 100)
    print("FLAGS ANALYSIS (offset 9: bit7=complex_init, bits3:0=slot_count)")
    print("-" * 80)

    flag_counts = {}
    for _, prog in programs:
        flag = prog['flags']
        if flag not in flag_counts:
            flag_counts[flag] = []
        flag_counts[flag].append(prog['name'])

    for flag, names in sorted(flag_counts.items()):
        ci = 'complex' if flag & 0x80 else 'simple'
        slots = flag & 0x0F
        print(f"  0x{flag:02X} ({ci}, {slots} slots): {len(names)} programs")
        if len(names) <= 8:
            print(f"    {', '.join(names)}")


def print_voice_data_analysis(programs):
    """Analyze voice init data pointers and content"""
    print("\n" + "=" * 100)
    print("VOICE INIT DATA ANALYSIS (offset 15-16, points to 7 bytes per slot)")
    print("-" * 80)

    # Collect all unique voice data pointers across all slots
    voice_ptrs = {}
    for _, prog in programs:
        for slot in prog['voice_slots']:
            ptr = slot['ptr']
            if ptr not in voice_ptrs:
                voice_ptrs[ptr] = {'data': slot['data'], 'programs': []}
            voice_ptrs[ptr]['programs'].append(prog['name'])

    print(f"\n{len(voice_ptrs)} unique voice data pointers:\n")
    print(f"{'Pointer':<9} {'Data (7 bytes)':<24} {'Programs'}")
    print("-" * 80)

    for ptr in sorted(voice_ptrs.keys()):
        info = voice_ptrs[ptr]
        data_str = ' '.join(f"{b:02X}" for b in info['data'])
        names_str = ', '.join(info['programs'][:4])
        if len(info['programs']) > 4:
            names_str += f" (+{len(info['programs']) - 4} more)"
        print(f"  0x{ptr:04X}  {data_str:<24} {names_str}")

    # Multi-slot programs
    multi_slot = [(idx, p) for idx, p in programs if len(p['voice_slots']) > 1]
    if multi_slot:
        print(f"\n{len(multi_slot)} multi-slot programs:")
        for idx, prog in multi_slot:
            ptrs = [f"0x{s['ptr']:04X}" for s in prog['voice_slots']]
            print(f"  {idx:2d} {prog['name']:<10} {len(prog['voice_slots'])} slots: {', '.join(ptrs)}")


def print_dram_entry0_analysis(programs):
    """Analyze first D-RAM entry (offset 12-14) patterns"""
    print("\n" + "=" * 100)
    print("D-RAM ENTRY 0 ANALYSIS (offset 12-14: val_lo, val_mid, ctrl)")
    print("-" * 80)

    # Group by ctrl byte value
    ctrl_groups = {}
    for _, prog in programs:
        ctrl_raw = prog['dram_entry0']['ctrl']['raw']
        if ctrl_raw not in ctrl_groups:
            ctrl_groups[ctrl_raw] = []
        ctrl_groups[ctrl_raw].append(prog['name'])

    print("\nControl byte distribution:")
    for ctrl_val, names in sorted(ctrl_groups.items()):
        decoded = decode_dram_ctrl(ctrl_val)
        print(f"  ctrl=0x{ctrl_val:02X} (addr={decoded['addr_nibble']}, "
              f"mix={decoded['mix_bits']}, "
              f"cond={'Y' if decoded['cond_flag'] else 'N'}): "
              f"{len(names)} programs")

    # Group by full entry
    entry_groups = {}
    for _, prog in programs:
        key = prog['dram_entry0']['raw']
        if key not in entry_groups:
            entry_groups[key] = []
        entry_groups[key].append(prog['name'])

    print(f"\n{len(entry_groups)} unique D-RAM entry0 values:")
    for entry_raw, names in sorted(entry_groups.items()):
        names_str = ', '.join(names[:4])
        if len(names) > 4:
            names_str += f" (+{len(names) - 4} more)"
        print(f"  [{entry_raw}]: {names_str}")


def print_dram_stream_analysis(programs):
    """Analyze D-RAM command stream patterns"""
    print("\n" + "=" * 100)
    print("D-RAM COMMAND STREAM ANALYSIS")
    print("  Format: variable-length commands, bits 5:3 select handler, bit 7 = terminator")
    print("  Handlers: 0x00=ad8f, 0x08=adbd, 0x10=b030, 0x18=b222,")
    print("            0x20=b278, 0x28=b2d2, 0x30=b2cf, 0x38=NOP(skip)")
    print("-" * 80)

    # Group by stream length
    len_groups = {}
    for _, prog in programs:
        slen = len(prog['dram_stream'])
        if slen not in len_groups:
            len_groups[slen] = []
        len_groups[slen].append(prog['name'])

    print("\nStream length distribution:")
    for slen, names in sorted(len_groups.items()):
        names_str = ', '.join(names[:5])
        if len(names) > 5:
            names_str += f" (+{len(names) - 5} more)"
        print(f"  {slen:3d} bytes: {len(names)} programs  ({names_str})")

    print(f"\n{'Idx':<4} {'Name':<10} {'Offset':<7} {'Len':<5} {'First 20 bytes'}")
    print("-" * 80)
    for idx, prog in programs:
        stream = prog['dram_stream']
        if not stream:
            continue
        first_bytes = ' '.join(f"{b:02X}" for b in stream[:20])
        print(f"{idx:<4} {prog['name']:<10} {prog['dram_stream_offset']:<7} "
              f"{len(stream):<5} {first_bytes}")


def main():
    rom = read_rom()
    print(f"ROM size: {len(rom)} bytes (0x{len(rom):X})")

    # Read all program pointers
    addrs = []
    for i in range(NUM_PROGRAMS):
        ptr_addr = PROGRAM_PTR_TABLE + (i * 2)
        prog_addr = get_word(rom, ptr_addr)
        addrs.append(prog_addr)

    # Build sorted address list for size calculation
    # (programs are NOT stored in table order in ROM)
    valid_addrs = sorted(set(
        a for a in addrs if a != 0xFFFF and a < len(rom)
    ))

    # Map each address to its size (distance to next program in memory)
    addr_to_size = {}
    for i, a in enumerate(valid_addrs):
        if i + 1 < len(valid_addrs):
            addr_to_size[a] = valid_addrs[i + 1] - a
        else:
            addr_to_size[a] = None

    # Parse each program
    programs = []
    for i, addr in enumerate(addrs):
        # Skip clearly invalid entries
        if addr == 0xFFFF or addr >= len(rom):
            continue

        next_addr_in_mem = addr + addr_to_size[addr] if addr_to_size.get(addr) else None
        prog = parse_program(rom, addr, next_addr_in_mem)
        programs.append((i, prog))

    print(f"Parsed {len(programs)} programs\n")

    print_program_summary(programs)
    print_flags_analysis(programs)
    print_dram_entry0_analysis(programs)
    print_dram_stream_analysis(programs)
    print_voice_data_analysis(programs)
    print_aram_analysis(programs)
    print_detailed_programs(programs)


if __name__ == '__main__':
    main()
