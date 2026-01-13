#!/usr/bin/env python3
"""
SAM8905 A-RAM Instruction Decoder

Decodes 15-bit A-RAM microcode instructions for the SAM8905 DSP.

Instruction format (from Appendix III):
  Bits 14-11: MAD (D-RAM address, 0-15)
  Bits 10-9:  Emitter select (00=RM, 01=RADD, 10=RP, 11=RSP)
  Bit 8:      WSP (active HIGH)
  Bit 7:      WA (active LOW)
  Bit 6:      WB (active LOW)
  Bit 5:      WM (active LOW)
  Bit 4:      WPHI (active LOW)
  Bit 3:      WXY (active LOW)
  Bit 2:      clearB (active LOW)
  Bit 1:      WWF (active LOW)
  Bit 0:      WACC (active LOW)

Emitters:
  RM   - Read from D-RAM at address MAD
  RADD - Output of adder (A + B)
  RP   - Output of multiplier (X * Y)
  RSP  - No emitter (NOP or external memory write)

Receivers (active when bit is LOW, except WSP which is active HIGH):
  WA    - Write to A register
  WB    - Write to B register
  WM    - Write to D-RAM at address MAD
  WPHI  - Write to phase register
  WXY   - Write to X and Y registers (for multiplier)
  clearB - Clear B register to zero
  WWF   - Write to waveform register
  WACC  - Accumulate product into L/R output accumulators

WSP modifier effects:
  WPHI WSP - Also set WF register to 0x100 (internal sinus)
  WXY WSP  - Also update MIXL/MIXR output attenuators
  WA WSP   - Special sampling operations
  WM WSP   - Special sampling operations
"""

from typing import NamedTuple, List, Optional


class DecodedInstruction(NamedTuple):
    """Decoded SAM8905 A-RAM instruction."""
    raw: int           # Original 15-bit instruction
    mad: int           # D-RAM address (0-15)
    emitter: str       # RM, RADD, RP, or RSP
    wsp: bool          # WSP modifier active
    wa: bool           # Write A register
    wb: bool           # Write B register
    wm: bool           # Write to D-RAM
    wphi: bool         # Write phase register
    wxy: bool          # Write X/Y registers
    clear_b: bool      # Clear B register
    wwf: bool          # Write waveform register
    wacc: bool         # Accumulate to output


EMITTER_NAMES = ['RM', 'RADD', 'RP', 'RSP']


def decode_instruction(inst: int) -> DecodedInstruction:
    """
    Decode a single 15-bit A-RAM instruction.

    Args:
        inst: 15-bit instruction word

    Returns:
        DecodedInstruction with all fields parsed
    """
    return DecodedInstruction(
        raw=inst,
        mad=(inst >> 11) & 0xF,
        emitter=EMITTER_NAMES[(inst >> 9) & 0x3],
        wsp=bool(inst & 0x100),      # Bit 8, Active HIGH
        wa=not (inst & 0x80),        # Bit 7, Active LOW
        wb=not (inst & 0x40),        # Bit 6, Active LOW
        wm=not (inst & 0x20),        # Bit 5, Active LOW
        wphi=not (inst & 0x10),      # Bit 4, Active LOW
        wxy=not (inst & 0x08),       # Bit 3, Active LOW
        clear_b=not (inst & 0x04),   # Bit 2, Active LOW
        wwf=not (inst & 0x02),       # Bit 1, Active LOW
        wacc=not (inst & 0x01),      # Bit 0, Active LOW
    )


def format_instruction(inst: DecodedInstruction, show_hex: bool = True) -> str:
    """
    Format a decoded instruction as assembly-like text.

    Args:
        inst: Decoded instruction
        show_hex: Include hex value in output

    Returns:
        Formatted string like "RM 5, <WA, WB>"
    """
    # Build emitter part
    if inst.emitter == 'RM':
        emitter_str = f"RM {inst.mad}"
    elif inst.wm and inst.emitter == 'RADD':
        emitter_str = f"RADD {inst.mad}"
    elif inst.wm and inst.emitter == 'RP':
        emitter_str = f"RP {inst.mad}"
    else:
        emitter_str = inst.emitter

    # Build receivers list
    receivers = []
    if inst.wa:
        receivers.append('WA')
    if inst.wb:
        receivers.append('WB')
    if inst.wm:
        receivers.append('WM')
    if inst.wphi:
        receivers.append('WPHI')
    if inst.wxy:
        receivers.append('WXY')
    if inst.clear_b:
        receivers.append('clearB')
    if inst.wwf:
        receivers.append('WWF')
    if inst.wacc:
        receivers.append('WACC')
    if inst.wsp:
        receivers.append('WSP')

    recv_str = ', '.join(receivers) if receivers else 'NOP'

    if show_hex:
        return f"{inst.raw:04X}  {emitter_str}, <{recv_str}>"
    else:
        return f"{emitter_str}, <{recv_str}>"


def decode_algorithm(instructions: List[int], alg_num: Optional[int] = None) -> str:
    """
    Decode a full algorithm (32 instructions at 44.1kHz).

    Args:
        instructions: List of 15-bit instruction words
        alg_num: Optional algorithm number for header

    Returns:
        Multi-line string with decoded assembly
    """
    lines = []
    if alg_num is not None:
        lines.append(f"=== Algorithm {alg_num} ===")
    lines.append("")

    for pc, raw in enumerate(instructions):
        inst = decode_instruction(raw)
        formatted = format_instruction(inst)
        marker = " ***" if inst.wsp else ""
        lines.append(f"PC{pc:02d}: {formatted}{marker}")

    return '\n'.join(lines)


def decode_all_algorithms(aram: List[int], sample_rate: str = "44.1kHz") -> str:
    """
    Decode all algorithms from a full A-RAM dump.

    Args:
        aram: Full 256-word A-RAM contents
        sample_rate: "44.1kHz" (8 algorithms x 32 instructions) or
                     "22.05kHz" (4 algorithms x 64 instructions)

    Returns:
        Multi-line string with all decoded algorithms
    """
    if sample_rate == "44.1kHz":
        alg_count = 8
        alg_size = 32
    else:
        alg_count = 4
        alg_size = 64

    lines = []
    for alg in range(alg_count):
        start = alg * alg_size
        end = start + alg_size
        alg_code = aram[start:end]
        lines.append(decode_algorithm(alg_code, alg))
        lines.append("")

    return '\n'.join(lines)


def get_dram_references(inst: DecodedInstruction) -> dict:
    """
    Analyze D-RAM references in an instruction.

    Returns:
        dict with 'read' and 'write' keys containing MAD if referenced
    """
    refs = {'read': None, 'write': None}

    if inst.emitter == 'RM':
        refs['read'] = inst.mad

    if inst.wm:
        refs['write'] = inst.mad

    return refs


def analyze_dram_usage(instructions: List[int]) -> dict:
    """
    Analyze which D-RAM addresses are used in an algorithm.

    Returns:
        dict mapping address -> {'read': count, 'write': count}
    """
    usage = {i: {'read': 0, 'write': 0} for i in range(16)}

    for raw in instructions:
        inst = decode_instruction(raw)
        refs = get_dram_references(inst)

        if refs['read'] is not None:
            usage[refs['read']]['read'] += 1
        if refs['write'] is not None:
            usage[refs['write']]['write'] += 1

    # Filter to only used addresses
    return {k: v for k, v in usage.items() if v['read'] > 0 or v['write'] > 0}


if __name__ == "__main__":
    # Example: decode algorithm 2 from the Keyfox10 reverb
    aram_alg2 = [
        0x79F7, 0x207F, 0x28BF, 0x7AFD, 0x082F, 0x7A3F, 0x4ADF, 0x387B,
        0x42DF, 0x7FFF, 0x10F7, 0x48EF, 0x7EFB, 0x487F, 0x7CF7, 0x20FD,
        0x086F, 0x00BE, 0x0ADF, 0x213F, 0x22DB, 0x43DF, 0x10B7, 0x187F,
        0x13DF, 0x307E, 0x40BF, 0x7ABF, 0x207F, 0x22DF, 0x7FFF, 0x7FFF
    ]

    print(decode_algorithm(aram_alg2, 2))
    print()
    print("D-RAM usage:")
    usage = analyze_dram_usage(aram_alg2)
    for addr, counts in sorted(usage.items()):
        print(f"  D-RAM[{addr:2d}]: read={counts['read']}, write={counts['write']}")
