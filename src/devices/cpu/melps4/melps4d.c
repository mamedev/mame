// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family disassembler

  Not counting the extra opcodes for peripherals (eg. timers, A/D),
  each MCU in the series has small differences in the opcode map.

*/

#include "emu.h"
#include "debugger.h"
#include "melps4.h"


// opcode mnemonics
enum e_mnemonics
{
	em_ILL,
	em_TAB, em_TBA, em_TAY, em_TYA, em_TEAB, em_TABE, em_TEPA, em_TXA, em_TAX,
	em_LXY, em_LZ, em_INY, em_DEY, em_LCPS, em_SADR,
	em_TAM, em_XAM, em_XAMD, em_XAMI,
	em_LA, em_AM, em_AMC, em_AMCS, em_A, em_SC, em_RC, em_SZC, em_CMA, em_RL, em_RR,
	em_SB, em_RB, em_SZB, em_SEAM, em_SEY,
	em_TLA, em_THA, em_TAJ, em_XAL, em_XAH, em_LC7, em_DEC, em_SHL, em_RHL, em_CPA, em_CPAS, em_CPAE, em_SZJ,
	em_T1AB, em_TRAB, em_T2AB, em_TAB1, em_TABR, em_TAB2, em_TVA, em_TWA, em_SNZ1, em_SNZ2,
	em_BA, em_SP, em_B, em_BM, em_RT, em_RTS, em_RTI,
	em_CLD, em_CLS, em_CLDS, em_SD, em_RD, em_SZD, em_OSAB, em_OSPA, em_OSE, em_IAS, em_OFA, em_IAF, em_OGA, em_IAK, em_SZK, em_SU, em_RU,
	em_EI, em_DI, em_INTH, em_INTL, em_NOP
};

static const char *const em_name[] =
{
	"?",
	"TAB", "TBA", "TAY", "TYA", "TEAB", "TABE", "TEPA", "TXA", "TAX",
	"LXY", "LZ", "INY", "DEY", "LCPS", "SADR",
	"TAM", "XAM", "XAMD", "XAMI",
	"LA", "AM", "AMC", "AMCS", "A", "SC", "RC", "SZC", "CMA", "RL", "RR",
	"SB", "RB", "SZB", "SEAM", "SEY",
	"TLA", "THA", "TAJ", "XAL", "XAH", "LC7", "DEC", "SHL", "RHL", "CPA", "CPAS", "CPAE", "SZJ",
	"T1AB", "TRAB", "T2AB", "TAB1", "TABR", "TAB2", "TVA", "TWA", "SNZ1", "SNZ2",
	"BA", "SP", "B", "BM", "RT", "RTS", "RTI",
	"CLD", "CLS", "CLDS", "SD", "RD", "SZD", "OSAB", "OSPA", "OSE", "IAS", "OFA", "IAF", "OGA", "IAK", "SZK", "SU", "RU",
	"EI", "DI", "INTH", "INTL", "NOP"
};

// number of bits per opcode parameter
static const UINT8 em_bits[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 1, 0, 0, 1, 2,
	2, 2, 2, 2,
	4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 0, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 4, 7, 7, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 0, 0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 em_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, _OVER, _OUT, _OUT, _OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};



// M58846 disasm

static const UINT8 m58846_opmap[0xc0] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	em_NOP,  em_BA,   em_INY,  em_DEY,  em_DI,   em_EI,   em_RU,   em_SU,   0,       em_TABE, em_AM,   em_OSE,  em_TYA,  0,       0,       em_CMA,  // 0x
	em_CLS,  em_CLDS, 0,       em_CLD,  em_RD,   em_SD,   em_TEPA, em_OSPA, em_RL,   em_RR,   em_TEAB, em_OSAB, em_TBA,  em_TAY,  em_TAB,  0,       // 1x
	em_SZB,  em_SZB,  em_SZB,  em_SZB,  0,       0,       em_SEAM, 0,       0,       0,       0,       em_SZD,  0,       0,       0,       em_SZC,  // 2x
	em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  em_SEY,  // 3x
	em_LCPS, em_LCPS, 0,       em_AMC,  em_RT,   em_RTS,  em_RTI,  0,       em_RC,   em_SC,   em_LZ,   em_LZ,   em_SB,   em_SB,   em_SB,   em_SB,   // 4x
	0,       0,       0,       em_AMCS, em_IAS,  em_IAS,  0,       em_IAK,  em_SZK,  em_SZK,  em_SZK,  em_SZK,  em_RB,   em_RB,   em_RB,   em_RB,   // 5x
	em_XAM,  em_XAM,  em_XAM,  em_XAM,  em_TAM,  em_TAM,  em_TAM,  em_TAM,  em_XAMD, em_XAMD, em_XAMD, em_XAMD, em_XAMI, em_XAMI, em_XAMI, em_XAMI, // 6x
	em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   em_SP,   // 7x
	0,       em_OFA,  em_SNZ1, em_SNZ2, em_OGA,  em_T2AB, em_TVA,  0,       0,       0,       em_TAB2, 0,       em_IAF,  0,       0,       0,       // 8x
	0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       // 9x
	em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    em_A,    // Ax
	em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA,   em_LA    // Bx
};

CPU_DISASSEMBLE(m58846)
{
	UINT16 op = (oprom[0] | oprom[1] << 8) & 0x1ff;
	char *dst = buffer;

	// get opcode
	UINT8 instr;
	if (op >= 0x180)
		instr = em_B;
	else if (op >= 0x100)
		instr = em_BM;
	else if (op >= 0xc0)
		instr = em_LXY;
	else
		instr = m58846_opmap[op];

	dst += sprintf(dst, "%-6s", em_name[instr]);

	// get immediate param
	UINT8 bits = em_bits[instr];

	// special case for LXY x,y
	if (instr == em_LXY)
	{
		UINT8 x = op >> 4 & 3;
		UINT8 y = op & 0xf;
		dst += sprintf(dst, " %d,%d", x, y);
	}
	else if (bits > 0)
	{
		UINT8 param = op & ((1 << bits) - 1);
		if (bits > 4)
			dst += sprintf(dst, " $%02X", param);
		else
			dst += sprintf(dst, " %d", param);
	}

	return 1 | em_flags[instr] | DASMFLAG_SUPPORTED;
}
