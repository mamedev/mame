// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// EVOLUTION disassembler

#include "emu.h"
#include "evod.h"

u32 evolution_disassembler::opcode_alignment() const
{
	return 1;
}

// 3ea2 is table 4x2

// 36b0 = Penguin Rescue
// 3593 = table of pairs (adr, 0040)

// 403e1f: indexes the table at 403ea2 somehow

/*
  start:
400051: fc80       ? fc80 (c c a)
400052: f842       push2 c
400053: f862       push2 d
400054: f802       push2 a
400055: f882       ? f882 (a c a)
400056: f8c2       ? f8c2 (a d c)
400057: f8e2       ? f8e2 (a d d)
400058: fc82       ? fc82 (c c a)
400059: fc9c       ? fc9c (c c a)
40005a: fc83       ? fc83 (c c a)
40005b: fc9d       ? fc9d (c c a)

  end:
4000a5: fcdd       ? fcdd (c d c)
4000a6: fcc3       ? fcc3 (c d c)
4000a7: fcdc       ? fcdc (c d c)
4000a8: fcc2       ? fcc2 (c d c)
4000a9: f8e3       ? f8e3 (a d d)
4000aa: f8c3       ? f8c3 (a d c)
4000ab: f883       ? f883 (a c a)
4000ac: f803       pull2 a
4000ad: f863       pull2 d
4000ae: f843       pull2 c
4000af: fcc0       ? fcc0 (c d c)

40019e: 8d..?  3e.3d = 5b8119, 40.3f = 400b5b, jst 403a4f

40309b: simple access?
-> 4015c6

406276: series of compares?

403a4f, 40332d, both use (3e, 3d) and (40, 3f) (see 40019e, 400204=5b8119, 400b5b)

40332d: fc8d       push 0d
40332e: da00       b = #0000
40332f: e28d       #e0# b 0d
403330: b205       b = 05
403331: cabe       ? cabe (b c)
403332: 9011       beq 403344
403333: b205       b = 05
403334: caa6       ? caa6 (b c)
403335: cac6       ? cac6 (b d)
403336: a205       05 = b
403337: dae8       b = #00e8
403338: c203       b.h = #03
403339: ea44       b = b - #1
40333a: 91fe       bne 403339
40333b: daff       b = #00ff
40333c: c201       b.h = #01
40333d: fd40 03c8  jsr 4003c8
40333f: d808       a = #0008
403340: fd40 02db  jsr 4002db
403342: fccd       pull 0d
403343: ff42       rts
403344: b205       b = 05
403345: caa6       ? caa6 (b c)
403346: cac6       ? cac6 (b d)
403347: a205       05 = b
403348: fccd       pull 0d
403349: ff42       rts

// Possible bit tests:
// - cd9f
// - cabe cac2 cade cafe

// Possible bit changes:
// - c88e c8b6 c8ee
// - caa6 cac6 caae

// vectors
//   10: 40023e
//   18: 400051  a1 ~ 1100 1010 1010 0110 + other stuff  0101
//   20: 4000b2  a1 ~ 1100 1000 1110 0110                0011
//   28: 4000c1  a1 ~ 1100 1000 1010 0110                0001
//   30: 4000d0  a1 ~ 1100 1010 1100 0110                0110
//   38: 4000d7  a1 ~ 1100 1010 1000 0110                0100
//   40: 4000de  a1 ~ 1100 1000 1100 0110                0010
//   48: 4000e5  a0 ~ 1100 1100 1100 0110                1010
//   50: rti
//       4000ec  a0 ~ 1100 1110 1110 0110                1111 + other stuff


-- fc[8-9]x is matched with fc[c-d]x  80 82 83 8d 9c 9d

-- 400b6c: bunch of calls tp 400e9f with various values of b

-- 4002e4, 4002ed, two probably symmetric things with 03/00/0d

-- 4001b7 writes? with pauses in between

Series of quasi-equivalent codes, probably program memory access

// Loads (1c:02).w in b, increment 02, if 0 increment 1c?
400369: e260       ? e260 (b b)
40036a: f862       ? f862 (a b)
40036b: e782       #e1# b = r02
40036c: cbe3       ? cbe3 (b d)
40036d: 9002       beq 400370
40036e: fe40 0373  jmp 400373
400370: e79c       #e1# b = r1c
400371: eb60       ? eb60 (b b)
400372: e39c       #e1# r1c = b
400373: f863       ? f863 (a b)
400374: ff42       rts

// Loads (1d:03).w in b, increment 03, if 0 increment 1d?
400381: e264       ? e264 (b b)
400382: f862       ? f862 (a b)
400383: e783       #e1# b = r03
400384: cbe3       ? cbe3 (b d)
400385: 9002       beq 400388
400386: fe40 038b  jmp 40038b
400388: e79d       #e1# b = r1d
400389: eb60       ? eb60 (b b)
40038a: e39d       #e1# r1d = b
40038b: f863       ? f863 (a b)
40038c: ff42       rts

// Write b to (1c:02).w, increment 02, if 0 increment 1c?
4003a3: e320       ? e320 (b a)
4003a4: e782       #e1# b = r02
4003a5: cbe3       ? cbe3 (b d)
4003a6: 9002       beq 4003a9
4003a7: fe40 03ac  jmp 4003ac
4003a9: e79c       #e1# b = r1c
4003aa: eb60       ? eb60 (b b)
4003ab: e39c       #e1# r1c = b
4003ac: ff42       rts

// Write b to (1d:03).w, increment 03, if 0 increment 1d?
4003b7: e324       ? e324 (b a)
4003b7: e324       ? e324 (b a)
4003b8: e783       #e1# b = r03
4003b9: cbe3       ? cbe3 (b d)
4003ba: 9002       beq 4003bd
4003bb: fe40 03c0  jmp 4003c0
4003bd: e79d       #e1# b = r1d
4003be: eb60       ? eb60 (b b)
4003bf: e39d       #e1# r1d = b
4003c0: ff42       rts

// Note: 400375/40038d seem to be used as reads, 400399/4003ad as writes

400375: e360       ? e360 (b b)
400376: f842       push b
400377: e682       #e0# b = r02
400378: cac2       ? cac2 (b d)
400379: 9002       beq 40037c
40037a: fe40 037f  jmp 40037f
40037c: e69c       #e0# b = r1c
40037d: ea40       b = b + #1
40037e: e29c       #e0# r1c = b
40037f: f843       pull b
400380: ff42       rts

40038d: e364       ? e364 (b b)
40038e: f842       push b
40038f: e683       #e0# b = r03
400390: cac2       ? cac2 (b d)
400391: 9002       beq 400394
400392: fe40 0397  jmp 400397
400394: e69d       #e0# b = r1d
400395: ea40       b = b + #1
400396: e29d       #e0# r1d = b
400397: f843       pull b
400398: ff42       rts

400399: e220       ? e220 (b a)
40039a: e682       #e0# b = r02
40039b: cac2       ? cac2 (b d)
40039c: 9002       beq 40039f
40039d: fe40 03a2  jmp 4003a2
40039f: e69c       #e0# b = r1c
4003a0: ea40       b = b + #1
4003a1: e29c       #e0# r1c = b
4003a2: ff42       rts

4003ad: e224       ? e224 (b a)
4003ae: e683       #e0# b = r03
4003af: cac2       ? cac2 (b d)
4003b0: 9002       beq 4003b3
4003b1: fe40 03b6  jmp 4003b6
4003b3: e69d       #e0# b = r1d
4003b4: ea40       b = b + #1
4003b5: e29d       #e0# r1d = b
4003b6: ff42       rts

// 400521 interesting but very not clear.  Called with c.l and a.hl set
//   4004da: called 98.9680   10000000
//   4004ea: called 0f.4240    1000000
//   4004fc: called 01.86a0     100000
//   40050e: called 00.2710      10000

4002b2 look like zone clears

400620: ?02 = 3572, ?1c = 0040, jsr 4005c3  4003572; 0b04 0b09 fff0 ffff

4005c3: fc8d       push 0d
4005c4: da00       b = #0000
4005c5: e28d       #e0# b 0d
4005c6: da01       b = #0001
4005c7: f858       ? f858 (a b)
4005c8: e240       ? e240 (b b)
4005c9: dbff       b = b -2 #00ff
4005ca: c3ff       b = b - #ff00
4005cb: eb53       ? eb53 (b b)
4005cc: 9009       beq 4005d6
4005cd: fd40 0591  jsr 400591
4005cf: f8e8       ? f8e8 (a d)
4005d0: cac2       ? cac2 (b d)
4005d1: 9106       bne 4005d8
4005d2: f8c8       ? f8c8 (a d)
4005d3: ea40       b = b + #1
4005d4: f858       ? f858 (a b)
4005d5: 8ff2       bra 4005c8
4005d6: da00       b = #0000
4005d7: cac2       ? cac2 (b d)
4005d8: fccd       pull 0d
4005d9: ff42       rts

400591: da00       b = #0000
400592: f85c       ? f85c (a b)
400593: fd40 0369  jsr 400369
400595: dbf0       b = b -2 #00f0
400596: c3ff       b = b - #ff00
400597: eb72       ? eb72 (b b)
400598: 9101       bne 40059a
400599: 8016       bra 4005b0
40059a: dbff       b = b -2 #00ff
40059b: cbc3       ? cbc3 (b d)
40059c: a338       (38) = b
40059d: e2ba       #e0# b 3a
40059e: e7ba       #e1# d 3a
40059f: c300       b = b - #0000
4005a0: a33a       (3a) = b
4005a1: fd40 0369  jsr 400369
4005a3: dbf0       b = b -2 #00f0
4005a4: c3ff       b = b - #ff00
4005a5: eb72       ? eb72 (b b)
4005a6: 9101       bne 4005a8
4005a7: 8008       bra 4005b0
4005a8: dbff       b = b -2 #00ff
4005a9: cbc3       ? cbc3 (b d)
4005aa: a339       (39) = b
4005ab: e2ba       #e0# b 3a
4005ac: e7ba       #e1# d 3a
4005ad: c300       b = b - #0000
4005ae: a33b       (3b) = b
4005af: 8001       bra 4005b1
4005b0: ff42       rts
4005b1: b238       b = 38
4005b2: ea1a       ? ea1a (b a)
4005b3: 9501       bxx 5, 4005b5
4005b4: 8fde       bra 400593
4005b5: b239       b = 39
4005b6: ea1a       ? ea1a (b a)
4005b7: 9301       bxx 3, 4005b9
4005b8: 8fda       bra 400593
4005b9: b23a       b = 3a
4005ba: ea50       ? ea50 (b b)
4005bb: 9501       bxx 5, 4005bd
4005bc: 8fd6       bra 400593
4005bd: b23b       b = 3b
4005be: ea50       ? ea50 (b b)
4005bf: 94d3       bxx 4, 400593
4005c0: f8c8       ? f8c8 (a d)
4005c1: f85c       ? f85c (a b)
4005c2: 8fed       bra 4005b0

4010b7: shows reads and writes of r
*/

// 4005e9: 3e.3d = 5b1981 = address of a table of addresses

const char *const evolution_disassembler::regs[] = { "a", "b", "c", "d", "e", "f", "g", "h" };

const evolution_disassembler::instruction evolution_disassembler::instructions[] = {
	{ 0x8000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bra %06x", (pc + 1 + util::sext(opcode, 12)) & 0xffffff); }},
	{ 0x9000, 0xff00, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "beq %06x", (pc + 1 + util::sext(opcode, 8)) & 0xffffff); }},
	{ 0x9100, 0xff00, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bne %06x", (pc + 1 + util::sext(opcode, 8)) & 0xffffff); }},
	{ 0x9000, 0xf000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "bxx %x, %06x", (opcode >> 8) & 0xf, (pc + 1 + s8(opcode)) & 0xffffff); }},
	{ 0xa000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%02x = %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xa100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "(%02x) = %s", opcode & 0xff, regs[(opcode >> 9) & 3]); }},
	{ 0xb000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xb100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = (%02x)", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xc000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s.h = #%02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xc100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "tstf %s, #%04x", regs[(opcode >> 9) & 3], (opcode & 0xff) << 8); }},
	{ 0xd000, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s.l = #%02x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd100, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - #%02x", regs[(opcode >> 9) & 3], regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd800, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = #%04x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xd900, 0xf900, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "tst0 %s, #%04x", regs[(opcode >> 9) & 3], opcode & 0xff); }},
	{ 0xe080, 0xfdc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "#e0# r%02x = %s", opcode & 0x3f, regs[(opcode >> 9) & 1]); }},
	{ 0xe180, 0xfdc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "#e1# r%02x = %s", opcode & 0x3f, regs[(opcode >> 9) & 1]); }},
	{ 0xe480, 0xfdc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "#e0# %s = r%02x", regs[(opcode >> 9) & 1], opcode & 0x3f); }},
	{ 0xe580, 0xfdc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "#e1# %s = r%02x", regs[(opcode >> 9) & 1], opcode & 0x3f); }},
	{ 0xe800, 0xf93c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s + #%x", regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3], 1 << (opcode & 3)); }},
	{ 0xe804, 0xf93c, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "%s = %s - #%x", regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3], 1 << (opcode & 3)); }},
	{ 0xf802, 0xff3f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push %s", regs[(opcode >> 6) & 3]); }},
	{ 0xf803, 0xff3f, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pull %s", regs[(opcode >> 6) & 3]); }},
	{ 0xfc80, 0xffc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "push %02x", opcode & 0x3f); }},
	{ 0xfcc0, 0xffc0, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "pull %02x", opcode & 0x3f); }},
	{ 0xfd00, 0xff00, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jsr %06x", ((opcode & 0xff) << 16) | arg); }},
	{ 0xfe00, 0xff00, 2, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "jmp %06x", ((opcode & 0xff) << 16) | arg); }},
	{ 0xff41, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "rti"); }},
	{ 0xff42, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "rts"); }},
	{ 0xffff, 0xffff, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "nop"); }},

	{ 0x0000, 0x0000, 1, [](std::ostream &stream, u16 opcode, u16 arg, u32 pc) { util::stream_format(stream, "? %04x (%s %s)", opcode, regs[(opcode >> 9) & 3], regs[(opcode >> 6) & 3]); }}
};

offs_t evolution_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);

	int i = 0;
	while((opcode & instructions[i].mask) != instructions[i].value)
		i++;

	instructions[i].fct(stream, opcode, (instructions[i].size >= 2) ? opcodes.r16(pc+1) : 0, pc);

	return instructions[i].size;
}
