/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

#include "arcompactdasm_ops.h"

char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vsprintf(output, fmt, vl);
	va_end(vl);
}


// condition codes (basic ones are the same as arc 
static const char *conditions[0x20] =
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
	/* 0xxx0 */ UNUSED_REG,	/* 0xxx1 */ UNUSED_REG,	/* 0xxx2 */ UNUSED_REG,	/* 0xxx3 */ UNUSED_REG,	/* 0xxx4 */ UNUSED_REG,	/* 0xxx5 */ UNUSED_REG,	/* 0xxx6 */ UNUSED_REG,	/* 0xxx7 */ UNUSED_REG,	/* 0xxx8 */ UNUSED_REG,	/* 0xxx9 */ UNUSED_REG,	/* 0xxxa */ UNUSED_REG,	/* 0xxxb */ UNUSED_REG,	/* 0xxxc */ UNUSED_REG,	/* 0xxxd */ UNUSED_REG,	/* 0xxxe */ UNUSED_REG,	/* 0xxxf */ UNUSED_REG,

// the Auxiliary Register set is actually a 2^32 dword address space (so 16 GB / 34-bit)
// this table just allows us to improve the debugger display for some of the common core / internal ones
static const char *auxregnames[0x420] =
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

static const char *datasize[0x4] =
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

static const char *dataextend[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".ZX", // Zero Extend (can use no extension, using .ZX to be explicit)
#else
	/* 00 */ "", // Zero Extend
#endif
	/* 01 */ ".X" // Sign Extend
};

static const char *addressmode[0x4] =
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

static const char *cachebit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".EN", // Data Cache Enabled (can use no extension, using .EN to be explicit)
#else
	/* 00 */ "", // Data Cache Enabled
#endif
	/* 01 */ ".DI" // Direct to Memory (Cache Bypass)
};

static const char *flagbit[0x2] =
{
#ifdef EXPLICIT_EXTENSIONS
	/* 00 */ ".NF", // Don't Set Flags (can use no extension, using .NF to be explicit)
#else
	/* 00 */ "", // Don't Set Flags
#endif
	/* 01 */ ".F" // Set Flags
};

static const char *delaybit[0x2] =
{
	/* 00 */ ".ND", // Don't execute opcode in delay slot
	/* 01 */ ".D"   // Execute Opcode in delay slot
};


static const char *regnames[0x40] =
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
	/* 1a */ "r26(GP)",
	/* 1b */ "r27(FP)",
	/* 1c */ "r28(SP)",
	/* 1d */ "r29(ILINK1)",
	/* 1e */ "r30(ILINK2)",
	/* 1f */ "r31(BLINK)",

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



#define GET_01_01_01_BRANCH_ADDR \
	INT32 address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f); \
	op &= ~ 0x00fe800f;


#define GROUP_0e_GET_h \
	h =  ((op & 0x0007) << 3); \
    h |= ((op & 0x00e0) >> 5); \
	op &= ~0x00e7; \

#define COMMON32_GET_breg \
	int b_temp = (op & 0x07000000) >> 24; op &= ~0x07000000; \
	int B_temp = (op & 0x00007000) >> 12; op &= ~0x00007000; \
	int breg = b_temp | (B_temp << 3); \

#define COMMON32_GET_s12 \
		int S_temp = (op & 0x0000003f) >> 0; op &= ~0x0000003f; \
		int s_temp = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0; \
		int S = s_temp | (S_temp<<6); \

#define COMMON32_GET_CONDITION \
		UINT8 condition = op & 0x0000001f;  op &= ~0x0000001f;


#define COMMON16_GET_breg \
	breg =  ((op & 0x0700) >>8); \
	op &= ~0x0700; \

#define COMMON16_GET_creg \
	creg =  ((op & 0x00e0) >>5); \
	op &= ~0x00e0; \

#define COMMON16_GET_areg \
	areg =  ((op & 0x0007) >>0); \
	op &= ~0x0007; \

#define COMMON16_GET_u3 \
	u =  ((op & 0x0007) >>0); \
	op &= ~0x0007; \

#define COMMON16_GET_u5 \
	u =  ((op & 0x001f) >>0); \
	op &= ~0x001f; \

#define COMMON16_GET_u8 \
	u =  ((op & 0x00ff) >>0); \
	op &= ~0x00ff; \

#define COMMON16_GET_u7 \
	u =  ((op & 0x007f) >>0); \
	op &= ~0x007f; \

// registers used in 16-bit opcodes hae a limited range
// and can only address registers r0-r3 and r12-r15

#define REG_16BIT_RANGE(_reg_) \
	if (_reg_>3) _reg_+= 8; \


// this is as messed up as the rest of the 16-bit alignment in LE mode...

#define GET_LIMM \
	limm = oprom[4] | (oprom[5] << 8); \
	limm |= (oprom[2] << 16) | (oprom[3] << 24); \

#define PC_ALIGNED32 \
	(pc&0xfffffffc)


/************************************************************************************************************************************
*                                                                                                                                   *
* individual opcode handlers (disassembly)                                                                                          *
*                                                                                                                                   *
************************************************************************************************************************************/

int arcompact_handle00_00_dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	COMMON32_GET_CONDITION

	output  += sprintf( output, "B%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address * 2));
	return size;
}

int arcompact_handle00_01_dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch Unconditionally Far
	// 0000 0sss ssss sss1 SSSS SSSS SSNR TTTT
	INT32 address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	output  += sprintf( output, "B%s %08x", delaybit[n], PC_ALIGNED32 + (address * 2) );
	if (res)  output += sprintf(output, "(reserved bit set)");

	return size;
}

int arcompact_handle01_00_00dasm(DASM_OPS_32)
{
	int size = 4;

	// Branch and Link Conditionally
	// 00001 sssssssss 00 SSSSSSSSSS N QQQQQ
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);	
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	COMMON32_GET_CONDITION

	output  += sprintf( output, "BL%s(%s) %08x", delaybit[n], conditions[condition], PC_ALIGNED32 + (address *2) );
	return size;
}

int arcompact_handle01_00_01dasm(DASM_OPS_32)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	INT32 address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);	
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	output  += sprintf( output, "BL%s %08x", delaybit[n], PC_ALIGNED32 + (address *2) );
	if (res)  output += sprintf(output, "(reserved bit set)");

	return size;
}



int arcompact_01_01_00_helper(DASM_OPS_32, const char* optext)
{
	int size = 4;

	// Branch on Compare / Bit Test - Register-Register
	// 00001 bbb sssssss 1 S BBB CCCCCC N 0 iiii
	GET_01_01_01_BRANCH_ADDR


	int c = (op & 0x00000fc0) >> 6;
	COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	if ((breg != LIMM_REG) && (c != LIMM_REG))
	{
		print("%s%s %s, %s %08x (%08x)", optext, delaybit[n], regnames[breg], regnames[c], PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
	}
	else
	{
		UINT32 limm;
		GET_LIMM_32;
		size = 8;

		if ((breg == LIMM_REG) && (c != LIMM_REG))
		{
			print("%s%s (%08x) %s %08x (%08x)", optext, delaybit[n], limm, regnames[c], PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
		}
		else if ((c == LIMM_REG) && (breg != LIMM_REG))
		{
			print("%s%s %s, (%08x) %08x (%08x)", optext, delaybit[n], regnames[breg], limm, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
		}
		else
		{
			// b and c are LIMM? invalid??
			print("%s%s (%08x), (%08x) (illegal?) %08x (%08x)", optext, delaybit[n], limm, limm, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);

		}
	}

	return size;
}


// register - register cases
int arcompact_handle01_01_00_00_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BREQ"); }
int arcompact_handle01_01_00_01_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRNE"); }
int arcompact_handle01_01_00_02_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRLT"); }
int arcompact_handle01_01_00_03_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRGE"); }
int arcompact_handle01_01_00_04_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRLO"); }
int arcompact_handle01_01_00_05_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BRHS"); }
int arcompact_handle01_01_00_0e_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BBIT0");}
int arcompact_handle01_01_00_0f_dasm(DASM_OPS_32)  { return arcompact_01_01_00_helper( DASM_PARAMS, "BBIT1");}

int arcompact_01_01_01_helper(DASM_OPS_32, const char* optext)
{
	int size = 4;

	// using 'b' as limm here makes no sense (comparing a long immediate against a short immediate) so I assume it isn't
	// valid?

	// Branch on Compare / Bit Test - Register-Immediate
	// 0000 1bbb ssss sss1 SBBB uuuu uuN1 iiii
	GET_01_01_01_BRANCH_ADDR

	int u = (op & 0x00000fc0) >> 6;
	COMMON32_GET_breg;
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	op &= ~0x07007fe0;

	print("%s%s %s, 0x%02x %08x (%08x)", optext, delaybit[n], regnames[breg], u, PC_ALIGNED32 + (address * 2), op & ~0xf8fe800f);
	
	return size;
}

// register -immediate cases
int arcompact_handle01_01_01_00_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BREQ"); }
int arcompact_handle01_01_01_01_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRNE"); }
int arcompact_handle01_01_01_02_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRLT"); }
int arcompact_handle01_01_01_03_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRGE"); }
int arcompact_handle01_01_01_04_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRLO"); }
int arcompact_handle01_01_01_05_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BRHS"); }
int arcompact_handle01_01_01_0e_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BBIT0"); }
int arcompact_handle01_01_01_0f_dasm(DASM_OPS_32)  { return arcompact_01_01_01_helper(DASM_PARAMS, "BBIT1"); }


int arcompact_handle02_dasm(DASM_OPS_32)
{
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 0bbb ssss ssss SBBB DaaZ ZXAA AAAA
	int size = 4;

	int A = (op & 0x0000003f) >> 0;  //op &= ~0x0000003f;
	int X = (op & 0x00000040) >> 6;  //op &= ~0x00000040;
	int Z = (op & 0x00000180) >> 7;  //op &= ~0x00000180;
	int a = (op & 0x00000600) >> 9;  //op &= ~0x00000600;
	int D = (op & 0x00000800) >> 11;// op &= ~0x00000800;
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;
	COMMON32_GET_breg;

	int sdat = s | (S << 8); // todo - signed

	UINT32 limm = 0;
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
	}

	output  += sprintf( output, "LD");
	output  += sprintf( output, "%s", datasize[Z]);
	output  += sprintf( output, "%s", dataextend[X]);
	output  += sprintf( output, "%s", addressmode[a]);
	output  += sprintf( output, "%s", cachebit[D]);
	output  += sprintf( output, " ");
	output  += sprintf( output, "%s <- ", regnames[A]);
	output  += sprintf( output, "[");
	if (breg == LIMM_REG) output  += sprintf( output, "(%08x), ", limm);
	else output  += sprintf( output, "%s, ", regnames[breg]);
	output  += sprintf( output, "%03x", sdat);
	output  += sprintf( output, "]");

	return size;
}

int arcompact_handle03_dasm(DASM_OPS_32)
{
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0; 
	// bitpos
	// 1111 1111 1111 1111 0000 0000 0000 0000
	// fedc ba98 7654 3210 fedc ba98 7654 3210
	// fields
	// 0001 1bbb ssss ssss SBBB CCCC CCDa aZZR
	int S = (op & 0x00008000) >> 15;// op &= ~0x00008000;
	int s = (op & 0x00ff0000) >> 16;// op &= ~0x00ff0000;

	COMMON32_GET_breg;
	int sdat = s | (S << 8); // todo - signed

	int R = (op & 0x00000001) >> 0; op &= ~0x00000001;
	int Z = (op & 0x00000006) >> 1; op &= ~0x00000006;
	int a = (op & 0x00000018) >> 3; op &= ~0x00000018;
	int D = (op & 0x00000020) >> 5; op &= ~0x00000020;
	int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
	
	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
	}


	output  += sprintf( output, "ST");
	output  += sprintf( output, "%s", datasize[Z]);
	output  += sprintf( output, "%s", addressmode[a]);
	output  += sprintf( output, "%s", cachebit[D]);
	output  += sprintf( output, " ");

	output  += sprintf( output, "[");
	if (breg == LIMM_REG) output  += sprintf( output, "(%08x), ", limm);
	else output  += sprintf( output, "%s, ", regnames[breg]);
	output  += sprintf( output, "%03x", sdat);
	output  += sprintf( output, "] <- ");

	if (C == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		output += sprintf(output, "(%08x)", limm);

	}
	else
	{
		output += sprintf(output, "%s", regnames[C]);
	}

	if (R) output  += sprintf( output, "(reserved bit set)");


	return size;
}


int arcompact_handle04_helper_dasm(DASM_OPS_32, const char* optext, int ignore_dst, int b_reserved)
{
	//           PP
	// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA
	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int F = (op & 0x00008000) >> 15; op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	
	if (!b_reserved)
	{
		if (breg == LIMM_REG)
		{
			GET_LIMM_32;
			size = 8;
			got_limm = 1;
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output += sprintf(output, " %s, ", regnames[breg]);
		}
	}
	else
	{
		if (breg) output += sprintf(output, "reserved(%s), ", regnames[breg]);
	}


	if (p == 0)
	{
		// 0010 0bbb 00ii iiii FBBB CCCC CCAA AAAA

		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			output  += sprintf( output, "(%08x) ", limm );
			if (ignore_dst == 0)
			{
				if (A != LIMM_REG) output += sprintf(output, "DST(%s)", regnames[A]);
				else output += sprintf(output, "<no dst>");
			}
			else
			{
				if (ignore_dst == 1) { if (A) output += sprintf(output, "unused(%s)", regnames[A]); }
				else { if (A != LIMM_REG) output += sprintf(output, "invalid(%s)", regnames[A]); } // mul operations expect A to be set to LIMM (no output)
			}
		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
			if (ignore_dst == 0)
			{
				if (A != LIMM_REG)  output += sprintf(output, "DST(%s)", regnames[A]);
				else output += sprintf(output, "<no dst>");
			}
			else
			{
				if (ignore_dst == 1) { if (A) output += sprintf(output, "unused(%s)", regnames[A]); }
				else { if (A != LIMM_REG) output += sprintf(output, "invalid(%s)", regnames[A]); } // mul operations expect A to be set to LIMM (no output)
			}

		}
	}
	else if (p == 1)
	{
		// 0010 0bbb 00ii iiii FBBB UUUU UUAA AAAA
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		output  += sprintf( output, "U(%02x) ", U );
		if (ignore_dst == 0)
		{
			if (A != LIMM_REG)  output += sprintf(output, "DST(%s)", regnames[A]);
			else output += sprintf(output, "<no dst>");
		}
		else
		{
			if (ignore_dst == 1) { if (A) output += sprintf(output, "unused(%s)", regnames[A]); }
			else { if (A != LIMM_REG) output += sprintf(output, "invalid(%s)", regnames[A]); } // mul operations expect A to be set to LIMM (no output)
		}
	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

		output  += sprintf( output, "S(%02x)", S);

	}
	else if (p == 3)
	{
		int M = (op & 0x00000020) >> 5; op &= ~0x00000020;
		COMMON32_GET_CONDITION	

		output  += sprintf( output, " M(%d)", M);
		output  += sprintf( output, " Cond<%s> ", conditions[condition]);

		if (M == 0)
		{
			int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
			
			if (C == LIMM_REG)
			{
				if (!got_limm)
				{
					GET_LIMM_32;
					size = 8;
				}
				output += sprintf(output, "(%08x)", limm);
			}
			else
			{
				output += sprintf(output, "C(%s)", regnames[C]);
			}

		}
		else if (M == 1)
		{
			int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
			output  += sprintf( output, "U(%02x)", U);

		}

	}

	return size;
}

int arcompact_handle04_00_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD", 0,0);
}

int arcompact_handle04_01_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADC", 0,0);
}

int arcompact_handle04_02_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB", 0,0);
}

int arcompact_handle04_03_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SBC", 0,0);
}

int arcompact_handle04_04_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "AND", 0,0);
}

int arcompact_handle04_05_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "OR", 0,0);
}

int arcompact_handle04_06_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BIC", 0,0);
}

int arcompact_handle04_07_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "XOR", 0,0);
}

int arcompact_handle04_08_dasm(DASM_OPS_32)  
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MAX", 0,0);
}

int arcompact_handle04_09_dasm(DASM_OPS_32) 
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MIN", 0,0);
}


int arcompact_handle04_0a_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MOV", 1,0);
}

int arcompact_handle04_0b_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "TST", 1,0);
}

int arcompact_handle04_0c_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "CMP", 1,0);
}

int arcompact_handle04_0d_dasm(DASM_OPS_32)
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "RCMP", 1,0);
}

int arcompact_handle04_0e_dasm(DASM_OPS_32)
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "RSUB", 0,0);
}

int arcompact_handle04_0f_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BSET", 0,0);
}

int arcompact_handle04_10_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BCLR", 0,0);
}

int arcompact_handle04_11_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BTST", 0,0);
}

int arcompact_handle04_12_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BXOR", 0,0);
}

int arcompact_handle04_13_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "BMSK", 0,0);
}

int arcompact_handle04_14_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD1", 0,0);
}

int arcompact_handle04_15_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD2", 0,0);
}

int arcompact_handle04_16_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADD3", 0,0);
}

int arcompact_handle04_17_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB1", 0,0);
}

int arcompact_handle04_18_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB2", 0,0);
}

int arcompact_handle04_19_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUB3", 0,0);
}

int arcompact_handle04_1a_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPY", 0,0);
} // *

int arcompact_handle04_1b_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYH", 0,0);
} // *

int arcompact_handle04_1c_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYHU", 0,0);
} // *

int arcompact_handle04_1d_dasm(DASM_OPS_32)  
{ 
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "MPYU", 0,0);
} // *



int arcompact_handle04_20_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "J", 1,1);
}



int arcompact_handle04_21_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "J.D", 1,1);
}

int arcompact_handle04_22_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "JL", 1,1);
}

int arcompact_handle04_23_dasm(DASM_OPS_32)
{
	return arcompact_handle04_helper_dasm(DASM_PARAMS, "JL.D", 1,1);
}




int arcompact_handle04_28_dasm(DASM_OPS_32) // LPcc (loop setup)
{
	COMMON32_GET_breg; // breg is reserved
	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;

	if (p == 0x00)
	{
		print("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		print("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		COMMON32_GET_s12
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		output += sprintf(output, "LP (start %08x, end %08x)", pc + 4, pc + S*2);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		int u = (op & 0x00000fc0)>>6;
		COMMON32_GET_CONDITION
		output += sprintf(output, "LP<%s> (start %08x, end %08x)", conditions[condition], pc + 4, pc + u*2);

		int unused = (op & 0x00000020)>>5;
		if (unused==0) 	output += sprintf(output, "(unused bit not set)");

	}

	if (breg) output += sprintf(output, "(reseved B bits set %02x)", breg);

	return 4;
}

#define PRINT_AUX_REGNAME \
		if ((auxreg >= 0) && (auxreg < 0x420)) \
		{ \
			if (strcmp(auxregnames[auxreg],"unusedreg")) \
				output += sprintf(output, "[%s]", auxregnames[auxreg]); \
			else \
				output  += sprintf( output, "[%03x]", auxreg); \
		} \
		else \
			output  += sprintf( output, "[%03x]", auxreg); \

int arcompact_handle04_2a_dasm(DASM_OPS_32)  // Load FROM Auxiliary register TO register
{

	//           pp        F
	// 0010 0bbb 0010 1010 0BBB CCCC CCRR RRRR
	// 0010 0bbb 0010 1010 0BBB 1111 10RR RRRR
	// 0010 0bbb 0110 1010 0BBB uuuu uu00 0000
	// 0010 0bbb 1010 1010 0BBB ssss ssSS SSSS


	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int F = (op & 0x00008000) >> 15; op &= ~0x00008000; // must be 0

	output  += sprintf( output, "LR");
	if (F) output  += sprintf( output, ".<F set, illegal>");
//	output  += sprintf( output, " p(%d)", p);
	
	

	if (breg == LIMM_REG)
	{
		output  += sprintf( output, "<no dest>" ); // illegal encoding?
	}
	else
	{
		output += sprintf(output, " %s, ", regnames[breg]);
	}



	if (p == 0)
	{

		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int res = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			output  += sprintf( output, "(%08x) ", limm );
	
		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
		}

		if (res) output  += sprintf( output, "reserved(%02x) ", res );
	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int res = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		int auxreg = U;
		PRINT_AUX_REGNAME
		
		if (res) output  += sprintf( output, "reserved(%02x) ", res );
	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

		int auxreg = S;
		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		output  += sprintf( output, " <mode 3, illegal>");
	}

	return size;
}

int arcompact_handle04_2b_dasm(DASM_OPS_32)  // Store TO Auxiliary register FROM register
{	
	// code at ~ 40073DFE in leapster bios is manually setting up a loop this way
	// rather than using the lPcc opcode

	int size = 4;
	UINT32 limm = 0;
	int got_limm = 0;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int F = (op & 0x00008000) >> 15; op &= ~0x00008000;

	output  += sprintf( output, "SR");
	if (F) output  += sprintf( output, ".<F set, illegal>");
//	output  += sprintf( output, " p(%d)", p);
	
	

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		output  += sprintf( output, " %08x -> ", limm );

	}
	else
	{
		output += sprintf(output, " %s -> ", regnames[breg]);
	}



	if (p == 0)
	{

		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int res = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		if (C == LIMM_REG)
		{
			if (!got_limm)
			{
				GET_LIMM_32;
				size = 8;
			}

			output  += sprintf( output, "[%08x]", limm );

		}
		else
		{
			output  += sprintf( output, "[%s]", regnames[C]);


		}

		if (res) output  += sprintf( output, " (reserved %02x) ", res );


	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
		int res = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

		int auxreg = U;
		PRINT_AUX_REGNAME

		if (res) output  += sprintf( output, " (reserved %02x) ", res );


	}
	else if (p == 2)
	{
		COMMON32_GET_s12;

		int auxreg = S;

		PRINT_AUX_REGNAME

	}
	else if (p == 3)
	{
		output  += sprintf( output, " <mode 3, illegal>");
	}

	return size;}


int arcompact_handle04_29_dasm(DASM_OPS_32)  { print("FLAG (%08x)", op); return 4;}


int arcompact_handle04_2f_helper_dasm(DASM_OPS_32, const char* optext)
{
	//           
	// 0010 0bbb pp10 1111 FBBB CCCC CCII IIII
	int size = 4;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int F = (op & 0x00008000) >> 15; op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	if (breg == LIMM_REG)
	{
		output += sprintf(output, " <no dst>, ");
		// if using the 'EX' opcode this is illegal
	}
	else
	{
		output += sprintf(output, " %s, ", regnames[breg]);
	}

	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		if (C == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;	
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
		}
	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		output  += sprintf( output, "U(0x%02x) ", U);
	}
	else if (p == 2)
	{
		output  += sprintf( output, "<04_2f illegal p=10>");
	}
	else if (p == 3)
	{
		output  += sprintf( output, "<04_2f illegal p=11>");
	}

	return size;
}


int arcompact_handle04_2f_00_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ASL"); } // ASL
int arcompact_handle04_2f_01_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ASR"); } // ASR
int arcompact_handle04_2f_02_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "LSR"); } // LSR
int arcompact_handle04_2f_03_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ROR"); } // ROR
int arcompact_handle04_2f_04_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "RCC"); } // RCC
int arcompact_handle04_2f_05_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "SEXB"); } // SEXB
int arcompact_handle04_2f_06_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "SEXW"); } // SEXW
int arcompact_handle04_2f_07_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EXTB"); } // EXTB
int arcompact_handle04_2f_08_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EXTW"); } // EXTW
int arcompact_handle04_2f_09_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "ABS"); } // ABS
int arcompact_handle04_2f_0a_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "NOT"); } // NOT
int arcompact_handle04_2f_0b_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "RCL"); } // RLC
int arcompact_handle04_2f_0c_dasm(DASM_OPS_32)  { return arcompact_handle04_2f_helper_dasm(DASM_PARAMS, "EX"); } // EX


int arcompact_handle04_2f_3f_01_dasm(DASM_OPS_32)  { print("SLEEP (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_02_dasm(DASM_OPS_32)  { print("SWI / TRAP0 (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_03_dasm(DASM_OPS_32)  { print("SYNC (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_04_dasm(DASM_OPS_32)  { print("RTIE (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_05_dasm(DASM_OPS_32)  { print("BRK (%08x)", op); return 4;}





// format on these is..

// 0010 0bbb aa11 0ZZX DBBB CCCC CCAA AAAA
// note, bits  11 0ZZX are part of the sub-opcode # already - this is a special encoding
int arcompact_handle04_3x_helper_dasm(DASM_OPS_32, int dsize, int extend)
{
	int size = 4;
	UINT32 limm=0;
	int got_limm = 0;

	output += sprintf(output, "LD");
	output += sprintf(output, "%s", datasize[dsize]);
	output += sprintf(output, "%s", dataextend[extend]);

	int mode = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int D = (op & 0x00008000) >> 15; op &= ~0x00008000;
	int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;
	int A = (op & 0x0000003f) >> 0; op &= ~0x0000003f;

	output += sprintf(output, "%s", addressmode[mode]);
	output += sprintf(output, "%s", cachebit[D]);

	output  += sprintf( output, " %s. ", regnames[A]);

	if (breg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;
		got_limm = 1;
		output  += sprintf( output, "[%08x, ", limm );

	}
	else
	{
		output += sprintf(output, "[%s, ", regnames[breg]);
	}

	if (C == LIMM_REG)
	{
		if (!got_limm)
		{
			GET_LIMM_32;
			size = 8;
		}
		output  += sprintf( output, "(%08x)]", limm );

	}
	else
	{
		output  += sprintf( output, "%s]", regnames[C]);
	}	


	return size;
	


}

int arcompact_handle04_30_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
int arcompact_handle04_31_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,0,1); }
int arcompact_handle04_32_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,1,0); }
int arcompact_handle04_33_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,1,1); }
int arcompact_handle04_34_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,2,0); }
int arcompact_handle04_35_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,2,1); }
// ZZ value of 0x3 is illegal
int arcompact_handle04_36_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,3,0); }
int arcompact_handle04_37_dasm(DASM_OPS_32)  { return arcompact_handle04_3x_helper_dasm(DASM_PARAMS,3,1); }






int arcompact_handle05_00_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASL", 0,0); }
int arcompact_handle05_01_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "LSR", 0,0); }
int arcompact_handle05_02_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASR", 0,0); }
int arcompact_handle05_03_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ROR", 0,0); }
int arcompact_handle05_04_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MUL64", 2,0); } // special
int arcompact_handle05_05_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "MULU64", 2,0);} // special
int arcompact_handle05_06_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADDS", 0,0); }
int arcompact_handle05_07_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUBS", 0,0); }
int arcompact_handle05_08_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "DIVAW", 0,0); }



int arcompact_handle05_0a_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASLS", 0,0); }
int arcompact_handle05_0b_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ASRS", 0,0); }

int arcompact_handle05_28_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "ADDSDW", 0,0); }
int arcompact_handle05_29_dasm(DASM_OPS_32)  { return arcompact_handle04_helper_dasm(DASM_PARAMS, "SUBSDW", 0,0); }



int arcompact_handle05_2f_0x_helper_dasm(DASM_OPS_32, const char* optext)
{
	//           
	// 0010 1bbb pp10 1111 FBBB CCCC CCII IIII when pp == 0x00
	// or
	// 0010 1bbb pp10 1111 FBBB UUUU UUII IIII when pp == 0x01
	// otherwise invalid

	int size = 4;

	int p = (op & 0x00c00000) >> 22; op &= ~0x00c00000;
	COMMON32_GET_breg;
	int F = (op & 0x00008000) >> 15;op &= ~0x00008000;

	output  += sprintf( output, "%s", optext);
	output  += sprintf( output, "%s", flagbit[F]);
//	output  += sprintf( output, " p(%d)", p);
	
	
	output += sprintf(output, " %s, ", regnames[breg]);

	if (p == 0)
	{
		int C = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		if (C == LIMM_REG)
		{
			UINT32 limm;
			GET_LIMM_32;
			size = 8;	
			output  += sprintf( output, "(%08x) ", limm );

		}
		else
		{
			output  += sprintf( output, "C(%s) ", regnames[C]);
		}
	}
	else if (p == 1)
	{
		int U = (op & 0x00000fc0) >> 6; op &= ~0x00000fc0;

		output  += sprintf( output, "U(0x%02x) ", U);
	}
	else if (p == 2)
	{
		output  += sprintf( output, "<05_2f illegal p=10>");
	}
	else if (p == 3)
	{
		output  += sprintf( output, "<05_2f illegal p=11>");
	}

	return size;
}


int arcompact_handle05_2f_00_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "SWAP");  }
int arcompact_handle05_2f_01_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NORM");  }
int arcompact_handle05_2f_02_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "SAT16"); }
int arcompact_handle05_2f_03_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "RND16"); }
int arcompact_handle05_2f_04_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "ABSSW"); }
int arcompact_handle05_2f_05_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "ABSS");  }
int arcompact_handle05_2f_06_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NEGSW"); }
int arcompact_handle05_2f_07_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NEGS");  }
int arcompact_handle05_2f_08_dasm(DASM_OPS_32)  { return arcompact_handle05_2f_0x_helper_dasm(DASM_PARAMS, "NORMW"); }


int arcompact_handle06_dasm(DASM_OPS_32)
{
	print("op a,b,c (06 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_handle07_dasm(DASM_OPS_32)
{
	print("op a,b,c (07 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle08_dasm(DASM_OPS_32)
{
	print("op a,b,c (08 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle09_dasm(DASM_OPS_32)
{
	print("op a,b,c (09 Market ext) (%08x)", op );
	return 4;
}

int arcompact_handle0a_dasm(DASM_OPS_32)
{
	print("op a,b,c (0a Market ext) (%08x)",  op );
	return 4;
}

int arcompact_handle0b_dasm(DASM_OPS_32)
{
	print("op a,b,c (0b Market ext) (%08x)",  op );
	return 4;
}



int arcompact_handle0c_helper_dasm(DASM_OPS_16, const char* optext)
{
	int areg, breg, creg;

	COMMON16_GET_areg;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(areg);
	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	print("%s %s <- [%s, %s]", optext, regnames[areg], regnames[breg], regnames[creg]);
	return 2;
}


int arcompact_handle0c_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LD_S");
}

int arcompact_handle0c_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDB_S");
}

int arcompact_handle0c_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "LDW_S");
}

int arcompact_handle0c_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0c_helper_dasm(DASM_PARAMS, "ADD_S");
}


int arcompact_handle0d_helper_dasm(DASM_OPS_16, const char* optext)
{
	int u, breg, creg;

	COMMON16_GET_u3;
	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	print("%s %s <- [%s, 0x%02x]", optext, regnames[creg], regnames[breg], u);
	return 2;
}


int arcompact_handle0d_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ADD_S");
}

int arcompact_handle0d_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "SUB_S");
}

int arcompact_handle0d_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ASL_S");
}

int arcompact_handle0d_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0d_helper_dasm(DASM_PARAMS, "ASR_S");
}



int arcompact_handle0e_0x_helper_dasm(DASM_OPS_16, const char* optext, int revop)
{
	int h,breg;
	int size = 2;

	GROUP_0e_GET_h;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);
	
	if (h == LIMM_REG)
	{
		UINT32 limm;
		GET_LIMM;
		size = 6;
		if (!revop) print("%s %s, (%08x) (%04x)", optext, regnames[breg], limm, op);
		else print("%s (%08x), %s (%04x)", optext, limm, regnames[breg], op);
	}
	else
	{
		if (!revop) print("%s %s, %s (%04x)", optext, regnames[breg], regnames[h], op);
		else print("%s %s, %s (%04x)", optext, regnames[h], regnames[breg], op);

	}

	return size;

}

int arcompact_handle0e_00_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "ADD_S", 0);
}

int arcompact_handle0e_01_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "MOV_S", 0);
}

int arcompact_handle0e_02_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "CMP_S", 0);
}

int arcompact_handle0e_03_dasm(DASM_OPS_16)
{
	return arcompact_handle0e_0x_helper_dasm(DASM_PARAMS, "MOV_S", 1);
}



int arcompact_handle0f_00_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg;

	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);
	
	print("%s %s", optext, regnames[breg]);

	return 2;

}



int arcompact_handle0f_00_00_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "J_S"); }
int arcompact_handle0f_00_01_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "J_S.D"); }
int arcompact_handle0f_00_02_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "JL_S");  }
int arcompact_handle0f_00_03_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "JL_S.D");  }
int arcompact_handle0f_00_06_dasm(DASM_OPS_16)  { return arcompact_handle0f_00_0x_helper_dasm(DASM_PARAMS, "SUB_S.NE"); }




// Zero parameters (ZOP)
int arcompact_handle0f_00_07_00_dasm(DASM_OPS_16)  { print("NOP_S"); return 2;}
int arcompact_handle0f_00_07_01_dasm(DASM_OPS_16)  { print("UNIMP_S"); return 2;} // Unimplemented Instruction, same as illegal, but recommended to fill blank space
int arcompact_handle0f_00_07_04_dasm(DASM_OPS_16)  { print("JEQ_S [blink]"); return 2;}
int arcompact_handle0f_00_07_05_dasm(DASM_OPS_16)  { print("JNE_S [blink]"); return 2;}
int arcompact_handle0f_00_07_06_dasm(DASM_OPS_16)  { print("J_S [blink]"); return 2;}
int arcompact_handle0f_00_07_07_dasm(DASM_OPS_16)  { print("J_S.D [blink]"); return 2;}





int arcompact_handle0f_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg, creg;

	COMMON16_GET_breg;
	COMMON16_GET_creg;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);


	print("%s %s <- %s", optext, regnames[breg], regnames[creg]);
	return 2;
}

int arcompact_handle0f_02_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SUB_S");}
int arcompact_handle0f_04_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "AND_S");  }
int arcompact_handle0f_05_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "OR_S");   }
int arcompact_handle0f_06_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "BIC_S");  }
int arcompact_handle0f_07_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "XOR_S");  }
int arcompact_handle0f_0b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "TST_S");  }
int arcompact_handle0f_0d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXB_S"); }
int arcompact_handle0f_0e_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "SEXW_S"); }
int arcompact_handle0f_0f_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTB_S"); }
int arcompact_handle0f_10_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "EXTW_S"); }
int arcompact_handle0f_11_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ABS_S");  }
int arcompact_handle0f_12_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NOT_S");  }
int arcompact_handle0f_13_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "NEG_S");  }
int arcompact_handle0f_14_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD1_S"); }
int arcompact_handle0f_15_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD2_S"); }
int arcompact_handle0f_16_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ADD3_S"); }
int arcompact_handle0f_18_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL_S");  }
int arcompact_handle0f_19_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR_S");  }
int arcompact_handle0f_1a_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR_S");  }
int arcompact_handle0f_1b_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASL1_S"); }
int arcompact_handle0f_1c_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "ASR1_S"); }
int arcompact_handle0f_1d_dasm(DASM_OPS_16)  { return arcompact_handle0f_0x_helper_dasm(DASM_PARAMS, "LSR1_S"); }

int arcompact_handle0f_0c_dasm(DASM_OPS_16)  { print("MUL64_S mulres <- b * c  (%08x)", op); return 2;} // special
int arcompact_handle0f_1e_dasm(DASM_OPS_16)  { print("TRAP_S (%08x)", op); return 2;} // special

int arcompact_handle0f_1f_dasm(DASM_OPS_16)  // special
{
	int bc = (op & 0x07e0)>>5; op &= ~0x07e0;

	if (bc == 0x003f)
	{
		print("BRK_S");
	}
	else
	{
		print("<illegal BRK_S>");
	}
	return 2;
}


int arcompact_handle_ld_helper_dasm(DASM_OPS_16, const char* optext, int shift, int swap)
{
	int breg, creg, u;

	COMMON16_GET_breg;
	COMMON16_GET_creg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);
	REG_16BIT_RANGE(creg);

	u <<= shift;

	if (!swap) print("%s %s, [%s, 0x%02x] (%04x)", optext, regnames[creg], regnames[breg], u, op);
	else  print("%s [%s, 0x%02x], %s (%04x)", optext, regnames[breg], u, regnames[creg], op);
	return 2;

}


int arcompact_handle10_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LD_S", 2, 0);
}

int arcompact_handle11_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDB_S", 0, 0);
}

int arcompact_handle12_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDW_S", 1, 0);
}

int arcompact_handle13_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "LDW_S.X", 1, 0);
}

int arcompact_handle14_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "ST_S", 2, 1);
}

int arcompact_handle15_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "STB_S", 0, 1);
}

int arcompact_handle16_dasm(DASM_OPS_16)
{
	return arcompact_handle_ld_helper_dasm(DASM_PARAMS, "STW_S", 1, 1);
}


int arcompact_handle_l7_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	print("%s %s, 0x%02x", optext, regnames[breg], u);

	return 2;

}

int arcompact_handle17_00_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "ASL_S");
}

int arcompact_handle17_01_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "LSR_S");
}

int arcompact_handle17_02_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "ASR_S");
}

int arcompact_handle17_03_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "SUB_S");
}

int arcompact_handle17_04_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BSET_S");
}

int arcompact_handle17_05_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BCLR_S");
}

int arcompact_handle17_06_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BSMK_S");
}

int arcompact_handle17_07_dasm(DASM_OPS_16)
{
	return arcompact_handle_l7_0x_helper_dasm(DASM_PARAMS, "BTST_S");
}


// op bits remaining for 0x18_xx subgroups 0x071f 

int arcompact_handle18_0x_helper_dasm(DASM_OPS_16, const char* optext, int st)
{
	int breg, u;

	COMMON16_GET_breg;
	COMMON16_GET_u5;

	REG_16BIT_RANGE(breg);

	output  += sprintf( output, "%s %s ", optext, regnames[breg]);
	if (st==1) output  += sprintf( output, "-> ");
	else output  += sprintf( output, "<- ");
	output  += sprintf( output, "[SP, 0x%02x]", u*4);

	return 2;
}

int arcompact_handle18_00_dasm(DASM_OPS_16) 
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "LD_S", 0);
}

int arcompact_handle18_01_dasm(DASM_OPS_16) 
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "LDB_S", 0);
}

int arcompact_handle18_02_dasm(DASM_OPS_16) 
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "ST_S", 1);
}

int arcompact_handle18_03_dasm(DASM_OPS_16) 
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "STB_S", 1);
}

int arcompact_handle18_04_dasm(DASM_OPS_16) 
{
	return arcompact_handle18_0x_helper_dasm(DASM_PARAMS, "ADD_S", 1); // check format
}

// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_handle18_05_00_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("ADD_S SP, SP, %02x", u*4);
	return 2;

}

int arcompact_handle18_05_01_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("SUB_S SP, SP, %02x", u*4);
	return 2;
}

// op bits remaining for 0x18_06_xx subgroups 0x0700 
int arcompact_handle18_06_01_dasm(DASM_OPS_16) 
{
	int breg = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used
	REG_16BIT_RANGE(breg)

	print("POP_S %s", regnames[breg]);

	return 2;
}

int arcompact_handle18_06_11_dasm(DASM_OPS_16) 
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("POP_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("POP_S [BLINK]");

	return 2;
}

// op bits remaining for 0x18_07_xx subgroups 0x0700 
int arcompact_handle18_07_01_dasm(DASM_OPS_16) 
{
	int breg = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used
	REG_16BIT_RANGE(breg)

	print("PUSH_S %s", regnames[breg]);

	return 2;
}


int arcompact_handle18_07_11_dasm(DASM_OPS_16) 
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("PUSH_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("PUSH_S [BLINK]");

	return 2;
}

int arcompact_handle19_00_dasm(DASM_OPS_16)  { print("LD_S r0 <- m[GP + s11].long (%04x)",  op); return 2;}
int arcompact_handle19_01_dasm(DASM_OPS_16)  { print("LDB_S r0 <- m[GP + s9].byte (%04x)",  op); return 2;}
int arcompact_handle19_02_dasm(DASM_OPS_16)  { print("LDW_S r0 <- m[GP + s10].word (%04x)",  op); return 2;}
int arcompact_handle19_03_dasm(DASM_OPS_16)  { print("ADD_S r0 <- GP + s11 (%04x)",  op); return 2;}

int arcompact_handle1a_dasm(DASM_OPS_16)
{
	print("PCL Instr (%04x)", op);
	return 2;
}

int arcompact_handle1b_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u8;
	REG_16BIT_RANGE(breg);

	print("MOV_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1c_00_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	print("ADD_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1c_01_dasm(DASM_OPS_16)
{
	int breg, u;
	COMMON16_GET_breg;
	COMMON16_GET_u7;
	REG_16BIT_RANGE(breg);

	print("CMP_S %s, %02x", regnames[breg], u);
	return 2;
}

int arcompact_handle1d_helper_dasm(DASM_OPS_16, const char* optext)
{
	int breg;
	COMMON16_GET_breg;
	REG_16BIT_RANGE(breg);

	int s = (op & 0x007f) >> 0;	op &= ~0x007f;
	if (s & 0x40) s = -0x40 + (s & 0x3f);

	print("%s %s %08x", optext, regnames[breg], PC_ALIGNED32 + s*2);
	return 2;
}


int arcompact_handle1d_00_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BREQ_S"); }
int arcompact_handle1d_01_dasm(DASM_OPS_16)  { return arcompact_handle1d_helper_dasm(DASM_PARAMS,"BRNE_S"); }


int arcompact_handle1e_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x01ff) >> 0;	op &= ~0x01ff;
	if (s & 0x100) s = -0x100 + (s & 0xff);

	print("%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}



int arcompact_handle1e_00_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BL_S");  }
int arcompact_handle1e_01_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BEQ_S"); }
int arcompact_handle1e_02_dasm(DASM_OPS_16)  { return arcompact_handle1e_0x_helper_dasm(DASM_PARAMS, "BNE_S"); }

int arcompact_handle1e_03_0x_helper_dasm(DASM_OPS_16, const char* optext)
{
	int s = (op & 0x003f) >> 0;	op &= ~0x003f;
	if (s & 0x020) s = -0x20 + (s & 0x1f);

	print("%s %08x", optext, PC_ALIGNED32 + s*2);
	return 2;
}

int arcompact_handle1e_03_00_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BGT_S"); }
int arcompact_handle1e_03_01_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BGE_S"); }
int arcompact_handle1e_03_02_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLT_S"); }
int arcompact_handle1e_03_03_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLE_S"); }
int arcompact_handle1e_03_04_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BHI_S"); }
int arcompact_handle1e_03_05_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BHS_S"); }
int arcompact_handle1e_03_06_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLO_S"); }
int arcompact_handle1e_03_07_dasm(DASM_OPS_16)  { return arcompact_handle1e_03_0x_helper_dasm(DASM_PARAMS, "BLS_S"); }

int arcompact_handle1f_dasm(DASM_OPS_16)
{
	int s = (op & 0x07ff) >> 0;	op &= ~0x07ff;
	if (s & 0x400) s = -0x400 + (s & 0x3ff);

	print("BL_S %08x", PC_ALIGNED32 + (s*4));
	return 2;
}

/************************************************************************************************************************************
*                                                                                                                                   *
* illegal opcode handlers (disassembly)                                                                                             *
*                                                                                                                                   *
************************************************************************************************************************************/

int arcompact_handle01_01_00_06_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_06> (%08x)", op); return 4; }
int arcompact_handle01_01_00_07_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_07> (%08x)", op); return 4; }
int arcompact_handle01_01_00_08_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_08> (%08x)", op); return 4; }
int arcompact_handle01_01_00_09_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_09> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0a_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0a> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0b_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0b> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0c_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0c> (%08x)", op); return 4; }
int arcompact_handle01_01_00_0d_dasm(DASM_OPS_32)  { print("<illegal 01_01_00_0d> (%08x)", op); return 4; }

int arcompact_handle01_01_01_06_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_06> (%08x)", op); return 4; }
int arcompact_handle01_01_01_07_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_07> (%08x)", op); return 4; }
int arcompact_handle01_01_01_08_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_08> (%08x)", op); return 4; }
int arcompact_handle01_01_01_09_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_09> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0a_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0a> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0b_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0b> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0c_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0c> (%08x)", op); return 4; }
int arcompact_handle01_01_01_0d_dasm(DASM_OPS_32)  { print("<illegal 01_01_01_0d> (%08x)", op); return 4; }


int arcompact_handle04_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_1e> (%08x)", op); return 4;}
int arcompact_handle04_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_1f> (%08x)", op); return 4;}

int arcompact_handle04_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_24> (%08x)", op); return 4;}
int arcompact_handle04_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_25> (%08x)", op); return 4;}
int arcompact_handle04_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_26> (%08x)", op); return 4;}
int arcompact_handle04_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_27> (%08x)", op); return 4;}

int arcompact_handle04_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2c> (%08x)", op); return 4;}
int arcompact_handle04_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2d> (%08x)", op); return 4;}
int arcompact_handle04_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2e> (%08x)", op); return 4;}

int arcompact_handle04_2f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0d> (%08x)", op); return 4;}
int arcompact_handle04_2f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0e> (%08x)", op); return 4;}
int arcompact_handle04_2f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_0f> (%08x)", op); return 4;}
int arcompact_handle04_2f_10_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_10> (%08x)", op); return 4;}
int arcompact_handle04_2f_11_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_11> (%08x)", op); return 4;}
int arcompact_handle04_2f_12_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_12> (%08x)", op); return 4;}
int arcompact_handle04_2f_13_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_13> (%08x)", op); return 4;}
int arcompact_handle04_2f_14_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_14> (%08x)", op); return 4;}
int arcompact_handle04_2f_15_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_15> (%08x)", op); return 4;}
int arcompact_handle04_2f_16_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_16> (%08x)", op); return 4;}
int arcompact_handle04_2f_17_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_17> (%08x)", op); return 4;}
int arcompact_handle04_2f_18_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_18> (%08x)", op); return 4;}
int arcompact_handle04_2f_19_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_19> (%08x)", op); return 4;}
int arcompact_handle04_2f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1a> (%08x)", op); return 4;}
int arcompact_handle04_2f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1b> (%08x)", op); return 4;}
int arcompact_handle04_2f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1c> (%08x)", op); return 4;}
int arcompact_handle04_2f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1d> (%08x)", op); return 4;}
int arcompact_handle04_2f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1e> (%08x)", op); return 4;}
int arcompact_handle04_2f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_1f> (%08x)", op); return 4;}
int arcompact_handle04_2f_20_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_20> (%08x)", op); return 4;}
int arcompact_handle04_2f_21_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_21> (%08x)", op); return 4;}
int arcompact_handle04_2f_22_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_22> (%08x)", op); return 4;}
int arcompact_handle04_2f_23_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_23> (%08x)", op); return 4;}
int arcompact_handle04_2f_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_24> (%08x)", op); return 4;}
int arcompact_handle04_2f_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_25> (%08x)", op); return 4;}
int arcompact_handle04_2f_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_26> (%08x)", op); return 4;}
int arcompact_handle04_2f_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_27> (%08x)", op); return 4;}
int arcompact_handle04_2f_28_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_28> (%08x)", op); return 4;}
int arcompact_handle04_2f_29_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_29> (%08x)", op); return 4;}
int arcompact_handle04_2f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2a> (%08x)", op); return 4;}
int arcompact_handle04_2f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2b> (%08x)", op); return 4;}
int arcompact_handle04_2f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2c> (%08x)", op); return 4;}
int arcompact_handle04_2f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2d> (%08x)", op); return 4;}
int arcompact_handle04_2f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2e> (%08x)", op); return 4;}
int arcompact_handle04_2f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_2f> (%08x)", op); return 4;}
int arcompact_handle04_2f_30_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_30> (%08x)", op); return 4;}
int arcompact_handle04_2f_31_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_31> (%08x)", op); return 4;}
int arcompact_handle04_2f_32_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_32> (%08x)", op); return 4;}
int arcompact_handle04_2f_33_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_33> (%08x)", op); return 4;}
int arcompact_handle04_2f_34_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_34> (%08x)", op); return 4;}
int arcompact_handle04_2f_35_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_35> (%08x)", op); return 4;}
int arcompact_handle04_2f_36_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_36> (%08x)", op); return 4;}
int arcompact_handle04_2f_37_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_37> (%08x)", op); return 4;}
int arcompact_handle04_2f_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_38> (%08x)", op); return 4;}
int arcompact_handle04_2f_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_39> (%08x)", op); return 4;}
int arcompact_handle04_2f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3e> (%08x)", op); return 4;}



int arcompact_handle05_2f_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_09> (%08x)", op); return 4;}
int arcompact_handle05_2f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0a> (%08x)", op); return 4;}
int arcompact_handle05_2f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0b> (%08x)", op); return 4;}
int arcompact_handle05_2f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0c> (%08x)", op); return 4;}
int arcompact_handle05_2f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0d> (%08x)", op); return 4;}
int arcompact_handle05_2f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0e> (%08x)", op); return 4;}
int arcompact_handle05_2f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_0f> (%08x)", op); return 4;}
int arcompact_handle05_2f_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_10> (%08x)", op); return 4;}
int arcompact_handle05_2f_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_11> (%08x)", op); return 4;}
int arcompact_handle05_2f_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_12> (%08x)", op); return 4;}
int arcompact_handle05_2f_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_13> (%08x)", op); return 4;}
int arcompact_handle05_2f_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_14> (%08x)", op); return 4;}
int arcompact_handle05_2f_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_15> (%08x)", op); return 4;}
int arcompact_handle05_2f_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_16> (%08x)", op); return 4;}
int arcompact_handle05_2f_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_17> (%08x)", op); return 4;}
int arcompact_handle05_2f_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_18> (%08x)", op); return 4;}
int arcompact_handle05_2f_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_19> (%08x)", op); return 4;}
int arcompact_handle05_2f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1a> (%08x)", op); return 4;}
int arcompact_handle05_2f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1b> (%08x)", op); return 4;}
int arcompact_handle05_2f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1c> (%08x)", op); return 4;}
int arcompact_handle05_2f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1d> (%08x)", op); return 4;}
int arcompact_handle05_2f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1e> (%08x)", op); return 4;}
int arcompact_handle05_2f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_1f> (%08x)", op); return 4;}
int arcompact_handle05_2f_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_20> (%08x)", op); return 4;}
int arcompact_handle05_2f_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_21> (%08x)", op); return 4;}
int arcompact_handle05_2f_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_22> (%08x)", op); return 4;}
int arcompact_handle05_2f_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_23> (%08x)", op); return 4;}
int arcompact_handle05_2f_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_24> (%08x)", op); return 4;}
int arcompact_handle05_2f_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_25> (%08x)", op); return 4;}
int arcompact_handle05_2f_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_26> (%08x)", op); return 4;}
int arcompact_handle05_2f_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_27> (%08x)", op); return 4;}
int arcompact_handle05_2f_28_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_28> (%08x)", op); return 4;}
int arcompact_handle05_2f_29_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_29> (%08x)", op); return 4;}
int arcompact_handle05_2f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2a> (%08x)", op); return 4;}
int arcompact_handle05_2f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2b> (%08x)", op); return 4;}
int arcompact_handle05_2f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2c> (%08x)", op); return 4;}
int arcompact_handle05_2f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2d> (%08x)", op); return 4;}
int arcompact_handle05_2f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2e> (%08x)", op); return 4;}
int arcompact_handle05_2f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_2f> (%08x)", op); return 4;}
int arcompact_handle05_2f_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_30> (%08x)", op); return 4;}
int arcompact_handle05_2f_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_31> (%08x)", op); return 4;}
int arcompact_handle05_2f_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_32> (%08x)", op); return 4;}
int arcompact_handle05_2f_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_33> (%08x)", op); return 4;}
int arcompact_handle05_2f_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_34> (%08x)", op); return 4;}
int arcompact_handle05_2f_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_35> (%08x)", op); return 4;}
int arcompact_handle05_2f_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_36> (%08x)", op); return 4;}
int arcompact_handle05_2f_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_37> (%08x)", op); return 4;}
int arcompact_handle05_2f_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_38> (%08x)", op); return 4;}
int arcompact_handle05_2f_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_39> (%08x)", op); return 4;}
int arcompact_handle05_2f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3e> (%08x)", op); return 4;}


int arcompact_handle04_2f_3f_00_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_00> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_06_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_06> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_07_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_07> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_08_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_08> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_09_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_09> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_0f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_10_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_10> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_11_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_11> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_12_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_12> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_13_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_13> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_14_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_14> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_15_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_15> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_16_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_16> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_17_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_17> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_18_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_18> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_19_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_19> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_1f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_20_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_20> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_21_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_21> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_22_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_22> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_23_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_23> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_24_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_24> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_25_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_25> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_26_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_26> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_27_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_27> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_28_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_28> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_29_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_29> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_2f> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_30_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_30> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_31_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_31> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_32_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_32> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_33_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_33> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_34_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_34> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_35_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_35> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_36_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_36> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_37_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_37> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_38> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_39> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3a> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3b> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3c> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3d> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3e> (%08x)", op); return 4;}
int arcompact_handle04_2f_3f_3f_dasm(DASM_OPS_32)  { print("<illegal 0x04_2f_3f_3f> (%08x)", op); return 4;}

int arcompact_handle05_2f_3f_00_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_00> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_01_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_01> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_02_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_02> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_03_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_03> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_04_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_04> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_05_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_05> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_06_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_06> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_07_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_07> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_08_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_08> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_09> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_0f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_10> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_11> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_12> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_13> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_14> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_15> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_16> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_17> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_18> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_19> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_1f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_20> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_21> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_22> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_23> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_24> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_25> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_26> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_27> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_28_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_28> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_29_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_29> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_2f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_2f> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_30> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_31> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_32> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_33> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_34> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_35> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_36> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_37> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_38> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_39> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3a> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3b> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3c> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3d> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3e> (%08x)", op); return 4;}
int arcompact_handle05_2f_3f_3f_dasm(DASM_OPS_32)  { print("<illegal 0x05_2f_3f_3f> (%08x)", op); return 4;}




int arcompact_handle04_38_dasm(DASM_OPS_32)  { print("<illegal 0x04_38> (%08x)", op); return 4;}
int arcompact_handle04_39_dasm(DASM_OPS_32)  { print("<illegal 0x04_39> (%08x)", op); return 4;}
int arcompact_handle04_3a_dasm(DASM_OPS_32)  { print("<illegal 0x04_3a> (%08x)", op); return 4;}
int arcompact_handle04_3b_dasm(DASM_OPS_32)  { print("<illegal 0x04_3b> (%08x)", op); return 4;}
int arcompact_handle04_3c_dasm(DASM_OPS_32)  { print("<illegal 0x04_3c> (%08x)", op); return 4;}
int arcompact_handle04_3d_dasm(DASM_OPS_32)  { print("<illegal 0x04_3d> (%08x)", op); return 4;}
int arcompact_handle04_3e_dasm(DASM_OPS_32)  { print("<illegal 0x04_3e> (%08x)", op); return 4;}
int arcompact_handle04_3f_dasm(DASM_OPS_32)  { print("<illegal 0x04_3f> (%08x)", op); return 4;}


int arcompact_handle05_09_dasm(DASM_OPS_32)  { print("<illegal 0x05_09> (%08x)", op); return 4;}
int arcompact_handle05_0c_dasm(DASM_OPS_32)  { print("<illegal 0x05_0c> (%08x)", op); return 4;}
int arcompact_handle05_0d_dasm(DASM_OPS_32)  { print("<illegal 0x05_0d> (%08x)", op); return 4;}
int arcompact_handle05_0e_dasm(DASM_OPS_32)  { print("<illegal 0x05_0e> (%08x)", op); return 4;}
int arcompact_handle05_0f_dasm(DASM_OPS_32)  { print("<illegal 0x05_0f> (%08x)", op); return 4;}
int arcompact_handle05_10_dasm(DASM_OPS_32)  { print("<illegal 0x05_10> (%08x)", op); return 4;}
int arcompact_handle05_11_dasm(DASM_OPS_32)  { print("<illegal 0x05_11> (%08x)", op); return 4;}
int arcompact_handle05_12_dasm(DASM_OPS_32)  { print("<illegal 0x05_12> (%08x)", op); return 4;}
int arcompact_handle05_13_dasm(DASM_OPS_32)  { print("<illegal 0x05_13> (%08x)", op); return 4;}
int arcompact_handle05_14_dasm(DASM_OPS_32)  { print("<illegal 0x05_14> (%08x)", op); return 4;}
int arcompact_handle05_15_dasm(DASM_OPS_32)  { print("<illegal 0x05_15> (%08x)", op); return 4;}
int arcompact_handle05_16_dasm(DASM_OPS_32)  { print("<illegal 0x05_16> (%08x)", op); return 4;}
int arcompact_handle05_17_dasm(DASM_OPS_32)  { print("<illegal 0x05_17> (%08x)", op); return 4;}
int arcompact_handle05_18_dasm(DASM_OPS_32)  { print("<illegal 0x05_18> (%08x)", op); return 4;}
int arcompact_handle05_19_dasm(DASM_OPS_32)  { print("<illegal 0x05_19> (%08x)", op); return 4;}
int arcompact_handle05_1a_dasm(DASM_OPS_32)  { print("<illegal 0x05_1a> (%08x)", op); return 4;}
int arcompact_handle05_1b_dasm(DASM_OPS_32)  { print("<illegal 0x05_1b> (%08x)", op); return 4;}
int arcompact_handle05_1c_dasm(DASM_OPS_32)  { print("<illegal 0x05_1c> (%08x)", op); return 4;}
int arcompact_handle05_1d_dasm(DASM_OPS_32)  { print("<illegal 0x05_1d> (%08x)", op); return 4;}
int arcompact_handle05_1e_dasm(DASM_OPS_32)  { print("<illegal 0x05_1e> (%08x)", op); return 4;}
int arcompact_handle05_1f_dasm(DASM_OPS_32)  { print("<illegal 0x05_1f> (%08x)", op); return 4;}
int arcompact_handle05_20_dasm(DASM_OPS_32)  { print("<illegal 0x05_20> (%08x)", op); return 4;}
int arcompact_handle05_21_dasm(DASM_OPS_32)  { print("<illegal 0x05_21> (%08x)", op); return 4;}
int arcompact_handle05_22_dasm(DASM_OPS_32)  { print("<illegal 0x05_22> (%08x)", op); return 4;}
int arcompact_handle05_23_dasm(DASM_OPS_32)  { print("<illegal 0x05_23> (%08x)", op); return 4;}
int arcompact_handle05_24_dasm(DASM_OPS_32)  { print("<illegal 0x05_24> (%08x)", op); return 4;}
int arcompact_handle05_25_dasm(DASM_OPS_32)  { print("<illegal 0x05_25> (%08x)", op); return 4;}
int arcompact_handle05_26_dasm(DASM_OPS_32)  { print("<illegal 0x05_26> (%08x)", op); return 4;}
int arcompact_handle05_27_dasm(DASM_OPS_32)  { print("<illegal 0x05_27> (%08x)", op); return 4;}

int arcompact_handle05_2a_dasm(DASM_OPS_32)  { print("<illegal 0x05_2a> (%08x)", op); return 4;}
int arcompact_handle05_2b_dasm(DASM_OPS_32)  { print("<illegal 0x05_2b> (%08x)", op); return 4;}
int arcompact_handle05_2c_dasm(DASM_OPS_32)  { print("<illegal 0x05_2c> (%08x)", op); return 4;}
int arcompact_handle05_2d_dasm(DASM_OPS_32)  { print("<illegal 0x05_2d> (%08x)", op); return 4;}
int arcompact_handle05_2e_dasm(DASM_OPS_32)  { print("<illegal 0x05_2e> (%08x)", op); return 4;}

int arcompact_handle05_30_dasm(DASM_OPS_32)  { print("<illegal 0x05_30> (%08x)", op); return 4;}
int arcompact_handle05_31_dasm(DASM_OPS_32)  { print("<illegal 0x05_31> (%08x)", op); return 4;}
int arcompact_handle05_32_dasm(DASM_OPS_32)  { print("<illegal 0x05_32> (%08x)", op); return 4;}
int arcompact_handle05_33_dasm(DASM_OPS_32)  { print("<illegal 0x05_33> (%08x)", op); return 4;}
int arcompact_handle05_34_dasm(DASM_OPS_32)  { print("<illegal 0x05_34> (%08x)", op); return 4;}
int arcompact_handle05_35_dasm(DASM_OPS_32)  { print("<illegal 0x05_35> (%08x)", op); return 4;}
int arcompact_handle05_36_dasm(DASM_OPS_32)  { print("<illegal 0x05_36> (%08x)", op); return 4;}
int arcompact_handle05_37_dasm(DASM_OPS_32)  { print("<illegal 0x05_37> (%08x)", op); return 4;}
int arcompact_handle05_38_dasm(DASM_OPS_32)  { print("<illegal 0x05_38> (%08x)", op); return 4;}
int arcompact_handle05_39_dasm(DASM_OPS_32)  { print("<illegal 0x05_39> (%08x)", op); return 4;}
int arcompact_handle05_3a_dasm(DASM_OPS_32)  { print("<illegal 0x05_3a> (%08x)", op); return 4;}
int arcompact_handle05_3b_dasm(DASM_OPS_32)  { print("<illegal 0x05_3b> (%08x)", op); return 4;}
int arcompact_handle05_3c_dasm(DASM_OPS_32)  { print("<illegal 0x05_3c> (%08x)", op); return 4;}
int arcompact_handle05_3d_dasm(DASM_OPS_32)  { print("<illegal 0x05_3d> (%08x)", op); return 4;}
int arcompact_handle05_3e_dasm(DASM_OPS_32)  { print("<illegal 0x05_3e> (%08x)", op); return 4;}
int arcompact_handle05_3f_dasm(DASM_OPS_32)  { print("<illegal 0x05_3f> (%08x)", op); return 4;}

int arcompact_handle0f_00_04_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_00> (%08x)", op); return 2;}
int arcompact_handle0f_00_05_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_00> (%08x)", op); return 2;}
int arcompact_handle0f_00_07_02_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_07_02> (%08x)", op); return 2;}
int arcompact_handle0f_00_07_03_dasm(DASM_OPS_16)  { print("<illegal 0x0f_00_07_03> (%08x)", op); return 2;}
int arcompact_handle0f_01_dasm(DASM_OPS_16)  { print("<illegal 0x0f_01> (%08x)", op); return 2;}
int arcompact_handle0f_03_dasm(DASM_OPS_16)  { print("<illegal 0x0f_03> (%08x)", op); return 2;}
int arcompact_handle0f_08_dasm(DASM_OPS_16)  { print("<illegal 0x0f_08> (%08x)", op); return 2;}
int arcompact_handle0f_09_dasm(DASM_OPS_16)  { print("<illegal 0x0f_09> (%08x)", op); return 2;}
int arcompact_handle0f_0a_dasm(DASM_OPS_16)  { print("<illegal 0x0f_0a> (%08x)", op); return 2;}
int arcompact_handle0f_17_dasm(DASM_OPS_16)  { print("<illegal 0x0f_17> (%08x)", op); return 2;}

int arcompact_handle18_05_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_02> (%04x)", op); return 2;}
int arcompact_handle18_05_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_03> (%04x)", op); return 2;}
int arcompact_handle18_05_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_04> (%04x)", op); return 2;}
int arcompact_handle18_05_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_05> (%04x)", op); return 2;}
int arcompact_handle18_05_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_06> (%04x)", op); return 2;}
int arcompact_handle18_05_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_07> (%04x)", op); return 2;}
int arcompact_handle18_06_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_00> (%04x)",  op); return 2;}
int arcompact_handle18_06_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_02> (%04x)", op); return 2;}
int arcompact_handle18_06_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_03> (%04x)", op); return 2;}
int arcompact_handle18_06_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_04> (%04x)", op); return 2;}
int arcompact_handle18_06_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_05> (%04x)", op); return 2;}
int arcompact_handle18_06_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_06> (%04x)", op); return 2;}
int arcompact_handle18_06_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_07> (%04x)", op); return 2;}
int arcompact_handle18_06_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_08> (%04x)", op); return 2;}
int arcompact_handle18_06_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_09> (%04x)", op); return 2;}
int arcompact_handle18_06_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0a> (%04x)", op); return 2;}
int arcompact_handle18_06_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0b> (%04x)", op); return 2;}
int arcompact_handle18_06_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0c> (%04x)", op); return 2;}
int arcompact_handle18_06_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0d> (%04x)", op); return 2;}
int arcompact_handle18_06_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0e> (%04x)", op); return 2;}
int arcompact_handle18_06_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0f> (%04x)", op); return 2;}
int arcompact_handle18_06_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_10> (%04x)", op); return 2;}
int arcompact_handle18_06_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_12> (%04x)",  op); return 2;}
int arcompact_handle18_06_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_13> (%04x)",  op); return 2;}
int arcompact_handle18_06_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_14> (%04x)",  op); return 2;}
int arcompact_handle18_06_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_15> (%04x)",  op); return 2;}
int arcompact_handle18_06_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_16> (%04x)",  op); return 2;}
int arcompact_handle18_06_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_17> (%04x)",  op); return 2;}
int arcompact_handle18_06_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_18> (%04x)",  op); return 2;}
int arcompact_handle18_06_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_19> (%04x)",  op); return 2;}
int arcompact_handle18_06_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1a> (%04x)",  op); return 2;}
int arcompact_handle18_06_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1b> (%04x)",  op); return 2;}
int arcompact_handle18_06_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1c> (%04x)",  op); return 2;}
int arcompact_handle18_06_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1d> (%04x)",  op); return 2;}
int arcompact_handle18_06_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1e> (%04x)",  op); return 2;}
int arcompact_handle18_06_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1f> (%04x)",  op); return 2;}
int arcompact_handle18_07_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_00> (%04x)",  op); return 2;}
int arcompact_handle18_07_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_02> (%04x)", op); return 2;}
int arcompact_handle18_07_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_03> (%04x)", op); return 2;}
int arcompact_handle18_07_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_04> (%04x)", op); return 2;}
int arcompact_handle18_07_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_05> (%04x)", op); return 2;}
int arcompact_handle18_07_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_06> (%04x)", op); return 2;}
int arcompact_handle18_07_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_07> (%04x)", op); return 2;}
int arcompact_handle18_07_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_08> (%04x)", op); return 2;}
int arcompact_handle18_07_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_09> (%04x)", op); return 2;}
int arcompact_handle18_07_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0a> (%04x)", op); return 2;}
int arcompact_handle18_07_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0b> (%04x)", op); return 2;}
int arcompact_handle18_07_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0c> (%04x)", op); return 2;}
int arcompact_handle18_07_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0d> (%04x)", op); return 2;}
int arcompact_handle18_07_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0e> (%04x)", op); return 2;}
int arcompact_handle18_07_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0f> (%04x)", op); return 2;}
int arcompact_handle18_07_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_10> (%04x)", op); return 2;}
int arcompact_handle18_07_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_12> (%04x)",  op); return 2;}
int arcompact_handle18_07_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_13> (%04x)",  op); return 2;}
int arcompact_handle18_07_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_14> (%04x)",  op); return 2;}
int arcompact_handle18_07_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_15> (%04x)",  op); return 2;}
int arcompact_handle18_07_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_16> (%04x)",  op); return 2;}
int arcompact_handle18_07_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_17> (%04x)",  op); return 2;}
int arcompact_handle18_07_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_18> (%04x)",  op); return 2;}
int arcompact_handle18_07_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_19> (%04x)",  op); return 2;}
int arcompact_handle18_07_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1a> (%04x)",  op); return 2;}
int arcompact_handle18_07_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1b> (%04x)",  op); return 2;}
int arcompact_handle18_07_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1c> (%04x)",  op); return 2;}
int arcompact_handle18_07_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1d> (%04x)",  op); return 2;}
int arcompact_handle18_07_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1e> (%04x)",  op); return 2;}
int arcompact_handle18_07_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1f> (%04x)",  op); return 2;}

