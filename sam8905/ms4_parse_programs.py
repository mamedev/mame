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
NUM_PROGRAMS = 247

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


def scan_dram_stream(rom, start):
    """
    Forward-scan a D-RAM config stream to find its end.

    The stream encodes up to 16 D-RAM word values using variable-length commands.
    Each dispatch byte's bits 5:3 select a handler with known byte consumption.
    Stream terminates when:
      - 16 D-RAM words processed (counter exhausted)
      - Dispatch byte has bit 7 set (terminator value)
      - Handler 0x20 reached (output config, terminates loop)
      - Handler 0x30 reached (skip remaining, no bytes consumed)

    Handler byte consumption (from firmware at CODE:AA9A dispatch loop):
      bits 5:3 = 000 (0x00): 3 or 4 bytes (4 if byte[3] & 0xE0 == 0)
      bits 5:3 = 001 (0x08): 10 bytes (pitch/frequency)
      bits 5:3 = 010 (0x10): 9 bytes (amplitude/level)
      bits 5:3 = 011 (0x18): 4 bytes (D-RAM write)
      bits 5:3 = 100 (0x20): 4 bytes (output config, TERMINATES)
      bits 5:3 = 101 (0x28): 1 byte (write constant)
      bits 5:3 = 110 (0x30): 0 bytes (skip remaining, TERMINATES)
      bits 5:3 = 111 (0x38): 1 byte (write constant, default case)

    Returns: address past the last consumed byte.
    """
    pos = start
    words_remaining = 16  # INTMEM_39 initialized to 0x10

    while words_remaining > 0 and pos < len(rom):
        dispatch = rom[pos]

        # Bit 7 set = terminator (writes value, exits)
        if dispatch & 0x80:
            pos += 1  # the terminator byte itself is consumed
            break

        handler = dispatch & 0x38

        if handler == 0x00:
            # 3 bytes + check byte[3]
            if pos + 3 < len(rom) and (rom[pos + 3] & 0xE0) == 0:
                pos += 4
            else:
                pos += 3  # byte[3] is next dispatch byte
        elif handler == 0x08:
            pos += 10
        elif handler == 0x10:
            pos += 9
        elif handler == 0x18:
            pos += 4
        elif handler == 0x20:
            pos += 4  # terminates
            break
        elif handler == 0x28:
            pos += 1
        elif handler == 0x30:
            # skip remaining - no bytes consumed, terminates
            pos += 1  # consume the 0x30 dispatch byte itself
            break
        else:  # 0x38 default
            pos += 1

        words_remaining -= 1

    return pos


HANDLER_NAMES = {
    0x00: 'short_wr',
    0x08: 'pitch',
    0x10: 'amplitude',
    0x18: 'dram_wr',
    0x20: 'output',
    0x28: 'const28',
    0x30: 'skip_rem',
    0x38: 'const38',
}


def decode_dram_stream(stream):
    """
    Decode a D-RAM config stream into per-word handler entries.

    Returns list of dicts: {word, handler, name, bytes, terminator}
    """
    entries = []
    pos = 0
    word = 0

    while word < 16 and pos < len(stream):
        dispatch = stream[pos]

        if dispatch & 0x80:
            entries.append({
                'word': word, 'handler': 'termin',
                'name': 'terminator',
                'bytes': stream[pos:pos + 1], 'offset': pos,
            })
            break

        handler = dispatch & 0x38

        if handler == 0x00:
            if pos + 3 < len(stream) and (stream[pos + 3] & 0xE0) == 0:
                n = 4
            else:
                n = 3
        elif handler == 0x08:
            n = 10
        elif handler == 0x10:
            n = 9
        elif handler == 0x18:
            n = 4
        elif handler == 0x20:
            n = 4
            entries.append({
                'word': word, 'handler': f'0x{handler:02X}',
                'name': HANDLER_NAMES[handler],
                'bytes': stream[pos:pos + n], 'offset': pos,
            })
            break
        elif handler == 0x28:
            n = 1
        elif handler == 0x30:
            entries.append({
                'word': word, 'handler': f'0x{handler:02X}',
                'name': HANDLER_NAMES[handler],
                'bytes': stream[pos:pos + 1], 'offset': pos,
            })
            break
        else:  # 0x38
            n = 1

        entries.append({
            'word': word, 'handler': f'0x{handler:02X}',
            'name': HANDLER_NAMES[handler],
            'bytes': stream[pos:pos + n], 'offset': pos,
        })
        pos += n
        word += 1

    return entries


def decode_dram_init(stream, alg_key='027A'):
    """
    Decode D-RAM stream into initial values for interpreter.

    Returns dict with:
        'dram': 16-word array of initial D-RAM values (19-bit)
        'pitch': {note_offset, fine_tune, vel_sens} for runtime pitch calc
        'amp': {level, amp, vel_sens, env_ctrl} for runtime amplitude calc
        'waveform_words': list of D-RAM indices that need waveform select

    Note: Some values require runtime calculation based on MIDI note/velocity.
    The returned dram array uses placeholder values that should be overridden.
    """
    dram = [0] * 16
    pitch_params = {'note_offset': 0, 'fine_tune': 0, 'vel_sens': 0}
    amp_params = {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0}

    # Algorithm-specific waveform words (need internal sine = 0x100 << 9)
    # Based on WWF instructions in each algorithm (RM with WWF bit active)
    # Algorithms with empty list use WPHI+WSP combo which sets WF=0x100 internally
    waveform_words = {
        '027A': [4, 10, 15],
        '02BA': [4, 15],
        '02FA': [11, 12, 15],
        '033A': [],
        '037A': [15],
        '03BA': [],
        '03FA': [],
        '043A': [15],
        '047A': [15],
        '04BA': [13, 15],
        '04FA': [],
        '053A': [15],
        '057A': [15],
        '05BA': [],
        '05FA': [],
        '063A': [],
        '067A': [13, 14],
    }
    wf_words = waveform_words.get(alg_key, [])

    entries = decode_dram_stream(stream)

    for entry in entries:
        word = entry['word']
        b = entry['bytes']
        name = entry['name']

        if name == 'pitch' and len(b) >= 10:
            # Pitch handler: note_offset, fine tune, vel_sens
            pitch_params['vel_sens'] = b[1]
            pitch_params['note_offset'] = b[2]
            pitch_params['fine_tune'] = b[5] | (b[6] << 8)
            # D[0] will be phase increment - placeholder, needs runtime calc
            dram[word] = 0  # Will be set based on MIDI note

        elif name == 'amplitude' and len(b) >= 9:
            # Amplitude handler: level, amp, envelope
            amp_params['level'] = b[1]
            amp_params['amp'] = b[2]
            amp_params['env_ctrl'] = b[3]
            amp_params['vel_sens'] = b[7] if len(b) > 7 else 0
            # Set amplitude in bus format: amp << 7 | mix
            # Default mix = 7,7 (full volume)
            if b[2] > 0:
                dram[word] = (b[2] << 7) | 0x3F  # amp in upper bits, mix=7,7

        elif name == 'dram_wr' and len(b) >= 4:
            # Direct D-RAM write - value in first byte, extended format
            # The value might need to be in bus format
            dram[word] = b[0]

        elif name in ('const28', 'const38'):
            # Constant value - dispatch byte has value in lower bits
            dram[word] = b[0] & 0x3F

        elif name == 'terminator':
            # Final value - byte with bit 7 set
            dram[word] = b[0] & 0x7F

        elif name == 'short_wr' and len(b) >= 3:
            # Short write: val_lo, val_mid, ctrl
            dram[word] = b[0] | (b[1] << 8)

    # Set waveform words to internal sine (WF=0x100)
    internal_sine = 0x100 << 9  # WWF reads bus[17:9]
    for w in wf_words:
        if dram[w] == 0:  # Only if not already set
            dram[w] = internal_sine

    return {
        'dram': dram,
        'pitch': pitch_params,
        'amp': amp_params,
        'waveform_words': wf_words,
    }


def format_handler_detail(entry):
    """
    Format decoded details for a handler entry's bytes.

    Returns a string with the decoded meaning of each byte, or None
    if no detailed decode is available for this handler type.
    """
    b = entry['bytes']
    handler = entry.get('handler', '')

    if handler == '0x08' and len(b) >= 10:
        # Pitch handler (dram_config_handler_08, CODE:ADBD)
        # byte[0]: dispatch (bits 2:0 = sub-flags)
        # byte[1]: velocity sensitivity (0=none)
        # byte[2]: note offset (unsigned, added to MIDI note, octave-wrapped)
        # byte[3]: ctrl (bit3=MIX override, bit5=portamento, bit7=modulation)
        # byte[4]: pitch bend range (stored & 0x7F)
        # byte[5]: fine tune low
        # byte[6]: fine tune high
        # byte[7]: (skipped by firmware, unused)
        # byte[8]: portamento/mod rate
        # byte[9]: portamento/mod depth
        flags = []
        if b[3] & 0x80:
            flags.append('MOD')
        if b[3] & 0x20:
            flags.append('PORT')
        if b[3] & 0x08:
            flags.append('MIX_OVR')
        flags_str = '|'.join(flags) if flags else 'none'
        fine = (b[6] << 8) | b[5]
        vel_str = f"vel_sens=0x{b[1]:02X}" if b[1] else "vel_sens=off"
        fine_str = f"fine=0x{fine:04X}" if fine else "fine=0"
        bend_str = f"bend={b[4] & 0x7F}" if b[4] & 0x7F else ""
        mod_str = ""
        if b[8] or b[9]:
            mod_str = f" rate=0x{b[8]:02X} depth=0x{b[9]:02X}"
        parts = [f"note_ofs=0x{b[2]:02X}", vel_str, f"ctrl=[{flags_str}]"]
        if fine:
            parts.append(fine_str)
        if bend_str:
            parts.append(bend_str)
        if mod_str:
            parts.append(mod_str.strip())
        return ' '.join(parts)

    elif handler == '0x10' and len(b) >= 9:
        # Amplitude handler (dram_config_handler_10, CODE:B030)
        # byte[0]: dispatch/flags (bit0=output_sel, bit1=copy_level, bit2=phase_inv)
        # byte[1]: base level
        # byte[2]: amplitude (0=skip/NOP path)
        # byte[3]: envelope ctrl (bit7=env_on, bit6=no_vel_scale, bit4=special, bit0=mod_enable)
        # byte[4]: attack/decay rate (bit3=portamento flag)
        # byte[5]: (unused in read path)
        # byte[6]: sustain level
        # byte[7]: velocity sensitivity
        # byte[8]: modulation amount
        if b[2] == 0:
            return "amplitude=0 (skip/NOP)"
        flags = []
        if b[3] & 0x80:
            flags.append('ENV')
        if b[3] & 0x40:
            flags.append('NO_VEL_SCALE')
        if b[3] & 0x10:
            flags.append('SPECIAL')
        if b[3] & 0x01:
            flags.append('MOD')
        flags_str = '|'.join(flags) if flags else 'none'
        parts = [f"amp=0x{b[2]:02X}", f"level=0x{b[1]:02X}",
                 f"env=[{flags_str}]", f"atk=0x{b[4]:02X}",
                 f"sus=0x{b[6]:02X}"]
        if b[7]:
            parts.append(f"vel=0x{b[7]:02X}")
        if b[8]:
            parts.append(f"mod=0x{b[8]:02X}")
        return ' '.join(parts)

    elif handler == '0x18' and len(b) >= 4:
        # D-RAM write handler (dram_config_handler_18, CODE:B222)
        # byte[0]: value written to D-RAM slot
        # byte[1-3]: modulation/velocity data
        val = b[0]
        return f"val=0x{val:02X} mod=[{b[1]:02X} {b[2]:02X} {b[3]:02X}]"

    elif handler == '0x20' and len(b) >= 4:
        # Output config handler (dram_config_handler_20, CODE:B278)
        # byte[0]: dispatch
        # byte[1]: XRAM 0xF9 value
        # byte[2]: XRAM 0xFA value
        # byte[3]: bits 2:0 → XRAM 0xFB low bits
        return f"xram_F9=0x{b[1]:02X} xram_FA=0x{b[2]:02X} xram_FB_lo={b[3] & 7}"

    elif entry['name'] == 'terminator':
        return f"val=0x{b[0]:02X} → D-RAM word value"

    return None


def format_voice_init_detail(data):
    """
    Decode 7-byte voice init data (copied to XRAM voice slot by voice_init_copy_and_envelope).

    Layout (from firmware at CODE:AB73):
      [0:1]: Envelope segment table pointer (LE) - points to 3-byte entries in code space
      [2]:   Control flags:
             bit 7 = envelope enable (triggers rate/level processing)
             bit 6 = level sign flag (ORs 0x80 into velocity level)
             (byte & 0x48) != 0x40 → clears slot[8:9]
      [3]:   Reserved (always 0x00 in MS4 ROM)
      [4]:   Envelope sustain/attack parameter
      [5]:   Envelope depth/modulation parameter
      [6]:   Envelope rate (processed to rate<<2 when envelope enabled, 0=default)
    """
    if len(data) < 7:
        return None
    env_ptr = data[0] | (data[1] << 8)
    flags = []
    if data[2] & 0x80:
        flags.append('ENV')
    if data[2] & 0x40:
        flags.append('LSIGN')
    flags_str = '|'.join(flags) if flags else 'none'
    parts = [f"env_tbl=0x{env_ptr:04X}", f"ctrl=[{flags_str}]"]
    if data[4] or data[5] or data[6]:
        parts.append(f"atk=0x{data[4]:02X}")
        parts.append(f"depth=0x{data[5]:02X}")
        parts.append(f"rate=0x{data[6]:02X}")
    return ' '.join(parts)


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

    # D-RAM command stream: variable-length encoding for 16 D-RAM words.
    # Forward-scan using handler byte consumption from firmware analysis.
    prog['dram_stream_offset'] = dram_stream_offset
    prog['dram_stream'] = []
    if (prog['size'] is None or prog['size'] > dram_stream_offset) and \
       addr + dram_stream_offset < len(rom):
        stream_start = addr + dram_stream_offset
        stream_end = scan_dram_stream(rom, stream_start)
        prog['dram_stream'] = list(rom[stream_start:stream_end])

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
                detail = format_voice_init_detail(slot['data'])
                if detail:
                    print(f"             {detail}")
        else:
            print(f"  Voice slots: NONE (ptr=0x0000)")

        # D-RAM command stream with per-handler breakdown
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
    print(f"{'Pointer':<9} {'Data (7 bytes)':<24} {'Decoded':<40} {'Programs'}")
    print("-" * 120)

    for ptr in sorted(voice_ptrs.keys()):
        info = voice_ptrs[ptr]
        data_str = ' '.join(f"{b:02X}" for b in info['data'])
        detail = format_voice_init_detail(info['data']) or ''
        names_str = ', '.join(info['programs'][:3])
        if len(info['programs']) > 3:
            names_str += f" (+{len(info['programs']) - 3} more)"
        print(f"  0x{ptr:04X}  {data_str:<24} {detail:<40} {names_str}")

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
    print("  Handlers: 0x00=short_write(3-4B), 0x08=pitch(10B), 0x10=amplitude(9B),")
    print("            0x18=dram_write(4B), 0x20=output_cfg(4B,TERM), 0x28=const(1B),")
    print("            0x30=skip_remaining(TERM), 0x38=const(1B)")
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

    print(f"\n{'Idx':<4} {'Name':<10} {'Len':<5} {'Words':<6} {'Handlers used'}")
    print("-" * 100)
    for idx, prog in programs:
        stream = prog['dram_stream']
        if not stream:
            continue
        entries = decode_dram_stream(stream)
        handler_summary = ' '.join(
            f"{e['name']}({len(e['bytes'])})" for e in entries
        )
        print(f"{idx:<4} {prog['name']:<10} {len(stream):<5} "
              f"{len(entries):<6} {handler_summary}")


def export_programs_python(programs, output_path):
    """
    Export programs grouped by algorithm to a Python file.

    Generates ms4_programs.py with:
    - ALGORITHMS: dict of A-RAM instruction arrays keyed by hex address
    - PROGRAMS_XXXX: lists of program dicts for each algorithm
    """
    # Group programs by A-RAM pointer, skip SILENCE entries (aram_ptr=0x0000)
    by_algorithm = {}
    skipped = 0
    for idx, prog in programs:
        ptr = prog['aram_ptr']
        if ptr == 0x0000:
            skipped += 1
            continue  # Skip SILENCE/invalid programs
        ptr_key = f"{ptr:04X}"
        if ptr_key not in by_algorithm:
            by_algorithm[ptr_key] = {
                'aram_data': prog['aram_data'],
                'programs': []
            }
        by_algorithm[ptr_key]['programs'].append((idx, prog))

    if skipped:
        print(f"Skipped {skipped} SILENCE programs (aram_ptr=0x0000)")

    with open(output_path, 'w') as f:
        f.write('"""\n')
        f.write('MS4 Program Data Export\n')
        f.write('Generated by ms4_parse_programs.py\n')
        f.write('\n')
        f.write('Contains A-RAM algorithm data and D-RAM configurations\n')
        f.write('for all 66 MS4 programs, grouped by algorithm.\n')
        f.write('"""\n\n')

        # Export algorithms
        f.write('# A-RAM algorithm data (32 x 15-bit instruction words)\n')
        f.write('# Keys are hex addresses in ROM\n')
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

            for idx, prog in progs:
                f.write('    {\n')
                f.write(f"        'idx': {idx},\n")
                f.write(f"        'name': '{prog['name']}',\n")
                f.write(f"        'addr': 0x{prog['addr']:04X},\n")
                f.write(f"        'flags': 0x{prog['flags']:02X},\n")
                f.write(f"        'slot_count': {prog['slot_count']},\n")
                f.write(f"        'complex_init': {prog['complex_init']},\n")

                # D-RAM entry 0
                de0 = prog['dram_entry0']
                f.write(f"        'dram_entry0': {{'word': 0x{de0['dram_word']:04X}, "
                        f"'addr_nibble': {de0['ctrl']['addr_nibble']}, "
                        f"'mix_bits': {de0['ctrl']['mix_bits']}}},\n")

                # Voice slots (envelope/modulation data)
                f.write("        'voice_slots': [\n")
                for slot in prog['voice_slots']:
                    data_hex = ', '.join(f'0x{b:02X}' for b in slot['data'])
                    f.write(f"            {{'ptr': 0x{slot['ptr']:04X}, 'data': [{data_hex}]}},\n")
                f.write("        ],\n")

                # D-RAM stream (raw bytes)
                stream = prog['dram_stream']
                if stream:
                    stream_hex = ', '.join(f'0x{b:02X}' for b in stream)
                    f.write(f"        'dram_stream': [{stream_hex}],\n")
                else:
                    f.write("        'dram_stream': [],\n")

                # Decoded D-RAM init values
                if stream:
                    dram_init = decode_dram_init(stream, ptr_key)
                    # D-RAM array (16 x 19-bit values)
                    dram_hex = ', '.join(f'0x{v:05X}' for v in dram_init['dram'])
                    f.write(f"        'dram_init': [{dram_hex}],\n")
                    # Pitch parameters (for runtime note calculation)
                    pp = dram_init['pitch']
                    f.write(f"        'pitch_params': {{'note_offset': {pp['note_offset']}, "
                            f"'fine_tune': {pp['fine_tune']}, 'vel_sens': {pp['vel_sens']}}},\n")
                    # Amplitude parameters (for runtime envelope/velocity)
                    ap = dram_init['amp']
                    f.write(f"        'amp_params': {{'level': {ap['level']}, 'amp': 0x{ap['amp']:02X}, "
                            f"'vel_sens': {ap['vel_sens']}, 'env_ctrl': 0x{ap['env_ctrl']:02X}}},\n")
                    # Waveform words (D-RAM indices that need waveform select)
                    f.write(f"        'waveform_words': {dram_init['waveform_words']},\n")
                else:
                    f.write("        'dram_init': [0] * 16,\n")
                    f.write("        'pitch_params': {'note_offset': 0, 'fine_tune': 0, 'vel_sens': 0},\n")
                    f.write("        'amp_params': {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0x00},\n")
                    f.write("        'waveform_words': [],\n")

                f.write('    },\n')

            f.write(']\n\n')

        # Export algorithm list for easy iteration
        f.write('# List of all algorithm keys\n')
        f.write(f"ALGORITHM_KEYS = {sorted(by_algorithm.keys())}\n\n")

        # Export mapping from algorithm to program list
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

    # Check for --export flag
    if '--export' in sys.argv:
        output_path = os.path.join(os.path.dirname(__file__), 'ms4_programs.py')
        export_programs_python(programs, output_path)
        return

    print_program_summary(programs)
    print_flags_analysis(programs)
    print_dram_entry0_analysis(programs)
    print_dram_stream_analysis(programs)
    print_voice_data_analysis(programs)
    print_aram_analysis(programs)
    print_detailed_programs(programs)


if __name__ == '__main__':
    main()
