#!/usr/bin/env python3
"""Extract SAM[FX] A-RAM and D-RAM data from MAME logs."""
import re
import sys

aram = [0] * 256
dram = [0] * 256

# Parse D-RAM writes
with open('/tmp/dram_writes.log', 'r') as f:
    for line in f:
        if 'SAM[FX]' not in line:
            continue
        m = re.search(r'D-RAM\[([0-9A-F]+)\] = ([0-9A-F]+)', line)
        if m:
            addr = int(m.group(1), 16)
            val = int(m.group(2), 16)
            dram[addr] = val

# Parse A-RAM writes
with open('/tmp/aram_writes.log', 'r') as f:
    for line in f:
        if 'SAM[FX]' not in line:
            continue
        m = re.search(r'A-RAM\[0x([0-9A-F]+)\] write MSB.*word = 0x([0-9A-F]+)', line)
        if m:
            addr = int(m.group(1), 16)
            val = int(m.group(2), 16)
            aram[addr] = val

print("# SAM[FX] A-RAM Data (256 x 15-bit)")
print()
for alg in range(8):
    start = alg * 32
    print(f"## Algorithm {alg} (A-RAM 0x{start:02X}-0x{start+31:02X})")
    row = []
    for i in range(32):
        row.append(f"0x{aram[start + i]:04X}")
    # Print as array
    print("aram_alg{} = [".format(alg))
    for i in range(0, 32, 8):
        chunk = ", ".join(row[i:i+8])
        comma = "," if i < 24 else ""
        print(f"    {chunk}{comma}")
    print("]")
    print()

print()
print("# SAM[FX] D-RAM Data (16 slots x 16 words)")
print()
for slot in range(16):
    start = slot * 16
    nonzero = any(dram[start + i] != 0 for i in range(16))
    if not nonzero:
        continue
    print(f"## Slot {slot} (D-RAM 0x{start:02X}-0x{start+15:02X})")
    print(f"# Word 15 config: 0x{dram[start + 15]:05X}")
    w15 = dram[start + 15]
    idle = (w15 >> 11) & 1
    alg = (w15 >> 8) & 7
    print(f"#   IDLE={idle}, ALG={alg}")
    print(f"dram_slot{slot} = [")
    for i in range(16):
        val = dram[start + i]
        print(f"    0x{val:05X},  # word {i}")
    print("]")
    print()
