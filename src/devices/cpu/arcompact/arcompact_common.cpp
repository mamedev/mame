// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact Core

\*********************************/

// condition codes (basic ones are the same as arc
const char *conditions[0x20] =
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
const char *auxregnames[0x420] =
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

const char *datasize[0x4] =
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

const char *dataextend[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".ZX", // Zero Extend (can use no extension, using .ZX to be explicit)
#else
	/* 00 */ "", // Zero Extend
#endif
	/* 01 */ ".X" // Sign Extend
};

const char *addressmode[0x4] =
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

const char *cachebit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".EN", // Data Cache Enabled (can use no extension, using .EN to be explicit)
#else
	/* 00 */ "", // Data Cache Enabled
#endif
	/* 01 */ ".DI" // Direct to Memory (Cache Bypass)
};

const char *flagbit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".NF", // Don't Set Flags (can use no extension, using .NF to be explicit)
#else
	/* 00 */ "", // Don't Set Flags
#endif
	/* 01 */ ".F" // Set Flags
};

const char *delaybit[0x2] =
{
	/* 00 */ ".ND", // Don't execute opcode in delay slot
	/* 01 */ ".D"   // Execute Opcode in delay slot
};


const char *regnames[0x40] =
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
const char *opcodes_temp[0x40] =
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


const char *opcodes_04[0x40] =
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
	/* 13 */ "BSMK",
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
