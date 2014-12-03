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
	/* 04 */ "Bcc",
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


CPU_DISASSEMBLE( arc )
{
	UINT32 op = oprom[0] | (oprom[1] << 8) | (oprom[2] << 16) | (oprom[3] << 24);
	op = BIG_ENDIANIZE_INT32(op);

	output = buffer;

	UINT8 opcode = (op & 0xf8000000) >> 27;

	print("%s (%08x)", basic[opcode], op &~ 0xf8000000);

	return 4 | DASMFLAG_SUPPORTED;
}
