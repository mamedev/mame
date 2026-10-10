#!/usr/bin/env python3
# license:BSD-3-Clause
# copyright-holders:R. Belmont
"""Generate the DSP3210 dispatch table, dsp3210tbl.hxx.

The core dispatches on op >> 21 (2048 entries): bits 10:4 of the index are
the 7-bit family op[31:25], bits 3:0 are op[24:21].  Every slot names a
handler (template instantiations for the repetitive families); the seven
architected illegal opcode families (top 6 bits 000000 000001 000010
001111 010110 010111 100010) raise the Illegal Opcode exception, every
other reserved encoding is a no-op (op_reserved).

Usage: dsp3210make.py [output path]   (default: dsp3210tbl.hxx beside this script)
"""

import collections
import os
import sys

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "dsp3210tbl.hxx")

ILLEGAL_TOP6 = (0x00, 0x01, 0x02, 0x0f, 0x16, 0x17, 0x22)

table = [None] * 2048


def fill(lo, hi, name):
    for i in range(lo, hi + 1):
        assert table[i] is None, "slot %03x already %s, wanted %s" % (i, table[i], name)
        table[i] = name


def tf(b):
    return "true" if b else "false"


# --- the seven illegal families: index >> 5 == top 6 bits
for t6 in ILLEGAL_TOP6:
    fill(t6 << 5, (t6 << 5) + 0x1f, "op_illegal")

# --- CA formats
fill(0x060, 0x07f, "dec_goto")                            # 3a  if (rM-- >= 0) goto
fill(0x080, 0x09f, "call")                                # 4a  call {N,rB,rB+N} (rM)
fill(0x0a0, 0x0bf, "add_si<false>")                       # 5a  rD = (short) rS3 + N
fill(0x4a0, 0x4bf, "add_si<true>")                        # 5b  rD = rS3 + N
for f in range(16):                                       # 6a-6d  ALU (F = index[3:0])
    if f == 5:
        fill(0x0c0 + f, 0x0c0 + f, "op_reserved")
        fill(0x0d0 + f, 0x0d0 + f, "op_reserved")
        fill(0x4c0 + f, 0x4c0 + f, "op_reserved")
        fill(0x4d0 + f, 0x4d0 + f, "op_reserved")
    else:
        fill(0x0c0 + f, 0x0c0 + f, "alu_rr<%d, false>" % f)
        fill(0x0d0 + f, 0x0d0 + f, "alu_ri<%d, false>" % f)
        fill(0x4c0 + f, 0x4c0 + f, "alu_rr<%d, true>" % f)
        fill(0x4d0 + f, 0x4d0 + f, "alu_ri<%d, true>" % f)
for t in range(2):                                        # 7a/7c/7d  moves (T = index[3], W = index[2:0])
    for w in range(8):
        i = (t << 3) | w
        if w in (5, 6):
            fill(0x0e0 + i, 0x0e0 + i, "op_reserved")
            fill(0x4e0 + i, 0x4e0 + i, "op_reserved")
            fill(0x4f0 + i, 0x4f0 + i, "op_reserved")
        else:
            fill(0x0e0 + i, 0x0e0 + i, "move_direct<%s, %d>" % (tf(t), w))      # 7a  rH <-> *L
            fill(0x4e0 + i, 0x4e0 + i, "move_ind<%s, %d>" % (tf(t), w))         # 7b (bit 10) / 7c
            fill(0x4f0 + i, 0x4f0 + i, "move_ior_mem<%s, %d>" % (tf(t), w))     # 7d  ior <-> *rP
fill(0x0f0, 0x0ff, "op_reserved")                         # 0001111 reserved (the DSP32C's 7b)
for c in range(64):                                       # 0b/1b  if (C) goto {N,rB,rB+N}
    fill(0x400 + c, 0x400 + c, "goto_c<%d>" % c)
fill(0x460, 0x46f, "do_imm")                              # 3b  do/dolock/doblock K,L
fill(0x470, 0x47f, "do_reg")                              # 3c  do/dolock/doblock K,rM
fill(0x480, 0x49f, "shift_or")                            # 4b  rD = rS <<| N
fill(0x500, 0x5ff, "goto24")                              # 8a  goto {M,rB+M}
fill(0x600, 0x6ff, "load24")                              # 8b  rD = (ushort24) M
fill(0x700, 0x7ff, "call24")                              # 8c  call M (rM)

# --- DA formats: index = family << 4 | F S N1 N0; family = fff MMM b25
for m in range(6):                                        # formats 1/2/3, M = a0..a3, 0.0, 1.0
    for fs in range(4):
        nega, negp = bool(fs & 2), bool(fs & 1)
        for n in range(4):
            i = (fs << 2) | n
            fill(0x100 + 0x20 * m + i, 0x100 + 0x20 * m + i, "da14<%d, %s, %s, false>" % (m, tf(nega), tf(negp)))
            fill(0x200 + 0x20 * m + i, 0x200 + 0x20 * m + i, "da23<%d, %s, %s, true>" % (m, tf(nega), tf(negp)))
            fill(0x300 + 0x20 * m + i, 0x300 + 0x20 * m + i, "da23<%d, %s, %s, false>" % (m, tf(nega), tf(negp)))
    fill(0x110 + 0x20 * m, 0x11f + 0x20 * m, "op_reserved")   # DA bit 25 set
    fill(0x210 + 0x20 * m, 0x21f + 0x20 * m, "op_reserved")
    fill(0x310 + 0x20 * m, 0x31f + 0x20 * m, "op_reserved")
for fs in range(4):                                       # format 4 (fmt 1, M = 110): aN = [-](Z=Y) +- X
    nega, negp = bool(fs & 2), bool(fs & 1)
    for n in range(4):
        i = (fs << 2) | n
        fill(0x1c0 + i, 0x1c0 + i, "da14<5, %s, %s, true>" % (tf(nega), tf(negp)))
fill(0x1d0, 0x1df, "op_reserved")                         # format 4 with bit 25 set
for g in range(16):                                       # format 5: [Z =] aN = G(Y)
    for n in range(4):
        i = 0x3c0 + (g << 2) + n
        fill(i, i, "op_reserved" if g in (10, 11, 15) else "da5<%d>" % g)

assert None not in table, "unfilled slots: %s" % [i for i, t in enumerate(table) if t is None]

with open(OUT, "w") as f:
    f.write("// license:BSD-3-Clause\n// copyright-holders:R. Belmont\n")
    f.write("// DSP3210 dispatch table, indexed by op >> 21.\n")
    f.write("// Generated by dsp3210make.py - do not edit by hand.\n\n")
    f.write("const dsp3210_device::opcode_handler dsp3210_device::s_ops[2048] =\n{\n")
    for row in range(0, 2048, 4):
        f.write("\t/* %03x */ " % row)
        f.write(", ".join("&dsp3210_device::%s" % table[i] for i in range(row, row + 4)))
        f.write(",\n")
    f.write("};\n")

counts = collections.Counter(t.split("<")[0] for t in table)
print("wrote %s" % OUT)
for name, cnt in sorted(counts.items(), key=lambda kv: -kv[1]):
    print("  %-14s %4d slots" % (name, cnt))
