// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#include "emu.h"
#include "arcompactdasm_internal.h"

u32 arcompact_disassembler::opcode_alignment() const
{
	return 2;
}

// condition codes (basic ones are the same as arc
const char *const arcompact_disassembler::conditions[0x20] =
{
	/* 00 */ "AL", // (aka RA         - Always)
	/* 01 */ "EQ", // (aka Z          - Zero
	/* 02 */ "NE", // (aka NZ         - Non-Zero)
	/* 03 */ "PL", // (aka P          - Positive)
	/* 04 */ "MI", // (aka N          - Negative)
	/* 05 */ "CS", // (aka C,  LO     - Carry set / Lower than) (unsigned)
	/* 06 */ "CC", // (aka CC, NC, HS - Carry Clear / Higher or Same) (unsigned)
	/* 07 */ "VS", // (aka V          - Overflow set)
	/* 08 */ "VC", // (aka NV         - Overflow clear)
	/* 09 */ "GT", // (               - Greater than) (signed)
	/* 0a */ "GE", // (               - Greater than or Equal) (signed)
	/* 0b */ "LT", // (               - Less than) (signed)
	/* 0c */ "LE", // (               - Less than or Equal) (signed)
	/* 0d */ "HI", // (               - Higher than) (unsigned)
	/* 0e */ "LS", // (               - Lower or Same) (unsigned)
	/* 0f */ "PNZ",// (               - Positive non-0 value)
	/* 10 */ "0x10 Reserved", // possible CPU implementation specifics
	/* 11 */ "0x11 Reserved",
	/* 12 */ "0x12 Reserved",
	/* 13 */ "0x13 Reserved",
	/* 14 */ "0x14 Reserved",
	/* 15 */ "0x15 Reserved",
	/* 16 */ "0x16 Reserved",
	/* 17 */ "0x17 Reserved",
	/* 18 */ "0x18 Reserved",
	/* 19 */ "0x19 Reserved",
	/* 1a */ "0x1a Reserved",
	/* 1b */ "0x1b Reserved",
	/* 1c */ "0x1c Reserved",
	/* 1d */ "0x1d Reserved",
	/* 1e */ "0x1e Reserved",
	/* 1f */ "0x1f Reserved"
};

#define UNUSED_REG "unusedreg"

#define AUX_UNUSED_16 \
	/* 0xxx0 */ UNUSED_REG, /* 0xxx1 */ UNUSED_REG, /* 0xxx2 */ UNUSED_REG, /* 0xxx3 */ UNUSED_REG, /* 0xxx4 */ UNUSED_REG, /* 0xxx5 */ UNUSED_REG, /* 0xxx6 */ UNUSED_REG, /* 0xxx7 */ UNUSED_REG, /* 0xxx8 */ UNUSED_REG, /* 0xxx9 */ UNUSED_REG, /* 0xxxa */ UNUSED_REG, /* 0xxxb */ UNUSED_REG, /* 0xxxc */ UNUSED_REG, /* 0xxxd */ UNUSED_REG, /* 0xxxe */ UNUSED_REG, /* 0xxxf */ UNUSED_REG,

// the Auxiliary Register set is actually a 2^32 dword address space (so 16 GB / 34-bit)
// this table just allows us to improve the debugger display for some of the common core / internal ones
const char *const arcompact_disassembler::auxregnames[0x420] =
{
	/* 0x000 */ "STATUS",
	/* 0x001 */ "SEMAPHOR",
	/* 0x002 */ "LP_START",
	/* 0x003 */ "LP_END",
	/* 0x004 */ "IDENTITY",
	/* 0x005 */ "DEBUG",
	/* 0x006 */ "PC",
	/* 0x007 */ UNUSED_REG,
	/* 0x008 */ UNUSED_REG,
	/* 0x009 */ UNUSED_REG,
	/* 0x00a */ "STATUS32",
	/* 0x00b */ "STATUS32_L1",
	/* 0x00c */ "STATUS32_L2",
	/* 0x00d */ UNUSED_REG,
	/* 0x00e */ UNUSED_REG,
	/* 0x00f */ UNUSED_REG,

	/* 0x010 */ UNUSED_REG,
	/* 0x011 */ UNUSED_REG,
	/* 0x012 */ "MULHI", // extension register
	/* 0x013 */ UNUSED_REG,
	/* 0x014 */ UNUSED_REG,
	/* 0x015 */ UNUSED_REG,
	/* 0x016 */ UNUSED_REG,
	/* 0x017 */ UNUSED_REG,
	/* 0x018 */ UNUSED_REG,
	/* 0x019 */ UNUSED_REG,
	/* 0x01a */ UNUSED_REG,
	/* 0x01b */ UNUSED_REG,
	/* 0x01c */ UNUSED_REG,
	/* 0x01d */ UNUSED_REG,
	/* 0x01e */ UNUSED_REG,
	/* 0x01f */ UNUSED_REG,

	/* 0x020 */ UNUSED_REG,
	/* 0x021 */ "COUNT0",
	/* 0x022 */ "CONTROL0",
	/* 0x023 */ "LIMIT0",
	/* 0x024 */ UNUSED_REG,
	/* 0x025 */ "INT_VECTOR_BASE",
	/* 0x026 */ UNUSED_REG,
	/* 0x027 */ UNUSED_REG,
	/* 0x028 */ UNUSED_REG,
	/* 0x029 */ UNUSED_REG,
	/* 0x02a */ UNUSED_REG,
	/* 0x02b */ UNUSED_REG,
	/* 0x02c */ UNUSED_REG,
	/* 0x02d */ UNUSED_REG,
	/* 0x02e */ UNUSED_REG,
	/* 0x02f */ UNUSED_REG,
	AUX_UNUSED_16 /* 0x030 - 0x03f */
	/* 0x040 */ UNUSED_REG,
	/* 0x041 */ "AUX_MACMODE",
	/* 0x042 */ UNUSED_REG,
	/* 0x043 */ "AUX_IRQLV12",
	/* 0x044 */ UNUSED_REG,
	/* 0x045 */ UNUSED_REG,
	/* 0x046 */ UNUSED_REG,
	/* 0x047 */ UNUSED_REG,
	/* 0x048 */ UNUSED_REG,
	/* 0x049 */ UNUSED_REG,
	/* 0x04a */ UNUSED_REG,
	/* 0x04b */ UNUSED_REG,
	/* 0x04c */ UNUSED_REG,
	/* 0x04d */ UNUSED_REG,
	/* 0x04e */ UNUSED_REG,
	/* 0x04f */ UNUSED_REG,
	AUX_UNUSED_16 /* 0x050 - 0x05f */
	// build configuration registers 0x060 - 0x07f
	/* 0x060 */ "RESERVED AUX 0x60",/* 0x061 */ "RESERVED AUX 0x61",/* 0x062 */ "RESERVED AUX 0x62",/* 0x063 */ "RESERVED AUX 0x63",/* 0x064 */ "RESERVED AUX 0x64",/* 0x065 */ "RESERVED AUX 0x65",/* 0x066 */ "RESERVED AUX 0x66",/* 0x067 */ "RESERVED AUX 0x67",/* 0x068 */ "RESERVED AUX 0x68",/* 0x069 */ "RESERVED AUX 0x69",/* 0x06a */ "RESERVED AUX 0x6a",/* 0x06b */ "RESERVED AUX 0x6b",/* 0x06c */ "RESERVED AUX 0x6c",/* 0x06d */ "RESERVED AUX 0x6d",/* 0x06e */ "RESERVED AUX 0x6e",/* 0x06f */ "RESERVED AUX 0x6f",
	/* 0x070 */ "RESERVED AUX 0x70",/* 0x071 */ "RESERVED AUX 0x71",/* 0x072 */ "RESERVED AUX 0x72",/* 0x073 */ "RESERVED AUX 0x73",/* 0x074 */ "RESERVED AUX 0x74",/* 0x075 */ "RESERVED AUX 0x75",/* 0x076 */ "RESERVED AUX 0x76",/* 0x077 */ "RESERVED AUX 0x77",/* 0x078 */ "RESERVED AUX 0x78",/* 0x079 */ "RESERVED AUX 0x79",/* 0x07a */ "RESERVED AUX 0x7a",/* 0x07b */ "RESERVED AUX 0x7b",/* 0x07c */ "RESERVED AUX 0x7c",/* 0x07d */ "RESERVED AUX 0x7d",/* 0x07e */ "RESERVED AUX 0x7e",/* 0x07f */ "RESERVED AUX 0x7f",
	AUX_UNUSED_16 /* 0x080 - 0x08f */
	AUX_UNUSED_16 /* 0x090 - 0x09f */
	AUX_UNUSED_16 /* 0x0a0 - 0x0af */
	AUX_UNUSED_16 /* 0x0b0 - 0x0bf */
		// build configuration registers 0x0c0 - 0x0ff
	/* 0x0c0 */ "RESERVED AUX 0xc0",/* 0x0c1 */ "RESERVED AUX 0xc1",/* 0x0c2 */ "RESERVED AUX 0xc2",/* 0x0c3 */ "RESERVED AUX 0xc3",/* 0x0c4 */ "RESERVED AUX 0xc4",/* 0x0c5 */ "RESERVED AUX 0xc5",/* 0x0c6 */ "RESERVED AUX 0xc6",/* 0x0c7 */ "RESERVED AUX 0xc7",/* 0x0c8 */ "RESERVED AUX 0xc8",/* 0x0c9 */ "RESERVED AUX 0xc9",/* 0x0ca */ "RESERVED AUX 0xca",/* 0x0cb */ "RESERVED AUX 0xcb",/* 0x0cc */ "RESERVED AUX 0xcc",/* 0x0cd */ "RESERVED AUX 0xcd",/* 0x0ce */ "RESERVED AUX 0xce",/* 0x0cf */ "RESERVED AUX 0xcf",
	/* 0x0d0 */ "RESERVED AUX 0xd0",/* 0x0d1 */ "RESERVED AUX 0xd1",/* 0x0d2 */ "RESERVED AUX 0xd2",/* 0x0d3 */ "RESERVED AUX 0xd3",/* 0x0d4 */ "RESERVED AUX 0xd4",/* 0x0d5 */ "RESERVED AUX 0xd5",/* 0x0d6 */ "RESERVED AUX 0xd6",/* 0x0d7 */ "RESERVED AUX 0xd7",/* 0x0d8 */ "RESERVED AUX 0xd8",/* 0x0d9 */ "RESERVED AUX 0xd9",/* 0x0da */ "RESERVED AUX 0xda",/* 0x0db */ "RESERVED AUX 0xdb",/* 0x0dc */ "RESERVED AUX 0xdc",/* 0x0dd */ "RESERVED AUX 0xdd",/* 0x0de */ "RESERVED AUX 0xde",/* 0x0df */ "RESERVED AUX 0xdf",
	/* 0x0e0 */ "RESERVED AUX 0xe0",/* 0x0e1 */ "RESERVED AUX 0xe1",/* 0x0e2 */ "RESERVED AUX 0xe2",/* 0x0e3 */ "RESERVED AUX 0xe3",/* 0x0e4 */ "RESERVED AUX 0xe4",/* 0x0e5 */ "RESERVED AUX 0xe5",/* 0x0e6 */ "RESERVED AUX 0xe6",/* 0x0e7 */ "RESERVED AUX 0xe7",/* 0x0e8 */ "RESERVED AUX 0xe8",/* 0x0e9 */ "RESERVED AUX 0xe9",/* 0x0ea */ "RESERVED AUX 0xea",/* 0x0eb */ "RESERVED AUX 0xeb",/* 0x0ec */ "RESERVED AUX 0xec",/* 0x0ed */ "RESERVED AUX 0xed",/* 0x0ee */ "RESERVED AUX 0xee",/* 0x0ef */ "RESERVED AUX 0xef",
	/* 0x0f0 */ "RESERVED AUX 0xf0",/* 0x0f1 */ "RESERVED AUX 0xf1",/* 0x0f2 */ "RESERVED AUX 0xf2",/* 0x0f3 */ "RESERVED AUX 0xf3",/* 0x0f4 */ "RESERVED AUX 0xf4",/* 0x0f5 */ "RESERVED AUX 0xf5",/* 0x0f6 */ "RESERVED AUX 0xf6",/* 0x0f7 */ "RESERVED AUX 0xf7",/* 0x0f8 */ "RESERVED AUX 0xf8",/* 0x0f9 */ "RESERVED AUX 0xf9",/* 0x0fa */ "RESERVED AUX 0xfa",/* 0x0fb */ "RESERVED AUX 0xfb",/* 0x0fc */ "RESERVED AUX 0xfc",/* 0x0fd */ "RESERVED AUX 0xfd",/* 0x0fe */ "RESERVED AUX 0xfe",/* 0x0ff */ "RESERVED AUX 0xff",
	/* 0x100 */ "COUNT1",
	/* 0x101 */ "CONTROL1",
	/* 0x102 */ "LIMIT1",
	/* 0x103 */ UNUSED_REG,
	/* 0x104 */ UNUSED_REG,
	/* 0x105 */ UNUSED_REG,
	/* 0x106 */ UNUSED_REG,
	/* 0x107 */ UNUSED_REG,
	/* 0x108 */ UNUSED_REG,
	/* 0x109 */ UNUSED_REG,
	/* 0x10a */ UNUSED_REG,
	/* 0x10b */ UNUSED_REG,
	/* 0x10c */ UNUSED_REG,
	/* 0x10d */ UNUSED_REG,
	/* 0x10e */ UNUSED_REG,
	/* 0x10f */ UNUSED_REG,
	AUX_UNUSED_16 /* 0x110 - 0x11f */
	AUX_UNUSED_16 /* 0x120 - 0x12f */
	AUX_UNUSED_16 /* 0x130 - 0x13f */
	AUX_UNUSED_16 /* 0x140 - 0x14f */
	AUX_UNUSED_16 /* 0x150 - 0x15f */
	AUX_UNUSED_16 /* 0x160 - 0x16f */
	AUX_UNUSED_16 /* 0x170 - 0x17f */
	AUX_UNUSED_16 /* 0x180 - 0x18f */
	AUX_UNUSED_16 /* 0x190 - 0x19f */
	AUX_UNUSED_16 /* 0x1a0 - 0x1af */
	AUX_UNUSED_16 /* 0x1b0 - 0x1bf */
	AUX_UNUSED_16 /* 0x1c0 - 0x1cf */
	AUX_UNUSED_16 /* 0x1d0 - 0x1df */
	AUX_UNUSED_16 /* 0x1e0 - 0x1ef */
	AUX_UNUSED_16 /* 0x1f0 - 0x1ff */
	/* 0x200 */ "AUX_IRQ_LEV",
	/* 0x201 */ "AUX_IRQ_HINT",
	/* 0x203 */ UNUSED_REG,
	/* 0x203 */ UNUSED_REG,
	/* 0x204 */ UNUSED_REG,
	/* 0x205 */ UNUSED_REG,
	/* 0x206 */ UNUSED_REG,
	/* 0x207 */ UNUSED_REG,
	/* 0x208 */ UNUSED_REG,
	/* 0x209 */ UNUSED_REG,
	/* 0x20a */ UNUSED_REG,
	/* 0x20b */ UNUSED_REG,
	/* 0x20c */ UNUSED_REG,
	/* 0x20d */ UNUSED_REG,
	/* 0x20e */ UNUSED_REG,
	/* 0x20f */ UNUSED_REG,
	AUX_UNUSED_16 /* 0x210 - 0x21f */
	AUX_UNUSED_16 /* 0x220 - 0x22f */
	AUX_UNUSED_16 /* 0x230 - 0x23f */
	AUX_UNUSED_16 /* 0x240 - 0x24f */
	AUX_UNUSED_16 /* 0x250 - 0x25f */
	AUX_UNUSED_16 /* 0x260 - 0x26f */
	AUX_UNUSED_16 /* 0x270 - 0x27f */
	AUX_UNUSED_16 /* 0x280 - 0x28f */
	AUX_UNUSED_16 /* 0x290 - 0x29f */
	AUX_UNUSED_16 /* 0x2a0 - 0x2af */
	AUX_UNUSED_16 /* 0x2b0 - 0x2bf */
	AUX_UNUSED_16 /* 0x2c0 - 0x2cf */
	AUX_UNUSED_16 /* 0x2d0 - 0x2df */
	AUX_UNUSED_16 /* 0x2e0 - 0x2ef */
	AUX_UNUSED_16 /* 0x2f0 - 0x2ff */

	AUX_UNUSED_16 /* 0x300 - 0x30f */
	AUX_UNUSED_16 /* 0x310 - 0x31f */
	AUX_UNUSED_16 /* 0x320 - 0x32f */
	AUX_UNUSED_16 /* 0x330 - 0x33f */
	AUX_UNUSED_16 /* 0x340 - 0x34f */
	AUX_UNUSED_16 /* 0x350 - 0x35f */
	AUX_UNUSED_16 /* 0x360 - 0x36f */
	AUX_UNUSED_16 /* 0x370 - 0x37f */
	AUX_UNUSED_16 /* 0x380 - 0x38f */
	AUX_UNUSED_16 /* 0x390 - 0x39f */
	AUX_UNUSED_16 /* 0x3a0 - 0x3af */
	AUX_UNUSED_16 /* 0x3b0 - 0x3bf */
	AUX_UNUSED_16 /* 0x3c0 - 0x3cf */
	AUX_UNUSED_16 /* 0x3d0 - 0x3df */
	AUX_UNUSED_16 /* 0x3e0 - 0x3ef */
	AUX_UNUSED_16 /* 0x3f0 - 0x3ff */

	/* 0x400 */ "ERET",
	/* 0x401 */ "ERBTA",
	/* 0x403 */ "ERSTATUS",
	/* 0x403 */ "ECR",
	/* 0x404 */ "EFA",
	/* 0x405 */ UNUSED_REG,
	/* 0x406 */ UNUSED_REG,
	/* 0x407 */ UNUSED_REG,
	/* 0x408 */ UNUSED_REG,
	/* 0x409 */ UNUSED_REG,
	/* 0x40a */ "ICAUSE1",
	/* 0x40b */ "ICAUSE2",
	/* 0x40c */ "AUX_IENABLE",
	/* 0x40d */ "AUX_ITRIGGER",
	/* 0x40e */ UNUSED_REG,
	/* 0x40f */ UNUSED_REG,

	/* 0x410 */ "XPU",
	/* 0x411 */ UNUSED_REG,
	/* 0x412 */ "BTA",
	/* 0x413 */ "BTA_L1",
	/* 0x414 */ "BTA_L2",
	/* 0x415 */ "AUX_IRQ_PULSE_CANCEL",
	/* 0x416 */ "AUX_IRQ_PENDING",
	/* 0x417 */ UNUSED_REG,
	/* 0x418 */ UNUSED_REG,
	/* 0x419 */ UNUSED_REG,
	/* 0x41a */ UNUSED_REG,
	/* 0x41b */ UNUSED_REG,
	/* 0x41c */ UNUSED_REG,
	/* 0x41d */ UNUSED_REG,
	/* 0x41e */ UNUSED_REG,
	/* 0x41f */ UNUSED_REG
};

//#define EXPLICIT_EXTENSIONS

const char *const arcompact_disassembler::datasize[0x4] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".L", // Dword (default) (can use no extension, using .L to be explicit)
#else
	/* 00 */ "",// Dword (default)
#endif
	/* 01 */ ".B", // Byte
	/* 02 */ ".W", // Word
	/* 03 */ ".<illegal data size>"
};

const char *const arcompact_disassembler::dataextend[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".ZX", // Zero Extend (can use no extension, using .ZX to be explicit)
#else
	/* 00 */ "", // Zero Extend
#endif
	/* 01 */ ".X" // Sign Extend
};

const char *const arcompact_disassembler::addressmode[0x4] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".AN", // No Writeback (can use no extension, using .AN to be explicit)
#else
	/* 00 */ "", // No Writeback
#endif
	/* 01 */ ".AW", // Writeback pre memory access
	/* 02 */ ".AB", // Writeback post memory access
	/* 03 */ ".AS"  // scaled
};

const char *const arcompact_disassembler::cachebit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".EN", // Data Cache Enabled (can use no extension, using .EN to be explicit)
#else
	/* 00 */ "", // Data Cache Enabled
#endif
	/* 01 */ ".DI" // Direct to Memory (Cache Bypass)
};

const char *const arcompact_disassembler::flagbit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".NF", // Don't Set Flags (can use no extension, using .NF to be explicit)
#else
	/* 00 */ "", // Don't Set Flags
#endif
	/* 01 */ ".F" // Set Flags
};

const char *const arcompact_disassembler::delaybit[0x2] =
{
	/* 00 */ ".ND", // Don't execute opcode in delay slot
	/* 01 */ ".D"   // Execute Opcode in delay slot
};


const char *const arcompact_disassembler::regnames[0x40] =
{
	/* 00 */ "r0",
	/* 01 */ "r1",
	/* 02 */ "r2",
	/* 03 */ "r3",
	/* 04 */ "r4",
	/* 05 */ "r5",
	/* 06 */ "r6",
	/* 07 */ "r7",
	/* 08 */ "r8",
	/* 09 */ "r9",
	/* 0a */ "r10",
	/* 0b */ "r11",
	/* 0c */ "r12",
	/* 0d */ "r13",
	/* 0e */ "r14",
	/* 0f */ "r15",

	/* 10 */ "r16",
	/* 11 */ "r17",
	/* 12 */ "r18",
	/* 13 */ "r19",
	/* 14 */ "r20",
	/* 15 */ "r21",
	/* 16 */ "r22",
	/* 17 */ "r23",
	/* 18 */ "r24",
	/* 19 */ "r25",
	/* 1a */ "r26_GP",
	/* 1b */ "r27_FP",
	/* 1c */ "r28_SP",
	/* 1d */ "r29_ILINK1",
	/* 1e */ "r30_ILINK2",
	/* 1f */ "r31_BLINK",

	/* 20 */ "r32(ext)",
	/* 21 */ "r33(ext)",
	/* 22 */ "r34(ext)",
	/* 23 */ "r35(ext)",
	/* 24 */ "r36(ext)",
	/* 25 */ "r37(ext)",
	/* 26 */ "r38(ext)",
	/* 27 */ "r39(ext)",
	/* 28 */ "r40(ext)",
	/* 29 */ "r41(ext)",
	/* 2a */ "r42(ext)",
	/* 2b */ "r43(ext)",
	/* 2c */ "r44(ext)",
	/* 2d */ "r45(ext)",
	/* 2e */ "r46(ext)",
	/* 2f */ "r47(ext)",

	/* 30 */ "r48(ext)",
	/* 31 */ "r49(ext)",
	/* 32 */ "r50(ext)",
	/* 33 */ "r51(ext)",
	/* 34 */ "r52(ext)",
	/* 35 */ "r53(ext)",
	/* 36 */ "r54(ext)",
	/* 37 */ "r55(ext)",
	/* 38 */ "r56(ext)",
	/* 39 */ "r57(M-LO)",  // MLO  (result registers for optional multply functions)
	/* 3a */ "r58(M-MID)", // MMID
	/* 3b */ "r59(M-HI)",  // MHI
	/* 3c */ "r60(LP_COUNT)",
	/* 3d */ "r61(reserved)",
	/* 3e */ "r62(LIMM)", // use Long Immediate Data instead of register
	/* 3f */ "r63(PCL)"
};

#if 0
const char *const arcompact_disassembler::opcodes_temp[0x40] =
{
	/* 00 */ "0x00",
	/* 01 */ "0x01",
	/* 02 */ "0x02",
	/* 03 */ "0x03",
	/* 04 */ "0x04",
	/* 05 */ "0x05",
	/* 06 */ "0x06",
	/* 07 */ "0x07",
	/* 08 */ "0x08",
	/* 09 */ "0x09",
	/* 0a */ "0x0a",
	/* 0b */ "0x0b",
	/* 0c */ "0x0c",
	/* 0d */ "0x0d",
	/* 0e */ "0x0e",
	/* 0f */ "0x0f",

	/* 10 */ "0x10",
	/* 11 */ "0x11",
	/* 12 */ "0x12",
	/* 13 */ "0x13",
	/* 14 */ "0x14",
	/* 15 */ "0x15",
	/* 16 */ "0x16",
	/* 17 */ "0x17",
	/* 18 */ "0x18",
	/* 19 */ "0x19",
	/* 1a */ "0x1a",
	/* 1b */ "0x1b",
	/* 1c */ "0x1c",
	/* 1d */ "0x1d",
	/* 1e */ "0x1e",
	/* 1f */ "0x1f",

	/* 20 */ "0x20",
	/* 21 */ "0x21",
	/* 22 */ "0x22",
	/* 23 */ "0x23",
	/* 24 */ "0x24",
	/* 25 */ "0x25",
	/* 26 */ "0x26",
	/* 27 */ "0x27",
	/* 28 */ "0x28",
	/* 29 */ "0x29",
	/* 2a */ "0x2a",
	/* 2b */ "0x2b",
	/* 2c */ "0x2c",
	/* 2d */ "0x2d",
	/* 2e */ "0x2e",
	/* 2f */ "0x2f",

	/* 30 */ "0x30",
	/* 31 */ "0x31",
	/* 32 */ "0x32",
	/* 33 */ "0x33",
	/* 34 */ "0x34",
	/* 35 */ "0x35",
	/* 36 */ "0x36",
	/* 37 */ "0x37",
	/* 38 */ "0x38",
	/* 39 */ "0x39",
	/* 3a */ "0x3a",
	/* 3b */ "0x3b",
	/* 3c */ "0x3c",
	/* 3d */ "0x3d",
	/* 3e */ "0x3e",
	/* 3f */ "0x3f",
};
#endif


const char *const arcompact_disassembler::opcodes_04[0x40] =
{
	/* 00 */ "ADD",
	/* 01 */ "ADC",
	/* 02 */ "SUB",
	/* 03 */ "SBC",
	/* 04 */ "AND",
	/* 05 */ "OR",
	/* 06 */ "BIC",
	/* 07 */ "XOR",
	/* 08 */ "MAX",
	/* 09 */ "MIN",
	/* 0a */ "MOV",
	/* 0b */ "TST",
	/* 0c */ "CMP",
	/* 0d */ "RCMP",
	/* 0e */ "RSUB",
	/* 0f */ "BSET",

	/* 10 */ "BCLR",
	/* 11 */ "BTST",
	/* 12 */ "BXOR",
	/* 13 */ "BMSK",
	/* 14 */ "ADD1",
	/* 15 */ "ADD2",
	/* 16 */ "ADD3",
	/* 17 */ "SUB1",
	/* 18 */ "SUB2",
	/* 19 */ "SUB3",
	/* 1a */ "MPY",
	/* 1b */ "MPYH",
	/* 1c */ "MPYHU",
	/* 1d */ "MPYU",
	/* 1e */ "0x1e",
	/* 1f */ "0x1f",

	/* 20 */ "Jcc",
	/* 21 */ "Jcc.D",
	/* 22 */ "JLcc",
	/* 23 */ "JLcc.D",
	/* 24 */ "0x24",
	/* 25 */ "0x25",
	/* 26 */ "0x26",
	/* 27 */ "0x27",
	/* 28 */ "LPcc",
	/* 29 */ "FLAG",
	/* 2a */ "LR",
	/* 2b */ "SR",
	/* 2c */ "0x2c",
	/* 2d */ "0x2d",
	/* 2e */ "0x2e",
	/* 2f */ "SOP table",

	/* 30 */ "LD",
	/* 31 */ "LD",
	/* 32 */ "LD",
	/* 33 */ "LD",
	/* 34 */ "LD",
	/* 35 */ "LD",
	/* 36 */ "LD",
	/* 37 */ "LD",
	/* 38 */ "0x38",
	/* 39 */ "0x39",
	/* 3a */ "0x3a",
	/* 3b */ "0x3b",
	/* 3c */ "0x3c",
	/* 3d */ "0x3d",
	/* 3e */ "0x3e",
	/* 3f */ "0x3f",
};



/*****************************************************************************/

offs_t arcompact_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int size;

	uint32_t op = opcodes.r16(pc);

	uint8_t instruction = ((op & 0xf800) >> 11);

	if (instruction < 0x0c)
	{
		size = 4;
		op <<= 16;
		op |= opcodes.r16(pc+2);

		switch (instruction & 0x3f) // 32-bit instructions (with optional extra dword for immediate data)
		{
			default: break; // size = -1;
			case 0x00: // Bcc
			{
				uint8_t subinstr = (op & 0x00010000) >> 16;

				switch (subinstr & 0x01)
				{
					default: break; // size = -1;
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// B<cc><.d> s21                   0000 0sss ssss sss0   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_B_cc_D_s21(stream, pc, op, opcodes); break; // Branch Conditionally
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// B<.d> s25                       0000 0sss ssss sss1   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_B_D_s25(stream, pc, op, opcodes); break; // Branch Unconditionally Far
					}
				}
				break;
			}
			case 0x01: // BLcc/BRcc
			{
				uint8_t subinstr = (op & 0x00010000) >> 16;
				switch (subinstr & 0x01)
				{
					default: break; // size = -1;
					case 0x00: // Branch & Link
					{
						uint8_t subinstr2 = (op & 0x00020000) >> 17;
						switch (subinstr2)
						{
							default: break; // size = -1;
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BL<.cc><.d> s21                 0000 1sss ssss ss00   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_BL_cc_d_s21(stream, pc, op, opcodes); break; // Branch and Link Conditionally
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BL<.d> s25                      0000 1sss ssss ss10   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_BL_d_s25(stream, pc, op, opcodes); break; // Branch and Link Unconditional Far
							}
						}
						break;
					}
					case 0x01: // Branch on Compare
					{
						uint8_t subinstr2 = (op & 0x00000010) >> 4;

						switch (subinstr2)
						{
							default: break; // size = -1;
							case 0x00: // Branch on Compare Register-Register
							{
								uint8_t subinstr3 = op & 0x0000000f;
								switch (subinstr3)
								{
									case 0x00:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0000
// BREQ b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0000 (+ Limm)
// BREQ limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BREQ_reg_reg(stream, pc, op, opcodes); break;  // BREQ (reg-reg)
									}
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0001
// BRNE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0001 (+ Limm)
// BRNE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRNE_reg_reg(stream, pc, op, opcodes); break; // BRNE (reg-reg)
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0010
// BRLT b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0010 (+ Limm)
// BRLT limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRLT_reg_reg(stream, pc, op, opcodes); break;  // BRLT (reg-reg)
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0011
// BRGE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0011 (+ Limm)
// BRGE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRGE_reg_reg(stream, pc, op, opcodes); break; // BRGE (reg-reg)
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0100
// BRLO b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0100 (+ Limm)
// BRLO limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRLO_reg_reg(stream, pc, op, opcodes); break; // BRLO (reg-reg)
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0101 (+ Limm)
// BRHS limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0101 (+ Limm)
// BRHS<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRHS_reg_reg(stream, pc, op, opcodes); break; // BRHS (reg-reg)
									}
									case 0x0e:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BBIT0_reg_reg(stream, pc, op, opcodes); break; // BBIT0 (reg-reg)
									}
									case 0x0f:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BBIT1_reg_reg(stream, pc, op, opcodes); break;  // BBIT1 (reg-reg)
									}
									default:
									{
										// 0x06 - 0x0d
										size = handle::dasm_reserved(stream, pc, instruction, subinstr, subinstr2, subinstr3, op, opcodes); break;  // reserved
									}
								}
								break;
							}
							case 0x01: // Branch on Compare/Bit Test Register-Immediate
							{
								uint8_t subinstr3 = op & 0x0000000f;
								switch (subinstr3)
								{
									case 0x00:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BREQ_reg_imm(stream, pc, op, opcodes); break; // BREQ (reg-imm)
									}
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0001
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRNE_reg_imm(stream, pc, op, opcodes); break; // BRNE (reg-imm)
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0010
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRLT_reg_imm(stream, pc, op, opcodes); break;  // BRLT (reg-imm)
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0011
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRGE_reg_imm(stream, pc, op, opcodes); break;  // BRGE (reg-imm)
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0100
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRLO_reg_imm(stream, pc, op, opcodes); break; // BRLO (reg-imm)
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRHS_reg_imm(stream, pc, op, opcodes); break; // BRHS (reg-imm)
									}
									case 0x0e:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BBIT0_reg_imm(stream, pc, op, opcodes); break; // BBIT0 (reg-imm)
									}
									case 0x0f:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BBIT1_reg_imm(stream, pc, op, opcodes); break; // BBIT1 (reg-imm)
									}
									default:
									{
										// 0x06 - 0x0d
										size = handle::dasm_reserved(stream, pc, instruction, subinstr, subinstr2, subinstr3, op, opcodes); break;  // reserved
									}
								}
							}
						}
					}
				}
				break;
			}
			case 0x02:
			{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// LD<zz><.x><.aa><.di> a,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZXAA AAAA
// LD<zz><.x><.aa><.di> 0,[b,s9]   0001 0bbb ssss ssss   SBBB DaaZ ZX11 1110
// PREFETCH<.aa> [b,s9]            0001 0bbb ssss ssss   SBBB 0aa0 0011 1110
//
// LD<zz><.x><.di> a,[limm]        0001 0110 0000 0000   0111 DRRZ ZXAA AAAA (+ Limm)
// LD<zz><.x><.di> 0,[limm]        0001 0110 0000 0000   0111 DRRZ ZX11 1110 (+ Limm)
// PREFETCH [limm]                 0001 0110 0000 0000   0111 0RR0 0011 1110 (+ Limm)
//
// can be used as a POP alias for higher registers
// 40020310: 1404 3410
// LD.AB r16 <- [r28_SP, 004]      0001 0100 0000 0100   0011 0100 0001 0000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				size = handle::dasm32_LD_r_o(stream, pc, op, opcodes); break; // LD r+o
			}
			case 0x03:
			{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I
// ST<zz><.aa><.di> c,[b,s9]       0001 1bbb ssss ssss   SBBB CCCC CCDa aZZR
// ST<zz><.di> c,[limm]            0001 1110 0000 0000   0111 CCCC CCDR RZZR (+ Limm)
// ST<zz><.aa><.di> limm,[b,s9]    0001 1bbb ssss ssss   SBBB 1111 10Da aZZR (+ Limm)
//
// can be used as a PUSH alias for higher registers
// 400202A6: 1CFC B408
// ST.AW [r28_SP, 1fc] <- r16      0001 1100 1111 1100   1011 0100 0000 1000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				size = handle::dasm32_ST_r_o(stream, pc, op, opcodes); break; // ST r+o
			}
			case 0x04: // op a,b,c (basecase)
			{
				uint8_t subinstr = (op & 0x003f0000) >> 16;

				switch (subinstr & 0x3f)
				{
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                           PP
// General Operations Reg-Reg      0010 0bbb 00ii iiii   FBBB CCCC CCAA AAAA
// ADD<.f> a,b,c                   0010 0bbb 0000 0000   FBBB CCCC CCAA AAAA
// ADD<.f> a,limm,c                0010 0110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ADD<.f> a,b,limm                0010 0bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ADD<.f> 0,b,c                   0010 0bbb 0000 0000   FBBB CCCC CC11 1110
// ADD<.f> 0,b,limm                0010 0bbb 0000 0000   FBBB 1111 1011 1110 (+ Limm)
//
//                                           PP
// Gen Op Reg+6-bit unsigned Imm   0010 0bbb 01ii iiii   FBBB UUUU UUAA AAAA
// ADD<.f> a,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uuAA AAAA
// ADD<.f> 0,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uu11 1110
//
//                                           PP
// Gen Op Reg+12-bit signed Imm    0010 0bbb 10ii iiii   FBBB ssss ssSS SSSS
// ADD<.f> b,b,s12                 0010 0bbb 1000 0000   FBBB ssss ssSS SSSS
//
//                                           PP                      M
// Gen Op Conditional Register     0010 0bbb 11ii iiii   FBBB CCCC CC0Q QQQQ
// ADD<.cc><.f> b,b,c              0010 0bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ADD<.cc><.f> b,b,limm           0010 0bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
// ADD<.cc><.f> 0,limm,c           0010 0110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
//
//                                           PP                      M
// Gen Op ConReg 6-bit unsign Imm  0010 0bbb 11ii iiii   FBBB UUUU UU1Q QQQQ
// ADD<.cc><.f> b,b,u6             0010 0bbb 1100 0000   FBBB uuuu uu1Q QQQQ
//
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADD(stream, pc, op, opcodes); break; // ADD
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADC<.f> a,b,c                   0010 0bbb 0000 0001   FBBB CCCC CCAA AAAA
// ADC<.f> a,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uuAA AAAA
// ADC<.f> b,b,s12                 0010 0bbb 1000 0001   FBBB ssss ssSS SSSS
// ADC<.cc><.f> b,b,c              0010 0bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// ADC<.cc><.f> b,b,u6             0010 0bbb 1100 0001   FBBB uuuu uu1Q QQQQ
// ADC<.f> a,limm,c                0010 0110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// ADC<.f> a,b,limm                0010 0bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// ADC<.cc><.f> b,b,limm           0010 0bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADC<.f> 0,b,c                   0010 0bbb 0000 0001   FBBB CCCC CC11 1110
// ADC<.f> 0,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uu11 1110
// ADC<.f> 0,b,limm                0010 0bbb 0000 0001   FBBB 1111 1011 1110 (+ Limm)
// ADC<.cc><.f> 0,limm,c           0010 0110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADC(stream, pc, op, opcodes); break; // ADC
					}
					case 0x02:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUB<.f> a,b,c                   0010 0bbb 0000 0010   FBBB CCCC CCAA AAAA
// SUB<.f> a,b,u6                  0010 0bbb 0100 0010   FBBB uuuu uuAA AAAA
// SUB<.f> b,b,s12                 0010 0bbb 1000 0010   FBBB ssss ssSS SSSS
// SUB<.cc><.f> b,b, c             0010 0bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// SUB<.cc><.f> b,b,u6             0010 0bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// SUB<.f> a,limm,c                0010 0110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// SUB<.f> a,b,limm                0010 0bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// SUB<.cc><.f> b,b,limm           0010 0bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB <.f> 0,b,c                  0010 0bbb 0000 0010   FBBB CCCC CC11 1110
// SUB <.f> 0,b,u6                 0010 0bbb 0100 0010   FBBB uuuu uu11 1110
// SUB <.f> 0,b,limm               0010 0bbb 0000 0010   FBBB 1111 1011 1110 (+ Limm)
// SUB <.cc><.f> 0,limm,c          0010 0110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUB(stream, pc, op, opcodes); break; // SUB
					}
					case 0x03:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SBC<.f> a,b,c                   0010 0bbb 0000 0011   FBBB CCCC CCAA AAAA
// SBC<.f> a,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uuAA AAAA
// SBC<.f> b,b,s12                 0010 0bbb 1000 0011   FBBB ssss ssSS SSSS
// SBC<.cc><.f> b,b,c              0010 0bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// SBC<.cc><.f> b,b,u6             0010 0bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// SBC<.f> a,limm,c                0010 0110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// SBC<.f> a,b,limm                0010 0bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// SBC<.cc><.f> b,b,limm           0010 0bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// SBC<.f> 0,b,c                   0010 0bbb 0000 0011   FBBB CCCC CC11 1110
// SBC<.f> 0,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uu11 1110
// SBC<.f> 0,b,limm                0010 0bbb 0000 0011   FBBB 1111 1011 1110 (+ Limm)
// SBC<.cc><.f> 0,limm,c           0010 0110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SBC(stream, pc, op, opcodes); break; // SBC
					}
					case 0x04:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AND<.f> a,b,c                   0010 0bbb 0000 0100   FBBB CCCC CCAA AAAA
// AND<.f> a,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uuAA AAAA
// AND<.f> b,b,s12                 0010 0bbb 1000 0100   FBBB ssss ssSS SSSS
// AND<.cc><.f> b,b,c              0010 0bbb 1100 0100   FBBB CCCC CC0Q QQQQ
// AND<.cc><.f> b,b,u6             0010 0bbb 1100 0100   FBBB uuuu uu1Q QQQQ
// AND<.f> a,limm,c                0010 0110 0000 0100   F111 CCCC CCAA AAAA (+ Limm)
// AND<.f> a,b,limm                0010 0bbb 0000 0100   FBBB 1111 10AA AAAA (+ Limm)
// AND<.cc><.f> b,b,limm           0010 0bbb 1100 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// AND<.f> 0,b,c                   0010 0bbb 0000 0100   FBBB CCCC CC11 1110
// AND<.f> 0,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uu11 1110
// AND<.f> 0,b,limm                0010 0bbb 0000 0100   FBBB 1111 1011 1110 (+ Limm)
// AND<.cc><.f> 0,limm,c           0010 0110 1100 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_AND(stream, pc, op, opcodes); break; // AND
					}
					case 0x05:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OR<.f> a,b,c                    0010 0bbb 0000 0101   FBBB CCCC CCAA AAAA
// OR<.f> a,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uuAA AAAA
// OR<.f> b,b,s12                  0010 0bbb 1000 0101   FBBB ssss ssSS SSSS
// OR<.cc><.f> b,b,c               0010 0bbb 1100 0101   FBBB CCCC CC0Q QQQQ
// OR<.cc><.f> b,b,u6              0010 0bbb 1100 0101   FBBB uuuu uu1Q QQQQ
// OR<.f> a,limm,c                 0010 0110 0000 0101   F111 CCCC CCAA AAAA (+ Limm)
// OR<.f> a,b,limm                 0010 0bbb 0000 0101   FBBB 1111 10AA AAAA (+ Limm)
// OR<.cc><.f> b,b,limm            0010 0bbb 1100 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// OR<.f> 0,b,c                    0010 0bbb 0000 0101   FBBB CCCC CC11 1110
// OR<.f> 0,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uu11 1110
// OR<.f> 0,b,limm                 0010 0bbb 0000 0101   FBBB 1111 1011 1110 (+ Limm)
// OR<.cc><.f> 0,limm,c            0010 0110 1100 010  1 F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_OR(stream, pc, op, opcodes); break; // OR
					}
					case 0x06:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// BIC<.f> a,b,c                   0010 0bbb 0000 0110   FBBB CCCC CCAA AAAA
// BIC<.f> a,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uuAA AAAA
// BIC<.f> b,b,s12                 0010 0bbb 1000 0110   FBBB ssss ssSS SSSS
// BIC<.cc><.f> b,b,c              0010 0bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// BIC<.cc><.f> b,b,u6             0010 0bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// BIC<.f> a,limm,c                0010 0110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// BIC<.f> a,b,limm                0010 0bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// BIC<.cc><.f> b,b,limm           0010 0bbb 1100 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// BIC<.f> 0,b,c                   0010 0bbb 0000 0110   FBBB CCCC CC11 1110
// BIC<.f> 0,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uu11 1110
// BIC<.f> 0,b,limm                0010 0bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// BIC<.cc><.f> 0,limm,c           0010 0110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BIC(stream, pc, op, opcodes); break; // BIC
					}
					case 0x07:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// XOR<.f> a,b,c                   0010 0bbb 0000 0111   FBBB CCCC CCAA AAAA
// XOR<.f> a,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uuAA AAAA
// XOR<.f> b,b,s12                 0010 0bbb 1000 0111   FBBB ssss ssSS SSSS
// XOR<.cc><.f> b,b,c              0010 0bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// XOR<.cc><.f> b,b,u6             0010 0bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// XOR<.f> a,limm,c                0010 0110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// XOR<.f> a,b,limm                0010 0bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// XOR<.cc><.f> b,b,limm           0010 0bbb 1100 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// XOR<.f> 0,b,c                   0010 0bbb 0000 0111   FBBB CCCC CC11 1110
// XOR<.f> 0,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uu11 1110
// XOR<.f> 0,b,limm                0010 0bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// XOR<.cc><.f> 0,limm,c           0010 0110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_XOR(stream, pc, op, opcodes); break; // XOR
					}
					case 0x08:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MAX<.f> a,b,c                   0010 0bbb 0000 1000   FBBB CCCC CCAA AAAA
// MAX<.f> a,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uuAA AAAA
// MAX<.f> b,b,s12                 0010 0bbb 1000 1000   FBBB ssss ssSS SSSS
// MAX<.cc><.f> b,b,c              0010 0bbb 1100 1000   FBBB CCCC CC0Q QQQQ
// MAX<.cc><.f> b,b,u6             0010 0bbb 1100 1000   FBBB uuuu uu1Q QQQQ
// MAX<.f> a,limm,c                0010 0110 0000 1000   F111 CCCC CCAA AAAA (+ Limm)
// MAX<.f> a,b,limm                0010 0bbb 0000 1000   FBBB 1111 10AA AAAA (+ Limm)
// MAX<.cc><.f> b,b,limm           0010 0bbb 1100 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// MAX<.f> 0,b,c                   0010 0bbb 0000 1000   FBBB CCCC CC11 1110
// MAX<.f> 0,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uu11 1110
// MAX<.f> 0,b,limm                0010 0bbb 0000 1000   FBBB 1111 1011 1110 (+ Limm)
// MAX<.cc><.f> 0,limm,c           0010 0110 1100 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MAX(stream, pc, op, opcodes); break; // MAX
					}
					case 0x09:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MIN<.f> a,b,c                   0010 0bbb 0000 1001   FBBB CCCC CCAA AAAA
// MIN<.f> a,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uuAA AAAA
// MIN<.f> b,b,s12                 0010 0bbb 1000 1001   FBBB ssss ssSS SSSS
// MIN<.cc><.f> b,b,c              0010 0bbb 1100 1001   FBBB CCCC CC0Q QQQQ
// MIN<.cc><.f> b,b,u6             0010 0bbb 1100 1001   FBBB uuuu uu1Q QQQQ
// MIN<.f> a,limm,c                0010 0110 0000 1001   F111 CCCC CCAA AAAA (+ Limm)
// MIN<.f> a,b,limm                0010 0bbb 0000 1001   FBBB 1111 10AA AAAA (+ Limm)
// MIN<.cc><.f> b,b,limm           0010 0bbb 1100 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// MIN<.f> 0,b,c                   0010 0bbb 0000 1001   FBBB CCCC CC11 1110
// MIN<.f> 0,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uu11 1110
// MIN<.f> 0,b,limm                0010 0bbb 0000 1001   FBBB 1111 1011 1110 (+ Limm)
// MIN<.cc><.f> 0,limm,c           0010 0110 1100 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MIN(stream, pc, op, opcodes); break; // MIN
					}
					case 0x0a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MOV<.f> b,s12                   0010 0bbb 1000 1010   FBBB ssss ssSS SSSS
// MOV<.f> 0,s12                   0010 0110 1000 1010   F111 ssss ssSS SSSS (is b is 'Limm' there's no destination)

// MOV<.cc><.f> b,c                0010 0bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// MOV<.cc><.f> b,u6               0010 0bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// MOV<.cc><.f> b,limm             0010 0bbb 1100 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MOV<.cc><.f> 0,c                0010 0110 1100 1010   F111 CCCC CC0Q QQQQ
// MOV<.cc><.f> 0,u6               0010 0110 1100 1010   F111 uuuu uu1Q QQQQ
// MOV<.cc><.f> 0,limm             0010 0110 1100 1010   F111 1111 100Q QQQQ (+ Limm)
//
// NOP                             0010 0110 0100 1010   0111 0000 0000 0000 (NOP is a custom encoded MOV where b is 'LIMM')
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MOV(stream, pc, op, opcodes); break; // MOV
					}
					case 0x0b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// TST b,s12                       0010 0bbb 1000 1011   1BBB ssss ssSS SSSS
// TST<.cc> b,c                    0010 0bbb 1100 1011   1BBB CCCC CC0Q QQQQ
// TST<.cc> b,u6                   0010 0bbb 1100 1011   1BBB uuuu uu1Q QQQQ
// TST<.cc> b,limm                 0010 0bbb 1100 1011   1BBB 1111 100Q QQQQ (+ Limm)
// TST<.cc> limm,c                 0010 0110 1100 1011   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_TST(stream, pc, op, opcodes); break; // TST
					}
					case 0x0c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// CMP b,s12                       0010 0bbb 1000 1100   1BBB ssss ssSS SSSS
// CMP<.cc> b,c                    0010 0bbb 1100 1100   1BBB CCCC CC0Q QQQQ
// CMP<.cc> b,u6                   0010 0bbb 1100 1100   1BBB uuuu uu1Q QQQQ
// CMP<.cc> b,limm                 0010 0bbb 1100 1100   1BBB 1111 100Q QQQQ (+ Limm)
// CMP<.cc> limm,c                 0010 0110 1100 1100   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_CMP(stream, pc, op, opcodes); break; // CMP
					}
					case 0x0d:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RCMP b,s12                      0010 0bbb 1000 1101   1BBB ssss ssSS SSSS
// RCMP<.cc> b,c                   0010 0bbb 1100 1101   1BBB CCCC CC0Q QQQQ
// RCMP<.cc> b,u6                  0010 0bbb 1100 1101   1BBB uuuu uu1Q QQQQ
// RCMP<.cc> b,limm                0010 0bbb 1100 1101   1BBB 1111 100Q QQQQ (+ Limm)
// RCMP<.cc> limm,c                0010 0110 1100 1101   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_RCMP(stream, pc, op, opcodes); break; // RCMP
					}
					case 0x0e:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RSUB<.f> a,b,c                  0010 0bbb 0000 1110   FBBB CCCC CCAA AAAA
// RSUB<.f> a,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uuAA AAAA
// NEG<.f> a,b                     0010 0bbb 0100 1110   FBBB 0000 00AA AAAA (NEG is an alias)
//
// RSUB<.f> b,b,s12                0010 0bbb 1000 1110   FBBB ssss ssSS SSSS
// RSUB<.cc><.f> b,b,c             0010 0bbb 1100 1110   FBBB CCCC CC0Q QQQQ
// RSUB<.cc><.f> b,b,u6            0010 0bbb 1100 1110   FBBB uuuu uu1Q QQQQ
// NEG<.cc><.f> b,b                0010 0bbb 1100 1110   FBBB 0000 001Q QQQQ (NEG is an alias)
//
// RSUB<.f> a,limm,c               0010 0110 0000 1110   F111 CCCC CCAA AAAA (+ Limm)
// RSUB<.f> a,b,limm               0010 0bbb 0000 1110   FBBB 1111 10AA AAAA (+ Limm)
// RSUB<.cc><.f> b,b,limm          0010 0bbb 1100 1110   FBBB 1111 100Q QQQQ (+ Limm)
//
// RSUB<.f> 0,b,c                  0010 0bbb 0000 1110   FBBB CCCC CC11 1110
// RSUB<.f> 0,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uu11 1110
// RSUB<.f> 0,b,limm               0010 0bbb 0000 1110   FBBB 1111 1011 1110 (+ Limm)
// RSUB<.cc><.f> 0,limm,c          0010 0110 1100 1110   F111 CCCC CC0Q QQQQ (+ Limm)
//
//                                 IIII I      SS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_RSUB(stream, pc, op, opcodes); break; // RSUB
					}
					case 0x0f:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BSET<.f> a,b,c                  0010 0bbb 0000 1111   FBBB CCCC CCAA AAAA
// BSET<.f> a,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uuAA AAAA
// BSET<.cc><.f> b,b,c             0010 0bbb 1100 1111   FBBB CCCC CC0Q QQQQ
// BSET<.cc><.f> b,b,u6            0010 0bbb 1100 1111   FBBB uuuu uu1Q QQQQ
// BSET<.f> a,limm,c               0010 0110 0000 1111   F111 CCCC CCAA AAAA (+ Limm)
//
// BSET<.f> 0,b,c                  0010 0bbb 0000 1111   FBBB CCCC CC11 1110
// BSET<.f> 0,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uu11 1110
// BSET<.cc><.f> 0,limm,c          0010 0110 1100 1111   F110 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BSET(stream, pc, op, opcodes); break; // BSET
					}
					case 0x10:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BCLR<.f> a,b,c                  0010 0bbb 0001 0000   FBBB CCCC CCAA AAAA
// BCLR<.f> a,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uuAA AAAA
// BCLR<.cc><.f> b,b,c             0010 0bbb 1101 0000   FBBB CCCC CC0Q QQQQ
// BCLR<.cc><.f> b,b,u6            0010 0bbb 1101 0000   FBBB uuuu uu1Q QQQQ
// BCLR<.f> a,limm,c               0010 0110 0001 0000   F111 CCCC CCAA AAAA (+ Limm)
//
// BCLR<.f> 0,b,c                  0010 0bbb 0001 0000   FBBB CCCC CC11 1110
// BCLR<.f> 0,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uu11 1110
// BCLR<.cc><.f> 0,limm,c          0010 0110 1101 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BCLR(stream, pc, op, opcodes); break; // BCLR
					}
					case 0x11:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BTST<.cc> b,c                   0010 0bbb 1101 0001   1BBB CCCC CC0Q QQQQ
// BTST<.cc> b,u6                  0010 0bbb 1101 0001   1BBB uuuu uu1Q QQQQ
// BTST<.cc> limm,c                0010 0110 1101 0001   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BTST(stream, pc, op, opcodes); break; // BTST
					}
					case 0x12:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BXOR<.f> a,b,c                  0010 0bbb 0001 0010   FBBB CCCC CCAA AAAA
// BXOR<.f> a,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uuAA AAAA
// BXOR<.cc><.f> b,b,c             0010 0bbb 1101 0010   FBBB CCCC CC0Q QQQQ
// BXOR<.cc><.f> b,b,u6            0010 0bbb 1101 0010   FBBB uuuu uu1Q QQQQ
// BXOR<.f> a,limm,c               0010 0110 0001 0010   F111 CCCC CCAA AAAA (+ Limm)
//
// BXOR<.f> 0,b,c                  0010 0bbb 0001 0010   FBBB CCCC CC11 1110
// BXOR<.f> 0,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uu11 1110
// BXOR<.cc><.f> 0,limm,c          0010 0110 1101 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BXOR(stream, pc, op, opcodes); break; // BXOR
					}
					case 0x13:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BMSK<.f> a,b,c                  0010 0bbb 0001 0011   FBBB CCCC CCAA AAAA
// BMSK<.f> a,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uuAA AAAA
// BMSK<.cc><.f> b,b,c             0010 0bbb 1101 0011   FBBB CCCC CC0Q QQQQ
// BMSK<.cc><.f> b,b,u6            0010 0bbb 1101 0011   FBBB uuuu uu1Q QQQQ
// BMSK<.f> a,limm,c               0010 0110 0001 0011   F111 CCCC CCAA AAAA (+ Limm)
//
// BMSK<.f> 0,b,c                  0010 0bbb 0001 0011   FBBB CCCC CC11 1110
// BMSK<.f> 0,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uu11 1110
// BMSK<.cc><.f> 0,limm,c          0010 0110 1101 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_BMSK(stream, pc, op, opcodes); break; // BMSK
					}
					case 0x14:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD1<.f> a,b,c                  0010 0bbb 0001 0100   FBBB CCCC CCAA AAAA
// ADD1<.f> a,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uuAA AAAA
// ADD1<.f> b,b,s12                0010 0bbb 1001 0100   FBBB ssss ssSS SSSS
// ADD1<.cc><.f> b,b,c             0010 0bbb 1101 0100   FBBB CCCC CC0Q QQQQ
// ADD1<.cc><.f> b,b,u6            0010 0bbb 1101 0100   FBBB uuuu uu1Q QQQQ
// ADD1<.f> a,limm,c               0010 0110 0001 0100   F111 CCCC CCAA AAAA (+ Limm)
// ADD1<.f> a,b,limm               0010 0bbb 0001 0100   FBBB 1111 10AA AAAA (+ Limm)
// ADD1<.cc><.f> b,b,limm          0010 0bbb 1101 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD1<.f> 0,b,c                  0010 0bbb 0001 0100   FBBB CCCC CC11 1110
// ADD1<.f> 0,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uu11 1110
// ADD1<.f> 0,b,limm               0010 0bbb 0001 0100   FBBB 1111 1011 1110 (+ Limm)
// ADD1<.cc><.f> 0,limm,c          0010 0110 1101 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADD1(stream, pc, op, opcodes); break; // ADD1
					}
					case 0x15:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD2<.f> a,b,c                  0010 0bbb 0001 0101   FBBB CCCC CCAA AAAA
// ADD2<.f> a,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uuAA AAAA
// ADD2<.f> b,b,s12                0010 0bbb 1001 0101   FBBB ssss ssSS SSSS
// ADD2<.cc><.f> b,b,c             0010 0bbb 1101 0101   FBBB CCCC CC0Q QQQQ
// ADD2<.cc><.f> b,b,u6            0010 0bbb 1101 0101   FBBB uuuu uu1Q QQQQ
// ADD2<.f> a,limm,c               0010 0110 0001 0101   F111 CCCC CCAA AAAA (+ Limm)
// ADD2<.f> a,b,limm               0010 0bbb 0001 0101   FBBB 1111 10AA AAAA (+ Limm)
// ADD2<.cc><.f> b,b,limm          0010 0bbb 1101 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD2<.f> 0,b,c                  0010 0bbb 0001 0101   FBBB CCCC CC11 1110
// ADD2<.f> 0,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uu11 1110
// ADD2<.f> 0,b,limm               0010 0bbb 0001 0101   FBBB 1111 1011 1110 (+ Limm)
// ADD2<.cc><.f> 0,limm,c          0010 0110 1101 0101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADD2(stream, pc, op, opcodes); break; // ADD2
					}
					case 0x16:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD3<.f> a,b,c                  0010 0bbb 0001 0110   FBBB CCCC CCAA AAAA
// ADD3<.f> a,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uuAA AAAA
// ADD3<.f> b,b,s12                0010 0bbb 1001 0110   FBBB ssss ssSS SSSS
// ADD3<.cc><.f> b,b,c             0010 0bbb 1101 0110   FBBB CCCC CC0Q QQQQ
// ADD3<.cc><.f> b,b,u6            0010 0bbb 1101 0110   FBBB uuuu uu1Q QQQQ
// ADD3<.f> a,limm,c               0010 0110 0001 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADD3<.f> a,b,limm               0010 0bbb 0001 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADD3<.cc><.f> b,b,limm          0010 0bbb 1101 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD3<.f> 0,b,c                  0010 0bbb 0001 0110   FBBB CCCC CC11 1110
// ADD3<.f> 0,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uu11 1110
// ADD3<.f> 0,b,limm               0010 0bbb 0001 0110   FBBB 1111 1011 1110 (+ Limm)
// ADD3<.cc><.f> 0,limm,c          0010 0110 1101 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADD3(stream, pc, op, opcodes); break; // ADD3
					}
					case 0x17:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB1<.f> a,b,c                  0010 0bbb 0001 0111   FBBB CCCC CCAA AAAA
// SUB1<.f> a,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uuAA AAAA
// SUB1<.f> b,b,s12                0010 0bbb 1001 0111   FBBB ssss ssSS SSSS
// SUB1<.cc><.f> b,b,c             0010 0bbb 1101 0111   FBBB CCCC CC0Q QQQQ
// SUB1<.cc><.f> b,b,u6            0010 0bbb 1101 0111   FBBB uuuu uu1Q QQQQ
// SUB1<.f> a,limm,c               0010 0110 0001 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUB1<.f> a,b,limm               0010 0bbb 0001 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUB1<.cc><.f> b,b,limm          0010 0bbb 1101 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB1<.f> 0,b,c                  0010 0bbb 0001 0111   FBBB CCCC CC11 1110
// SUB1<.f> 0,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uu11 1110
// SUB1<.f> 0,b,limm               0010 0bbb 0001 0111   FBBB 1111 1011 1110 (+ Limm)
// SUB1<.cc><.f> 0,limm,c          0010 0110 1101 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUB1(stream, pc, op, opcodes); break; // SUB1
					}
					case 0x18:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB2<.f> a,b,c                  0010 0bbb 0001 1000   FBBB CCCC CCAA AAAA
// SUB2<.f> a,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uuAA AAAA
// SUB2<.f> b,b,s12                0010 0bbb 1001 1000   FBBB ssss ssSS SSSS
// SUB2<.cc><.f> b,b,c             0010 0bbb 1101 1000   FBBB CCCC CC0Q QQQQ
// SUB2<.cc><.f> b,b,u6            0010 0bbb 1101 1000   FBBB uuuu uu1Q QQQQ
// SUB2<.f> a,limm,c               0010 0110 0001 1000   F111 CCCC CCAA AAAA (+ Limm)
// SUB2<.f> a,b,limm               0010 0bbb 0001 1000   FBBB 1111 10AA AAAA (+ Limm)
// SUB2<.cc><.f> b,b,limm          0010 0bbb 1101 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB2<.f> 0,b,c                  0010 0bbb 0001 1000   FBBB CCCC CC11 1110
// SUB2<.f> 0,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uu11 1110
// SUB2<.f> 0,b,limm               0010 0bbb 0001 1000   FBBB 1111 1011 1110 (+ Limm)
// SUB2<.cc><.f> 0,limm,c          0010 0110 1101 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUB2(stream, pc, op, opcodes); break; // SUB2
					}
					case 0x19:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB3<.f> a,b,c                  0010 0bbb 0001 1001   FBBB CCCC CCAA AAAA
// SUB3<.f> a,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uuAA AAAA
// SUB3<.f> b,b,s12                0010 0bbb 1001 1001   FBBB ssss ssSS SSSS
// SUB3<.cc><.f> b,b,c             0010 0bbb 1101 1001   FBBB CCCC CC0Q QQQQ
// SUB3<.cc><.f> b,b,u6            0010 0bbb 1101 1001   FBBB uuuu uu1Q QQQQ
// SUB3<.f> a,limm,c               0010 0110 0001 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUB3<.f> a,b,limm               0010 0bbb 0001 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUB3<.cc><.f> b,b,limm          0010 0bbb 1101 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB3<.f> 0,b,c                  0010 0bbb 0001 1001   FBBB CCCC CC11 1110
// SUB3<.f> 0,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uu11 1110
// SUB3<.f> 0,limm,c               0010 0110 0001 1001   F111 CCCC CC11 1110 (+ Limm)
// SUB3<.cc><.f> 0,limm,c          0010 0110 1101 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUB3(stream, pc, op, opcodes); break; // SUB3
					}
					case 0x1a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPY<.f> a,b,c                   0010 0bbb 0001 1010   FBBB CCCC CCAA AAAA
// MPY<.f> a,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uuAA AAAA
// MPY<.f> b,b,s12                 0010 0bbb 1001 1010   FBBB ssss ssSS SSSS
// MPY<.cc><.f> b,b,c              0010 0bbb 1101 1010   FBBB CCCC CC0Q QQQQ
// MPY<.cc><.f> b,b,u6             0010 0bbb 1101 1010   FBBB uuuu uu1Q QQQQ
// MPY<.f> a,limm,c                0010 0110 0001 1010   F111 CCCC CCAA AAAA (+ Limm)
// MPY<.f> a,b,limm                0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPY<.cc><.f> b,b,limm           0010 0bbb 1101 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPY<.f> 0,b,c                   0010 0bbb 0001 1010   FBBB CCCC CC11 1110
// MPY<.f> 0,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uu11 1110
// MPY<.cc><.f> 0,limm,c           0010 0110 1101 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MPY(stream, pc, op, opcodes); break; // MPY *
					}
					case 0x1b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPYH<.f> a,b,c                  0010 0bbb 0001 1011   FBBB CCCC CCAA AAAA
// MPYH<.f> a,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uuAA AAAA
// MPYH<.f> b,b,s12                0010 0bbb 1001 1011   FBBB ssss ssSS SSSS
// MPYH<.cc><.f> b,b,c             0010 0bbb 1101 1011   FBBB CCCC CC0Q QQQQ
// MPYH<.cc><.f> b,b,u6            0010 0bbb 1101 1011   FBBB uuuu uu1Q QQQQ
// MPYH<.f> a,limm,c               0010 0110 0001 1011   F111 CCCC CCAA AAAA (+ Limm)
// MPYH<.f> a,b,limm               0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPYH<.cc><.f> b,b,limm          0010 0bbb 1101 1011   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYH<.f> 0,b,c                  0010 0bbb 0001 1011   FBBB CCCC CC11 1110
// MPYH<.f> 0,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uu11 1110
// MPYH<.cc><.f> 0,limm,c          0010 0110 1101 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MPYH(stream, pc, op, opcodes); break; // MPYH *
					}
					case 0x1c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MPYHU<.f> a,b,c                 0010 0bbb 0001 1100   FBBB CCCC CCAA AAAA
// MPYHU<.f> a,b,u6                0010 0bbb 0101 1100   FBBB uuuu uuAA AAAA
// MPYHU<.f> b,b,s12               0010 0bbb 1001 1100   FBBB ssss ssSS SSSS
// MPYHU<.cc><.f> b,b,c            0010 0bbb 1101 1100   FBBB CCCC CC0Q QQQQ
// MPYHU<.cc><.f> b,b,u6           0010 0bbb 1101 1100   FBBB uuuu uu1Q QQQQ
// MPYHU<.f> a,limm,c              0010 0110 0001 1100   F111 CCCC CCAA AAAA (+ Limm)
// MPYHU<.f> a,b,limm              0010 0bbb 0001 1100   FBBB 1111 10AA AAAA (+ Limm)
// MPYHU<.cc><.f> b,b,limm         0010 0bbb 1101 1100   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYHU<.f> 0,b,c                 0010 0bbb 0001 1100   FBBB CCCC CC11 1110
// MPYHU<.f> 0,b,u6                0010 0bbb 0101 1100   FBBB uuuu uu11 1110
// MPYHU<.cc><.f> 0,limm,c         0010 0110 1101 1100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MPYHU(stream, pc, op, opcodes); break; // MPYHU *
					}
					case 0x1d:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MPYU<.f> a,b,c                  0010 0bbb 0001 1101   FBBB CCCC CCAA AAAA
// MPYU<.f> a,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uuAA AAAA
// MPYU<.f> b,b,s12                0010 0bbb 1001 1101   FBBB ssss ssSS SSSS
// MPYU<.cc><.f> b,b,c             0010 0bbb 1101 1101   FBBB CCCC CC0Q QQQQ
// MPYU<.cc><.f> b,b,u6            0010 0bbb 1101 1101   FBBB uuuu uu1Q QQQQ
// MPYU<.f> a,limm,c               0010 0110 0001 1101   F111 CCCC CCAA AAAA (+ Limm)
// MPYU<.f> a,b,limm               0010 0bbb 0001 1101   FBBB 1111 10AA AAAA (+ Limm)
// MPYU<.cc><.f> b,b,limm          0010 0bbb 1101 1101   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYU<.f> 0,b,c                  0010 0bbb 0001 1101   FBBB CCCC CC11 1110
// MPYU<.f> 0,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uu11 1110
// MPYU<.cc><.f> 0,limm,c          0010 0110 1101 1101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MPYU(stream, pc, op, opcodes); break; // MPYU *
					}
					case 0x20:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc [c]                         0010 0RRR 1110 0000   0RRR CCCC CC0Q QQQQ
// Jcc limm                        0010 0RRR 1110 0000   0RRR 1111 100Q QQQQ (+ Limm)
// Jcc u6                          0010 0RRR 1110 0000   0RRR uuuu uu1Q QQQQ
// Jcc.F [ilink1]                  0010 0RRR 1110 0000   1RRR 0111 010Q QQQQ
// Jcc.F [ilink2]                  0010 0RRR 1110 0000   1RRR 0111 100Q QQQQ
//                                 IIII I      SS SSSS
// J [c]                           0010 0RRR 0010 0000   0RRR CCCC CCRR RRRR
// J.F [ilink1]                    0010 0RRR 0010 0000   1RRR 0111 01RR RRRR
// J.F [ilink2]                    0010 0RRR 0010 0000   1RRR 0111 10RR RRRR
// J limm                          0010 0RRR 0010 0000   0RRR 1111 10RR RRRR (+ Limm)
// J u6                            0010 0RRR 0110 0000   0RRR uuuu uuRR RRRR
// J s12                           0010 0RRR 1010 0000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_Jcc(stream, pc, op, opcodes); break; // Jcc
					}
					case 0x21:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc.D u6                        0010 0RRR 1110 0001   0RRR uuuu uu1Q QQQQ
// Jcc.D [c]                       0010 0RRR 1110 0001   0RRR CCCC CC0Q QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// J.D [c]                         0010 0RRR 0010 0001   0RRR CCCC CCRR RRRR
// J.D u6                          0010 0RRR 0110 0001   0RRR uuuu uuRR RRRR
// J.D s12                         0010 0RRR 1010 0001   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

						size = handle::dasm32_Jcc_D(stream, pc, op, opcodes); break; // Jcc.D
					}
					case 0x22:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc [c]                        0010 0RRR 1110 0010   0RRR CCCC CC0Q QQQQ
// JLcc limm                       0010 0RRR 1110 0010   0RRR 1111 100Q QQQQ (+ Limm)
// JLcc u6                         0010 0RRR 1110 0010   0RRR uuuu uu1Q QQQQ
// JL [c]                          0010 0RRR 0010 0010   0RRR CCCC CCRR RRRR
// JL limm                         0010 0RRR 0010 0010   0RRR 1111 10RR RRRR (+ Limm)
// JL u6                           0010 0RRR 0110 0010   0RRR uuuu uuRR RRRR
// JL s12                          0010 0RRR 1010 0010   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_JLcc(stream, pc, op, opcodes); break; // JLcc
					}
					case 0x23:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc.D u6                       0010 0RRR 1110 0011   0RRR uuuu uu1Q QQQQ
// JLcc.D [c]                      0010 0RRR 1110 0011   0RRR CCCC CC0Q QQQQ
// JL.D [c]                        0010 0RRR 0010 0011   0RRR CCCC CCRR RRRR
// JL.D u6                         0010 0RRR 0110 0011   0RRR uuuu uuRR RRRR
// JL.D s12                        0010 0RRR 1010 0011   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_JLcc_D(stream, pc, op, opcodes); break; // JLcc.D
					}
					case 0x28:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LP<cc> u7                       0010 0RRR 1110 1000   0RRR uuuu uu1Q QQQQ
// LP s13                          0010 0RRR 1010 1000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_LP(stream, pc, op, opcodes); break; // LPcc
					}
					case 0x29:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
//                                           PP
// General Operations Reg-Reg      0010 0bbb 00ii iiii   FBBB CCCC CCAA AAAA
// FLAG c                          0010 0000 0010 1001   0000 0000 0100 0000 (Leapster BIOS uses this redundant encoding where A is unused?)
//
//                                           PP
// Gen Op Reg+6-bit unsigned Imm   0010 0bbb 01ii iiii   FBBB UUUU UUAA AAAA
// no listed FLAG encodings
//
//                                           PP
// Gen Op Reg+12-bit signed Imm    0010 0bbb 10ii iiii   FBBB ssss ssSS SSSS
// FLAG s12                        0010 0rrr 1010 1001   0RRR ssss ssSS SSSS
//
//                                           PP                      M
// Gen Op Conditional Register     0010 0bbb 11ii iiii   FBBB CCCC CC0Q QQQQ
// FLAG<.cc> c                     0010 0rrr 1110 1001   0RRR CCCC CC0Q QQQQ
// FLAG<.cc> limm                  0010 0rrr 1110 1001   0RRR 1111 100Q QQQQ (+ Limm)
//
//                                           PP                      M
// Gen Op ConReg 6-bit unsign Imm  0010 0bbb 11ii iiii   FBBB UUUU UU1Q QQQQ
// FLAG<.cc> u6                    0010 0rrr 1110 1001   0RRR uuuu uu1Q QQQQ
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_FLAG(stream, pc, op, opcodes); break; // FLAG
					}
					case 0x2a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LR b,[c]                        0010 0bbb 0010 1010   0BBB CCCC CCRR RRRR
// LR b,[limm]                     0010 0bbb 0010 1010   0BBB 1111 10RR RRRR (+ Limm)
// LR b,[u6]                       0010 0bbb 0110 1010   0BBB uuuu uu00 0000
// LR b,[s12]                      0010 0bbb 1010 1010   0BBB ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_LR(stream, pc, op, opcodes); break; // LR
					}
					case 0x2b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SR b,[c]                        0010 0bbb 0010 1011   0BBB CCCC CCRR RRRR
// SR b,[limm]                     0010 0bbb 0010 1011   0BBB 1111 10RR RRRR (+ Limm)
// SR b,[u6]                       0010 0bbb 0110 1011   0BBB uuuu uu00 0000
// SR b,[s12]                      0010 0bbb 1010 1011   0BBB ssss ssSS SSSS
// SR limm,[c]                     0010 0110 0010 1011   0111 CCCC CCRR RRRR (+ Limm)
// SR limm,[u6]                    0010 0110 0110 1011   0111 uuuu uu00 0000
// SR limm,[s12]                   0010 0110 1010 1011   0111 ssss ssSS SSSS (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SR(stream, pc, op, opcodes); break; // SR
					}
					case 0x2f: // Sub Opcode
					{
						uint8_t subinstr2 = op & 0x0000003f;
						switch (subinstr2 & 0x3f)
						{
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASL<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0000
// ASL<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0000
// ASL<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// ASL<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0000
// ASL<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0000
// ASL<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ASL_single(stream, pc, op, opcodes); break; // ASL
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0001
// ASR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0001
// ASR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// ASR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0001
// ASR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0001
// ASR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ASR_single(stream, pc, op, opcodes); break; // ASR
							}
							case 0x02:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// LSR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0010
// LSR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0010
// LSR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// LSR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0010
// LSR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0010
// LSR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_LSR_single(stream, pc, op, opcodes); break; // LSR
							}
							case 0x03:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ROR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0011
// ROR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0011
// ROR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// ROR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0011
// ROR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0011
// ROR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ROR_single(stream, pc, op, opcodes); break; // ROR
							}
							case 0x04:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// RRC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0100
// RRC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0100
// RRC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// RRC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0100
// RRC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0100
// RRC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_RRC(stream, pc, op, opcodes); break; // RCC
							}
							case 0x05:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0101
// SEXB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0101
// SEXB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// SEXB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0101
// SEXB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0101
// SEXB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_SEXB(stream, pc, op, opcodes); break; // SEXB
							}
							case 0x06:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0110
// SEXW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0110
// SEXW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// SEXW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0110
// SEXW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0110
// SEXW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_SEXW(stream, pc, op, opcodes); break; // SEXW
							}
							case 0x07:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0111
// EXTB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0111
// EXTB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// EXTB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0111
// EXTB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0111
// EXTB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_EXTB(stream, pc, op, opcodes); break; // EXTB
							}
							case 0x08:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 1000
// EXTW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 1000
// EXTW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// EXTW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 1000
// EXTW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 1000
// EXTW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_EXTW(stream, pc, op, opcodes); break; // EXTW
							}
							case 0x09:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ABS<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1001
// ABS<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1001
// ABS<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1001 (+ Limm)
//
// ABS<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1001
// ABS<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1001
// ABS<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ABS(stream, pc, op, opcodes); break; // ABS
							}
							case 0x0a:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// NOT<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1010
// NOT<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1010
// NOT<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1010 (+ Limm)
//
// NOT<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1010
// NOT<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1010
// NOT<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1010 (+ Limm)
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_NOT(stream, pc, op, opcodes); break; // NOT
							}
							case 0x0b:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// RLC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1011
// RLC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1011
// RLC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1011 (+ Limm)
//
// RLC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1011
// RLC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1011
// RLC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_RLC(stream, pc, op, opcodes); break; // RLC
							}
							case 0x0c:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EX<.di> b,[c]                   0010 0bbb 0010 1111   DBBB CCCC CC00 1100
// EX<.di> b,[u6]                  0010 0bbb 0110 1111   DBBB uuuu uu00 1100
// EX<.di> b,[limm]                0010 0bbb 0010 1111   DBBB 1111 1000 1100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_EX(stream, pc, op, opcodes); break; // EX
							}
							case 0x3f: // ZOPs (Zero Operand Opcodes)
							{
								uint8_t subinstr3 = (op & 0x07000000) >> 24;
								subinstr3 |= ((op & 0x00007000) >> 12) << 3;

								switch (subinstr3 & 0x3f)
								{
									case 0x01:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SLEEP <u6>                      0010 0001 0110 1111   0000 uuuu uu11 1111
// SLEEP c                         0010 0001 0010 1111   0000 CCCC CC11 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_SLEEP(stream, pc, op, opcodes); break; // SLEEP
									}
									case 0x02:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SWI/TRAP0                       0010 0010 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_SWI(stream, pc, op, opcodes); break; // SWI / TRAP9
									}
									case 0x03:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// SYNC                            0010 0011 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_SYNC(stream, pc, op, opcodes); break; // SYNC
									}
									case 0x04:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// RTIE                            0010 0100 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_RTIE(stream, pc, op, opcodes); break; // RTIE
									}
									case 0x05:
									{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I$$$   SS SSSS    $$$        ss ssss
// BRK                             0010 0101 0110 1111   0000 0000 0011 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
										size = handle::dasm32_BRK(stream, pc, op, opcodes); break; // BRK
									}

									default:
									{
										// 0x00, 0x06-0x3f
										size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, subinstr3, op, opcodes); break;  // illegal
									}
								}
								break;
							}
							default:
							{
								// 0x0d - 0x3e
								size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;  // illegal
							}
						}
						break;
					}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LD<zz><.x><.aa><.di> a,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CCAA AAAA
// LD<zz><.x><.aa><.di> 0,[b,c]    0010 0bbb aa11 0ZZX   DBBB CCCC CC11 1110
// PREFETCH<.aa> [b,c]             0010 0bbb aa11 0000   0BBB CCCC CC11 1110 (ZZXD is 0) (prefetch is an alias)
//
// LD<zz><.x><.aa><.di> a,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 10AA AAAA (+ Limm) (C is 62)
// LD<zz><.x><.aa><.di> 0,[b,limm] 0010 0bbb aa11 0ZZX   DBBB 1111 1011 1110 (+ Limm) (C is 62)
// PREFETCH<.aa> [b,limm]          0010 0bbb aa11 0000   0BBB 1111 1011 1110 (+ Limm) (C is 62) (ZZXD is 0) (prefetch is an alias)
//
// if b is 62 (Limm) then aa becomes RR (reserved)
// LD<zz><.x><.di> a,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CCAA AAAA (+ Limm) (b is 62)
// LD<zz><.x><.di> 0,[limm,c]      0010 0110 RR11 0ZZX   D111 CCCC CC11 1110 (+ Limm) (b is 62)
// PREFETCH [limm,c]               0010 0110 RR11 0000   0111 CCCC CC11 1110 (+ Limm) (b is 62) (ZZXD is 0) (prefetch is an alias)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					case 0x30: size = handle::dasm32_LD_0(stream, pc, op, opcodes); break; // LD r-r
					case 0x31: size = handle::dasm32_LD_1(stream, pc, op, opcodes); break; // LD r-r
					case 0x32: size = handle::dasm32_LD_2(stream, pc, op, opcodes); break; // LD r-r
					case 0x33: size = handle::dasm32_LD_3(stream, pc, op, opcodes); break; // LD r-r
					case 0x34: size = handle::dasm32_LD_4(stream, pc, op, opcodes); break; // LD r-r
					case 0x35: size = handle::dasm32_LD_5(stream, pc, op, opcodes); break; // LD r-r
					case 0x36: size = handle::dasm32_LD_6(stream, pc, op, opcodes); break; // LD r-r
					case 0x37: size = handle::dasm32_LD_7(stream, pc, op, opcodes); break; // LD r-r
					default:
					{
						size = handle::dasm_illegal(stream, pc, instruction, subinstr, op, opcodes); break;  // illegal
					}
				}
				break;
			}
			case 0x05: // op a,b,c (05 ARC ext)
			{
				uint8_t subinstr = (op & 0x003f0000) >> 16;

				switch (subinstr)
				{
					case 0x00:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ASL<.f> a,b,c                   0010 1bbb 0000 0000   FBBB CCCC CCAA AAAA
// ASL<.f> a,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uuAA AAAA
// ASL<.f> b,b,s12                 0010 1bbb 1000 0000   FBBB ssss ssSS SSSS
// ASL<.cc><.f> b,b,c              0010 1bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ASL<.cc><.f> b,b,u6             0010 1bbb 1100 0000   FBBB uuuu uu1Q QQQQ
// ASL<.f> a,limm,c                0010 1110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ASL<.f> a,b,limm                0010 1bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ASL<.cc><.f> b,b,limm           0010 1bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASL<.f> 0,b,c                   0010 1bbb 0000 0000   FBBB CCCC CC11 1110
// ASL<.f> 0,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uu11 1110
// ASL<.cc><.f> 0,limm,c           0010 1110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ASL_multiple(stream, pc, op, opcodes); break; // ASL
					}
					case 0x01:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSR<.f> a,b,c                   0010 1bbb 0000 0001   FBBB CCCC CCAA AAAA
// LSR<.f> a,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uuAA AAAA
// LSR<.f> b,b,s12                 0010 1bbb 1000 0001   FBBB ssss ssSS SSSS
// LSR<.cc><.f> b,b,c              0010 1bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// LSR<.cc><.f> b,b,u6             0010 1bbb 1100 0001   FBBB uuuu uu1Q QQQQ
// LSR<.f> a,limm,c                0010 1110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// LSR<.f> a,b,limm                0010 1bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// LSR<.cc><.f> b,b,limm           0010 1bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
//
// LSR<.f> 0,b,c                   0010 1bbb 0000 0001   FBBB CCCC CC11 1110
// LSR<.f> 0,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uu11 1110
// LSR<.cc><.f> 0,limm,c           0010 1110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_LSR_multiple(stream, pc, op, opcodes); break; // LSR
					}
					case 0x02:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ASR<.f> a,b,c                   0010 1bbb 0000 0010   FBBB CCCC CCAA AAAA
// ASR<.f> a,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uuAA AAAA
// ASR<.f> b,b,s12                 0010 1bbb 1000 0010   FBBB ssss ssSS SSSS
// ASR<.cc><.f> b,b,c              0010 1bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// ASR<.cc><.f> b,b,u6             0010 1bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// ASR<.f> a,limm,c                0010 1110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// ASR<.f> a,b,limm                0010 1bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// ASR<.cc><.f> b,b,limm           0010 1bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASR<.f> 0,b,c                   0010 1bbb 0000 0010   FBBB CCCC CC11 1110
// ASR<.f> 0,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uu11 1110
// ASR<.cc><.f> 0,limm,c           0010 1110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ASR_multiple(stream, pc, op, opcodes); break; // ASR
					}
					case 0x03:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ROR<.f> a,b,c                   0010 1bbb 0000 0011   FBBB CCCC CCAA AAAA
// ROR<.f> a,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uuAA AAAA
// ROR<.f> b,b,s12                 0010 1bbb 1000 0011   FBBB ssss ssSS SSSS
// ROR<.cc><.f> b,b,c              0010 1bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// ROR<.cc><.f> b,b,u6             0010 1bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// ROR<.f> a,limm,c                0010 1110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// ROR<.f> a,b,limm                0010 1bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// ROR<.cc><.f> b,b,limm           0010 1bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// ROR<.f> 0,b,c                   0010 1bbb 0000 0011   FBBB CCCC CC11 1110
// ROR<.f> 0,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uu11 1110
// ROR<.cc><.f> 0,limm,c           0010 1110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ROR_multiple(stream, pc, op, opcodes); break; // ROR
					}
					case 0x04:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MUL64 <0,>b,c                   0010 1bbb 0000 0100   0BBB CCCC CC11 1110
// MUL64 <0,>b,u6                  0010 1bbb 0100 0100   0BBB uuuu uu11 1110
// MUL64 <0,>b,s12                 0010 1bbb 1000 0100   0BBB ssss ssSS SSSS
// MUL64 <0,>limm,c                0010 1110 0000 0100   0111 CCCC CC11 1110 (+ Limm)
//
// MUL64<.cc> <0,>b,c              0010 1bbb 1100 0100   0BBB CCCC CC0Q QQQQ
// MUL64<.cc> <0,>b,u6             0010 1bbb 1100 0100   0BBB uuuu uu1Q QQQQ
// MUL64<.cc> <0,>limm,c           0010 1110 1100 0100   0111 CCCC CC0Q QQQQ (+ Limm)
// MUL64<.cc> <0,>b,limm           0010 1bbb 1100 0100   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MUL64(stream, pc, op, opcodes); break; // MUL64
					}
					case 0x05:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MULU64 <0,>b,c                  0010 1bbb 0000 0101   0BBB CCCC CC11 1110
// MULU64 <0,>b,u6                 0010 1bbb 0100 0101   0BBB uuuu uu11 1110
// MULU64 <0,>b,s12                0010 1bbb 1000 0101   0BBB ssss ssSS SSSS
// MULU64 <0,>limm,c               0010 1110 0000 0101   0111 CCCC CC11 1110 (+ Limm)
//
// MULU64<.cc> <0,>b,c             0010 1bbb 1100 0101   0BBB CCCC CC0Q QQQQ
// MULU64<.cc> <0,>b,u6            0010 1bbb 1100 0101   0BBB uuuu uu1Q QQQQ
// MULU64<.cc> <0,>limm,c          0010 1110 1100 0101   0111 CCCC CC0Q QQQQ (+ Limm)
// MULU64<.cc> <0,>b,limm          0010 1bbb 1100 0101   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_MULU64(stream, pc, op, opcodes); break; // MULU64
					}
					case 0x06:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADDS<.f> a,b,c                  0010 1bbb 0000 0110   FBBB CCCC CCAA AAAA
// ADDS<.f> a,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uuAA AAAA
// ADDS<.f> b,b,s12                0010 1bbb 1000 0110   FBBB ssss ssSS SSSS
// ADDS<.cc><.f> b,b,c             0010 1bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// ADDS<.cc><.f> b,b,u6            0010 1bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// ADDS<.f> a,limm,c               0010 1110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADDS<.f> a,b,limm               0010 1bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADDS<.cc><.f> b,b,limm          0010 1bbb 1100 0110   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDS<.f> 0,b,c                  0010 1bbb 0000 0110   FBBB CCCC CC11 1110
// ADDS<.f> 0,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uu11 1110
// ADDS<.f> 0,b,limm               0010 1bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// ADDS<.cc><.f> 0,limm,c          0010 1110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADDS(stream, pc, op, opcodes); break; // ADDS
					}
					case 0x07:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUBS<.f> a,b,c                  0010 1bbb 0000 0111   FBBB CCCC CCAA AAAA
// SUBS<.f> a,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uuAA AAAA
// SUBS<.f> b,b,s12                0010 1bbb 1000 0111   FBBB ssss ssSS SSSS
// SUBS<.cc><.f> b,b,c             0010 1bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// SUBS<.cc><.f> b,b,u6            0010 1bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// SUBS<.f> a,limm,c               0010 1110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUBS<.f> a,b,limm               0010 1bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUBS<.cc><.f> b,b,limm          0010 1bbb 1100 0111   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBS<.f> 0,b,c                  0010 1bbb 0000 0111   FBBB CCCC CC11 1110
// SUBS<.f> 0,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uu11 1110
// SUBS<.f> 0,b,limm               0010 1bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// SUBS<.cc><.f> 0,limm,c          0010 1110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUBS(stream, pc, op, opcodes); break; // SUBS
					}
					case 0x08:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIVAW a,b,c                     0010 1bbb 0000 1000   0BBB CCCC CCAA AAAA
// DIVAW a,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uuAA AAAA
// DIVAW b,b,s12                   0010 1bbb 1000 1000   0BBB ssss ssSS SSSS
// DIVAW<.cc> b,b,c                0010 1bbb 1100 1000   0BBB CCCC CC0Q QQQQ
// DIVAW<.cc> b,b,u6               0010 1bbb 1100 1000   0BBB uuuu uu1Q QQQQ
// DIVAW a,limm,c                  0010 1110 0000 1000   0111 CCCC CCAA AAAA (+ Limm)
// DIVAW a,b,limm                  0010 1bbb 0000 1000   0BBB 1111 10AA AAAA (+ Limm)
// DIVAW<.cc> b,b,limm             0010 1bbb 1100 1000   0BBB 1111 10QQ QQQQ (+ Limm)
//
// DIVAW 0,b,c                     0010 1bbb 0000 1000   0BBB CCCC CC11 1110
// DIVAW 0,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uu11 1110
// DIVAW<.cc> 0,limm,c             0010 1110 1100 1000   0111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_DIVAW(stream, pc, op, opcodes); break; // DIVAW
					}
					case 0x0a:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ASLS<.f> a,b,c                  0010 1bbb 0000 1010   FBBB CCCC CCAA AAAA
// ASLS<.f> a,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uuAA AAAA
// ASLS<.f> b,b,s12                0010 1bbb 1000 1010   FBBB ssss ssSS SSSS
// ASLS<.cc><.f> b,b,c             0010 1bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// ASLS<.cc><.f> b,b,u6            0010 1bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// ASLS<.f> a,limm,c               0010 1110 0000 1010   F111 CCCC CCAA AAAA (+ Limm)
// ASLS<.f> a,b,limm               0010 1bbb 0000 1010   FBBB 1111 10AA AAAA (+ Limm)
// ASLS<.cc><.f> b,b,limm          0010 1bbb 1100 1010   FBBB 1111 10QQ QQQQ (+ Limm)
// ASLS<.f> 0,b,c                  0010 1bbb 0000 1010   FBBB CCCC CC11 1110
// ASLS<.f> 0,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uu11 1110
// ASLS<.cc><.f> 0,limm,c          0010 1110 1100 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ASLS(stream, pc, op, opcodes); break; // ASLS
					}
					case 0x0b:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ASRS<.f> a,b,c                  0010 1bbb 0000 1011   FBBB CCCC CCAA AAAA
// ASRS<.f> a,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uuAA AAAA
// ASRS<.f> b,b,s12                0010 1bbb 1000 1011   FBBB ssss ssSS SSSS
// ASRS<.cc><.f> b,b,c             0010 1bbb 1100 1011   FBBB CCCC CC0Q QQQQ
// ASRS<.cc><.f> b,b,u6            0010 1bbb 1100 1011   FBBB uuuu uu1Q QQQQ
// ASRS<.f> a,limm,c               0010 1110 0000 1011   F111 CCCC CCAA AAAA (+ Limm)
// ASRS<.f> a,b,limm               0010 1bbb 0000 1011   FBBB 1111 10AA AAAA (+ Limm)
// ASRS<.cc><.f> b,b,limm          0010 1bbb 1100 1011   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ASRS<.f> 0,b,c                  0010 1bbb 0000 1011   FBBB CCCC CC11 1110
// ASRS<.f> 0,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uu11 1110
// ASRS<.cc><.f> 0,limm,c          0010 1110 1100 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ASRS(stream, pc, op, opcodes); break; // ASRS
					}
					case 0x0c:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

						// Leapster BIOS uses this in loop at 400384a0
						size = handle04_helper_dasm(stream, pc, op, opcodes, "UNKNOWN 0x05-0x0c op", 0,0);
						break;
					}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					case 0x10:
					{
						// Leapster BIOS uses this in loop at 400384a0
						size = handle04_helper_dasm(stream, pc, op, opcodes, "UNKNOWN 0x05-0x10 op", 0,0);
						break;
					}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unknown Extension Op
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					case 0x14:
					{
						// Leapster BIOS uses this in loop at 400384a0
						size = handle04_helper_dasm(stream, pc, op, opcodes, "UNKNOWN 0x05-0x14 op", 0,0);
						break;
					}
					case 0x28:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADDSDW<.f> a,b,c                0010 1bbb 0010 1000   FBBB CCCC CCAA AAAA
// ADDSDW<.f> a,b,u6               0010 1bbb 0110 1000   FBBB uuuu uuAA AAAA
// ADDSDW<.f> b,b,s12              0010 1bbb 1010 1000   FBBB ssss ssSS SSSS
// ADDSDW<.cc><.f> b,b,c           0010 1bbb 1110 1000   FBBB CCCC CC0Q QQQQ
// ADDSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1000   FBBB uuuu uu1Q QQQQ
// ADDSDW<.f> a,limm,c             0010 1110 0010 1000   F111 CCCC CCAA AAAA (+ Limm)
// ADDSDW<.f> a,b,limm             0010 1bbb 0010 1000   FBBB 1111 10AA AAAA (+ Limm)
// ADDSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1000   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDSDW<.f> 0,b,c                0010 1bbb 0010 1000   FBBB CCCC CC11 1110
// ADDSDW<.f> 0,b,u6               0010 1bbb 0110 1000   FBBB uuuu uu11 1110
// ADDSDW<.cc><.f> 0,limm,c        0010 1110 1110 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_ADDSDW(stream, pc, op, opcodes); break; // ADDSDW
					}
					case 0x29:
					{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUBSDW<.f> a,b,c                0010 1bbb 0010 1001   FBBB CCCC CCAA AAAA
// SUBSDW<.f> a,b,u6               0010 1bbb 0110 1001   FBBB uuuu uuAA AAAA
// SUBSDW<.f> b,b,s12              0010 1bbb 1010 1001   FBBB ssss ssSS SSSS
// SUBSDW<.cc><.f> b,b,c           0010 1bbb 1110 1001   FBBB CCCC CC0Q QQQQ
// SUBSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1001   FBBB uuuu uu1Q QQQQ
// SUBSDW<.f> a,limm,c             0010 1110 0010 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUBSDW<.f> a,b,limm             0010 1bbb 0010 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUBSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1001   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBSDW<.f> 0,b,c                0010 1bbb 0010 1001   FBBB CCCC CC11 1110
// SUBSDW<.f> 0,b,u6               0010 1bbb 0110 1001   FBBB uuuu uu11 1110
// SUBSDW<.cc><.f> 0,limm,c        0010 1110 1110 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						size = handle::dasm32_SUBSDW(stream, pc, op, opcodes); break; // SUBSDW
					}
					case 0x2f: // SOPs
					{
						uint8_t subinstr2 = op & 0x0000003f;
						switch (subinstr2)
						{
							case 0x00:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SWAP<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0000
// SWAP<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0000
// SWAP<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// SWAP<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0000
// SWAP<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0000
// SWAP<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_SWAP(stream, pc, op, opcodes); break; // SWAP
							}
							case 0x01:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORM<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0001
// NORM<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0001
// NORM<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// NORM<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0001
// NORM<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0001
// NORM<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_NORM(stream, pc, op, opcodes); break; // NORM
							}
							case 0x02:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SAT16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0010
// SAT16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0010
// SAT16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// SAT16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0010
// SAT16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0010
// SAT16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_SAT16(stream, pc, op, opcodes); break; // SAT16
							}
							case 0x03:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RND16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0011
// RND16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0011
// RND16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// RND16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0011
// RND16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0011
// RND16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_RND16(stream, pc, op, opcodes); break; // RND16
							}
							case 0x04:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0100
// ABSSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0100
// ABSSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// ABSSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0100
// ABSSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0100
// ABSSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ABSSW(stream, pc, op, opcodes); break; // ABSSW
							}
							case 0x05:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0101
// ABSS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0101
// ABSS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// ABSS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0101
// ABSS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0101
// ABSS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_ABSS(stream, pc, op, opcodes); break; // ABSS
							}
							case 0x06:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0110
// NEGSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0110
// NEGSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// NEGSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0110
// NEGSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0110
// NEGSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_NEGSW(stream, pc, op, opcodes); break; // NEGSW
							}
							case 0x07:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0111
// NEGS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0111
// NEGS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// NEGS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0111
// NEGS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0111
// NEGS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_NEGS(stream, pc, op, opcodes); break; // NEGS
							}
							case 0x08:
							{
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORMW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 1000
// NORMW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 1000
// NORMW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// NORMW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 1000
// NORMW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 1000
// NORMW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
								size = handle::dasm32_NORMW(stream, pc, op, opcodes); break; // NORMW
							}
							case 0x3f: // ZOPs (Zero Operand Opcodes)
							{
								uint8_t subinstr3 = (op & 0x07000000) >> 24;
								subinstr3 |= ((op & 0x00007000) >> 12) << 3;

								switch (subinstr3)
								{
									default:
									{
										size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, subinstr3, op, opcodes); break;  // illegal
									}
								}
								break;
							}
							default:
							{
								size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;  // illegal
							}
						}
						break;
					}
					default:
					{
						size = handle::dasm_illegal(stream, pc, instruction, subinstr, op, opcodes); break;  // illegal
					}

				}
				break;
			}
			case 0x06: size = handle::dasm32_ARC_EXT06(stream, pc, op, opcodes); break;   // op a,b,c (06 ARC ext)
			case 0x07: size = handle::dasm32_USER_EXT07(stream, pc, op, opcodes); break;  // op a,b,c (07 User ext)
			case 0x08: size = handle::dasm32_USER_EXT08(stream, pc, op, opcodes); break;  // op a,b,c (08 User ext)
			case 0x09: size = handle::dasm32_MARKET_EXT09(stream, pc, op, opcodes); break; // op a,b,c (09 Market ext)
			case 0x0a: size = handle::dasm32_MARKET_EXT0a(stream, pc, op, opcodes); break; // op a,b,c (0a Market ext)
			case 0x0b: size = handle::dasm32_MARKET_EXT0b(stream, pc, op, opcodes); break; // op a,b,c (0b Market ext)
		}
	}
	else
	{
		size = 2;

		switch (instruction) // 16-bit instructions
		{
			default: break; // size = -1;
			case 0x0c: // Load/Add reg-reg
			{
				uint8_t subinstr = (op & 0x0018) >> 3;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LD_S a,[b,c]                    0110 0bbb ccc0 0aaa
// #######################################################################################################################
						size = handle::dasm_LD_S_a_b_c(stream, pc, op, opcodes); break; // LD_S a,[b,c]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LDB_S a,[b,c]                   0110 0bbb ccc0 1aaa
// #######################################################################################################################
						size = handle::dasm_LDB_S_a_b_c(stream, pc, op, opcodes); break; // LDB_S a,[b,c]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// LDW_S a,[b,c]                   0110 0bbb ccc1 0aaa
// #######################################################################################################################
						size = handle::dasm_LDW_S_a_b_c(stream, pc, op, opcodes); break; // LDW_S a,[b,c]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ADD_S a,b,c                     0110 0bbb ccc1 1aaa
// #######################################################################################################################
						size = handle::dasm_ADD_S_a_b_c(stream, pc, op, opcodes); break; // ADD_S a,b,c
					}
				}
				break;
			}
			case 0x0d: // Add/Sub/Shft imm
			{
				uint8_t subinstr = (op & 0x0018) >> 3;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
// ADD_S c,b,u3                    0110 1bbb ccc0 0uuu
// #######################################################################################################################
						size = handle::dasm_ADD_S_c_b_u3(stream, pc, op, opcodes); break; // ADD_S c,b,u3
					}
					case 0x01:
					{
// #######################################################################################################################
// SUB_S c,b,u3                    0110 1bbb ccc0 1uuu
// #######################################################################################################################
						size = handle::dasm_SUB_S_c_b_u3(stream, pc, op, opcodes); break; // SUB_S c,b,u3
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ASL_S c,b,u3                    0110 1bbb ccc1 0uuu
// #######################################################################################################################
						size = handle::dasm_ASL_S_c_b_u3(stream, pc, op, opcodes); break; // ASL_S c,b,u3
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ASR_S c,b,u3                    0110 1bbb ccc1 1uuu
// #######################################################################################################################
						size = handle::dasm_ASR_S_c_b_u3(stream, pc, op, opcodes); break; // ASR_S c,b,u3
					}
				}
				break;
			}
			case 0x0e: // Mov/Cmp/Add
			{
				uint8_t subinstr = (op & 0x0018) >> 3;

				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I       S S
// ADD_S b,b,h                     0111 0bbb hhh0 0HHH
// ADD_S b,b,limm                  0111 0bbb 1100 0111 (+ Limm)
// #######################################################################################################################
						size = handle::dasm_ADD_S_b_b_h_or_limm(stream, pc, op, opcodes); break; // ADD_S b,b,h  or  ADD_S b,b,limm
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I       S S
// MOV_S b,h                       0111 0bbb hhh0 1HHH
// MOV_S b,limm                    0111 0bbb 1100 1111 (+ Limm)
// #######################################################################################################################
						size = handle::dasm_MOV_S_b_h_or_limm(stream, pc, op, opcodes); break; // MOV_S b,h  or  MOV_S b,limm
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S S
// CMP_S b,h                       0111 0bbb hhh1 0HHH
// CMP_S b,limm                    0111 0bbb 1101 0111 (+ Limm)
// #######################################################################################################################
						size = handle::dasm_CMP_S_b_h_or_limm(stream, pc, op, opcodes); break; // CMP_S b,h  or  CMP_S b,limm
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I       S S
// MOV_S h,b                       0111 0bbb hhh1 1HHH
// #######################################################################################################################
						size = handle::dasm_MOV_S_h_b(stream, pc, op, opcodes); break; // MOV_S h,b
					}
				}
				break;
			}
			case 0x0f: // op_S b,b,c (single 16-bit ops)
			{
				uint8_t subinstr = op & 0x01f;

				switch (subinstr)
				{
					case 0x00: // SOPs
					{
						uint8_t subinstr2 = (op & 0x00e0) >> 5;

						switch (subinstr2)
						{
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S [b]                         0111 1bbb 0000 0000
// #######################################################################################################################
								size = handle::dasm_J_S_b(stream, pc, op, opcodes); break; // J_S [b]
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// J_S.D [b]                       0111 1bbb 0010 0000
// #######################################################################################################################
								size = handle::dasm_J_S_D_b(stream, pc, op, opcodes); break; // J_S.D [b]
							}
							case 0x02:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S [b]                        0111 1bbb 0100 0000
// #######################################################################################################################
								size = handle::dasm_JL_S_b(stream, pc, op, opcodes); break; // JL_S [b]
							}
							case 0x03:
							{
// #######################################################################################################################
//                                 IIII I    sssS SSSS
// JL_S.D [b]                      0111 1bbb 0110 0000
// #######################################################################################################################
								size = handle::dasm_JL_S_D_b(stream, pc, op, opcodes); break; // JL_S.D [b]
							}
							case 0x06:
							{
// #######################################################################################################################
// SUB_S.NE b,b,b                  0111 1bbb 1100 0000
//                                 IIII I    sssS SSSS
// #######################################################################################################################
								size = handle::dasm_SUB_S_NE_b_b_b(stream, pc, op, opcodes); break; // SUB_S.NE b,b,b
							}
							case 0x07: // ZOPs
							{
								uint8_t subinstr3 = (op & 0x0700) >> 8;

								switch (subinstr3)
								{
									case 0x00:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// NOP_S                           0111 1000 1110 0000
// #######################################################################################################################
										size = handle::dasm_NOP_S(stream, pc, op, opcodes); break; // NOP_S
									}
									case 0x01:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// UNIMP_S                         0111 1001 1110 0000
// #######################################################################################################################
										size = handle::dasm_UNIMP_S(stream, pc, op, opcodes); break; // UNIMP_S
									}
									case 0x04:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// JEQ_S [blink]                   0111 1100 1110 0000
// #######################################################################################################################
										size = handle::dasm_JEQ_S_blink(stream, pc, op, opcodes); break; // JEQ_S [BLINK]
									}
									case 0x05:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// JNE_S [blink]                   0111 1101 1110 0000
// #######################################################################################################################
										size = handle::dasm_JNE_S_blink(stream, pc, op, opcodes); break; // JNE_S [BLINK]
									}
									case 0x06:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// J_S [blink]                     0111 1110 1110 0000
// #######################################################################################################################
										size = handle::dasm_J_S_blink(stream, pc, op, opcodes); break; // J_S [BLINK]
									}
									case 0x07:
									{
// #######################################################################################################################
//                                 IIII I$$$ sssS SSSS
// J_S.D [blink]                   0111 1111 1110 0000
// #######################################################################################################################
										size = handle::dasm_J_S_D_blink(stream, pc, op, opcodes); break; // J_S.D [BLINK]
									}

									default: // 0x02, 0x03
									{
										size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, subinstr3, op, opcodes); break;
									}
								}
								break;
							}
							default: // 0x04, 0x05
							{
								size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;
							}
						}
						break;
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// SUB_S b,b,c                     0111 1bbb ccc0 0010
// #######################################################################################################################
						size = handle::dasm_SUB_S_b_b_c(stream, pc, op, opcodes); break; // SUB_S b,b,c
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// AND_S b,b,c                     0111 1bbb ccc0 0100
// #######################################################################################################################
						size = handle::dasm_AND_S_b_b_c(stream, pc, op, opcodes); break; // AND_S b,b,c
					}
					case 0x05:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// OR_S b,b,c                      0111 1bbb ccc0 0101
// #######################################################################################################################
						size = handle::dasm_OR_S_b_b_c(stream, pc, op, opcodes); break; // OR_S b,b,c
					}
					case 0x06:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// BIC_S b,b,c                     0111 1bbb ccc0 0110
// #######################################################################################################################
						size = handle::dasm_BIC_S_b_b_c(stream, pc, op, opcodes); break; // BIC_S b,b,c
					}
					case 0x07:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// XOR_S b,b,c                     0111 1bbb ccc0 0111
// #######################################################################################################################
						size = handle::dasm_XOR_S_b_b_c(stream, pc, op, opcodes); break; // XOR_S b,b,c
					}
					case 0x0b:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// TST_S b,c                       0111 1bbb ccc0 1011
// #######################################################################################################################
						size = handle::dasm_TST_S_b_c(stream, pc, op, opcodes); break; // TST_S b,c
					}
					case 0x0c:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// MUL64_S <0,>b,c                 0111 1bbb ccc0 1100
// #######################################################################################################################
						size = handle::dasm_MUL64_S_0_b_c(stream, pc, op, opcodes); break; // MUL64_S <0,>b,c
					}
					case 0x0d:
					{

// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXB_S b,c                      0111 1bbb ccc0 1101
// #######################################################################################################################
						size = handle::dasm_SEXB_S_b_c(stream, pc, op, opcodes); break; // SEXB_S b,c
					}
					case 0x0e:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// SEXW_S b,c                      0111 1bbb ccc0 1110
// #######################################################################################################################
						size = handle::dasm_SEXW_S_b_c(stream, pc, op, opcodes); break; // SEXW_S b,c
					}
					case 0x0f:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTB_S b,c                      0111 1bbb ccc0 1111
// #######################################################################################################################
						size = handle::dasm_EXTB_S_b_c(stream, pc, op, opcodes); break; // EXTB_S b,c
					}
					case 0x10:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// EXTW_S b,c                      0111 1bbb ccc1 0000
// #######################################################################################################################
						size = handle::dasm_EXTW_S_b_c(stream, pc, op, opcodes); break; // EXTW_S b,c
					}
					case 0x11:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ABS_S b,c                       0111 1bbb ccc1 0001
// #######################################################################################################################
						size = handle::dasm_ABS_S_b_c(stream, pc, op, opcodes); break; // ABS_S b,c
					}
					case 0x12:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// NOT_S b,c                       0111 1bbb ccc1 0010
// #######################################################################################################################
						size = handle::dasm_NOT_S_b_c(stream, pc, op, opcodes); break; // NOT_S b,c
					}
					case 0x13:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// NEG_S b,c                       0111 1bbb ccc1 0011
// #######################################################################################################################
						size = handle::dasm_NEG_S_b_c(stream, pc, op, opcodes); break; // NEG_S b,c
					}
					case 0x14:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD1_S b,b,c                    0111 1bbb ccc1 0100
// #######################################################################################################################
						size = handle::dasm_ADD1_S_b_b_c(stream, pc, op, opcodes); break; // ADD1_S b,b,c
					}
					case 0x15:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD2_S b,b,c                    0111 1bbb ccc1 0101
// #######################################################################################################################
						size = handle::dasm_ADD2_S_b_b_c(stream, pc, op, opcodes); break; // ADD2_S b,b,c
					}
					case 0x16:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ADD3_S b,b,c                    0111 1bbb ccc1 0110
// #######################################################################################################################
						size = handle::dasm_ADD3_S_b_b_c(stream, pc, op, opcodes); break; // ADD3_S b,b,c
					}
					case 0x18:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,b,c                     0111 1bbb ccc1 1000
// #######################################################################################################################
						size = handle::dasm_ASL_S_b_b_c_multiple(stream, pc, op, opcodes); break; // ASL_S b,b,c (multiple)
					}
					case 0x19:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,b,c                     0111 1bbb ccc1 1001
// #######################################################################################################################
						size = handle::dasm_LSR_S_b_b_c_multiple(stream, pc, op, opcodes); break; // LSR_S b,b,c (multiple)
					}
					case 0x1a:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,b,c                     0111 1bbb ccc1 1010
// #######################################################################################################################
						size = handle::dasm_ASR_S_b_b_c_multiple(stream, pc, op, opcodes); break; // ASR_S b,b,c (multiple)
					}
					case 0x1b:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASL_S b,c                       0111 1bbb ccc1 1011
// #######################################################################################################################
						size = handle::dasm_ASL_S_b_c_single(stream, pc, op, opcodes); break; // ASL_S b,c (single)
					}
					case 0x1c:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// ASR_S b,c                       0111 1bbb ccc1 1100
// #######################################################################################################################
						size = handle::dasm_ASR_S_b_c_single(stream, pc, op, opcodes); break; // ASR_S b,c (single)
					}
					case 0x1d:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// LSR_S b,c                       0111 1bbb ccc1 1101
// #######################################################################################################################
						size = handle::dasm_LSR_S_b_c_single(stream, pc, op, opcodes); break; // LSR_S b,c (single)
					}
					case 0x1e:
					{
// #######################################################################################################################
//                                 IIII I       S SSSS
// TRAP_S u6                       0111 1uuu uuu1 1110
// #######################################################################################################################
						size = handle::dasm_TRAP_S_u6(stream, pc, op, opcodes); break; // TRAP_S u6 (not a5?)
					}
					case 0x1f:
					{
						uint8_t subinstr2 = (op & 0x07e0) >> 5;
						if (subinstr2 == 0x3f)
						{
// #######################################################################################################################
//                                 IIII Isss sssS SSSS
// BRK_S                           0111 1111 1111 1111
// #######################################################################################################################
							size = handle::dasm_BRK_S(stream, pc, op, opcodes); break; // BRK_S ( 0x7fff only? ) // BRK_S ( 0x7fff only? )
						}
						else
						{
							size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;
						}
					}
					default: // 0x01, 0x03, 0x08, 0x09, 0x0a, 0x17
					{
						size = handle::dasm_illegal(stream, pc, instruction, subinstr, op, opcodes); break;
					}
				}
				break;
			}
			case 0x10:
			{
// #######################################################################################################################
//                                 IIII I
// LD_S c,[b,u7]                   1000 0bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_LD_S_c_b_u7(stream, pc, op, opcodes); break; // LD_S c,[b,u7]
			}
			case 0x11:
			{
// #######################################################################################################################
//                                 IIII I
// LDB_S c,[b,u5]                  1000 1bbb cccu uuuu
// #######################################################################################################################
				size =  handle::dasm_LDB_S_c_b_u5(stream, pc, op, opcodes); break; // LDB_S c,[b,u5]
			}
			case 0x12:
			{
// #######################################################################################################################
//                                 IIII I
// LDW_S c,[b,u6]                  1001 0bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_LDW_S_c_b_u6(stream, pc, op, opcodes); break; // LDW_S c,[b,u6]
			}
			case 0x13:
			{
// #######################################################################################################################
//                                 IIII I
// LDW_S.X c,[b,u6]                1001 1bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_LDW_S_X_c_b_u6(stream, pc, op, opcodes); break; // LDW_S.X c,[b,u6]
			}
			case 0x14:
			{
// #######################################################################################################################
//                                 IIII I
// ST_S c,[b,u7]                   1010 0bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_ST_S_c_b_u7(stream, pc, op, opcodes); break; // ST_S c,[b,u7]
			}
			case 0x15:
			{
// #######################################################################################################################
//                                 IIII I
// STB_S c,[b,u5]                  1010 1bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_STB_S_c_b_u5(stream, pc, op, opcodes); break; // STB_S
			}
			case 0x16:
			{
// #######################################################################################################################
//                                 IIII I
// STW_S c,[b,u6]                  1011 0bbb cccu uuuu
// #######################################################################################################################
				size = handle::dasm_STW_S_c_b_u6(stream, pc, op, opcodes); break; // STW_S
			}
			case 0x17: // Shift/Sub/Bit
			{
				uint8_t subinstr = (op & 0x00e0) >> 5;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ASL_S b,b,u5                    1011 1bbb 000u uuuu
// #######################################################################################################################
						size = handle::dasm_ASL_S_b_b_u5(stream, pc, op, opcodes); break; // ASL_S b,b,u5
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LSR_S b,b,u5                    1011 1bbb 001u uuuu
// #######################################################################################################################
						size = handle::dasm_LSR_S_b_b_u5(stream, pc, op, opcodes); break; // LSR_S b,b,u5
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ASR_S b,b,u5                    1011 1bbb 010u uuuu
// #######################################################################################################################
						size = handle::dasm_ASR_S_b_b_u5(stream, pc, op, opcodes); break; // ASR_S b,b,u5
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// SUB_S b,b,u5                    1011 1bbb 011u uuuu
// #######################################################################################################################
						size = handle::dasm_SUB_S_b_b_u5(stream, pc, op, opcodes); break; // SUB_S b,b,u5
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BSET_S b,b,u5                   1011 1bbb 100u uuuu
// #######################################################################################################################
						size = handle::dasm_BSET_S_b_b_u5(stream, pc, op, opcodes); break; // BSET_S b,b,u5
					}
					case 0x05:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BCLR_S b,b,u5                   1011 1bbb 101u uuuu
// #######################################################################################################################
						size = handle::dasm_BCLR_S_b_b_u5(stream, pc, op, opcodes); break; // BCLR_S b,b,u5
					}
					case 0x06:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BMSK_S b,b,u5                   1011 1bbb 110u uuuu
// #######################################################################################################################
						size = handle::dasm_BMSK_S_b_b_u5(stream, pc, op, opcodes); break; // BMSK_S b,b,u5
					}
					case 0x07:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// BTST_S b,u5                     1011 1bbb 111u uuuu
// #######################################################################################################################
						size = handle::dasm_BTST_S_b_u5(stream, pc, op, opcodes); break; // BTST_S b,u5
					}
				}
				break;
			}
			case 0x18: // Stack Instr
			{
				uint8_t subinstr = (op & 0x00e0) >> 5;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LD_S b,[sp,u7]                  1100 0bbb 000u uuuu
// #######################################################################################################################
						size = handle::dasm_LD_S_b_sp_u7(stream, pc, op, opcodes); break; // LD_S b,[sp,u7]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// LDB_S b,[sp,u7]                 1100 0bbb 001u uuuu
// #######################################################################################################################
						size = handle::dasm_LDB_S_b_sp_u7(stream, pc, op, opcodes); break; // LDB_S b,[sp,u7]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ST_S b,[sp,u7]                  1100 0bbb 010u uuuu
// #######################################################################################################################
						size = handle::dasm_ST_S_b_sp_u7(stream, pc, op, opcodes); break; // ST_S b,[sp,u7]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// STB_S b,[sp,u7]                 1100 0bbb 011u uuuu
// #######################################################################################################################
						size = handle::dasm_STB_S_b_sp_u7(stream, pc, op, opcodes); break; // STB_S b,[sp,u7]
					}
					case 0x04:
					{
// #######################################################################################################################
//                                 IIII I    SSS
// ADD_S b,sp,u7                   1100 0bbb 100u uuuu
// #######################################################################################################################
						size = handle::dasm_ADD_S_b_sp_u7(stream, pc, op, opcodes); break; // ADD_S b,sp,u7
					}

					case 0x05: // subtable 18_05
					{
						uint8_t subinstr2 = (op & 0x0700) >> 8;
						switch (subinstr2)
						{
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII Isss SSS
// ADD_S sp,sp,u7                  1100 0000 101u uuuu
// #######################################################################################################################
								size = handle::dasm_ADD_S_sp_sp_u7(stream, pc, op, opcodes); break; // ADD_S sp,sp,u7
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII Isss SSS
// SUB_S sp,sp,u7                  1100 0001 101u uuuu
// #######################################################################################################################
								size = handle::dasm_SUB_S_sp_sp_u7(stream, pc, op, opcodes); break; // SUB_S sp,sp,u7
							}
							default: size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;
						}
						break;
					}
					case 0x06: // subtable 18_06
					{
						uint8_t subinstr2 = op & 0x001f;
						switch (subinstr2)
						{
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S b                         1100 0bbb 1100 0001
// #######################################################################################################################
								size = handle::dasm_POP_S_b(stream, pc, op, opcodes); break; // POP_S b
							}
							case 0x11:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// POP_S blink                     1100 0RRR 1101 0001
// #######################################################################################################################
								size = handle::dasm_POP_S_blink(stream, pc, op, opcodes); break; // POP_S blink
							}
							default: size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;
						}
						break;
					}
					case 0x07: // subtable 18_07
					{
						uint8_t subinstr2 = op & 0x001f;

						switch (subinstr2)
						{
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S b                        1100 0bbb 1110 0001
// #######################################################################################################################
								size = handle::dasm_PUSH_S_b(stream, pc, op, opcodes); break; // PUSH_S b
							}
							case 0x11:
							{
// #######################################################################################################################
//                                 IIII I    SSSs ssss
// PUSH_S blink                    1100 0RRR 1111 0001
// #######################################################################################################################
								size = handle::dasm_PUSH_S_blink(stream, pc, op, opcodes); break; // PUSH_S blink
							}
							default: size = handle::dasm_illegal(stream, pc, instruction, subinstr, subinstr2, op, opcodes); break;
						}
					}
				}
				break;
			}
			case 0x19: // GP Instr
			{
				uint8_t subinstr = (op & 0x0600) >> 9;

				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII ISS
// LD_S r0,[gp,s11]                1100 100s ssss ssss
// #######################################################################################################################
						size = handle::dasm_LD_S_r0_gp_s11(stream, pc, op, opcodes); break; // LD_S r0,[gp,s11]
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII ISS
// LDB_S r0,[gp,s9]                1100 101s ssss ssss
// #######################################################################################################################
						size = handle::dasm_LDB_S_r0_gp_s9(stream, pc, op, opcodes); break; // LDB_S r0,[gp,s9]
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII ISS
// LDW_S r0,[gp,s10]               1100 110s ssss ssss
// #######################################################################################################################
						size = handle::dasm_LDW_S_r0_gp_s10(stream, pc, op, opcodes); break; // LDW_S r0,[gp,s10]
					}
					case 0x03:
					{
// #######################################################################################################################
//                                 IIII ISS
// ADD_S r0,gp,s11                 1100 111s ssss ssss
// #######################################################################################################################
						size = handle::dasm_ADD_S_r0_gp_s11(stream, pc, op, opcodes); break; // ADD_S r0,gp,s11
					}
				}
				break;
			}
			case 0x1a:
			{
// #######################################################################################################################
//                                 IIII I
// LD_S b,[pcl,u10]                1101 0bbb uuuu uuuu
// #######################################################################################################################
				size = handle::dasm_LD_S_b_pcl_u10(stream, pc, op, opcodes); break; // LD_S b,[pcl,u10]
			}

			case 0x1b:
			{
// #######################################################################################################################
//                                 IIII I
// MOV_S b,u8                      1101 1bbb uuuu uuuu
// #######################################################################################################################
				size = handle::dasm_MOV_S_b_u8(stream, pc, op, opcodes); break; // MOV_S b, u8
			}

			case 0x1c: // ADD_S/CMP_S
			{
				uint8_t subinstr = (op & 0x0080) >> 7;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    s
// ADD_S b,b,u7                    1110 0bbb 0uuu uuuu
// #######################################################################################################################
						size = handle::dasm_ADD_S_b_b_u7(stream, pc, op, opcodes); break; // ADD_S b, b, u7
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    s
// CMP_S b,u7                      1110 0bbb 1uuu uuuu
// #######################################################################################################################
						size = handle::dasm_CMP_S_b_u7(stream, pc, op, opcodes); break; // CMP_S b, u7
					}
				}
				break;
			}
			case 0x1d: // BRcc_S
			{
				uint8_t subinstr = (op & 0x0080) >> 7;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII I    s
// BREQ_S b,0,s8                   1110 1bbb 0sss ssss
// #######################################################################################################################
						size = handle::dasm_BREQ_S_b_0_s8(stream, pc, op, opcodes); break; // BREQ_S b,0,s8
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII I    s
// BRNE_S b,0,s8                   1110 1bbb 1sss ssss
// #######################################################################################################################
						size =  handle::dasm_BRNE_S_b_0_s8(stream, pc, op, opcodes); break; // BRNE_S b,0,s8
					}
				}
				break;
			}
			case 0x1e: // Bcc_S
			{
				uint8_t subinstr = (op & 0x0600) >> 9;
				switch (subinstr)
				{
					default: break; // size = -1;
					case 0x00:
					{
// #######################################################################################################################
//                                 IIII ISS
// B_S s10                         1111 000s ssss ssss
// #######################################################################################################################
						size = handle::dasm_B_S_s10(stream, pc, op, opcodes); break; // B_S s10
					}
					case 0x01:
					{
// #######################################################################################################################
//                                 IIII ISS
// BEQ_S s10                       1111 001s ssss ssss
// #######################################################################################################################
						size = handle::dasm_BEQ_S_s10(stream, pc, op, opcodes); break; // BEQ_S s10
					}
					case 0x02:
					{
// #######################################################################################################################
//                                 IIII ISS
// BNE_S s10                       1111 010s ssss ssss
// #######################################################################################################################
						size = handle::dasm_BNE_S_s10(stream, pc, op, opcodes); break; // BNE_S s10
					}
					case 0x03: // Bcc_S
					{
						uint8_t subinstr2 = (op & 0x01c0) >> 6;
						switch (subinstr2)
						{
							default: break; // size = -1;
							case 0x00:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BGT_S s7                        1111 0110 00ss ssss
// #######################################################################################################################
								size = handle::dasm_BGT_S_s7(stream, pc, op, opcodes); break; // BGT_S s7
							}
							case 0x01:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BGE_S s7                        1111 0110 01ss ssss
// #######################################################################################################################
								size = handle::dasm_BGE_S_s7(stream, pc, op, opcodes); break; // BGE_S s7
							}
							case 0x02:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLT_S s7                        1111 0110 10ss ssss
// #######################################################################################################################
								size = handle::dasm_BLT_S_s7(stream, pc, op, opcodes); break; // BLT_S s7
							}
							case 0x03:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLE_S s7                        1111 0110 11ss ssss
// #######################################################################################################################
								size = handle::dasm_BLE_S_s7(stream, pc, op, opcodes); break; // BLE_S s7
							}
							case 0x04:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BHI_S s7                        1111 0111 00ss ssss
// #######################################################################################################################
								size = handle::dasm_BHI_S_s7(stream, pc, op, opcodes); break; // BHI_S s7
							}
							case 0x05:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BHS_S s7                        1111 0111 01ss ssss
// #######################################################################################################################
								size = handle::dasm_BHS_S_s7(stream, pc, op, opcodes); break; // BHS_S s7
							}
							case 0x06:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLO_S s7                        1111 0111 10ss ssss
// #######################################################################################################################
								size = handle::dasm_BLO_S_s7(stream, pc, op, opcodes); break; // BLO_S s7
							}

							case 0x07:
							{
// #######################################################################################################################
//                                 IIII ISSs ss
// BLS_S s7                        1111 0111 11ss ssss
// #######################################################################################################################
								size = handle::dasm_BLS_S_s7(stream, pc, op, opcodes); break; // BLS_S s7
							}
						}
					}
				}
				break;
			}
			case 0x1f:
			{
// #######################################################################################################################
//                                 IIII I
// BL_S s13                        1111 1sss ssss ssss
// #######################################################################################################################
				size = handle::dasm_BL_S_s13(stream, pc, op, opcodes); break; // BL_S s13
			}
		}
	}

	return size | SUPPORTED;
}

