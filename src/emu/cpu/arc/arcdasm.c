/*********************************\

 ARCtangent (A4? A5?) disassembler

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
	/* 05 */ "BLcc",
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

#define ARC_CONDITION ((op & 0x0000001f) >> 0 )

// used in jumps
#define ARC_BRANCH_DELAY     ((op & 0x00000060) >> 5 ) // aka NN
#define ARC_BRANCH_ADDR      ((op & 0x07ffff80) >> 7 ) // aka L

#define ARC_OPERATION ((op & 0xf8000000) >> 27) // aka QQQQQ

CPU_DISASSEMBLE(arc)
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	op = BIG_ENDIANIZE_INT32(op);

	output = buffer;

	UINT8 opcode = ARC_OPERATION;

	switch (opcode)
	{
		case 0x04:
		print("%s(%s)(%s) %08x", basic[opcode], conditions[ARC_CONDITION], delaytype[ARC_BRANCH_DELAY], (ARC_BRANCH_ADDR<<2)+pc+4);
		break;
	
		default:
		print("%s (%08x)", basic[opcode], op &~ 0xf8000000);
		break;
	}



	return 4 | DASMFLAG_SUPPORTED;
}
