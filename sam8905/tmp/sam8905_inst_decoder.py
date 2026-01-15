#!/usr/bin/env python3
"""
SAM8905 Instruction Decoder

Decodes SAM8905 A-RAM instructions and provides analysis utilities.

Usage:
    from sam8905_inst_decoder import decode_inst, decode_algorithm, print_algorithm
"""


def decode_inst(inst):
    """
    Decode a single SAM8905 15-bit instruction.

    Instruction format (from programmer's guide):
    | MAD(4) | CMD(2) | WSP | WA | WB | WM | WPHI | WXY | clrB | WWF | WACC |
    |  14:11 |  10:9  |  8  |  7 |  6 |  5 |   4  |  3  |   2  |  1  |   0  |

    Write flags are active-low (0 = active) except WSP (1 = active)

    Returns dict with all decoded fields.
    """
    mad = (inst >> 11) & 0xF
    cmd = (inst >> 9) & 0x3
    wsp = (inst >> 8) & 1
    wa = not ((inst >> 7) & 1)
    wb = not ((inst >> 6) & 1)
    wm = not ((inst >> 5) & 1)
    wphi = not ((inst >> 4) & 1)
    wxy = not ((inst >> 3) & 1)
    clrb = not ((inst >> 2) & 1)
    wwf = not ((inst >> 1) & 1)
    wacc = not (inst & 1)

    cmd_names = ["RM", "RADD", "RP", "RSP"]

    return {
        'inst': inst,
        'mad': mad,
        'cmd': cmd,
        'cmd_name': cmd_names[cmd],
        'wsp': wsp,       # Write Special (active high!)
        'wa': wa,         # Write A register
        'wb': wb,         # Write B register
        'wm': wm,         # Write Memory (D-RAM)
        'wphi': wphi,     # Write PHI (phase)
        'wxy': wxy,       # Write X/Y (waveform read)
        'clrb': clrb,     # Clear B register
        'wwf': wwf,       # Write WaveForm register
        'wacc': wacc,     # Write Accumulator (DAC output)
    }


def format_inst(d):
    """Format a decoded instruction as a string"""
    flags = []
    if d['wa']: flags.append('WA')
    if d['wb']: flags.append('WB')
    if d['wm']: flags.append('WM')
    if d['wphi']: flags.append('WPHI')
    if d['wxy']: flags.append('WXY')
    if d['clrb']: flags.append('clrB')
    if d['wwf']: flags.append('WWF')
    if d['wacc']: flags.append('WACC')
    if d['wsp']: flags.append('WSP')

    flag_str = ', '.join(flags) if flags else '-'
    return f"{d['cmd_name']:4s} {d['mad']:2d}, <{flag_str}>"


def decode_algorithm(aram, base, count):
    """
    Decode an algorithm from A-RAM.

    Args:
        aram: Full A-RAM array (256 words)
        base: Starting address of algorithm
        count: Number of instructions (32 for 44kHz, 64 for 22kHz)

    Returns list of decoded instructions.
    """
    result = []
    for i in range(count):
        inst = aram[base + i]
        d = decode_inst(inst)
        d['pc'] = i
        d['addr'] = base + i
        result.append(d)
    return result


def print_algorithm(instructions, name="Algorithm"):
    """Print a decoded algorithm"""
    print(f"=== {name} ({len(instructions)} instructions) ===\n")
    for d in instructions:
        markers = []
        if d['wxy'] and d['wsp']:
            markers.append('** WXY+WSP (MIX update) **')
        elif d['wxy']:
            markers.append('WXY (waveform read)')
        if d['wacc']:
            markers.append('WACC (DAC output)')

        marker_str = f"  {' '.join(markers)}" if markers else ""
        print(f"PC{d['pc']:02d} [0x{d['addr']:02X}]: 0x{d['inst']:04X}  {format_inst(d)}{marker_str}")


def analyze_wxy_wsp(instructions):
    """Find all WXY WSP instructions (MIX update)"""
    result = []
    for d in instructions:
        if d['wxy'] and d['wsp']:
            result.append(d)
    return result


def analyze_wacc(instructions):
    """Find all WACC instructions (DAC output)"""
    result = []
    for d in instructions:
        if d['wacc']:
            result.append(d)
    return result


# Example A-RAM data (FX chip ALG 0 and ALG 2 in 22kHz mode)
FX_ALG0_ARAM = [
    0x00F7, 0x607F, 0x58BF, 0x5A5F, 0x30BF, 0x5DDF, 0x082D, 0x593F,
    0x5ADF, 0x58F7, 0x406F, 0x2CDF, 0x48BF, 0x58F7, 0x42DF, 0x749F,
    0x68F7, 0x38FD, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x406F,
    0x50BF, 0x42DF, 0x683F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F,
    0x7A3F, 0x7A3F, 0x7AF7, 0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF,
    0x78FD, 0x18EF, 0x58F7, 0x7FFF, 0x50EF, 0x08FD, 0x24DF, 0x7FFF,
    0x20F7, 0x287F, 0x00EF, 0x7CBF, 0x2ADF, 0x20F7, 0x707F, 0x7CBF,
    0x28EF, 0x78FD, 0x10F7, 0x7A7F, 0x7CBF, 0x6A5B, 0x7FFF, 0x7FFF,
]

FX_ALG1_ARAM = [
    # A-RAM 0x40-0x7F: 22kHz ALG 1 (D-RAM ALG=1, Used by Slot 5 - Diffusion)
    0x30EF, 0x48FD, 0x6ADF, 0x703F, 0x0000, 0x6BDF, 0x38EF, 0x50FC,  # 0x40-0x47
    0x687F, 0x7CBE, 0x18F7, 0x7A7F, 0x40EF, 0x58FC, 0x7CBE, 0x6ADF,  # 0x48-0x4F
    0x18F7, 0x00BF, 0x307F, 0x32CE, 0x48FC, 0x387F, 0x3ADF, 0x407F,  # 0x50-0x57
    0x42DF, 0x7CBF, 0x20F7, 0x687F, 0x38EF, 0x50FD, 0x7A7E, 0x7CBE,  # 0x58-0x5F
    0x28F7, 0x7A7F, 0x40EF, 0x58FD, 0x7CBE, 0x6ADE, 0x28F7, 0x08BF,  # 0x60-0x67
    0x307F, 0x32DF, 0x493F, 0x4ADF, 0x60BF, 0x4BDF, 0x08BF, 0x387F,  # 0x68-0x6F
    0x3ADF, 0x513F, 0x52DF, 0x60BF, 0x53DF, 0x08BF, 0x407F, 0x42DF,  # 0x70-0x77
    0x593F, 0x5ADF, 0x60BF, 0x5BDF, 0x7CBE, 0x687E, 0x7FFF, 0x7FFF,  # 0x78-0x7F
]

FX_ALG2_ARAM = [
    0x5ADF, 0x68BF, 0x72DF, 0x10FD, 0x006F, 0x18BF, 0x02DF, 0x29B7,
    0x707F, 0x7CBF, 0x7A7E, 0x72DE, 0x30F7, 0x20BF, 0x6CDF, 0x006F,
    0x02DF, 0x087F, 0x28F7, 0x0ADF, 0x7CEF, 0x78FD, 0x40F7, 0x687F,
    0x7CBF, 0x6ADF, 0x38F7, 0x707F, 0x7CBF, 0x725E, 0x50BF, 0x797B,
    0x7AEF, 0x78FD, 0x60F7, 0x70BF, 0x7C7F, 0x72D7, 0x10FD, 0x086F,
    0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x18BF, 0x0ACF, 0x703F,
    0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7AF7,
    0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x587B, 0x7FFF, 0x7FFF,
]

FX_ALG3_ARAM = [
    # A-RAM 0xC0-0xFF: 22kHz ALG 3 (D-RAM ALG=6, Used by Slot 6 - All-pass filter, MUTED)
    0x2ADF, 0x10FD, 0x006F, 0x18BF, 0x02DF, 0x39F7, 0x287F, 0x7CBF,  # 0xC0-0xC7
    0x40F7, 0x32DF, 0x7C3F, 0x6ADE, 0x006F, 0x20BF, 0x02DF, 0x087F,  # 0xC8-0xCF
    0x40F7, 0x0ADF, 0x74DF, 0x38F7, 0x307F, 0x7CEF, 0x60FD, 0x48F7,  # 0xD0-0xD7
    0x7FFF, 0x7CBF, 0x70EF, 0x48F7, 0x325F, 0x68BF, 0x7A7F, 0x7CBF,  # 0xD8-0xDF
    0x7A7F, 0x6ADE, 0x30F7, 0x10FD, 0x086F, 0x7FFB, 0x7FFB, 0x7EFB,  # 0xE0-0xE7
    0x7EFB, 0x7FFF, 0x18BF, 0x0ADF, 0x303F, 0x7A3F, 0x7A3F, 0x7A3F,  # 0xE8-0xEF
    0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x08EF, 0x7AF7, 0x7FFB, 0x7FFB,  # 0xF0-0xF7
    0x7EFB, 0x7EFB, 0x7FFF, 0x687B, 0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,  # 0xF8-0xFF
]


def analyze_algorithm(name, data, base_addr):
    """Analyze and print statistics for an algorithm"""
    alg = [decode_inst(inst) for inst in data]
    for i, d in enumerate(alg):
        d['pc'] = i
        d['addr'] = base_addr + i

    wxy_wsp = analyze_wxy_wsp(alg)
    wacc = analyze_wacc(alg)

    print(f"=== {name} ===")
    print(f"WXY+WSP (MIX update): {len(wxy_wsp)}")
    for d in wxy_wsp:
        print(f"  PC{d['pc']:02d} [0x{d['addr']:02X}]: 0x{d['inst']:04X} - MAD={d['mad']}")
    print(f"WACC (DAC output): {len(wacc)}")
    for d in wacc:
        print(f"  PC{d['pc']:02d} [0x{d['addr']:02X}]: 0x{d['inst']:04X}")
    print()
    return alg


if __name__ == "__main__":
    import sys

    print("SAM8905 Instruction Decoder\n")

    # Analyze all four FX algorithms
    alg0 = analyze_algorithm("FX ALG 0 (A-RAM 0x00-0x3F, Slot 4 - Input conditioning)", FX_ALG0_ARAM, 0x00)
    alg1 = analyze_algorithm("FX ALG 1 (A-RAM 0x40-0x7F, Slot 5 - Diffusion)", FX_ALG1_ARAM, 0x40)
    alg2 = analyze_algorithm("FX ALG 2 (A-RAM 0x80-0xBF, Slots 7-11 - Delay taps)", FX_ALG2_ARAM, 0x80)
    alg3 = analyze_algorithm("FX ALG 3 (A-RAM 0xC0-0xFF, Slot 6 - All-pass, MUTED)", FX_ALG3_ARAM, 0xC0)

    # Print full disassembly if requested
    if len(sys.argv) > 1 and sys.argv[1] == '-v':
        print("\n" + "="*60)
        print_algorithm(alg0, "ALG 0 Full Disassembly")
        print("\n" + "="*60)
        print_algorithm(alg1, "ALG 1 Full Disassembly")
        print("\n" + "="*60)
        print_algorithm(alg2, "ALG 2 Full Disassembly")
        print("\n" + "="*60)
        print_algorithm(alg3, "ALG 3 Full Disassembly")
