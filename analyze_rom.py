#!/usr/bin/env python3
"""
Keyfox10 ROM analyzer - correlates GAL access patterns with ROM content
"""

import sys
from pathlib import Path

ROM_PATH = Path(__file__).parent / "kf10_ic27_v2.bin"
GAL_LOG = Path("/home/jeff/bastel/mame/mame/gal_inputs.log")

def load_rom():
    with open(ROM_PATH, "rb") as f:
        return f.read()

def parse_gal_log():
    """Parse GAL input log and extract access info"""
    accesses = []
    if not GAL_LOG.exists():
        return accesses

    with open(GAL_LOG) as f:
        for line in f:
            if line.startswith("#"):
                continue
            parts = line.strip().split("|")
            if len(parts) >= 3:
                bits = parts[0].strip().split()
                hex_val = parts[1].strip()
                access = parts[2].strip().split()
                if len(access) >= 2:
                    access_type = access[0]  # PSEN, RD, WR
                    addr = int(access[1], 16)
                    t0 = int(bits[6]) if len(bits) > 6 else 1
                    t1 = int(bits[7]) if len(bits) > 7 else 1
                    accesses.append({
                        "type": access_type,
                        "addr": addr,
                        "t0": t0,
                        "t1": t1,
                        "hex": hex_val
                    })
    return accesses

def analyze_data_reads(rom, accesses):
    """Analyze RD accesses and show ROM content"""
    print("=" * 70)
    print("DATA READ ANALYSIS (MOVX reads -> ROM upper 64KB when A16=1)")
    print("=" * 70)
    print(f"{'Data Addr':<12} {'ROM Offset':<12} {'T0 T1':<8} {'Content'}")
    print("-" * 70)

    seen = set()
    for acc in accesses:
        if acc["type"] != "RD":
            continue
        addr = acc["addr"]
        if addr in seen:
            continue
        seen.add(addr)

        # During MOVX read, ~RD inverted -> A16=1, so ROM offset = 0x10000 + addr
        rom_offset = 0x10000 + addr

        # Determine memory region
        if addr < 0x2000:
            region = "RAM"
            content = "(RAM - not ROM)"
        elif addr < 0x8000:
            region = "ROM"
            if rom_offset < len(rom):
                data = rom[rom_offset:rom_offset+16]
                content = data.hex() + " | " + "".join(chr(b) if 32 <= b < 127 else "." for b in data)
            else:
                content = "(out of range)"
        elif addr <= 0x8004:
            region = "SAM_SND"
            content = "(SAM Sound chip)"
        elif addr >= 0xE000 and addr <= 0xE004:
            region = "SAM_FX"
            content = "(SAM Effects chip)"
        else:
            region = "ROM?"
            if rom_offset < len(rom):
                data = rom[rom_offset:rom_offset+16]
                content = data.hex() + " | " + "".join(chr(b) if 32 <= b < 127 else "." for b in data)
            else:
                content = "(unmapped/out of range)"

        print(f"0x{addr:04X}       0x{rom_offset:05X}      {acc['t0']} {acc['t1']}     [{region}] {content}")

def find_rhythm_styles(rom):
    """Find rhythm style names in ROM"""
    print("\n" + "=" * 70)
    print("RHYTHM STYLES IN ROM (upper 64KB)")
    print("=" * 70)

    styles = []
    for offset in range(0x10000, min(0x20000, len(rom)), 0x100):
        name = rom[offset:offset+8]
        # Check if it looks like a style name (starts with uppercase, printable)
        if name[0:1].isalpha() and all(32 <= b < 127 for b in name[:8] if b != 0):
            decoded = name.decode('ascii', errors='replace').rstrip('\x00 ')
            if len(decoded) >= 4 and decoded[0].isupper():
                data_addr = offset - 0x10000  # Address in data space
                styles.append((offset, data_addr, decoded))
                print(f"ROM 0x{offset:05X} (data 0x{data_addr:04X}): {decoded}")

    return styles

def analyze_lookup_tables(rom):
    """Analyze lookup tables at start of upper ROM"""
    print("\n" + "=" * 70)
    print("LOOKUP TABLES (ROM 0x10000-0x100FF)")
    print("=" * 70)

    # Velocity/volume curve at 0x10000
    print("\n0x10000: Velocity curve (24 bytes):")
    data = rom[0x10000:0x10018]
    print("  ", " ".join(f"{b:3d}" for b in data))
    print("  ", " ".join(f"{b:02X}" for b in data))

def analyze_t0_t1_patterns(accesses):
    """Analyze how T0/T1 affect memory access"""
    print("\n" + "=" * 70)
    print("T0/T1 ACCESS PATTERN ANALYSIS")
    print("=" * 70)

    patterns = {}
    for acc in accesses:
        key = (acc["t0"], acc["t1"], acc["type"])
        if key not in patterns:
            patterns[key] = []
        patterns[key].append(acc["addr"])

    for (t0, t1, atype), addrs in sorted(patterns.items()):
        print(f"\nT0={t0} T1={t1} {atype}:")
        for addr in sorted(set(addrs)):
            region = "RAM" if addr < 0x2000 else "ROM_DATA" if addr < 0x8000 else "HIGH"
            print(f"  0x{addr:04X} ({region})")

    print("\n" + "-" * 70)
    print("INTERPRETATION:")
    print("  T1=1: Normal operation (SAM chips enabled at 0x8000, 0xE000)")
    print("  T1=0: ROM data mode (full 64KB ROM accessible, SAM disabled)")
    print("  This explains the CLR T1 / SETB T0 / MOVX / SETB T1 pattern!")

def main():
    rom = load_rom()
    print(f"ROM size: {len(rom)} bytes (0x{len(rom):X})")
    print(f"ROM file: {ROM_PATH}")

    accesses = parse_gal_log()
    if accesses:
        print(f"GAL log: {len(accesses)} unique access patterns")
        analyze_data_reads(rom, accesses)
        analyze_t0_t1_patterns(accesses)
    else:
        print(f"GAL log not found or empty: {GAL_LOG}")

    find_rhythm_styles(rom)
    analyze_lookup_tables(rom)

if __name__ == "__main__":
    main()
