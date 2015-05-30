// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCtangent A4 disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vsprintf(output, fmt, vl);
	va_end(vl);
}

/*****************************************************************************/



/*****************************************************************************/

static const char *basic[0x20] =
{
	/* 00 */ "LD r+r",
	/* 01 */ "LD r+o",
	/* 02 */ "ST r+o",
	/* 03 */ "extended",
	/* 04 */ "B",
	/* 05 */ "BL",
	/* 06 */ "LPcc",
	/* 07 */ "Jcc JLcc",
	/* 08 */ "ADD",
	/* 09 */ "ADC",
	/* 0a */ "SUB",
	/* 0b */ "SBC",
	/* 0c */ "AND",
	/* 0d */ "OR",
	/* 0e */ "BIC",
	/* 0f */ "XOR",
	/* 10 */ "ASL",
	/* 11 */ "LSR",
	/* 12 */ "ASR",
	/* 13 */ "ROR",
	/* 14 */ "MUL64",
	/* 15 */ "MULU64",
	/* 16 */ "undefined",
	/* 17 */ "undefined",
	/* 18 */ "undefined",
	/* 19 */ "undefined",
	/* 1a */ "undefined",
	/* 1b */ "undefined",
	/* 1c */ "undefined",
	/* 1d */ "undefined",
	/* 1e */ "MAX",
	/* 1f */ "MIN"
};

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

static const char *delaytype[0x4] =
{
	"ND", // NO DELAY - execute next instruction only when NOT jumping
	"D",  // always execute next instruction
	"JD", // only execute next instruction when jumping
	"Res!", // reserved / invalid
};

static const char *regnames[0x40] =
{
	/* 0x00 */ "r00",
	/* 0x01 */ "r01",
	/* 0x02 */ "r02",
	/* 0x03 */ "r03",
	/* 0x04 */ "r04",
	/* 0x05 */ "r05",
	/* 0x06 */ "r06",
	/* 0x07 */ "r07",
	/* 0x08 */ "r08",
	/* 0x09 */ "r09",
	/* 0x0a */ "r10",
	/* 0x0b */ "r11",
	/* 0x0c */ "r12",
	/* 0x0d */ "r13",
	/* 0x0e */ "r14",
	/* 0x0f */ "r15",

	/* 0x10 */ "r16",
	/* 0x11 */ "r17",
	/* 0x12 */ "r18",
	/* 0x13 */ "r19",
	/* 0x14 */ "r20",
	/* 0x15 */ "r21",
	/* 0x16 */ "r22",
	/* 0x17 */ "r23",
	/* 0x18 */ "r24",
	/* 0x19 */ "r25",
	/* 0x1a */ "r26",
	/* 0x1b */ "r27",
	/* 0x1c */ "r28",
	/* 0x1d */ "ILINK1",
	/* 0x1e */ "ILINK2",
	/* 0x1f */ "BLINK",

	/* 0x20 */ "r32res", // reserved for manufacturer specific extensions
	/* 0x21 */ "r33res",
	/* 0x22 */ "r34res",
	/* 0x23 */ "r35res",
	/* 0x24 */ "r36res",
	/* 0x25 */ "r37res",
	/* 0x26 */ "r38res",
	/* 0x27 */ "r39res",
	/* 0x28 */ "r40res",
	/* 0x29 */ "r41res",
	/* 0x2a */ "r42res",
	/* 0x2b */ "r43res",
	/* 0x2c */ "r44res",
	/* 0x2d */ "r45res",
	/* 0x2e */ "r46res",
	/* 0x2f */ "r47res",

	/* 0x30 */ "r48res",
	/* 0x31 */ "r49res",
	/* 0x32 */ "r50res",
	/* 0x33 */ "r51res",
	/* 0x34 */ "r52res",
	/* 0x35 */ "r53res",
	/* 0x36 */ "r54res",
	/* 0x37 */ "r55res",
	/* 0x38 */ "r56res",
	/* 0x39 */ "r57res",
	/* 0x3a */ "r58res",
	/* 0x3b */ "r59res",
	/* 0x3c */ "LPCOUNT",
	/* 0x3d */ "sImm F",
	/* 0x3e */ "lImm",
	/* 0x3f */ "sImm NF",
};

#define ARC_CONDITION ((op & 0x0000001f) >> 0 ) // aka Q

// used in jumps
#define ARC_BRANCH_DELAY     ((op & 0x00000060) >> 5 ) // aka N
#define ARC_BRANCH_ADDR      ((op & 0x07ffff80) >> 7 ) // aka L

#define ARC_OPERATION ((op & 0xf8000000) >> 27)

#define ARC_REGOP_DEST      ((op & 0x07e00000) >> 21 ) // aka A
#define ARC_REGOP_OP1       ((op & 0x001f8000) >> 15 ) // aka B
#define ARC_REGOP_OP2       ((op & 0x00007e00) >> 9  ) // aka C
#define ARC_REGOP_SHIMM     ((op & 0x000001ff) >> 0  ) // aka D


CPU_DISASSEMBLE(arc)
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	op = BIG_ENDIANIZE_INT32(op);

	output = buffer;

	UINT8 opcode = ARC_OPERATION;

	switch (opcode)
	{
		case 0x04: // B
		case 0x05: // BL
		print("%s(%s)(%s) %08x", basic[opcode], conditions[ARC_CONDITION], delaytype[ARC_BRANCH_DELAY], (ARC_BRANCH_ADDR<<2)+pc+4);
		break;

		case 0x08: // ADD
		// todo, short / long immediate formats
		print("%s %s , %s , %s (%08x)", basic[opcode], regnames[ARC_REGOP_DEST], regnames[ARC_REGOP_OP1], regnames[ARC_REGOP_OP2], op &~ 0xfffffe00);
		break;


		default:
		print("%s (%08x)", basic[opcode], op &~ 0xf8000000);
		break;
	}



	return 4 | DASMFLAG_SUPPORTED;
}
