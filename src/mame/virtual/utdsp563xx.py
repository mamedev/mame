#!/usr/bin/python
# license:BSD-3-Clause
# copyright-holders:Olivier Galibert

import sys

if len(sys.argv) == 1:
    print("Usage:\n%s dsp563xx.lst dsp563xx.ipp" % sys.argv[0])
    sys.exit(0)

# name testlist-start testlist-end
testblock = []

# code-start code-end pre-start pre-end post-start post-end
testlist = []

# 24-bits values
code = []

# reg value
regs = []

dsp_regs = [
    'a', 'b', 'a0', 'a1', 'a2', 'b0', 'b1', 'b2',
    'x0', 'y0', 'x1', 'y1', 'x', 'y',
    'r0', 'r1', 'r2', 'r3', 'r4', 'r5', 'r6', 'r7',
    'm0', 'm1', 'm2', 'm3', 'm4', 'm5', 'm6', 'm7',
    'n0', 'n1', 'n2', 'n3', 'n4', 'n5', 'n6', 'n7',
    'sr', 'omr', 'vba', 'ep', 'lc', 'la'
    ]

dsp_flags = [
    'ccr.c', 'ccr.z', 'ccr.v', 'ccr.n', 'ccr.u', 'ccr.e', 'ccr.l', 'ccr.s',
    'sr.s0', 'sr.s1'
    ]


for l in open(sys.argv[1], 'rt'):
    if l[0] == '#':
        continue
    ls = l.rstrip('\r\n').split()
    if len(ls) == 0:
        continue
    if l[0] != '\t' and l[0] != ' ':
        if len(testblock) != 0:
            testblock[-1][2] = len(testlist)
        testblock.append([ls[0], len(testlist), -1])
    else:
        tl = [len(code), -1, len(regs), -1, -1, -1]
        mode = 0
        for j in range(len(ls)):
            if ls[j] == '<':
                mode = 1
                tl[1] = len(code)
            elif ls[j] == '>':
                tl[3] = len(regs)
                tl[4] = len(regs)
                mode = 2
            else:
                if mode == 0:
                    code.append(int(ls[j], 16))
                else:
                    l1 = ls[j].split('=')
                    if len(l1) != 2:
                        print(ls[j])
                    assert(len(l1) == 2)
                    val = int(l1[1], 16)
                    if l1[0] not in dsp_regs and l1[0] not in dsp_flags:
                        if l1[0][:3] == 'mem':
                            val = (int(l1[0][5:], 16) << 24) | val
                        else:
                            print("Unknown register %s" % l1[0])
                    
                    regs.append([l1[0], val])
        assert(mode == 2)
        tl[5] = len(regs)
        testlist.append(tl)

testblock[-1][2] = len(testlist)
testlist[-1][5] = len(regs)
    
out = open(sys.argv[2], 'wt')
print("// license:GPL", file=out)
print("// copyright-holders: dsp56300 <dsp56300@users.noreply.github.com>", file=out)
print("", file=out)
print("// Generated file, use python utdsp563xx.py dsp563xx.lst dsp563xx.ipp to regenerate", file=out)

print("", file=out)

print("const utdsp563xx_state::testblock utdsp563xx_state::testblocks[] = {", file=out)
for t in testblock:
    print("\t{ \"%s\", %d, %d }," % (t[0], t[1], t[2]), file=out)
print("\t{ nullptr, 0, 0 }", file=out)
print("};", file=out)
print("", file=out)

print("const utdsp563xx_state::testlist utdsp563xx_state::testlists[] = {", file=out)
for t in testlist:
    print("\t{ %d, %d, %d, %d, %d, %d }," % (t[0], t[1], t[2], t[3], t[4], t[5]), file=out)
print("};", file=out)
print("", file=out)

print("const u32 utdsp563xx_state::code[] = {", file=out)
for i in range(0, len(code), 16):
    s = ''
    for j in range(min(len(code) - i, 16)):
        s += (' ' if j else '\t') + ("0x%06x" % code[i+j]) + ","
    print(s, file=out)
print("};", file=out)
print("", file=out)

print("const utdsp563xx_state::reg utdsp563xx_state::regs[] = {", file=out)
for r in regs:
    if r[0] in dsp_regs:
        print("\t{ DSP563XX_%s, 0x%x }," % (r[0].upper(), r[1]), file=out)
    elif r[0] in dsp_flags:
        print("\t{ F_%s, 0x%x }," % (r[0].upper().replace('.', '_'), r[1]), file=out)
    else:
        print("\t{ MEM_%s, 0x%x }," % (r[0][3].upper(), r[1]), file=out)
print("};", file=out)
