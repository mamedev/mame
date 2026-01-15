#!/usr/bin/env python3
"""
FX SRAM Access Pattern Analysis

Analyzes the FX chip algorithms to understand SRAM access patterns
and validate the 2-byte storage hypothesis.

Usage: python3 fx_sram_analysis.py
"""

# ALG 2 (22kHz mode, A-RAM 0x80-0xBF) - the main reverb algorithm
ALG2_ARAM = [
    0x5ADF, 0x68BF, 0x72DF, 0x10FD, 0x006F, 0x18BF, 0x02DF, 0x29B7,
    0x707F, 0x7CBF, 0x7A7E, 0x72DE, 0x30F7, 0x20BF, 0x6CDF, 0x006F,
    0x02DF, 0x087F, 0x28F7, 0x0ADF, 0x7CEF, 0x78FD, 0x40F7, 0x687F,
    0x7CBF, 0x6ADF, 0x38F7, 0x707F, 0x7CBF, 0x725E, 0x50BF, 0x797B,
    0x7AEF, 0x78FD, 0x60F7, 0x70BF, 0x7C7F, 0x72D7, 0x10FD, 0x086F,
    0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x18BF, 0x0ACF, 0x703F,
    0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7A3F, 0x7AF7,
    0x7FFB, 0x7FFB, 0x7EFB, 0x7EFB, 0x7FFF, 0x587B, 0x7FFF, 0x7FFF,
]

# Slot 7 D-RAM values (from MAME dump)
SLOT7_DRAM = {
    0: 0x2B800,   # PHI source 1
    1: 0x00080,   # PHI source 2
    2: 0x10402,   # WF config
    3: 0x00080,
    4: 0x00180,
    5: 0x6667F,   # amplitude + mix
    6: 0x79999,
    7: 0x10000,
    8: 0x00100,
    9: 0x00000,
    10: 0x00080,
    11: 0x00000,
    12: 0x00000,
    13: 0x00000,
    14: 0x00000,
    15: 0x3C480,  # ALG=4
}


def decode_inst(inst):
    """Decode a SAM8905 instruction"""
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

    cmd_names = ["RM", "RADD", "RSP", "RP"]

    return {
        'mad': mad, 'cmd': cmd_names[cmd], 'wsp': wsp,
        'wa': wa, 'wb': wb, 'wm': wm, 'wphi': wphi,
        'wxy': wxy, 'clrb': clrb, 'wwf': wwf, 'wacc': wacc,
        'inst': inst
    }


def calc_sram_addr(wf_word, phi_word):
    """Calculate SRAM address from WF and PHI D-RAM words

    SRAM addr = WF[6:0] << 8 | PHI[11:4]
    WAH0 = PHI[4] (bit 0 of SRAM address)
    """
    wf = (wf_word >> 10) & 0x7F   # WF[6:0] from bits 16:10
    phi = phi_word & 0xFFF        # PHI[11:0]
    phi_high = (phi >> 4) & 0xFF  # PHI[11:4]
    sram_addr = (wf << 8) | phi_high
    wah0 = (phi >> 4) & 1         # WAH0 = PHI[4]
    return sram_addr, wah0


def analyze_waveform_access():
    """Analyze waveform access patterns in ALG 2"""
    print("=== ALG 2 Waveform Access Analysis ===\n")
    print("Instructions that access waveforms (WWF sets WF, WXY reads waveform):\n")

    wf_source = None
    phi_source = None
    last_wxy_pc = None

    for pc, inst in enumerate(ALG2_ARAM):
        d = decode_inst(inst)

        events = []

        if d['wwf']:
            wf_source = d['mad']
            events.append(f"WF←D[{d['mad']}]")

        if d['wphi']:
            phi_source = d['mad']
            events.append(f"PHI←D[{d['mad']}]")

        if d['wxy']:
            gap = f" (gap={pc - last_wxy_pc})" if last_wxy_pc is not None else ""
            events.append(f"**WXY READ** WF=D[{wf_source}] PHI=D[{phi_source}]{gap}")
            last_wxy_pc = pc

        if d['wm']:
            events.append(f"D[{d['mad']}]←(result)")

        if events:
            flags = []
            if d['wsp']: flags.append('WSP')
            if d['wacc']: flags.append('WACC')
            flag_str = f" {flags}" if flags else ""
            print(f"PC{pc:02d}: {d['cmd']:4s} {d['mad']:2d} -> {', '.join(events)}{flag_str}")


def analyze_phi_patterns():
    """Analyze PHI values for address bit patterns"""
    print("\n=== PHI Value Analysis ===\n")
    print("Checking WAH0 (=PHI[4]) patterns in D-RAM:\n")

    for i, val in SLOT7_DRAM.items():
        phi_val = val & 0xFFF
        phi_bit4 = (phi_val >> 4) & 1
        print(f"D[{i:2d}] = 0x{val:05X}, PHI = 0x{phi_val:03X}, PHI[4] (WAH0) = {phi_bit4}")


def analyze_sram_addresses():
    """Calculate expected SRAM addresses from WXY reads"""
    print("\n=== Expected SRAM Addresses ===\n")
    print("SRAM addr = WF[6:0] << 8 | PHI[11:4]\n")

    wxy_configs = [
        ("PC07", 2, 0, "WF=D[2], PHI=D[0]"),
        ("PC12", 2, 0, "WF=D[2], PHI=D[0]"),
        ("PC18", 2, 0, "WF=D[2], PHI=D[0]"),
        ("PC22", 15, 15, "WF=D[15], PHI=D[15]"),
        ("PC26", 15, 15, "WF=D[15], PHI=D[15]"),
        ("PC34", 15, 15, "WF=D[15], PHI=D[15]"),
        ("PC55", 2, 1, "WF=D[2], PHI=D[1]"),
    ]

    for pc, wf_idx, phi_idx, desc in wxy_configs:
        wf_word = SLOT7_DRAM.get(wf_idx, 0)
        phi_word = SLOT7_DRAM.get(phi_idx, 0)
        addr, wah0 = calc_sram_addr(wf_word, phi_word)
        print(f"{pc}: {desc}")
        print(f"      SRAM addr = 0x{addr:04X}, WAH0 = {wah0}")


def timing_analysis():
    """Analyze timing for 2-byte SRAM access hypothesis"""
    print("\n=== Timing Analysis ===\n")

    sam_clock = 22.5792e6  # Hz
    sample_rate = 22050    # Hz
    inst_per_frame = 64

    frame_time = 1 / sample_rate
    time_per_inst = frame_time / inst_per_frame
    sram_access_time = 70e-9  # 70ns typical

    print(f"Sample rate: {sample_rate} Hz")
    print(f"Frame time: {frame_time*1e6:.2f} µs")
    print(f"Time per instruction: {time_per_inst*1e9:.0f} ns")
    print(f"SRAM access time: {sram_access_time*1e9:.0f} ns")
    print(f"SRAM accesses per instruction: {time_per_inst / sram_access_time:.1f}")
    print()
    print("Conclusion: Plenty of time for 2 SRAM reads per waveform access!")


if __name__ == "__main__":
    analyze_waveform_access()
    analyze_phi_patterns()
    analyze_sram_addresses()
    timing_analysis()
