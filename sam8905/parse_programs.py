#!/usr/bin/env python3
"""
SAM8905 Program Structure Parser - Common Module

Shared parsing functions for SAM8905-based synthesizers.
Device-specific parsers (MS4, XE9L, etc.) import from this module.

Program data format (common to MS4/XE9L firmware):
  Offset 0-7:   ASCII name (space-padded)
  Offset 8:     Null terminator (0x00)
  Offset 9:     Flags (bit7=complex_init, bits3:0=slot_count)
  Offset 10-11: A-RAM data pointer (LE: [10]=DPL, [11]=DPH)
  Offset 12-14: First D-RAM parameter entry (val_lo, val_mid, ctrl)
  Offset 15-16: Voice init data pointer (LE), 0x0000 = none
  Offset 17+:   Additional D-RAM parameter data
"""

import os
import sys

# Import A-RAM decoder from same directory
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sam8905_aram_decoder import decode_algorithm, analyze_dram_usage


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

    Handler byte consumption:
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
    words_remaining = 16

    while words_remaining > 0 and pos < len(rom):
        dispatch = rom[pos]

        if dispatch & 0x80:
            pos += 1
            break

        handler = dispatch & 0x38

        if handler == 0x00:
            if pos + 3 < len(rom) and (rom[pos + 3] & 0xE0) == 0:
                pos += 4
            else:
                pos += 3
        elif handler == 0x08:
            pos += 10
        elif handler == 0x10:
            pos += 9
        elif handler == 0x18:
            pos += 4
        elif handler == 0x20:
            pos += 4
            break
        elif handler == 0x28:
            pos += 1
        elif handler == 0x30:
            pos += 1
            break
        else:  # 0x38
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

    Returns list of dicts: {word, handler, name, bytes, offset}
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


def decode_dram_init(stream, waveform_words=None):
    """
    Decode D-RAM stream into initial values for interpreter.

    Args:
        stream: D-RAM config stream bytes
        waveform_words: List of D-RAM indices that need waveform select
                       (algorithm-specific, from WWF analysis)

    Returns dict with:
        'dram': 16-word array of initial D-RAM values (19-bit)
        'pitch': {note_offset, fine_tune, vel_sens} for runtime pitch calc
        'amp': {level, amp, vel_sens, env_ctrl} for runtime amplitude calc
        'waveform_words': list of D-RAM indices that need waveform select
    """
    dram = [0] * 16
    pitch_params = {'note_offset': 0, 'fine_tune': 0, 'vel_sens': 0}
    amp_params = {'level': 0, 'amp': 0x7F, 'vel_sens': 0, 'env_ctrl': 0}
    wf_words = waveform_words or []

    entries = decode_dram_stream(stream)

    for entry in entries:
        word = entry['word']
        b = entry['bytes']
        name = entry['name']

        if name == 'pitch' and len(b) >= 10:
            pitch_params['vel_sens'] = b[1]
            pitch_params['note_offset'] = b[2]
            pitch_params['fine_tune'] = b[5] | (b[6] << 8)
            dram[word] = 0

        elif name == 'amplitude' and len(b) >= 9:
            amp_params['level'] = b[1]
            amp_params['amp'] = b[2]
            amp_params['env_ctrl'] = b[3]
            amp_params['vel_sens'] = b[7] if len(b) > 7 else 0
            if b[2] > 0:
                dram[word] = (b[2] << 7) | 0x3F

        elif name == 'dram_wr' and len(b) >= 4:
            dram[word] = b[0]

        elif name in ('const28', 'const38'):
            dram[word] = b[0] & 0x3F

        elif name == 'terminator':
            dram[word] = b[0] & 0x7F

        elif name == 'short_wr' and len(b) >= 3:
            dram[word] = b[0] | (b[1] << 8)

    # Set waveform words to internal sine (WF=0x100)
    internal_sine = 0x100 << 9
    for w in wf_words:
        if dram[w] == 0:
            dram[w] = internal_sine

    return {
        'dram': dram,
        'pitch': pitch_params,
        'amp': amp_params,
        'waveform_words': wf_words,
    }


def format_handler_detail(entry):
    """Format decoded details for a handler entry's bytes."""
    b = entry['bytes']
    handler = entry.get('handler', '')

    if handler == '0x08' and len(b) >= 10:
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
        val = b[0]
        return f"val=0x{val:02X} mod=[{b[1]:02X} {b[2]:02X} {b[3]:02X}]"

    elif handler == '0x20' and len(b) >= 4:
        return f"xram_F9=0x{b[1]:02X} xram_FA=0x{b[2]:02X} xram_FB_lo={b[3] & 7}"

    elif entry['name'] == 'terminator':
        return f"val=0x{b[0]:02X} → D-RAM word value"

    return None


def format_voice_init_detail(data):
    """Decode 7-byte voice init data."""
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
    Parse a program structure starting at addr.

    Returns dict with program data or None if invalid.
    """
    if addr + 17 > len(rom):
        return None

    prog = {}
    prog['addr'] = addr
    prog['name'] = rom[addr:addr + 8].decode('ascii', errors='replace').rstrip('\x00 ')
    prog['null_term'] = rom[addr + 8]
    prog['flags'] = rom[addr + 9]
    prog['aram_ptr'] = get_word_le(rom, addr + 10)
    prog['dram_entry0'] = parse_dram_entry(
        rom[addr + 12], rom[addr + 13], rom[addr + 14]
    )
    prog['voice_data_ptr'] = get_word_le(rom, addr + 15)
    prog['complex_init'] = bool(prog['flags'] & 0x80)
    prog['slot_count'] = prog['flags'] & 0x0F

    if next_addr and next_addr > addr:
        prog['size'] = next_addr - addr
    else:
        prog['size'] = None

    prog['aram_data'] = parse_aram_data(rom, prog['aram_ptr'])

    # Voice slots
    prog['voice_slots'] = []
    vptr = prog['voice_data_ptr']
    if vptr != 0 and vptr != 0xFFFF and vptr + 7 <= len(rom):
        prog['voice_slots'].append({
            'ptr': vptr,
            'data': list(rom[vptr:vptr + 7])
        })

    # Additional slot pointers
    dram_stream_offset = 17
    if prog['voice_data_ptr'] != 0:
        pos = addr + 17
        max_pos = addr + (prog['size'] if prog['size'] else 256)
        while pos + 1 < max_pos:
            slot_ptr = get_word_le(rom, pos)
            pos += 2
            if slot_ptr == 0:
                break
            if slot_ptr + 7 <= len(rom):
                prog['voice_slots'].append({
                    'ptr': slot_ptr,
                    'data': list(rom[slot_ptr:slot_ptr + 7])
                })
        dram_stream_offset = pos - addr

    # D-RAM stream
    prog['dram_stream_offset'] = dram_stream_offset
    prog['dram_stream'] = []
    if (prog['size'] is None or prog['size'] > dram_stream_offset) and \
       addr + dram_stream_offset < len(rom):
        stream_start = addr + dram_stream_offset
        stream_end = scan_dram_stream(rom, stream_start)
        prog['dram_stream'] = list(rom[stream_start:stream_end])

    header_len = min(22, prog['size']) if prog['size'] else 22
    prog['raw_header'] = rom[addr:addr + header_len].hex(' ')

    return prog


def analyze_algorithms_wwf(rom, aram_ptrs):
    """
    Analyze WWF instructions for a list of A-RAM pointers.

    Returns dict mapping algorithm key to list of D-RAM word indices.
    """
    waveform_words = {}

    for ptr in aram_ptrs:
        if ptr == 0 or ptr + 64 > len(rom):
            continue

        words = []
        for i in range(32):
            lo = rom[ptr + i * 2]
            hi = rom[ptr + i * 2 + 1]
            words.append((hi << 8) | lo)

        wwf_words = []
        for pc, instr in enumerate(words):
            wwf_active = not (instr & 0x02)  # Bit 1 active LOW
            if wwf_active:
                dram_addr = (instr >> 11) & 0x0F
                emitter = (instr >> 9) & 0x03
                if emitter == 0:  # RM reads from D-RAM
                    wwf_words.append(dram_addr)

        ptr_key = f"{ptr:04X}"
        waveform_words[ptr_key] = sorted(set(wwf_words))

    return waveform_words
