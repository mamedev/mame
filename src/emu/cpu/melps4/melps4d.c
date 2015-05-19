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
	em_TAB, em_TBA, em_TAY, em_TYA, em_TEAB, em_TEPA, em_TXA, em_TAX,
	em_LXY, em_LZ, em_INY, em_DEY, em_LCPS, em_SADR,
	em_TAM, em_XAM, em_XAMD, em_XAMI,
	em_LA, em_AM, em_AMC, em_AMCS, em_A, em_SC, em_RC, em_SZC, em_CMA,
	em_SB, em_RB, em_SZB, em_SEAM, em_SEY,
	em_TLA, em_THA, em_TAJ, em_XAL, em_XAH, em_LC7, em_DEC, em_SHL, em_RHL, em_CPA, em_CPAS, em_CPAE, em_SZJ,
	em_T1AB, em_TRAB, em_TAB1, em_TABR, em_TAB2, em_TVA, em_TWA, em_SNZ1, em_SNZ2,
	em_BA, em_SP, em_B, em_BM, em_RT, em_RTS, em_RTI,
	em_CLD, em_CLS, em_CLDS, em_SD, em_RD, em_SZD, em_OSAB, em_OSPA, em_OSE, em_IAS, em_OFA, em_IAF, em_OGA, em_IAK, em_SZK, em_SU, em_RU,
	em_EI, em_DI, em_INTH, em_INTL,
	em_NOP, em_ILL
};

static const char *const em_name[] =
{
	"TAB", "TBA", "TAY", "TYA", "TEAB", "TEPA", "TXA", "TAX",
	"LXY", "LZ", "INY", "DEY", "LCPS", "SADR",
	"TAM", "XAM", "XAMD", "XAMI",
	"LA", "AM", "AMC", "AMCS", "A", "SC", "RC", "SZC", "CMA",
	"SB", "RB", "SZB", "SEAM", "SEY",
	"TLA", "THA", "TAJ", "XAL", "XAH", "LC7", "DEC", "SHL", "RHL", "CPA", "CPAS", "CPAE", "SZJ",
	"T1AB", "TRAB", "TAB1", "TABR", "TAB2", "TVA", "TWA", "SNZ1", "SNZ2",
	"BA", "SP", "B", "BM", "RT", "RTS", "RTI",
	"CLD", "CLS", "CLDS", "SD", "RD", "SZD", "OSAB", "OSPA", "OSE", "IAS", "OFA", "IAF", "OGA", "IAK", "SZK", "SU", "RU",
	"EI", "DI", "INTH", "INTL",
	"NOP", "?"
};

// number of bits per opcode parameter
static const UINT8 s_bits[] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 1, 0, 0, 1, 2,
	2, 2, 2, 2,
	4, 0, 0, 0, 4, 0, 0, 0, 0,
	2, 2, 2, 0, 4,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 4, 7, 7, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 0, 0,
	0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 em_flags[] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, _OVER, _OUT, _OUT, _OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0
};








CPU_DISASSEMBLE(m58846)
{
	return 1;
}
