// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

Toshiba TLCS-900/H disassembly

*******************************************************************/

#include "emu.h"
#include "dasm900.h"

const char *const tlcs900_disassembler::s_mnemonic[] =
{
	"adc", "add", "and", "andcf", "bit", "bs1b",
	"bs1f", "call", "calr", "ccf", "chg", "cp",
	"cpd", "cpdw", "cpdr", "cpdrw", "cpi", "cpir",
	"cpirw", "cpiw", "cpl", "daa", "db", "dec",
	"decf", "decw", "div", "divs", "djnz", "ei",
	"ex", "exts", "extz", "halt", "inc", "incf",
	"incw", "jp" ,"jr", "jrl", "ld", "lda",
	"ldc", "ldcf", "ldd", "lddr", "lddrw", "lddw",
	"ldf", "ldi", "ldir", "ldirw", "ldiw", "ldw",
	"ldx", "link", "max", "mdec1", "mdec2", "mdec4",
	"minc1", "minc2", "minc4", "mirr", "mul", "mula",
	"muls", "neg", "nop", "normal", "or", "orcf",
	"paa", "pop", "popw", "push", "pushw", "rcf",
	"res", "ret", "retd", "reti", "rl", "rlc",
	"rlcw", "rld", "rlw", "rr", "rrc", "rrcw",
	"rrd", "rrw", "sbc", "scc", "scf", "set",
	"sla", "slaw", "sll", "sllw", "sra", "sraw",
	"srl", "srlw", "stcf", "sub", "swi", "tset",
	"unlk", "xor", "xorcf", "zcf",
	"db", "db", "db", "db", "db", "db", "db", "db",
	"db", "db", "db", "db", "db", "db", "db"
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_80[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSH, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_RLD, O_A, O_M }, { M_RRD, O_A, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LDI, O_NONE, O_NONE }, { M_LDIR, O_NONE, O_NONE }, { M_LDD, O_NONE, O_NONE }, { M_LDDR, O_NONE, O_NONE },
	{ M_CPI, O_NONE, O_NONE }, { M_CPIR, O_NONE, O_NONE }, { M_CPD, O_NONE, O_NONE }, { M_CPDR, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LD, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_ADD, O_M, O_I8 }, { M_ADC, O_M, O_I8 }, { M_SUB, O_M, O_I8 }, { M_SBC, O_M, O_I8 },
	{ M_AND, O_M, O_I8 }, { M_XOR, O_M, O_I8 }, { M_OR, O_M, O_I8 }, { M_CP, O_M, O_I8 },

	/* 40 - 5F */
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },

	/* 60 - 7F */
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLC, O_M, O_NONE }, { M_RRC, O_M, O_NONE }, { M_RL, O_M, O_NONE }, { M_RR, O_M, O_NONE },
	{ M_SLA, O_M, O_NONE }, { M_SRA, O_M, O_NONE }, { M_SLL, O_M, O_NONE }, { M_SRL, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },

	/* A0 - BF */
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },

	/* C0 - DF */
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },

	/* E0 - FF */
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_88[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSH, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_RLD, O_A, O_M }, { M_RRD, O_A, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LD, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_ADD, O_M, O_I8 }, { M_ADC, O_M, O_I8 }, { M_SUB, O_M, O_I8 }, { M_SBC, O_M, O_I8 },
	{ M_AND, O_M, O_I8 }, { M_XOR, O_M, O_I8 }, { M_OR, O_M, O_I8 }, { M_CP, O_M, O_I8 },

	/* 40 - 5F */
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },

	/* 60 - 7F */
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLC, O_M, O_NONE }, { M_RRC, O_M, O_NONE }, { M_RL, O_M, O_NONE }, { M_RR, O_M, O_NONE },
	{ M_SLA, O_M, O_NONE }, { M_SRA, O_M, O_NONE }, { M_SLL, O_M, O_NONE }, { M_SRL, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },

	/* A0 - BF */
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },

	/* C0 - DF */
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },

	/* E0 - FF */
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_90[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSHW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LDIW, O_NONE, O_NONE }, { M_LDIRW, O_NONE, O_NONE }, { M_LDDW, O_NONE, O_NONE }, { M_LDDRW, O_NONE, O_NONE },
	{ M_CPIW, O_NONE, O_NONE }, { M_CPIRW, O_NONE, O_NONE }, { M_CPDW, O_NONE, O_NONE }, { M_CPDRW, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LDW, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_ADD, O_M, O_I16 }, { M_ADC, O_M, O_I16 }, { M_SUB, O_M, O_I16 }, { M_SBC, O_M, O_I16 },
	{ M_AND, O_M, O_I16 }, { M_XOR, O_M, O_I16 }, { M_OR, O_M, O_I16 }, { M_CP, O_M, O_I16 },

	/* 40 - 5F */
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },

	/* 60 - 7F */
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLCW, O_M, O_NONE }, { M_RRCW, O_M, O_NONE }, { M_RLW, O_M, O_NONE }, { M_RRW, O_M, O_NONE },
	{ M_SLAW, O_M, O_NONE }, { M_SRAW, O_M, O_NONE }, { M_SLLW, O_M, O_NONE }, { M_SRLW, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },

	/* A0 - BF */
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },

	/* C0 - DF */
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },

	/* E0 - FF */
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_98[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSHW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LDW, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_ADD, O_M, O_I16 }, { M_ADC, O_M, O_I16 }, { M_SUB, O_M, O_I16 }, { M_SBC, O_M, O_I16 },
	{ M_AND, O_M, O_I16 }, { M_XOR, O_M, O_I16 }, { M_OR, O_M, O_I16 }, { M_CP, O_M, O_I16 },

	/* 40 - 5F */
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },

	/* 60 - 7F */
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLCW, O_M, O_NONE }, { M_RRCW, O_M, O_NONE }, { M_RLW, O_M, O_NONE }, { M_RRW, O_M, O_NONE },
	{ M_SLAW, O_M, O_NONE }, { M_SRAW, O_M, O_NONE }, { M_SLLW, O_M, O_NONE }, { M_SRLW, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },

	/* A0 - BF */
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },

	/* C0 - DF */
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },

	/* E0 - FF */
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M },
	{ M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M },
	{ M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M },
	{ M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 },
	{ M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 },
	{ M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M },
	{ M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M },
	{ M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 },
	{ M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 },

	/* A0 - BF */
	{ M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M },
	{ M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M },
	{ M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 },
	{ M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 },
	{ M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M },
	{ M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M },
	{ M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 },
	{ M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 },

	/* C0 - DF */
	{ M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M },
	{ M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M },
	{ M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 },
	{ M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 },
	{ M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M },
	{ M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M },
	{ M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 },
	{ M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 },

	/* E0 - FF */
	{ M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M },
	{ M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M },
	{ M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 },
	{ M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 },
	{ M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M },
	{ M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M },
	{ M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 },
	{ M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ M_LD, O_M, O_I8 }, { M_DB, O_NONE, O_NONE }, { M_LD, O_M, O_I16 }, { M_DB, O_NONE, O_NONE },
	{ M_POP, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_POPW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_M16 }, { M_DB, O_NONE, O_NONE }, { M_LDW, O_M, O_M16 }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_ANDCF, O_A, O_M }, { M_ORCF, O_A, O_M }, { M_XORCF, O_A, O_M }, { M_LDCF, O_A, O_M },
	{ M_STCF, O_A, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },

	/* A0 - BF */
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },

	/* C0 - DF */
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },

	/* E0 - FF */
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE },
	{ M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE },
	{ M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE },
	{ M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }, { M_RET, O_CC, O_NONE }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ M_LD, O_M, O_I8 }, { M_DB, O_NONE, O_NONE }, { M_LD, O_M, O_I16 }, { M_DB, O_NONE, O_NONE },
	{ M_POP, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_POPW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_M16 }, { M_DB, O_NONE, O_NONE }, { M_LDW, O_M, O_M16 }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_ANDCF, O_A, O_M }, { M_ORCF, O_A, O_M }, { M_XORCF, O_A, O_M }, { M_LDCF, O_A, O_M },
	{ M_STCF, O_A, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },

	/* A0 - BF */
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },

	/* C0 - DF */
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },

	/* E0 - FF */
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSH, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_RLD, O_A, O_M }, { M_RRD, O_A, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LD, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M }, { M_LD, O_C8, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 }, { M_EX, O_M, O_C8 },
	{ M_ADD, O_M, O_I8 }, { M_ADC, O_M, O_I8 }, { M_SUB, O_M, O_I8 }, { M_SBC, O_M, O_I8 },
	{ M_AND, O_M, O_I8 }, { M_XOR, O_M, O_I8 }, { M_OR, O_M, O_I8 }, { M_CP, O_M, O_I8 },

	/* 40 - 5F */
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M }, { M_MUL, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M }, { M_MULS, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M }, { M_DIV, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },
	{ M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M }, { M_DIVS, O_MC16, O_M },

	/* 60 - 7F */
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M }, { M_INC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M }, { M_DEC, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLC, O_M, O_NONE }, { M_RRC, O_M, O_NONE }, { M_RL, O_M, O_NONE }, { M_RR, O_M, O_NONE },
	{ M_SLA, O_M, O_NONE }, { M_SRA, O_M, O_NONE }, { M_SLL, O_M, O_NONE }, { M_SRL, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M }, { M_ADD, O_C8, O_M },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 }, { M_ADD, O_M, O_C8 },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M }, { M_ADC, O_C8, O_M },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },
	{ M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 }, { M_ADC, O_M, O_C8 },

	/* A0 - BF */
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M }, { M_SUB, O_C8, O_M },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 }, { M_SUB, O_M, O_C8 },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M }, { M_SBC, O_C8, O_M },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },
	{ M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 }, { M_SBC, O_M, O_C8 },

	/* C0 - DF */
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M }, { M_AND, O_C8, O_M },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 }, { M_AND, O_M, O_C8 },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M }, { M_XOR, O_C8, O_M },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },
	{ M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 }, { M_XOR, O_M, O_C8 },

	/* E0 - FF */
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M }, { M_OR, O_C8, O_M },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 }, { M_OR, O_M, O_C8 },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M }, { M_CP, O_C8, O_M },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
	{ M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 }, { M_CP, O_M, O_C8 },
};


/* TODO: M_MUL_O_I8, M_MULS_O_I8, M_DIV_O_I8, M_DIVS_O_i8 need to be fixed */
const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_LD, O_R, O_I8 },
	{ M_PUSH, O_R, O_NONE }, { M_POP, O_R, O_NONE }, { M_CPL, O_R, O_NONE }, { M_NEG, O_R, O_NONE },
	{ M_MUL, O_R, O_I8 }, { M_MULS, O_R, O_I8 }, { M_DIV, O_R, O_I8 }, { M_DIVS, O_R, O_I8 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DAA, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DJNZ, O_R, O_D8 }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_ANDCF, O_I8, O_R }, { M_ORCF, O_I8, O_R }, { M_XORCF, O_I8, O_R }, { M_LDCF, O_I8, O_R },
	{ M_STCF, O_I8, O_R }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_ANDCF, O_A, O_R }, { M_ORCF, O_A, O_R }, { M_XORCF, O_A, O_R }, { M_LDCF, O_A, O_R },
	{ M_STCF, O_A, O_R }, { M_DB, O_NONE, O_NONE }, { M_LDC, O_CR8, O_R }, { M_LDC, O_R, O_CR8 },
	{ M_RES, O_I8, O_R }, { M_SET, O_I8, O_R }, { M_CHG, O_I8, O_R }, { M_BIT, O_I8, O_R },
	{ M_TSET, O_I8, O_R }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R },
	{ M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R }, { M_MUL, O_MC16, O_R },
	{ M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R },
	{ M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R }, { M_MULS, O_MC16, O_R },
	{ M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R },
	{ M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R }, { M_DIV, O_MC16, O_R },
	{ M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R },
	{ M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R }, { M_DIVS, O_MC16, O_R },

	/* 60 - 7F */
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },

	/* 80 - 9F */
	{ M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R },
	{ M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R }, { M_ADD, O_C8, O_R },
	{ M_LD, O_C8, O_R }, { M_LD, O_C8, O_R }, { M_LD, O_C8, O_R }, { M_LD, O_C8, O_R },
	{ M_LD, O_C8, O_R }, { M_LD, O_C8, O_R }, { M_LD, O_C8, O_R }, { M_LD, O_C8, O_R },
	{ M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R },
	{ M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R }, { M_ADC, O_C8, O_R },
	{ M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 },
	{ M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 }, { M_LD, O_R, O_C8 },

	/* A0 - BF */
	{ M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R },
	{ M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R }, { M_SUB, O_C8, O_R },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R },
	{ M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R }, { M_SBC, O_C8, O_R },
	{ M_EX, O_C8, O_R }, { M_EX, O_C8, O_R }, { M_EX, O_C8, O_R }, { M_EX, O_C8, O_R },
	{ M_EX, O_C8, O_R }, { M_EX, O_C8, O_R }, { M_EX, O_C8, O_R }, { M_EX, O_C8, O_R },

	/* C0 - DF */
	{ M_AND, O_C8, O_R }, { M_AND, O_C8, O_R }, { M_AND, O_C8, O_R }, { M_AND, O_C8, O_R },
	{ M_AND, O_C8, O_R }, { M_AND, O_C8, O_R }, { M_AND, O_C8, O_R }, { M_AND, O_C8, O_R },
	{ M_ADD, O_R, O_I8 }, { M_ADC, O_R, O_I8 }, { M_SUB, O_R, O_I8 }, { M_SBC, O_R, O_I8 },
	{ M_AND, O_R, O_I8 }, { M_XOR, O_R, O_I8 }, { M_OR, O_R, O_I8 }, { M_CP, O_R, O_I8 },
	{ M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R },
	{ M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R }, { M_XOR, O_C8, O_R },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },

	/* E0 - FF */
	{ M_OR, O_C8, O_R }, { M_OR, O_C8, O_R }, { M_OR, O_C8, O_R }, { M_OR, O_C8, O_R },
	{ M_OR, O_C8, O_R }, { M_OR, O_C8, O_R }, { M_OR, O_C8, O_R }, { M_OR, O_C8, O_R },
	{ M_RLC, O_I8, O_R }, { M_RRC, O_I8, O_R }, { M_RL, O_I8, O_R }, { M_RR, O_I8, O_R },
	{ M_SLA, O_I8, O_R }, { M_SRA, O_I8, O_R }, { M_SLL, O_I8, O_R }, { M_SRL, O_I8, O_R },
	{ M_CP, O_C8, O_R }, { M_CP, O_C8, O_R }, { M_CP, O_C8, O_R }, { M_CP, O_C8, O_R },
	{ M_CP, O_C8, O_R }, { M_CP, O_C8, O_R }, { M_CP, O_C8, O_R }, { M_CP, O_C8, O_R },
	{ M_RLC, O_A, O_R }, { M_RRC, O_A, O_R }, { M_RL, O_A, O_R }, { M_RR, O_A, O_R },
	{ M_SLA, O_A, O_R }, { M_SRA, O_A, O_R }, { M_SLL, O_A, O_R }, { M_SRL, O_A, O_R }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_PUSHW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_LDW, O_M16, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M }, { M_LD, O_C16, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 }, { M_EX, O_M, O_C16 },
	{ M_ADD, O_M, O_I16 }, { M_ADC, O_M, O_I16 }, { M_SUB, O_M, O_I16 }, { M_SBC, O_M, O_I16 },
	{ M_AND, O_M, O_I16 }, { M_XOR, O_M, O_I16 }, { M_OR, O_M, O_I16 }, { M_CP, O_M, O_I16 },

	/* 40 - 5F */
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M }, { M_MUL, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M }, { M_MULS, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M }, { M_DIV, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },
	{ M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M }, { M_DIVS, O_C32, O_M },

	/* 60 - 7F */
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M }, { M_INCW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M }, { M_DECW, O_I3, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_RLCW, O_M, O_NONE }, { M_RRCW, O_M, O_NONE }, { M_RLW, O_M, O_NONE }, { M_RRW, O_M, O_NONE },
	{ M_SLAW, O_M, O_NONE }, { M_SRAW, O_M, O_NONE }, { M_SLLW, O_M, O_NONE }, { M_SRLW, O_M, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M }, { M_ADD, O_C16, O_M },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 }, { M_ADD, O_M, O_C16 },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M }, { M_ADC, O_C16, O_M },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },
	{ M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 }, { M_ADC, O_M, O_C16 },

	/* A0 - BF */
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M }, { M_SUB, O_C16, O_M },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 }, { M_SUB, O_M, O_C16 },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M }, { M_SBC, O_C16, O_M },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },
	{ M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 }, { M_SBC, O_M, O_C16 },

	/* C0 - DF */
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M }, { M_AND, O_C16, O_M },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 }, { M_AND, O_M, O_C16 },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M }, { M_XOR, O_C16, O_M },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },
	{ M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 }, { M_XOR, O_M, O_C16 },

	/* E0 - FF */
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M }, { M_OR, O_C16, O_M },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 }, { M_OR, O_M, O_C16 },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M }, { M_CP, O_C16, O_M },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
	{ M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 }, { M_CP, O_M, O_C16 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_LD, O_R, O_I16 },
	{ M_PUSH, O_R, O_NONE }, { M_POP, O_R, O_NONE }, { M_CPL, O_R, O_NONE }, { M_NEG, O_R, O_NONE },
	{ M_MUL, O_R, O_I16 }, { M_MULS, O_R, O_I16 }, { M_DIV, O_R, O_I16 }, { M_DIVS, O_R, O_I16 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_BS1F, O_A, O_R }, { M_BS1B, O_A, O_R },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_EXTZ, O_R, O_NONE }, { M_EXTS, O_R, O_NONE },
	{ M_PAA, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_MIRR, O_R, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_MULA, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DJNZ, O_R, O_D8 }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_ANDCF, O_I8, O_R }, { M_ORCF, O_I8, O_R }, { M_XORCF, O_I8, O_R }, { M_LDCF, O_I8, O_R },
	{ M_STCF, O_I8, O_R }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_ANDCF, O_A, O_R }, { M_ORCF, O_A, O_R }, { M_XORCF, O_A, O_R }, { M_LDCF, O_A, O_R },
	{ M_STCF, O_A, O_R }, { M_DB, O_NONE, O_NONE }, { M_LDC, O_CR16, O_R }, { M_LDC, O_R, O_CR16 },
	{ M_RES, O_I8, O_R }, { M_SET, O_I8, O_R }, { M_CHG, O_I8, O_R }, { M_BIT, O_I8, O_R },
	{ M_TSET, O_I8, O_R }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_MINC1, O_I16, O_R }, { M_MINC2, O_I16, O_R }, { M_MINC4, O_I16, O_R }, { M_DB, O_NONE, O_NONE },
	{ M_MDEC1, O_I16, O_R }, { M_MDEC2, O_I16, O_R }, { M_MDEC4, O_I16, O_R }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R },
	{ M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R }, { M_MUL, O_C32, O_R },
	{ M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R },
	{ M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R }, { M_MULS, O_C32, O_R },
	{ M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R },
	{ M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R }, { M_DIV, O_C32, O_R },
	{ M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R },
	{ M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R }, { M_DIVS, O_C32, O_R },

	/* 60 - 7F */
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },
	{ M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R }, { M_SCC, O_CC, O_R },

	/* 80 - 9F */
	{ M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R },
	{ M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R }, { M_ADD, O_C16, O_R },
	{ M_LD, O_C16, O_R }, { M_LD, O_C16, O_R }, { M_LD, O_C16, O_R }, { M_LD, O_C16, O_R },
	{ M_LD, O_C16, O_R }, { M_LD, O_C16, O_R }, { M_LD, O_C16, O_R }, { M_LD, O_C16, O_R },
	{ M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R },
	{ M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R }, { M_ADC, O_C16, O_R },
	{ M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 },
	{ M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 }, { M_LD, O_R, O_C16 },

	/* A0 - BF */
	{ M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R },
	{ M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R }, { M_SUB, O_C16, O_R },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R },
	{ M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R }, { M_SBC, O_C16, O_R },
	{ M_EX, O_C16, O_R }, { M_EX, O_C16, O_R }, { M_EX, O_C16, O_R }, { M_EX, O_C16, O_R },
	{ M_EX, O_C16, O_R }, { M_EX, O_C16, O_R }, { M_EX, O_C16, O_R }, { M_EX, O_C16, O_R },

	/* C0 - DF */
	{ M_AND, O_C16, O_R }, { M_AND, O_C16, O_R }, { M_AND, O_C16, O_R }, { M_AND, O_C16, O_R },
	{ M_AND, O_C16, O_R }, { M_AND, O_C16, O_R }, { M_AND, O_C16, O_R }, { M_AND, O_C16, O_R },
	{ M_ADD, O_R, O_I16 }, { M_ADC, O_R, O_I16 }, { M_SUB, O_R, O_I16 }, { M_SBC, O_R, O_I16 },
	{ M_AND, O_R, O_I16 }, { M_XOR, O_R, O_I16 }, { M_OR, O_R, O_I16 }, { M_CP, O_R, O_I16 },
	{ M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R },
	{ M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R }, { M_XOR, O_C16, O_R },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },

	/* E0 - FF */
	{ M_OR, O_C16, O_R }, { M_OR, O_C16, O_R }, { M_OR, O_C16, O_R }, { M_OR, O_C16, O_R },
	{ M_OR, O_C16, O_R }, { M_OR, O_C16, O_R }, { M_OR, O_C16, O_R }, { M_OR, O_C16, O_R },
	{ M_RLC, O_I8, O_R }, { M_RRC, O_I8, O_R }, { M_RL, O_I8, O_R }, { M_RR, O_I8, O_R },
	{ M_SLA, O_I8, O_R }, { M_SRA, O_I8, O_R }, { M_SLL, O_I8, O_R }, { M_SRL, O_I8, O_R },
	{ M_CP, O_C16, O_R }, { M_CP, O_C16, O_R }, { M_CP, O_C16, O_R }, { M_CP, O_C16, O_R },
	{ M_CP, O_C16, O_R }, { M_CP, O_C16, O_R }, { M_CP, O_C16, O_R }, { M_CP, O_C16, O_R },
	{ M_RLC, O_A, O_R }, { M_RRC, O_A, O_R }, { M_RL, O_A, O_R }, { M_RR, O_A, O_R },
	{ M_SLA, O_A, O_R }, { M_SRA, O_A, O_R }, { M_SLL, O_A, O_R }, { M_SRL, O_A, O_R }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M },
	{ M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M }, { M_LD, O_C32, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M },
	{ M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M }, { M_ADD, O_C32, O_M },
	{ M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 },
	{ M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 }, { M_ADD, O_M, O_C32 },
	{ M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M },
	{ M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M }, { M_ADC, O_C32, O_M },
	{ M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 },
	{ M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 }, { M_ADC, O_M, O_C32 },

	/* A0 - BF */
	{ M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M },
	{ M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M }, { M_SUB, O_C32, O_M },
	{ M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 },
	{ M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 }, { M_SUB, O_M, O_C32 },
	{ M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M },
	{ M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M }, { M_SBC, O_C32, O_M },
	{ M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 },
	{ M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 }, { M_SBC, O_M, O_C32 },

	/* C0 - DF */
	{ M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M },
	{ M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M }, { M_AND, O_C32, O_M },
	{ M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 },
	{ M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 }, { M_AND, O_M, O_C32 },
	{ M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M },
	{ M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M }, { M_XOR, O_C32, O_M },
	{ M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 },
	{ M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 }, { M_XOR, O_M, O_C32 },

	/* E0 - FF */
	{ M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M },
	{ M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M }, { M_OR, O_C32, O_M },
	{ M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 },
	{ M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 }, { M_OR, O_M, O_C32 },
	{ M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M },
	{ M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M }, { M_CP, O_C32, O_M },
	{ M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 },
	{ M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 }, { M_CP, O_M, O_C32 },
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_LD, O_R, O_I32 },
	{ M_PUSH, O_R, O_NONE }, { M_POP, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LINK, O_R, O_I16 }, { M_UNLK, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_EXTZ, O_R, O_NONE }, { M_EXTS, O_R, O_NONE },
	{ M_PAA, O_R, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_LDC, O_CR32, O_R }, { M_LDC, O_R, O_CR32 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R }, { M_INC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R }, { M_DEC, O_I3, O_R },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R },
	{ M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R }, { M_ADD, O_C32, O_R },
	{ M_LD, O_C32, O_R }, { M_LD, O_C32, O_R }, { M_LD, O_C32, O_R }, { M_LD, O_C32, O_R },
	{ M_LD, O_C32, O_R }, { M_LD, O_C32, O_R }, { M_LD, O_C32, O_R }, { M_LD, O_C32, O_R },
	{ M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R },
	{ M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R }, { M_ADC, O_C32, O_R },
	{ M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 },
	{ M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 }, { M_LD, O_R, O_C32 },

	/* A0 - BF */
	{ M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R },
	{ M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R }, { M_SUB, O_C32, O_R },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 }, { M_LD, O_R, O_I3 },
	{ M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R },
	{ M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R }, { M_SBC, O_C32, O_R },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* C0 - DF */
	{ M_AND, O_C32, O_R }, { M_AND, O_C32, O_R }, { M_AND, O_C32, O_R }, { M_AND, O_C32, O_R },
	{ M_AND, O_C32, O_R }, { M_AND, O_C32, O_R }, { M_AND, O_C32, O_R }, { M_AND, O_C32, O_R },
	{ M_ADD, O_R, O_I32 }, { M_ADC, O_R, O_I32 }, { M_SUB, O_R, O_I32 }, { M_SBC, O_R, O_I32 },
	{ M_AND, O_R, O_I32 }, { M_XOR, O_R, O_I32 }, { M_OR, O_R, O_I32 }, { M_CP, O_R, O_I32 },
	{ M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R },
	{ M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R }, { M_XOR, O_C32, O_R },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },
	{ M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 }, { M_CP, O_R, O_I3 },

	/* E0 - FF */
	{ M_OR, O_C32, O_R }, { M_OR, O_C32, O_R }, { M_OR, O_C32, O_R }, { M_OR, O_C32, O_R },
	{ M_OR, O_C32, O_R }, { M_OR, O_C32, O_R }, { M_OR, O_C32, O_R }, { M_OR, O_C32, O_R },
	{ M_RLC, O_I8, O_R }, { M_RRC, O_I8, O_R }, { M_RL, O_I8, O_R }, { M_RR, O_I8, O_R },
	{ M_SLA, O_I8, O_R }, { M_SRA, O_I8, O_R }, { M_SLL, O_I8, O_R }, { M_SRL, O_I8, O_R },
	{ M_CP, O_C32, O_R }, { M_CP, O_C32, O_R }, { M_CP, O_C32, O_R }, { M_CP, O_C32, O_R },
	{ M_CP, O_C32, O_R }, { M_CP, O_C32, O_R }, { M_CP, O_C32, O_R }, { M_CP, O_C32, O_R },
	{ M_RLC, O_A, O_R }, { M_RRC, O_A, O_R }, { M_RL, O_A, O_R }, { M_RR, O_A, O_R },
	{ M_SLA, O_A, O_R }, { M_SRA, O_A, O_R }, { M_SLL, O_A, O_R }, { M_SRL, O_A, O_R }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ M_LD, O_M, O_I8 }, { M_DB, O_NONE, O_NONE }, { M_LD, O_M, O_I16 }, { M_DB, O_NONE, O_NONE },
	{ M_POP, O_M, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_POPW, O_M, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_M16 }, { M_DB, O_NONE, O_NONE }, { M_LDW, O_M, O_M16 }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M }, { M_LDA, O_C16, O_M },
	{ M_ANDCF, O_A, O_M }, { M_ORCF, O_A, O_M }, { M_XORCF, O_A, O_M }, { M_LDCF, O_A, O_M },
	{ M_STCF, O_A, O_M }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M }, { M_LDA, O_C32, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 40 - 5F */
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 }, { M_LD, O_M, O_C8 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 }, { M_LD, O_M, O_C16 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 60 - 7F */
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 }, { M_LD, O_M, O_C32 },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 80 - 9F */
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M }, { M_ANDCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M }, { M_ORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M }, { M_XORCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },
	{ M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M }, { M_LDCF, O_I3, O_M },

	/* A0 - BF */
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M }, { M_STCF, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M }, { M_TSET, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M }, { M_RES, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },
	{ M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M }, { M_SET, O_I3, O_M },

	/* C0 - DF */
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M }, { M_CHG, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M }, { M_BIT, O_I3, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },
	{ M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M }, { M_JP, O_CC, O_M },

	/* E0 - FF */
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M }, { M_CALL, O_CC, O_M },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }
};


const tlcs900_disassembler::tlcs900inst tlcs900_disassembler::mnemonic[256] =
{
	/* 00 - 1F */
	{ M_NOP, O_NONE, O_NONE }, { M_NORMAL, O_NONE, O_NONE }, { M_PUSH, O_SR, O_NONE }, { M_POP, O_SR, O_NONE },
	{ M_MAX, O_NONE, O_NONE }, { M_HALT, O_NONE, O_NONE }, { M_EI, O_I8, O_NONE }, { M_RETI, O_NONE, O_NONE },
	{ M_LD, O_M8, O_I8 }, { M_PUSH, O_I8, O_NONE }, { M_LD, O_M8, O_I16 }, { M_PUSH, O_I16, O_NONE },
	{ M_INCF, O_NONE, O_NONE }, { M_DECF, O_NONE, O_NONE }, { M_RET, O_NONE, O_NONE }, { M_RETD, O_I16, O_NONE },
	{ M_RCF, O_NONE, O_NONE }, { M_SCF, O_NONE, O_NONE }, { M_CCF, O_NONE, O_NONE }, { M_ZCF, O_NONE, O_NONE },
	{ M_PUSH, O_A, O_NONE }, { M_POP, O_A, O_NONE }, { M_EX, O_F, O_F }, { M_LDF, O_I8, O_NONE },
	{ M_PUSH, O_F, O_NONE }, { M_POP, O_F, O_NONE }, { M_JP, O_I16, O_NONE }, { M_JP, O_I24, O_NONE },
	{ M_CALL, O_I16, O_NONE }, { M_CALL, O_I24, O_NONE }, { M_CALR, O_D16, O_NONE }, { M_DB, O_NONE, O_NONE },

	/* 20 - 3F */
	{ M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 },
	{ M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 }, { M_LD, O_C8, O_I8 },
	{ M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE },
	{ M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE }, { M_PUSH, O_C16, O_NONE },
	{ M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 },
	{ M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 }, { M_LD, O_C16, O_I16 },
	{ M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE },
	{ M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE }, { M_PUSH, O_C32, O_NONE },

	/* 40 - 5F */
	{ M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 },
	{ M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 }, { M_LD, O_C32, O_I32 },
	{ M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE },
	{ M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE }, { M_POP, O_C16, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE },
	{ M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE },
	{ M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE }, { M_POP, O_C32, O_NONE },

	/* 60 - 7F */
	{ M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 },
	{ M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 },
	{ M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 },
	{ M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 }, { M_JR, O_CC, O_D8 },
	{ M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 },
	{ M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 },
	{ M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 },
	{ M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 }, { M_JRL, O_CC, O_D16 },

	/* 80 - 9F */
	{ M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE },
	{ M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE }, { M_80, O_NONE, O_NONE },
	{ M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE },
	{ M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE }, { M_88, O_NONE, O_NONE },
	{ M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE },
	{ M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE }, { M_90, O_NONE, O_NONE },
	{ M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE },
	{ M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE }, { M_98, O_NONE, O_NONE },

	/* A0 - BF */
	{ M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE },
	{ M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE }, { M_A0, O_NONE, O_NONE },
	{ M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE },
	{ M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE }, { M_A8, O_NONE, O_NONE },
	{ M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE },
	{ M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE }, { M_B0, O_NONE, O_NONE },
	{ M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE },
	{ M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE }, { M_B8, O_NONE, O_NONE },

	/* C0 - DF */
	{ M_C0, O_NONE, O_NONE }, { M_C0, O_NONE, O_NONE }, { M_C0, O_NONE, O_NONE }, { M_C0, O_NONE, O_NONE },
	{ M_C0, O_NONE, O_NONE }, { M_C0, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE },
	{ oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE },
	{ oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE }, { oC8, O_NONE, O_NONE },
	{ M_D0, O_NONE, O_NONE }, { M_D0, O_NONE, O_NONE }, { M_D0, O_NONE, O_NONE }, { M_D0, O_NONE, O_NONE },
	{ M_D0, O_NONE, O_NONE }, { M_D0, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE },
	{ oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE },
	{ oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE }, { oD8, O_NONE, O_NONE },

	/* E0 - FF */
	{ M_E0, O_NONE, O_NONE }, { M_E0, O_NONE, O_NONE }, { M_E0, O_NONE, O_NONE }, { M_E0, O_NONE, O_NONE },
	{ M_E0, O_NONE, O_NONE }, { M_E0, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE },
	{ M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE },
	{ M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE }, { M_E8, O_NONE, O_NONE },
	{ M_F0, O_NONE, O_NONE }, { M_F0, O_NONE, O_NONE }, { M_F0, O_NONE, O_NONE }, { M_F0, O_NONE, O_NONE },
	{ M_F0, O_NONE, O_NONE }, { M_F0, O_NONE, O_NONE }, { M_DB, O_NONE, O_NONE }, { M_LDX, O_NONE, O_NONE },
	{ M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE },
	{ M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE }, { M_SWI, O_I3, O_NONE }
};



const char *const tlcs900_disassembler::s_reg8[8] = { "W", "A", "B", "C", "D", "E", "H", "L" };
const char *const tlcs900_disassembler::s_reg16[8] = { "WA", "BC", "DE", "HL", "IX", "IY", "IZ", "SP" };
const char *const tlcs900_disassembler::s_reg32[8] = { "XWA", "XBC", "XDE", "XHL", "XIX", "XIY", "XIZ", "XSP" };
const char *const tlcs900_disassembler::s_mulreg16[8] = { "??", "WA", "??", "BC", "??", "DE", "??", "HL" };
const char *const tlcs900_disassembler::s_allreg8[256] =
{
	"RA0" ,"RW0" ,"QA0" ,"QW0" ,"RC0" ,"RB0" ,"QC0" ,"QB0" ,"RE0" ,"RD0" ,"QE0" ,"QD0" ,"RL0" ,"RH0" ,"QL0" ,"QH0" ,
	"RA1" ,"RW1" ,"QA1" ,"QW1" ,"RC1" ,"RB1" ,"QC1" ,"QB1" ,"RE1" ,"RD1" ,"QE1" ,"QD1" ,"RL1" ,"RH1" ,"QL1" ,"QH1" ,
	"RA2" ,"RW2" ,"QA2" ,"QW2" ,"RC2" ,"RB2" ,"QC2" ,"QB2" ,"RE2" ,"RD2" ,"QE2" ,"QD2" ,"RL2" ,"RH2" ,"QL2" ,"QH2" ,
	"RA3" ,"RW3" ,"QA3" ,"QW3" ,"RC3" ,"RB3" ,"QC3" ,"QB3" ,"RE3" ,"RD3" ,"QE3" ,"QD3" ,"RL3" ,"RH3" ,"QL3" ,"QH3" ,
	"r40B","r41B","r42B","r43B","r44B","r45B","r46B","r47B","r48B","r49B","r4AB","r4BB","r4CB","r4DB","r4EB","r4FB",
	"r50B","r51B","r52B","r53B","r54B","r55B","r56B","r57B","r58B","r59B","r5AB","r5BB","r5CB","r5DB","r5EB","r5FB",
	"r60B","r61B","r62B","r63B","r64B","r65B","r66B","r67B","r68B","r69B","r6AB","r6BB","r6CB","r6DB","r6EB","r6FB",
	"r70B","r71B","r72B","r73B","r74B","r75B","r76B","r77B","r78B","r79B","r7AB","r7BB","r7CB","r7DB","r7EB","r7FB",
	"r80B","r81B","r82B","r83B","r84B","r85B","r86B","r87B","r88B","r89B","r8AB","r8BB","r8CB","r8DB","r8EB","r8FB",
	"r90B","r91B","r92B","r93B","r94B","r95B","r96B","r97B","r98B","r99B","r9AB","r9BB","r9CB","r9DB","r9EB","r9FB",
	"rA0B","rA1B","rA2B","rA3B","rA4B","rA5B","rA6B","rA7B","rA8B","rA9B","rAAB","rABB","rACB","rADB","rAEB","rAFB",
	"rB0B","rB1B","rB2B","rB3B","rB4B","rB5B","rB6B","rB7B","rB8B","rB9B","rBAB","rBBB","rBCB","rBDB","rBEB","rBFB",
	"rC0B","rC1B","rC2B","rC3B","rC4B","rC5B","rC6B","rC7B","rC8B","rC9B","rCAB","rCBB","rCCB","rCDB","rCEB","rCFB",
	"RA-1","RW-1","QA-1","QW-1","RC-1","RB-1","QC-1","QB-1","RE-1","RD-1","QE-1","QD-1","RL-1","RH-1","QL-1","QH-1",
	"A"   ,"W"   ,"QA"  ,"QW"  ,"C"   ,"B"   ,"QC"  ,"QB"  ,"E"   ,"D"   ,"QE"  ,"QD"  ,"L"   ,"H"   ,"QL"  ,"QH"  ,
	"IXL" ,"IXH" ,"QIXL","QIXH","IYL" ,"IYH" ,"QIYL","QIYH","IZL" ,"IZH" ,"QIZL","QIZH","SPL" ,"SPH" ,"QSPL","QSPH",
};


const char *const tlcs900_disassembler::s_allreg16[256] =
{
	"RWA0","r01W","QWA0","r03W","RBC0","r05W","QBC0","r07W","RDE0","r09W","QDE0","r0BW","RHL0","r0DW","QHL0","r0FW",
	"RWA1","r11W","QWA1","r13W","RBC1","r15W","QBC1","r17W","RDE1","r19W","QDE1","r1BW","RHL1","r1DW","QHL1","r1FW",
	"RWA2","r21W","QWA2","r23W","RBC2","r25W","QBC2","r27W","RDE2","r29W","QDE2","r2BW","RHL2","r2DW","QHL2","r2FW",
	"RWA3","r31W","QWA3","r33W","RBC3","r35W","QBC3","r37W","RDE3","r39W","QDE3","r3BW","RHL3","r3DW","QHL3","r3FW",
	"r40W","r41W","r42W","r43W","r44W","r45W","r46W","r47W","r48W","r49W","r4AW","r4BW","r4CW","r4DW","r4EW","r4FW",
	"r50W","r51W","r52W","r53W","r54W","r55W","r56W","r57W","r58W","r59W","r5AW","r5BW","r5CW","r5DW","r5EW","r5FW",
	"r60W","r61W","r62W","r63W","r64W","r65W","r66W","r67W","r68W","r69W","r6AW","r6BW","r6CW","r6DW","r6EW","r6FW",
	"r70W","r71W","r72W","r73W","r74W","r75W","r76W","r77W","r78W","r79W","r7AW","r7BW","r7CW","r7DW","r7EW","r7FW",
	"r80W","r81W","r82W","r83W","r84W","r85W","r86W","r87W","r88W","r89W","r8AW","r8BW","r8CW","r8DW","r8EW","r8FW",
	"r90W","r91W","r92W","r93W","r94W","r95W","r96W","r97W","r98W","r99W","r9AW","r9BW","r9CW","r9DW","r9EW","r9FW",
	"rA0W","rA1W","rA2W","rA3W","rA4W","rA5W","rA6W","rA7W","rA8W","rA9W","rAAW","rABW","rACW","rADW","rAEW","rAFW",
	"rB0W","rB1W","rB2W","rB3W","rB4W","rB5W","rB6W","rB7W","rB8W","rB9W","rBAW","rBBW","rBCW","rBDW","rBEW","rBFW",
	"rC0W","rC1W","rC2W","rC3W","rC4W","rC5W","rC6W","rC7W","rC8W","rC9W","rCAW","rCBW","rCCW","rCDW","rCEW","rCFW",
	"RWA-1","rD1W","QWA-1","rD3W","RBC-1","rD5W","QBC-1","rD7W","RDE-1","rD9W","QDE-1","rDBW","RHL-1","rDDW","QHL-1","rDFW",
	"WA"  ,"rE1W","QWA" ,"rE3W","BC"  ,"rE5W","QBC" ,"rE7W","DE"  ,"rE9W","QDE" ,"rEBW","HL"  ,"rEDW","QHL" ,"rEFW",
	"IX"  ,"rF1W","QIX" ,"rF3W","IY"  ,"rF5W","QIY" ,"rF7W","IZ"  ,"rF9W","QIZ" ,"rFBW","SP"  ,"rFDW","QSP" ,"rFFW",
};


const char *const tlcs900_disassembler::s_allreg32[256] =
{
	"XWA0","XWA0","XWA0","r03L","XBC0","XBC0","XBC0","r07L","XDE0","XDE0","XDE0","r0BL","XHL0","XHL0","XHL0","r0FL",
	"XWA1","XWA1","XWA1","r13L","XBC1","XBC1","XBC1","r17L","XDE1","XDE1","XDE1","r1BL","XHL1","XHL1","XHL1","r1FL",
	"XWA2","XWA2","XWA2","r23L","XBC2","XBC2","XBC2","r27L","XDE2","XDE2","XDE2","r2BL","XHL2","XHL2","XHL2","r2FL",
	"XWA3","XWA3","XWA3","r33L","XBC3","XBC3","XBC3","r37L","XDE3","XDE3","XDE3","r3BL","XHL3","XHL3","XHL3","r3FL",
	"r40L","r41L","r42L","r43L","r44L","r45L","r46L","r47L","r48L","r49L","r4AL","r4BL","r4CL","r4DL","r4EL","r4FL",
	"r50L","r51L","r52L","r53L","r54L","r55L","r56L","r57L","r58L","r59L","r5AL","r5BL","r5CL","r5DL","r5EL","r5FL",
	"r60L","r61L","r62L","r63L","r64L","r65L","r66L","r67L","r68L","r69L","r6AL","r6BL","r6CL","r6DL","r6EL","r6FL",
	"r70L","r71L","r72L","r73L","r74L","r75L","r76L","r77L","r78L","r79L","r7AL","r7BL","r7CL","r7DL","r7EL","r7FL",
	"r80L","r81L","r82L","r83L","r84L","r85L","r86L","r87L","r88L","r89L","r8AL","r8BL","r8CL","r8DL","r8EL","r8FL",
	"r90L","r91L","r92L","r93L","r94L","r95L","r96L","r97L","r98L","r99L","r9AL","r9BL","r9CL","r9DL","r9EL","r9FL",
	"rA0L","rA1L","rA2L","rA3L","rA4L","rA5L","rA6L","rA7L","rA8L","rA9L","rAAL","rABL","rACL","rADL","rAEL","rAFL",
	"rB0L","rB1L","rB2L","rB3L","rB4L","rB5L","rB6L","rB7L","rB8L","rB9L","rBAL","rBBL","rBCL","rBDL","rBEL","rBFL",
	"rC0L","rC1L","rC2L","rC3L","rC4L","rC5L","rC6L","rC7L","rC8L","rC9L","rCAL","rCBL","rCCL","rCDL","rCEL","rCFL",
	"XWA-1","XWA-1","XWA-1","rD3L","XBC-1","XBC-1","XBC-1","rD7L","XDE-1","XDE-1","XDE-1","rDBL","XHL-1","XHL-1","XHL-1","rDFL",
	"XWA" ,"XWA" ,"XWA" ,"rE3L","XBC" ,"XBC", "XBC" ,"rE7L","XDE" ,"XDE" ,"XDE" ,"rEDL","XHL" ,"XHL" ,"XHL" ,"rEFL",
	"XIX" ,"XIX" ,"XIX" ,"rF3L","XIY" ,"XIY" ,"XIY" ,"rF7L","XIZ" ,"XIZ" ,"XIZ" ,"rFBL","XSP" ,"XSP" ,"XSP" ,"rFFL",
};


const char *const tlcs900_disassembler::s_cond[16] =
{
	"F","LT","LE","ULE","PE/OV","M/MI","Z","C","T","GE","GT","UGT","PO/NOV","P/PL","NZ","NC"
};


u32 tlcs900_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t tlcs900_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const tlcs900inst *dasm;
	std::string buf;
	uint8_t   op, op1;
	uint32_t  imm;
	int     flags = 0;
	offs_t  pos = pc;

	op = opcodes.r8( pos++ );

	dasm = &mnemonic[ op ];

	/* Check for extended addressing modes */
	switch( dasm->mnemonic )
	{
	default:
		break;

	case M_80:
		buf = string_format("%s", s_reg32[op & 0x07]);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_80[ op ];
		break;

	case M_88:
		imm = opcodes.r8( pos++ );
		buf = string_format("%s+0x%02x", s_reg32[op & 0x07], imm);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_88[ op ];
		break;

	case M_90:
		buf = string_format("%s", s_reg32[op & 0x07]);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_90[ op ];
		break;

	case M_98:
		imm = opcodes.r8( pos++ );
		buf = string_format("%s+0x%02x", s_reg32[op & 0x07], imm);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_98[ op ];
		break;

	case M_A0:
		buf = string_format("%s", s_reg32[op & 0x07]);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_a0[ op ];
		break;

	case M_A8:
		imm = opcodes.r8( pos++ );
		buf = string_format("%s+0x%02x", s_reg32[op & 0x07], imm);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_a0[ op ];
		break;

	case M_B0:
		buf = string_format("%s", s_reg32[op & 0x07]);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_b0[ op ];
		break;

	case M_B8:
		imm = opcodes.r8( pos++ );
		buf = string_format("%s+0x%02x", s_reg32[op & 0x07], imm);
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_b8[ op ];
		break;

	case M_C0:
		switch( op & 0x07 )
		{
		case 0x00:  /* 0xC0 */
			imm = opcodes.r8( pos++ );
			buf = string_format("0x%02x", imm);
			break;

		case 0x01:  /* 0xC1 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			buf = string_format("0x%04x", imm);
			break;

		case 0x02:  /* 0xC2 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			imm = imm | (opcodes.r8( pos++ ) << 16);
			buf = string_format("0x%06x", imm);
			break;

		case 0x03:  /* 0xC3 */
			imm = opcodes.r8( pos++ );
			switch( imm & 0x03 )
			{
			case 0x00:
				buf = string_format("%s", s_allreg32[imm]);
				break;

			case 0x01:
				op = imm;
				imm = opcodes.r8( pos++ );
				imm = imm | (opcodes.r8( pos++ ) << 8);
				buf = string_format("%s+0x%04x", s_allreg32[op], imm);
				break;

			case 0x02:
				buf = string_format("unknown");
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg8[op1]);
					break;

				case 0x07:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg16[op1]);
					break;

				case 0x13:
					imm = opcodes.r8( pos++ );
					imm = imm | (opcodes.r8( pos++ ) << 8);
					buf = string_format("0x%06x", pc + pos + (int16_t)imm);
					break;
				}
				break;
			}
			break;

		case 0x04:  /* 0xC4 */
			imm = opcodes.r8( pos++ );
			buf = string_format("-%s", s_allreg32[imm]);
			break;

		case 0x05:  /* 0xC5 */
			imm = opcodes.r8( pos++ );
			buf = string_format("%s+", s_allreg32[imm]);
			break;
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_c0[ op ];
		break;

	case oC8:
		if ( op & 0x08 )
		{
			buf = string_format("%s", s_reg8[ op & 0x07 ]);
		}
		else
		{
			imm = opcodes.r8( pos++ );
			buf = string_format("%s", s_allreg8[imm]);
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_c8[ op ];
		break;

	case M_D0:
		switch( op & 0x07 )
		{
		case 0x00:  /* 0xD0 */
			imm = opcodes.r8( pos++ );
			buf = string_format("0x%02x", imm);
			break;

		case 0x01:  /* 0xD1 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			buf = string_format("0x%04x", imm);
			break;

		case 0x02:  /* 0xD2 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			imm = imm | (opcodes.r8( pos++ ) << 16);
			buf = string_format("0x%06x", imm);
			break;

		case 0x03:  /* 0xD3 */
			imm = opcodes.r8( pos++ );
			switch( imm & 0x03 )
			{
			case 0x00:
				buf = string_format("%s", s_allreg32[imm]);
				break;

			case 0x01:
				op = imm;
				imm = opcodes.r8( pos++ );
				imm = imm | (opcodes.r8( pos++ ) << 8);
				buf = string_format("%s+0x%04x", s_allreg32[op], imm);
				break;

			case 0x02:
				buf = string_format("unknown");
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg8[op1]);
					break;

				case 0x07:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg16[op1]);
					break;

				case 0x13:
					imm = opcodes.r8( pos++ );
					imm = imm | (opcodes.r8( pos++ ) << 8);
					buf = string_format("0x%06x", pc + pos + (int16_t)imm);
					break;
				}
				break;
			}
			break;

		case 0x04:  /* 0xD4 */
			imm = opcodes.r8( pos++ );
			buf = string_format("-%s", s_allreg32[imm]);
			break;

		case 0x05:  /* 0xD5 */
			imm = opcodes.r8( pos++ );
			buf = string_format("%s+", s_allreg32[imm]);
			break;
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_d0[ op ];
		break;

	case oD8:
		if ( op & 0x08 )
		{
			buf = string_format("%s", s_reg16[ op & 0x07 ]);
		}
		else
		{
			imm = opcodes.r8( pos++ );
			buf = string_format("%s", s_allreg16[imm]);
		}

		op = opcodes.r8( pos++ );
		dasm = &mnemonic_d8[ op ];
		break;

	case M_E0:
		switch( op & 0x07 )
		{
		case 0x00:  /* 0xE0 */
			imm = opcodes.r8( pos++ );
			buf = string_format("0x%02x", imm);
			break;

		case 0x01:  /* 0xE1 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			buf = string_format("0x%04x", imm);
			break;

		case 0x02:  /* 0xE2 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			imm = imm | (opcodes.r8( pos++ ) << 16);
			buf = string_format("0x%06x", imm);
			break;

		case 0x03:  /* 0xE3 */
			imm = opcodes.r8( pos++ );
			switch( imm & 0x03 )
			{
			case 0x00:
				buf = string_format("%s", s_allreg32[imm]);
				break;

			case 0x01:
				op = imm;
				imm = opcodes.r8( pos++ );
				imm = imm | (opcodes.r8( pos++ ) << 8);
				buf = string_format("%s+0x%04x", s_allreg32[op], imm);
				break;

			case 0x02:
				buf = string_format("unknown");
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg8[op1]);
					break;

				case 0x07:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg16[op1]);
					break;

				case 0x13:
					imm = opcodes.r8( pos++ );
					imm = imm | (opcodes.r8( pos++ ) << 8);
					buf = string_format("0x%06x", pc + pos + (int16_t)imm);
					break;
				}
				break;
			}
			break;

		case 0x04:  /* 0xE4 */
			imm = opcodes.r8( pos++ );
			buf = string_format("-%s", s_allreg32[imm]);
			break;

		case 0x05:  /* 0xE5 */
			imm = opcodes.r8( pos++ );
			buf = string_format("%s+", s_allreg32[imm]);
			break;
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_e0[ op ];
		break;

	case M_E8:
		if ( op & 0x08 )
		{
			buf = string_format("%s", s_reg32[ op & 0x07 ]);
		}
		else
		{
			imm = opcodes.r8( pos++ );
			buf = string_format("%s", s_allreg32[imm]);
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_e8[ op ];
		break;

	case M_F0:
		switch( op & 0x07 )
		{
		case 0x00:  /* 0xF0 */
			imm = opcodes.r8( pos++ );
			buf = string_format("0x%02x", imm);
			break;

		case 0x01:  /* 0xF1 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			buf = string_format("0x%04x", imm);
			break;

		case 0x02:  /* 0xF2 */
			imm = opcodes.r8( pos++ );
			imm = imm | (opcodes.r8( pos++ ) << 8);
			imm = imm | (opcodes.r8( pos++ ) << 16);
			buf = string_format("0x%06x", imm);
			break;

		case 0x03:  /* 0xF3 */
			imm = opcodes.r8( pos++ );
			switch( imm & 0x03 )
			{
			case 0x00:
				buf = string_format("%s", s_allreg32[imm]);
				break;

			case 0x01:
				op = imm;
				imm = opcodes.r8( pos++ );
				imm = imm | (opcodes.r8( pos++ ) << 8);
				buf = string_format("%s+0x%04x", s_allreg32[op], imm);
				break;

			case 0x02:
				buf = string_format("unknown");
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg8[op1]);
					break;

				case 0x07:
					op = opcodes.r8( pos++ );
					op1 = opcodes.r8( pos++ );
					buf = string_format("%s+%s", s_allreg32[op], s_allreg16[op1]);
					break;

				case 0x13:
					imm = opcodes.r8( pos++ );
					imm = imm | (opcodes.r8( pos++ ) << 8);
					buf = string_format("0x%06x", pc + pos + (int16_t)imm);
					break;
				}
				break;
			}
			break;

		case 0x04:  /* 0xF4 */
			imm = opcodes.r8( pos++ );
			buf = string_format("-%s", s_allreg32[imm]);
			break;

		case 0x05:  /* 0xF5 */
			imm = opcodes.r8( pos++ );
			buf = string_format("%s+", s_allreg32[imm]);
			break;
		}
		op = opcodes.r8( pos++ );
		dasm = &mnemonic_f0[ op ];
		break;
	}

	util::stream_format(stream, "%s", s_mnemonic[ dasm->mnemonic ]);

	switch( dasm->mnemonic )
	{
	default:
		/* maybe assert */
		break;
	case M_CALL:
	case M_CALR:
		flags = STEP_OVER;
		break;
	case M_RET:
	case M_RETD:
	case M_RETI:
		flags = STEP_OUT;
		break;
	}

	switch( dasm->operand1 )
	{
		case O_NONE:
				break;

	case O_A:
		util::stream_format(stream, " A");
		break;

	case O_C8:
		util::stream_format(stream, " %s", s_reg8[op & 0x07]);
		break;

	case O_C16:
		util::stream_format(stream, " %s", s_reg16[op & 0x07]);
		break;

	case O_C32:
		util::stream_format(stream, " %s", s_reg32[op & 0x07]);
		break;

	case O_MC16:
		util::stream_format(stream, " %s", s_mulreg16[op & 0x07]);
		break;

	case O_CC:
		util::stream_format(stream, " %s", s_cond[op & 0x0F]);
		break;

	case O_CR8:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x22:
			util::stream_format(stream, " DMAM0");
			break;
		case 0x26:
			util::stream_format(stream, " DMAM1");
			break;
		case 0x2a:
			util::stream_format(stream, " DMAM2");
			break;
		case 0x2e:
			util::stream_format(stream, " DMAM3");
			break;
		default:
			util::stream_format(stream, " unknown");
			break;
		}
		break;

	case O_CR16:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x20:
			util::stream_format(stream, " DMAC0");
			break;
		case 0x24:
			util::stream_format(stream, " DMAC1");
			break;
		case 0x28:
			util::stream_format(stream, " DMAC2");
			break;
		case 0x2c:
			util::stream_format(stream, " DMAC3");
			break;
		default:
			util::stream_format(stream, " unknown");
			break;
		}
		break;

	case O_CR32:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x00:
			util::stream_format(stream, " DMAS0");
			break;
		case 0x04:
			util::stream_format(stream, " DMAS1");
			break;
		case 0x08:
			util::stream_format(stream, " DMAS2");
			break;
		case 0x0c:
			util::stream_format(stream, " DMAS3");
			break;
		case 0x10:
			util::stream_format(stream, " DMAD0");
			break;
		case 0x14:
			util::stream_format(stream, " DMAD1");
			break;
		case 0x18:
			util::stream_format(stream, " DMAD2");
			break;
		case 0x1c:
			util::stream_format(stream, " DMAD3");
			break;
		default:
			util::stream_format(stream, " unknown");
			break;
		}
		break;

	case O_D8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, " 0x%06x", ( pc + pos + (int8_t)imm ) & 0xFFFFFF);
		break;

	case O_D16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, " 0x%06x", ( pc + pos + (int16_t)imm ) & 0xFFFFFF);
		break;

	case O_F:
		util::stream_format(stream, " F");
		break;

	case O_I3:
		util::stream_format(stream, " %d", op & 0x07);
		break;

	case O_I8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, " 0x%02x", imm);
		break;

	case O_I16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, " 0x%04x", imm);
		break;

	case O_I24:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		imm = imm | (opcodes.r8( pos++ ) << 16);
		util::stream_format(stream, " 0x%06x", imm);
		break;

	case O_I32:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		imm = imm | (opcodes.r8( pos++ ) << 16);
		imm = imm | (opcodes.r8( pos++ ) << 24);
		util::stream_format(stream, "0x%08x", imm);
		break;

	case O_M:
		switch( dasm->mnemonic )
		{
		case M_CALL:
		case M_JP:
		case M_LDA:
			util::stream_format(stream, " %s", buf);
			break;
		default:
			util::stream_format(stream, " (%s)", buf);
			break;
		}
		break;

	case O_M8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, " (0x%02x)", imm);
		break;

	case O_M16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, " (0x%04x)", imm);
		break;

	case O_R:
		util::stream_format(stream, " %s", buf);
		break;

	case O_SR:
		util::stream_format(stream, " SR");
		break;
	}

	switch( dasm->operand2 )
	{
		case O_NONE:
				break;

		case O_A:
		util::stream_format(stream, ",A");
		break;

	case O_C8:
		util::stream_format(stream, ",%s", s_reg8[op & 0x07]);
		break;

	case O_C16:
		util::stream_format(stream, ",%s", s_reg16[op & 0x07]);
		break;

	case O_C32:
		util::stream_format(stream, ",%s", s_reg32[op & 0x07]);
		break;

	case O_MC16:
		util::stream_format(stream, ",%s", s_mulreg16[op & 0x07]);
		break;

	case O_CC:
		util::stream_format(stream, ",%s", s_cond[op & 0x0F]);
		break;

	case O_CR8:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x22:
			util::stream_format(stream, ",DMAM0");
			break;
		case 0x26:
			util::stream_format(stream, ",DMAM1");
			break;
		case 0x2a:
			util::stream_format(stream, ",DMAM2");
			break;
		case 0x2e:
			util::stream_format(stream, ",DMAM3");
			break;
		default:
			util::stream_format(stream, ",unknown");
			break;
		}
		break;

	case O_CR16:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x20:
			util::stream_format(stream, ",DMAC0");
			break;
		case 0x24:
			util::stream_format(stream, ",DMAC1");
			break;
		case 0x28:
			util::stream_format(stream, ",DMAC2");
			break;
		case 0x2c:
			util::stream_format(stream, ",DMAC3");
			break;
		default:
			util::stream_format(stream, ",unknown");
			break;
		}
		break;

	case O_CR32:
		imm = opcodes.r8( pos++ );
		switch( imm )
		{
		case 0x00:
			util::stream_format(stream, ",DMAS0");
			break;
		case 0x04:
			util::stream_format(stream, ",DMAS1");
			break;
		case 0x08:
			util::stream_format(stream, ",DMAS2");
			break;
		case 0x0c:
			util::stream_format(stream, ",DMAS3");
			break;
		case 0x10:
			util::stream_format(stream, ",DMAD0");
			break;
		case 0x14:
			util::stream_format(stream, ",DMAD1");
			break;
		case 0x18:
			util::stream_format(stream, ",DMAD2");
			break;
		case 0x1c:
			util::stream_format(stream, ",DMAD3");
			break;
		default:
			util::stream_format(stream, ",unknown");
			break;
		}
		break;

	case O_D8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, ",0x%06x", ( pc + pos + (int8_t)imm ) & 0xFFFFFF);
		break;

	case O_D16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, ",0x%06x", ( pc + pos + (int16_t)imm ) & 0xFFFFFF);
		break;

	case O_F:
		util::stream_format(stream, ",F'");
		break;

	case O_I3:
		util::stream_format(stream, ",%d", op & 0x07);
		break;

	case O_I8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, ",0x%02x", imm);
		break;

	case O_I16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, ",0x%04x", imm);
		break;

	case O_I24:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		imm = imm | (opcodes.r8( pos++ ) << 16);
		util::stream_format(stream, ",0x%06x", imm);
		break;

	case O_I32:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		imm = imm | (opcodes.r8( pos++ ) << 16);
		imm = imm | (opcodes.r8( pos++ ) << 24);
		util::stream_format(stream, ",0x%08x", imm);
		break;

	case O_M:
		switch( dasm->mnemonic )
		{
		case M_CALL:
		case M_JP:
		case M_LDA:
			util::stream_format(stream, ",%s", buf);
			break;
		default:
			util::stream_format(stream, ",(%s)", buf);
			break;
		}
		break;

	case O_M8:
		imm = opcodes.r8( pos++ );
		util::stream_format(stream, ",(0x%02x)", imm);
		break;

	case O_M16:
		imm = opcodes.r8( pos++ );
		imm = imm | (opcodes.r8( pos++ ) << 8);
		util::stream_format(stream, ",(0x%04x)", imm);
		break;

	case O_R:
		util::stream_format(stream, ",%s", buf);
		break;

	case O_SR:
		util::stream_format(stream, ",SR");
		break;
	}

	return (pos - pc) | flags | SUPPORTED;
}
