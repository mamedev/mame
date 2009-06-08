/*******************************************************************

Toshiba TLCS-900/H disassembly

*******************************************************************/

#include "cpuintrf.h"
#include "debugger.h"
#include "tlcs900.h"


enum e_mnemonics
{
	_ADC, _ADD, _AND, _ANDCF, _BIT, _BS1B,
	_BS1F, _CALL, _CALR, _CCF, _CHG, _CP,
	_CPD, _CPDW, _CPDR, _CPDRW, _CPI, _CPIR,
	_CPIRW, _CPIW, _CPL, _DAA, _DB, _DEC,
	_DECF, _DECW, _DIV, _DIVS, _DJNZ, _EI,
	_EX, _EXTS, _EXTZ, _HALT, _INC, _INCF,
	_INCW, _JP, _JR, _JRL, _LD, _LDA,
	_LDC, _LDCF, _LDD, _LDDR, _LDDRW, _LDDW,
	_LDF, _LDI, _LDIR, _LDIRW, _LDIW, _LDW,
	_LDX, _LINK, _MAX, _MDEC1, _MDEC2, _MDEC4,
	_MINC1, _MINC2, _MINC4, _MIRR, _MUL, _MULA,
	_MULS, _NEG, _NOP, _NORMAL, _OR, _ORCF,
	_PAA, _POP, _POPW, _PUSH, _PUSHW, _RCF,
	_RES, _RET, _RETD, _RETI, _RL, _RLC,
	_RLCW, _RLD, _RLW, _RR, _RRC, _RRCW,
	_RRD, _RRW, _SBC, _SCC, _SCF, _SET,
	_SLA, _SLAW, _SLL, _SLLW, _SRA, _SRAW,
	_SRL, _SRLW, _STCF, _SUB, _SWI, _TSET,
	_UNLK, _XOR, _XORCF, _ZCF,
	_80, _88, _90, _98, _A0, _A8, _B0, _B8,
	_C0, oC8, _D0, oD8, _E0, _E8, _F0
};


static const char *const s_mnemonic[] =
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


enum e_operand
{
	_A=1,		/* currect register set register A */
	_C8,		/* current register set byte */
	_C16,		/* current register set word */
	_C32,		/* current register set long word */
	_MC16,		/* current register set mul/div register word */
	_CC,		/* condition */
	_CR8,		/* byte control register */
	_CR16,		/* word control register */
	_CR32,		/* long word control register */
	_D8,		/* byte displacement */
	_D16,		/* word displacement */
	_F,			/* F register */
	_I3,		/* immediate 3 bit (part of last byte) */
	_I8,		/* immediate byte */
	_I16,		/* immediate word */
	_I24,		/* immediate 3 byte address */
	_I32,		/* immediate long word */
	_M,			/* memory location (defined by extension) */
	_M8,		/* (8) */
	_M16,		/* (i16) */
	_R,			/* register */
	_SR,		/* status register */
};


typedef struct
{
	int		mnemonic;
	int		operand1;
	int		operand2;
} tlcs900inst;


static const tlcs900inst mnemonic_80[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSH, _M, 0 }, { _DB, 0, 0 }, { _RLD, _A, _M }, { _RRD, _A, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LDI, 0, 0 }, { _LDIR, 0, 0 }, { _LDD, 0, 0 }, { _LDDR, 0, 0 },
	{ _CPI, 0, 0 }, { _CPIR, 0, 0 }, { _CPD, 0, 0 }, { _CPDR, 0, 0 },
	{ _DB, 0, 0 }, { _LD, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _ADD, _M, _I8 }, { _ADC, _M, _I8 }, { _SUB, _M, _I8 }, { _SBC, _M, _I8 },
	{ _AND, _M, _I8 }, { _XOR, _M, _I8 }, { _OR, _M, _I8 }, { _CP, _M, _I8 },

	/* 40 - 5F */
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },

	/* 60 - 7F */
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLC, _M, 0 }, { _RRC, _M, 0 }, { _RL, _M, 0 }, { _RR, _M, 0 },
	{ _SLA, _M, 0 }, { _SRA, _M, 0 }, { _SLL, _M, 0 }, { _SRL, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },

	/* A0 - BF */
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },

	/* C0 - DF */
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },

	/* E0 - FF */
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
};


static const tlcs900inst mnemonic_88[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSH, _M, 0 }, { _DB, 0, 0 }, { _RLD, _A, _M }, { _RRD, _A, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _LD, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _ADD, _M, _I8 }, { _ADC, _M, _I8 }, { _SUB, _M, _I8 }, { _SBC, _M, _I8 },
	{ _AND, _M, _I8 }, { _XOR, _M, _I8 }, { _OR, _M, _I8 }, { _CP, _M, _I8 },

	/* 40 - 5F */
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },

	/* 60 - 7F */
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLC, _M, 0 }, { _RRC, _M, 0 }, { _RL, _M, 0 }, { _RR, _M, 0 },
	{ _SLA, _M, 0 }, { _SRA, _M, 0 }, { _SLL, _M, 0 }, { _SRL, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },

	/* A0 - BF */
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },

	/* C0 - DF */
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },

	/* E0 - FF */
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
};


static const tlcs900inst mnemonic_90[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSHW, _M, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LDIW, 0, 0 }, { _LDIRW, 0, 0 }, { _LDDW, 0, 0 }, { _LDDRW, 0, 0 },
	{ _CPIW, 0, 0 }, { _CPIRW, 0, 0 }, { _CPDW, 0, 0 }, { _CPDRW, 0, 0 },
	{ _DB, 0, 0 }, { _LDW, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _ADD, _M, _I16 }, { _ADC, _M, _I16 }, { _SUB, _M, _I16 }, { _SBC, _M, _I16 },
	{ _AND, _M, _I16 }, { _XOR, _M, _I16 }, { _OR, _M, _I16 }, { _CP, _M, _I16 },

	/* 40 - 5F */
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },

	/* 60 - 7F */
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLCW, _M, 0 }, { _RRCW, _M, 0 }, { _RLW, _M, 0 }, { _RRW, _M, 0 },
	{ _SLAW, _M, 0 }, { _SRAW, _M, 0 }, { _SLLW, _M, 0 }, { _SRLW, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },

	/* A0 - BF */
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },

	/* C0 - DF */
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },

	/* E0 - FF */
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
};


static const tlcs900inst mnemonic_98[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSHW, _M, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _LDW, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _ADD, _M, _I16 }, { _ADC, _M, _I16 }, { _SUB, _M, _I16 }, { _SBC, _M, _I16 },
	{ _AND, _M, _I16 }, { _XOR, _M, _I16 }, { _OR, _M, _I16 }, { _CP, _M, _I16 },

	/* 40 - 5F */
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },

	/* 60 - 7F */
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLCW, _M, 0 }, { _RRCW, _M, 0 }, { _RLW, _M, 0 }, { _RRW, _M, 0 },
	{ _SLAW, _M, 0 }, { _SRAW, _M, 0 }, { _SLLW, _M, 0 }, { _SRLW, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },

	/* A0 - BF */
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },

	/* C0 - DF */
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },

	/* E0 - FF */
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
};


static const tlcs900inst mnemonic_a0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M },
	{ _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M },
	{ _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M },
	{ _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 },
	{ _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 },
	{ _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M },
	{ _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M },
	{ _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 },
	{ _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 },

	/* A0 - BF */
	{ _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M },
	{ _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M },
	{ _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 },
	{ _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 },
	{ _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M },
	{ _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M },
	{ _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 },
	{ _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 },

	/* C0 - DF */
	{ _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M },
	{ _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M },
	{ _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 },
	{ _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 },
	{ _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M },
	{ _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M },
	{ _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 },
	{ _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 },

	/* E0 - FF */
	{ _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M },
	{ _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M },
	{ _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 },
	{ _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 },
	{ _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M },
	{ _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M },
	{ _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 },
	{ _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 },
};


static const tlcs900inst mnemonic_b0[256] =
{
	/* 00 - 1F */
	{ _LD, _M, _I8 }, { _DB, 0, 0 }, { _LD, _M, _I16 }, { _DB, 0, 0 },
	{ _POP, _M, 0 }, { _DB, 0, 0 }, { _POPW, _M, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _M16 }, { _DB, 0, 0 }, { _LDW, _M, _M16 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _ANDCF, _A, _M }, { _ORCF, _A, _M }, { _XORCF, _A, _M }, { _LDCF, _A, _M },
	{ _STCF, _A, _M }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },

	/* A0 - BF */
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },

	/* C0 - DF */
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },

	/* E0 - FF */
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 },
	{ _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 },
	{ _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 },
	{ _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 }, { _RET, _CC, 0 }
};


static const tlcs900inst mnemonic_b8[256] =
{
	/* 00 - 1F */
	{ _LD, _M, _I8 }, { _DB, 0, 0 }, { _LD, _M, _I16 }, { _DB, 0, 0 },
	{ _POP, _M, 0 }, { _DB, 0, 0 }, { _POPW, _M, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _M16 }, { _DB, 0, 0 }, { _LDW, _M, _M16 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _ANDCF, _A, _M }, { _ORCF, _A, _M }, { _XORCF, _A, _M }, { _LDCF, _A, _M },
	{ _STCF, _A, _M }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },

	/* A0 - BF */
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },

	/* C0 - DF */
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },

	/* E0 - FF */
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }
};


static const tlcs900inst mnemonic_c0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSH, _M, 0 }, { _DB, 0, 0 }, { _RLD, _A, _M }, { _RRD, _A, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _LD, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M }, { _LD, _C8, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 }, { _EX, _M, _C8 },
	{ _ADD, _M, _I8 }, { _ADC, _M, _I8 }, { _SUB, _M, _I8 }, { _SBC, _M, _I8 },
	{ _AND, _M, _I8 }, { _XOR, _M, _I8 }, { _OR, _M, _I8 }, { _CP, _M, _I8 },

	/* 40 - 5F */
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M }, { _MUL, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M }, { _MULS, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M }, { _DIV, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },
	{ _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M }, { _DIVS, _MC16, _M },

	/* 60 - 7F */
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M }, { _INC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M }, { _DEC, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLC, _M, 0 }, { _RRC, _M, 0 }, { _RL, _M, 0 }, { _RR, _M, 0 },
	{ _SLA, _M, 0 }, { _SRA, _M, 0 }, { _SLL, _M, 0 }, { _SRL, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M }, { _ADD, _C8, _M },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 }, { _ADD, _M, _C8 },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M }, { _ADC, _C8, _M },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },
	{ _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 }, { _ADC, _M, _C8 },

	/* A0 - BF */
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M }, { _SUB, _C8, _M },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 }, { _SUB, _M, _C8 },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M }, { _SBC, _C8, _M },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },
	{ _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 }, { _SBC, _M, _C8 },

	/* C0 - DF */
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M }, { _AND, _C8, _M },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 }, { _AND, _M, _C8 },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M }, { _XOR, _C8, _M },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },
	{ _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 }, { _XOR, _M, _C8 },

	/* E0 - FF */
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M }, { _OR, _C8, _M },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 }, { _OR, _M, _C8 },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M }, { _CP, _C8, _M },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
	{ _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 }, { _CP, _M, _C8 },
};


/* TODO: _MUL_M_I8, _MULS_M_I8, _DIV_M_I8, _DIVS_M_i8 need to be fixed */
static const tlcs900inst mnemonic_c8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _LD, _R, _I8 },
	{ _PUSH, _R, 0 }, { _POP, _R, 0 }, { _CPL, _R, 0 }, { _NEG, _R, 0 },
	{ _MUL, _R, _I8 }, { _MULS, _R, _I8 }, { _DIV, _R, _I8 }, { _DIVS, _R, _I8 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DAA, _R, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DJNZ, _R, _D8 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _ANDCF, _I8, _R }, { _ORCF, _I8, _R }, { _XORCF, _I8, _R }, { _LDCF, _I8, _R },
	{ _STCF, _I8, _R }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _ANDCF, _A, _R }, { _ORCF, _A, _R }, { _XORCF, _A, _R }, { _LDCF, _A, _R },
	{ _STCF, _A, _R }, { _DB, 0, 0 }, { _LDC, _CR8, _R }, { _LDC, _R, _CR8 },
	{ _RES, _I8, _R }, { _SET, _I8, _R }, { _CHG, _I8, _R }, { _BIT, _I8, _R },
	{ _TSET, _I8, _R }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _MUL, _MC16, _R }, { _MUL, _MC16, _R }, { _MUL, _MC16, _R }, { _MUL, _MC16, _R },
	{ _MUL, _MC16, _R }, { _MUL, _MC16, _R }, { _MUL, _MC16, _R }, { _MUL, _MC16, _R },
	{ _MULS, _MC16, _R }, { _MULS, _MC16, _R }, { _MULS, _MC16, _R }, { _MULS, _MC16, _R },
	{ _MULS, _MC16, _R }, { _MULS, _MC16, _R }, { _MULS, _MC16, _R }, { _MULS, _MC16, _R },
	{ _DIV, _MC16, _R }, { _DIV, _MC16, _R }, { _DIV, _MC16, _R }, { _DIV, _MC16, _R },
	{ _DIV, _MC16, _R }, { _DIV, _MC16, _R }, { _DIV, _MC16, _R }, { _DIV, _MC16, _R },
	{ _DIVS, _MC16, _R }, { _DIVS, _MC16, _R }, { _DIVS, _MC16, _R }, { _DIVS, _MC16, _R },
	{ _DIVS, _MC16, _R }, { _DIVS, _MC16, _R }, { _DIVS, _MC16, _R }, { _DIVS, _MC16, _R },

	/* 60 - 7F */
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },

	/* 80 - 9F */
	{ _ADD, _C8, _R }, { _ADD, _C8, _R }, { _ADD, _C8, _R }, { _ADD, _C8, _R },
	{ _ADD, _C8, _R }, { _ADD, _C8, _R }, { _ADD, _C8, _R }, { _ADD, _C8, _R },
	{ _LD, _C8, _R }, { _LD, _C8, _R }, { _LD, _C8, _R }, { _LD, _C8, _R },
	{ _LD, _C8, _R }, { _LD, _C8, _R }, { _LD, _C8, _R }, { _LD, _C8, _R },
	{ _ADC, _C8, _R }, { _ADC, _C8, _R }, { _ADC, _C8, _R }, { _ADC, _C8, _R },
	{ _ADC, _C8, _R }, { _ADC, _C8, _R }, { _ADC, _C8, _R }, { _ADC, _C8, _R },
	{ _LD, _R, _C8 }, { _LD, _R, _C8 }, { _LD, _R, _C8 }, { _LD, _R, _C8 },
	{ _LD, _R, _C8 }, { _LD, _R, _C8 }, { _LD, _R, _C8 }, { _LD, _R, _C8 },

	/* A0 - BF */
	{ _SUB, _C8, _R }, { _SUB, _C8, _R }, { _SUB, _C8, _R }, { _SUB, _C8, _R },
	{ _SUB, _C8, _R }, { _SUB, _C8, _R }, { _SUB, _C8, _R }, { _SUB, _C8, _R },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _SBC, _C8, _R }, { _SBC, _C8, _R }, { _SBC, _C8, _R }, { _SBC, _C8, _R },
	{ _SBC, _C8, _R }, { _SBC, _C8, _R }, { _SBC, _C8, _R }, { _SBC, _C8, _R },
	{ _EX, _C8, _R }, { _EX, _C8, _R }, { _EX, _C8, _R }, { _EX, _C8, _R },
	{ _EX, _C8, _R }, { _EX, _C8, _R }, { _EX, _C8, _R }, { _EX, _C8, _R },

	/* C0 - DF */
	{ _AND, _C8, _R }, { _AND, _C8, _R }, { _AND, _C8, _R }, { _AND, _C8, _R },
	{ _AND, _C8, _R }, { _AND, _C8, _R }, { _AND, _C8, _R }, { _AND, _C8, _R },
	{ _ADD, _R, _I8 }, { _ADC, _R, _I8 }, { _SUB, _R, _I8 }, { _SBC, _R, _I8 },
	{ _AND, _R, _I8 }, { _XOR, _R, _I8 }, { _OR, _R, _I8 }, { _CP, _R, _I8 },
	{ _XOR, _C8, _R }, { _XOR, _C8, _R }, { _XOR, _C8, _R }, { _XOR, _C8, _R },
	{ _XOR, _C8, _R }, { _XOR, _C8, _R }, { _XOR, _C8, _R }, { _XOR, _C8, _R },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },

	/* E0 - FF */
	{ _OR, _C8, _R }, { _OR, _C8, _R }, { _OR, _C8, _R }, { _OR, _C8, _R },
	{ _OR, _C8, _R }, { _OR, _C8, _R }, { _OR, _C8, _R }, { _OR, _C8, _R },
	{ _RLC, _I8, _R }, { _RRC, _I8, _R }, { _RL, _I8, _R }, { _RR, _I8, _R },
	{ _SLA, _I8, _R }, { _SRA, _I8, _R }, { _SLL, _I8, _R }, { _SRL, _I8, _R },
	{ _CP, _C8, _R }, { _CP, _C8, _R }, { _CP, _C8, _R }, { _CP, _C8, _R },
	{ _CP, _C8, _R }, { _CP, _C8, _R }, { _CP, _C8, _R }, { _CP, _C8, _R },
	{ _RLC, _A, _R }, { _RRC, _A, _R }, { _RL, _A, _R }, { _RR, _A, _R },
	{ _SLA, _A, _R }, { _SRA, _A, _R }, { _SLL, _A, _R }, { _SRL, _A, _R }
};


static const tlcs900inst mnemonic_d0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _PUSHW, _M, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _LDW, _M16, _M }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M }, { _LD, _C16, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 }, { _EX, _M, _C16 },
	{ _ADD, _M, _I16 }, { _ADC, _M, _I16 }, { _SUB, _M, _I16 }, { _SBC, _M, _I16 },
	{ _AND, _M, _I16 }, { _XOR, _M, _I16 }, { _OR, _M, _I16 }, { _CP, _M, _I16 },

	/* 40 - 5F */
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M }, { _MUL, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M }, { _MULS, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M }, { _DIV, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },
	{ _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M }, { _DIVS, _C32, _M },

	/* 60 - 7F */
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M }, { _INCW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M }, { _DECW, _I3, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _RLCW, _M, 0 }, { _RRCW, _M, 0 }, { _RLW, _M, 0 }, { _RRW, _M, 0 },
	{ _SLAW, _M, 0 }, { _SRAW, _M, 0 }, { _SLLW, _M, 0 }, { _SRLW, _M, 0 },

	/* 80 - 9F */
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M }, { _ADD, _C16, _M },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 }, { _ADD, _M, _C16 },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M }, { _ADC, _C16, _M },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },
	{ _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 }, { _ADC, _M, _C16 },

	/* A0 - BF */
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M }, { _SUB, _C16, _M },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 }, { _SUB, _M, _C16 },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M }, { _SBC, _C16, _M },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },
	{ _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 }, { _SBC, _M, _C16 },

	/* C0 - DF */
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M }, { _AND, _C16, _M },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 }, { _AND, _M, _C16 },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M }, { _XOR, _C16, _M },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },
	{ _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 }, { _XOR, _M, _C16 },

	/* E0 - FF */
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M }, { _OR, _C16, _M },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 }, { _OR, _M, _C16 },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M }, { _CP, _C16, _M },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
	{ _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 }, { _CP, _M, _C16 },
};


static const tlcs900inst mnemonic_d8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _LD, _R, _I16 },
	{ _PUSH, _R, 0 }, { _POP, _R, 0 }, { _CPL, _R, 0 }, { _NEG, _R, 0 },
	{ _MUL, _R, _I16 }, { _MULS, _R, _I16 }, { _DIV, _R, _I16 }, { _DIVS, _R, _I16 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _BS1F, _A, _R }, { _BS1B, _A, _R },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _EXTZ, _R, 0 }, { _EXTS, _R, 0 },
	{ _PAA, _R, 0 }, { _DB, 0, 0 }, { _MIRR, _R, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _MULA, _R, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DJNZ, _R, _D8 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _ANDCF, _I8, _R }, { _ORCF, _I8, _R }, { _XORCF, _I8, _R }, { _LDCF, _I8, _R },
	{ _STCF, _I8, _R }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _ANDCF, _A, _R }, { _ORCF, _A, _R }, { _XORCF, _A, _R }, { _LDCF, _A, _R },
	{ _STCF, _A, _R }, { _DB, 0, 0 }, { _LDC, _CR16, _R }, { _LDC, _R, _CR16 },
	{ _RES, _I8, _R }, { _SET, _I8, _R }, { _CHG, _I8, _R }, { _BIT, _I8, _R },
	{ _TSET, _I8, _R }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _MINC1, _I16, _R }, { _MINC2, _I16, _R }, { _MINC4, _I16, _R }, { _DB, 0, 0 },
	{ _MDEC1, _I16, _R }, { _MDEC2, _I16, _R }, { _MDEC4, _I16, _R }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _MUL, _C32, _R }, { _MUL, _C32, _R }, { _MUL, _C32, _R }, { _MUL, _C32, _R },
	{ _MUL, _C32, _R }, { _MUL, _C32, _R }, { _MUL, _C32, _R }, { _MUL, _C32, _R },
	{ _MULS, _C32, _R }, { _MULS, _C32, _R }, { _MULS, _C32, _R }, { _MULS, _C32, _R },
	{ _MULS, _C32, _R }, { _MULS, _C32, _R }, { _MULS, _C32, _R }, { _MULS, _C32, _R },
	{ _DIV, _C32, _R }, { _DIV, _C32, _R }, { _DIV, _C32, _R }, { _DIV, _C32, _R },
	{ _DIV, _C32, _R }, { _DIV, _C32, _R }, { _DIV, _C32, _R }, { _DIV, _C32, _R },
	{ _DIVS, _C32, _R }, { _DIVS, _C32, _R }, { _DIVS, _C32, _R }, { _DIVS, _C32, _R },
	{ _DIVS, _C32, _R }, { _DIVS, _C32, _R }, { _DIVS, _C32, _R }, { _DIVS, _C32, _R },

	/* 60 - 7F */
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },
	{ _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R }, { _SCC, _CC, _R },

	/* 80 - 9F */
	{ _ADD, _C16, _R }, { _ADD, _C16, _R }, { _ADD, _C16, _R }, { _ADD, _C16, _R },
	{ _ADD, _C16, _R }, { _ADD, _C16, _R }, { _ADD, _C16, _R }, { _ADD, _C16, _R },
	{ _LD, _C16, _R }, { _LD, _C16, _R }, { _LD, _C16, _R }, { _LD, _C16, _R },
	{ _LD, _C16, _R }, { _LD, _C16, _R }, { _LD, _C16, _R }, { _LD, _C16, _R },
	{ _ADC, _C16, _R }, { _ADC, _C16, _R }, { _ADC, _C16, _R }, { _ADC, _C16, _R },
	{ _ADC, _C16, _R }, { _ADC, _C16, _R }, { _ADC, _C16, _R }, { _ADC, _C16, _R },
	{ _LD, _R, _C16 }, { _LD, _R, _C16 }, { _LD, _R, _C16 }, { _LD, _R, _C16 },
	{ _LD, _R, _C16 }, { _LD, _R, _C16 }, { _LD, _R, _C16 }, { _LD, _R, _C16 },

	/* A0 - BF */
	{ _SUB, _C16, _R }, { _SUB, _C16, _R }, { _SUB, _C16, _R }, { _SUB, _C16, _R },
	{ _SUB, _C16, _R }, { _SUB, _C16, _R }, { _SUB, _C16, _R }, { _SUB, _C16, _R },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _SBC, _C16, _R }, { _SBC, _C16, _R }, { _SBC, _C16, _R }, { _SBC, _C16, _R },
	{ _SBC, _C16, _R }, { _SBC, _C16, _R }, { _SBC, _C16, _R }, { _SBC, _C16, _R },
	{ _EX, _C16, _R }, { _EX, _C16, _R }, { _EX, _C16, _R }, { _EX, _C16, _R },
	{ _EX, _C16, _R }, { _EX, _C16, _R }, { _EX, _C16, _R }, { _EX, _C16, _R },

	/* C0 - DF */
	{ _AND, _C16, _R }, { _AND, _C16, _R }, { _AND, _C16, _R }, { _AND, _C16, _R },
	{ _AND, _C16, _R }, { _AND, _C16, _R }, { _AND, _C16, _R }, { _AND, _C16, _R },
	{ _ADD, _R, _I16 }, { _ADC, _R, _I16 }, { _SUB, _R, _I16 }, { _SBC, _R, _I16 },
	{ _AND, _R, _I16 }, { _XOR, _R, _I16 }, { _OR, _R, _I16 }, { _CP, _R, _I16 },
	{ _XOR, _C16, _R }, { _XOR, _C16, _R }, { _XOR, _C16, _R }, { _XOR, _C16, _R },
	{ _XOR, _C16, _R }, { _XOR, _C16, _R }, { _XOR, _C16, _R }, { _XOR, _C16, _R },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },

	/* E0 - FF */
	{ _OR, _C16, _R }, { _OR, _C16, _R }, { _OR, _C16, _R }, { _OR, _C16, _R },
	{ _OR, _C16, _R }, { _OR, _C16, _R }, { _OR, _C16, _R }, { _OR, _C16, _R },
	{ _RLC, _I8, _R }, { _RRC, _I8, _R }, { _RL, _I8, _R }, { _RR, _I8, _R },
	{ _SLA, _I8, _R }, { _SRA, _I8, _R }, { _SLL, _I8, _R }, { _SRL, _I8, _R },
	{ _CP, _C16, _R }, { _CP, _C16, _R }, { _CP, _C16, _R }, { _CP, _C16, _R },
	{ _CP, _C16, _R }, { _CP, _C16, _R }, { _CP, _C16, _R }, { _CP, _C16, _R },
	{ _RLC, _A, _R }, { _RRC, _A, _R }, { _RL, _A, _R }, { _RR, _A, _R },
	{ _SLA, _A, _R }, { _SRA, _A, _R }, { _SLL, _A, _R }, { _SRL, _A, _R }
};


static const tlcs900inst mnemonic_e0[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M },
	{ _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M }, { _LD, _C32, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M },
	{ _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M }, { _ADD, _C32, _M },
	{ _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 },
	{ _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 }, { _ADD, _M, _C32 },
	{ _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M },
	{ _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M }, { _ADC, _C32, _M },
	{ _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 },
	{ _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 }, { _ADC, _M, _C32 },

	/* A0 - BF */
	{ _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M },
	{ _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M }, { _SUB, _C32, _M },
	{ _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 },
	{ _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 }, { _SUB, _M, _C32 },
	{ _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M },
	{ _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M }, { _SBC, _C32, _M },
	{ _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 },
	{ _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 }, { _SBC, _M, _C32 },

	/* C0 - DF */
	{ _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M },
	{ _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M }, { _AND, _C32, _M },
	{ _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 },
	{ _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 }, { _AND, _M, _C32 },
	{ _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M },
	{ _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M }, { _XOR, _C32, _M },
	{ _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 },
	{ _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 }, { _XOR, _M, _C32 },

	/* E0 - FF */
	{ _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M },
	{ _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M }, { _OR, _C32, _M },
	{ _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 },
	{ _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 }, { _OR, _M, _C32 },
	{ _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M },
	{ _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M }, { _CP, _C32, _M },
	{ _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 },
	{ _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 }, { _CP, _M, _C32 },
};


static const tlcs900inst mnemonic_e8[256] =
{
	/* 00 - 1F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _LD, _R, _I32 },
	{ _PUSH, _R, 0 }, { _POP, _R, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LINK, _R, _I16 }, { _UNLK, _R, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _EXTZ, _R, 0 }, { _EXTS, _R, 0 },
	{ _PAA, _R, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _LDC, _CR32, _R }, { _LDC, _R, _CR32 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R }, { _INC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R }, { _DEC, _I3, _R },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ADD, _C32, _R }, { _ADD, _C32, _R }, { _ADD, _C32, _R }, { _ADD, _C32, _R },
	{ _ADD, _C32, _R }, { _ADD, _C32, _R }, { _ADD, _C32, _R }, { _ADD, _C32, _R },
	{ _LD, _C32, _R }, { _LD, _C32, _R }, { _LD, _C32, _R }, { _LD, _C32, _R },
	{ _LD, _C32, _R }, { _LD, _C32, _R }, { _LD, _C32, _R }, { _LD, _C32, _R },
	{ _ADC, _C32, _R }, { _ADC, _C32, _R }, { _ADC, _C32, _R }, { _ADC, _C32, _R },
	{ _ADC, _C32, _R }, { _ADC, _C32, _R }, { _ADC, _C32, _R }, { _ADC, _C32, _R },
	{ _LD, _R, _C32 }, { _LD, _R, _C32 }, { _LD, _R, _C32 }, { _LD, _R, _C32 },
	{ _LD, _R, _C32 }, { _LD, _R, _C32 }, { _LD, _R, _C32 }, { _LD, _R, _C32 },

	/* A0 - BF */
	{ _SUB, _C32, _R }, { _SUB, _C32, _R }, { _SUB, _C32, _R }, { _SUB, _C32, _R },
	{ _SUB, _C32, _R }, { _SUB, _C32, _R }, { _SUB, _C32, _R }, { _SUB, _C32, _R },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 }, { _LD, _R, _I3 },
	{ _SBC, _C32, _R }, { _SBC, _C32, _R }, { _SBC, _C32, _R }, { _SBC, _C32, _R },
	{ _SBC, _C32, _R }, { _SBC, _C32, _R }, { _SBC, _C32, _R }, { _SBC, _C32, _R },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* C0 - DF */
	{ _AND, _C32, _R }, { _AND, _C32, _R }, { _AND, _C32, _R }, { _AND, _C32, _R },
	{ _AND, _C32, _R }, { _AND, _C32, _R }, { _AND, _C32, _R }, { _AND, _C32, _R },
	{ _ADD, _R, _I32 }, { _ADC, _R, _I32 }, { _SUB, _R, _I32 }, { _SBC, _R, _I32 },
	{ _AND, _R, _I32 }, { _XOR, _R, _I32 }, { _OR, _R, _I32 }, { _CP, _R, _I32 },
	{ _XOR, _C32, _R }, { _XOR, _C32, _R }, { _XOR, _C32, _R }, { _XOR, _C32, _R },
	{ _XOR, _C32, _R }, { _XOR, _C32, _R }, { _XOR, _C32, _R }, { _XOR, _C32, _R },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },
	{ _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 }, { _CP, _R, _I3 },

	/* E0 - FF */
	{ _OR, _C32, _R }, { _OR, _C32, _R }, { _OR, _C32, _R }, { _OR, _C32, _R },
	{ _OR, _C32, _R }, { _OR, _C32, _R }, { _OR, _C32, _R }, { _OR, _C32, _R },
	{ _RLC, _I8, _R }, { _RRC, _I8, _R }, { _RL, _I8, _R }, { _RR, _I8, _R },
	{ _SLA, _I8, _R }, { _SRA, _I8, _R }, { _SLL, _I8, _R }, { _SRL, _I8, _R },
	{ _CP, _C32, _R }, { _CP, _C32, _R }, { _CP, _C32, _R }, { _CP, _C32, _R },
	{ _CP, _C32, _R }, { _CP, _C32, _R }, { _CP, _C32, _R }, { _CP, _C32, _R },
	{ _RLC, _A, _R }, { _RRC, _A, _R }, { _RL, _A, _R }, { _RR, _A, _R },
	{ _SLA, _A, _R }, { _SRA, _A, _R }, { _SLL, _A, _R }, { _SRL, _A, _R }
};


static const tlcs900inst mnemonic_f0[256] =
{
	/* 00 - 1F */
	{ _LD, _M, _I8 }, { _DB, 0, 0 }, { _LD, _M, _I16 }, { _DB, 0, 0 },
	{ _POP, _M, 0 }, { _DB, 0, 0 }, { _POPW, _M, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _M16 }, { _DB, 0, 0 }, { _LDW, _M, _M16 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M }, { _LDA, _C16, _M },
	{ _ANDCF, _A, _M }, { _ORCF, _A, _M }, { _XORCF, _A, _M }, { _LDCF, _A, _M },
	{ _STCF, _A, _M }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M }, { _LDA, _C32, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 40 - 5F */
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 }, { _LD, _M, _C8 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 }, { _LD, _M, _C16 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 60 - 7F */
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 }, { _LD, _M, _C32 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },

	/* 80 - 9F */
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M }, { _ANDCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M }, { _ORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M }, { _XORCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },
	{ _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M }, { _LDCF, _I3, _M },

	/* A0 - BF */
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M }, { _STCF, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M }, { _TSET, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M }, { _RES, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },
	{ _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M }, { _SET, _I3, _M },

	/* C0 - DF */
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M }, { _CHG, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M }, { _BIT, _I3, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },
	{ _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M }, { _JP, _CC, _M },

	/* E0 - FF */
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M }, { _CALL, _CC, _M },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }
};


static const tlcs900inst mnemonic[256] =
{
	/* 00 - 1F */
	{ _NOP, 0, 0 }, { _NORMAL, 0, 0 }, { _PUSH, _SR, 0 }, { _POP, _SR, 0 },
	{ _MAX, 0, 0 }, { _HALT, 0, 0 }, { _EI, _I8, 0 }, { _RETI, 0, 0 },
	{ _LD, _M8, _I8 }, { _PUSH, _I8, 0 }, { _LD, _M8, _I16 }, { _PUSH, _I16, 0 },
	{ _INCF, 0, 0 }, { _DECF, 0, 0 }, { _RET, 0, 0 }, { _RETD, _I16, 0 },
	{ _RCF, 0, 0 }, { _SCF, 0, 0 }, { _CCF, 0, 0 }, { _ZCF, 0, 0 },
	{ _PUSH, _A, 0 }, { _POP, _A, 0 }, { _EX, _F, _F }, { _LDF, _I8, 0 },
	{ _PUSH, _F, 0 }, { _POP, _F, 0 }, { _JP, _I16, 0 }, { _JP, _I24, 0 },
	{ _CALL, _I16, 0 }, { _CALL, _I24, 0 }, { _CALR, _D16, 0 }, { _DB, 0, 0 },

	/* 20 - 3F */
	{ _LD, _C8, _I8 }, { _LD, _C8, _I8 }, { _LD, _C8, _I8 }, { _LD, _C8, _I8 },
	{ _LD, _C8, _I8 }, { _LD, _C8, _I8 }, { _LD, _C8, _I8 }, { _LD, _C8, _I8 },
	{ _PUSH, _C16, 0 }, { _PUSH, _C16, 0 }, { _PUSH, _C16, 0 }, { _PUSH, _C16, 0 },
	{ _PUSH, _C16, 0 }, { _PUSH, _C16, 0 }, { _PUSH, _C16, 0 }, { _PUSH, _C16, 0 },
	{ _LD, _C16, _I16 }, { _LD, _C16, _I16 }, { _LD, _C16, _I16 }, { _LD, _C16, _I16 },
	{ _LD, _C16, _I16 }, { _LD, _C16, _I16 }, { _LD, _C16, _I16 }, { _LD, _C16, _I16 },
	{ _PUSH, _C32, 0 }, { _PUSH, _C32, 0 }, { _PUSH, _C32, 0 }, { _PUSH, _C32, 0 },
	{ _PUSH, _C32, 0 }, { _PUSH, _C32, 0 }, { _PUSH, _C32, 0 }, { _PUSH, _C32, 0 },

	/* 40 - 5F */
	{ _LD, _C32, _I32 }, { _LD, _C32, _I32 }, { _LD, _C32, _I32 }, { _LD, _C32, _I32 },
	{ _LD, _C32, _I32 }, { _LD, _C32, _I32 }, { _LD, _C32, _I32 }, { _LD, _C32, _I32 },
	{ _POP, _C16, 0 }, { _POP, _C16, 0 }, { _POP, _C16, 0 }, { _POP, _C16, 0 },
	{ _POP, _C16, 0 }, { _POP, _C16, 0 }, { _POP, _C16, 0 }, { _POP, _C16, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 }, { _DB, 0, 0 },
	{ _POP, _C32, 0 }, { _POP, _C32, 0 }, { _POP, _C32, 0 }, { _POP, _C32, 0 },
	{ _POP, _C32, 0 }, { _POP, _C32, 0 }, { _POP, _C32, 0 }, { _POP, _C32, 0 },

	/* 60 - 7F */
	{ _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 },
	{ _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 },
	{ _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 },
	{ _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 }, { _JR, _CC, _D8 },
	{ _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 },
	{ _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 },
	{ _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 },
	{ _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 }, { _JRL, _CC, _D16 },

	/* 80 - 9F */
	{ _80, 0, 0 }, { _80, 0, 0 }, { _80, 0, 0 }, { _80, 0, 0 },
	{ _80, 0, 0 }, { _80, 0, 0 }, { _80, 0, 0 }, { _80, 0, 0 },
	{ _88, 0, 0 }, { _88, 0, 0 }, { _88, 0, 0 }, { _88, 0, 0 },
	{ _88, 0, 0 }, { _88, 0, 0 }, { _88, 0, 0 }, { _88, 0, 0 },
	{ _90, 0, 0 }, { _90, 0, 0 }, { _90, 0, 0 }, { _90, 0, 0 },
	{ _90, 0, 0 }, { _90, 0, 0 }, { _90, 0, 0 }, { _90, 0, 0 },
	{ _98, 0, 0 }, { _98, 0, 0 }, { _98, 0, 0 }, { _98, 0, 0 },
	{ _98, 0, 0 }, { _98, 0, 0 }, { _98, 0, 0 }, { _98, 0, 0 },

	/* A0 - BF */
	{ _A0, 0, 0 }, { _A0, 0, 0 }, { _A0, 0, 0 }, { _A0, 0, 0 },
	{ _A0, 0, 0 }, { _A0, 0, 0 }, { _A0, 0, 0 }, { _A0, 0, 0 },
	{ _A8, 0, 0 }, { _A8, 0, 0 }, { _A8, 0, 0 }, { _A8, 0, 0 },
	{ _A8, 0, 0 }, { _A8, 0, 0 }, { _A8, 0, 0 }, { _A8, 0, 0 },
	{ _B0, 0, 0 }, { _B0, 0, 0 }, { _B0, 0, 0 }, { _B0, 0, 0 },
	{ _B0, 0, 0 }, { _B0, 0, 0 }, { _B0, 0, 0 }, { _B0, 0, 0 },
	{ _B8, 0, 0 }, { _B8, 0, 0 }, { _B8, 0, 0 }, { _B8, 0, 0 },
	{ _B8, 0, 0 }, { _B8, 0, 0 }, { _B8, 0, 0 }, { _B8, 0, 0 },

	/* C0 - DF */
	{ _C0, 0, 0 }, { _C0, 0, 0 }, { _C0, 0, 0 }, { _C0, 0, 0 },
	{ _C0, 0, 0 }, { _C0, 0, 0 }, { _DB, 0, 0 }, { oC8, 0, 0 },
	{ oC8, 0, 0 }, { oC8, 0, 0 }, { oC8, 0, 0 }, { oC8, 0, 0 },
	{ oC8, 0, 0 }, { oC8, 0, 0 }, { oC8, 0, 0 }, { oC8, 0, 0 },
	{ _D0, 0, 0 }, { _D0, 0, 0 }, { _D0, 0, 0 }, { _D0, 0, 0 },
	{ _D0, 0, 0 }, { _D0, 0, 0 }, { _DB, 0, 0 }, { oD8, 0, 0 },
	{ oD8, 0, 0 }, { oD8, 0, 0 }, { oD8, 0, 0 }, { oD8, 0, 0 },
	{ oD8, 0, 0 }, { oD8, 0, 0 }, { oD8, 0, 0 }, { oD8, 0, 0 },

	/* E0 - FF */
	{ _E0, 0, 0 }, { _E0, 0, 0 }, { _E0, 0, 0 }, { _E0, 0, 0 },
	{ _E0, 0, 0 }, { _E0, 0, 0 }, { _DB, 0, 0 }, { _E8, 0, 0 },
	{ _E8, 0, 0 }, { _E8, 0, 0 }, { _E8, 0, 0 }, { _E8, 0, 0 },
	{ _E8, 0, 0 }, { _E8, 0, 0 }, { _E8, 0, 0 }, { _E8, 0, 0 },
	{ _F0, 0, 0 }, { _F0, 0, 0 }, { _F0, 0, 0 }, { _F0, 0, 0 },
	{ _F0, 0, 0 }, { _F0, 0, 0 }, { _DB, 0, 0 }, { _LDX, 0, 0 },
	{ _SWI, _I3, 0 }, { _SWI, _I3, 0 }, { _SWI, _I3, 0 }, { _SWI, _I3, 0 },
	{ _SWI, _I3, 0 }, { _SWI, _I3, 0 }, { _SWI, _I3, 0 }, { _SWI, _I3, 0 }
};



static const char *const s_reg8[8] = { "W", "A", "B", "C", "D", "E", "H", "L" };
static const char *const s_reg16[8] = { "WA", "BC", "DE", "HL", "IX", "IY", "IZ", "SP" };
static const char *const s_reg32[8] = { "XWA", "XBC", "XDE", "XHL", "XIX", "XIY", "XIZ", "XSP" };
static const char *const s_mulreg16[8] = { "??", "WA", "??", "BC", "??", "DE", "??", "HL" };
static const char *const s_allreg8[256] =
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


static const char *const s_allreg16[256] =
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


static const char *const s_allreg32[256] =
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


static const char *const s_cond[16] =
{
	"F","LT","LE","ULE","PE/OV","M/MI","Z","C","T","GE","GT","UGT","PO/NOV","P/PL","NZ","NC"
};


CPU_DISASSEMBLE( tlcs900 )
{
	const tlcs900inst *dasm;
	char *dst = buffer;
	char buf[32];
	UINT8	op, op1;
	UINT32	imm;
	int		flags = 0;
	int		pos = 0;

	op = oprom[ pos++ ];

	dasm = &mnemonic[ op ];

	/* Check for extended addressing modes */
	switch( dasm->mnemonic )
	{
	case _80:
		sprintf( buf, "%s", s_reg32[op & 0x07] );
		op = oprom[ pos++ ];
		dasm = &mnemonic_80[ op ];
		break;

	case _88:
		imm = oprom[ pos++ ];
		sprintf( buf, "%s+0x%02x", s_reg32[op & 0x07], imm );
		op = oprom[ pos++ ];
		dasm = &mnemonic_88[ op ];
		break;

	case _90:
		sprintf( buf, "%s", s_reg32[op & 0x07] );
		op = oprom[ pos++ ];
		dasm = &mnemonic_90[ op ];
		break;

	case _98:
		imm = oprom[ pos++ ];
		sprintf( buf, "%s+0x%02x", s_reg32[op & 0x07], imm );
		op = oprom[ pos++ ];
		dasm = &mnemonic_98[ op ];
		break;

	case _A0:
		sprintf( buf, "%s", s_reg32[op & 0x07] );
		op = oprom[ pos++ ];
		dasm = &mnemonic_a0[ op ];
		break;

	case _A8:
		imm = oprom[ pos++ ];
		sprintf( buf, "%s+0x%02x", s_reg32[op & 0x07], imm );
		op = oprom[ pos++ ];
		dasm = &mnemonic_a0[ op ];
		break;

	case _B0:
		sprintf( buf, "%s", s_reg32[op & 0x07] );
		op = oprom[ pos++ ];
		dasm = &mnemonic_b0[ op ];
		break;

	case _B8:
		imm = oprom[ pos++ ];
		sprintf( buf, "%s+0x%02x", s_reg32[op & 0x07], imm );
		op = oprom[ pos++ ];
		dasm = &mnemonic_b8[ op ];
		break;

	case _C0:
		switch( op & 0x07 )
		{
		case 0x00:	/* 0xC0 */
			imm = oprom[ pos++ ];
			sprintf( buf, "0x%02x", imm );
			break;

		case 0x01:	/* 0xC1 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			sprintf( buf, "0x%04x", imm );
			break;

		case 0x02:	/* 0xC2 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			imm = imm | ( oprom[ pos++ ] << 16 );
			sprintf( buf, "0x%06x", imm );
			break;

		case 0x03:	/* 0xC3 */
			imm = oprom[ pos++ ];
			switch( imm & 0x03 )
			{
			case 0x00:
				sprintf( buf, "%s", s_allreg32[imm] );
				break;

			case 0x01:
				op = imm;
				imm = oprom[ pos++ ];
				imm = imm | ( oprom[ pos++ ] << 8 );
				sprintf( buf, "%s+0x%04x", s_allreg32[op], imm );
				break;

			case 0x02:
				sprintf( buf, "unknown" );
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg8[op1] );
					break;

				case 0x07:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg16[op1] );
					break;

				case 0x13:
					imm = oprom[ pos++ ];
					imm = imm | ( oprom[ pos++ ] << 8 );
					sprintf( buf, "0x%06x", pc + pos + (INT16)imm );
					break;
				}
				break;
			}
			break;

		case 0x04:	/* 0xC4 */
			imm = oprom[ pos++ ];
			sprintf( buf, "-%s", s_allreg32[imm] );
			break;

		case 0x05:	/* 0xC5 */
			imm = oprom[ pos++ ];
			sprintf( buf, "%s+", s_allreg32[imm] );
			break;
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_c0[ op ];
		break;

	case oC8:
		if ( op & 0x08 )
		{
			sprintf( buf, "%s", s_reg8[ op & 0x07 ] );
		}
		else
		{
			imm = oprom[ pos++ ];
			sprintf( buf, "%s", s_allreg8[imm] );
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_c8[ op ];
		break;

	case _D0:
		switch( op & 0x07 )
		{
		case 0x00:	/* 0xD0 */
			imm = oprom[ pos++ ];
			sprintf( buf, "0x%02x", imm );
			break;

		case 0x01:	/* 0xD1 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			sprintf( buf, "0x%04x", imm );
			break;

		case 0x02:	/* 0xD2 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			imm = imm | ( oprom[ pos++ ] << 16 );
			sprintf( buf, "0x%06x", imm );
			break;

		case 0x03:	/* 0xD3 */
			imm = oprom[ pos++ ];
			switch( imm & 0x03 )
			{
			case 0x00:
				sprintf( buf, "%s", s_allreg32[imm] );
				break;

			case 0x01:
				op = imm;
				imm = oprom[ pos++ ];
				imm = imm | ( oprom[ pos++ ] << 8 );
				sprintf( buf, "%s+0x%04x", s_allreg32[op], imm );
				break;

			case 0x02:
				sprintf( buf, "unknown" );
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg8[op1] );
					break;

				case 0x07:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg16[op1] );
					break;

				case 0x13:
					imm = oprom[ pos++ ];
					imm = imm | ( oprom[ pos++ ] << 8 );
					sprintf( buf, "0x%06x", pc + pos + (INT16)imm );
					break;
				}
				break;
			}
			break;

		case 0x04:	/* 0xD4 */
			imm = oprom[ pos++ ];
			sprintf( buf, "-%s", s_allreg32[imm] );
			break;

		case 0x05:	/* 0xD5 */
			imm = oprom[ pos++ ];
			sprintf( buf, "%s+", s_allreg32[imm] );
			break;
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_d0[ op ];
		break;

	case oD8:
		if ( op & 0x08 )
		{
			sprintf( buf, "%s", s_reg16[ op & 0x07 ] );
		}
		else
		{
			imm = oprom[ pos++ ];
			sprintf( buf, "%s", s_allreg16[imm] );
		}

		op = oprom[ pos++ ];
		dasm = &mnemonic_d8[ op ];
		break;

	case _E0:
		switch( op & 0x07 )
		{
		case 0x00:	/* 0xE0 */
			imm = oprom[ pos++ ];
			sprintf( buf, "0x%02x", imm );
			break;

		case 0x01:	/* 0xE1 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			sprintf( buf, "0x%04x", imm );
			break;

		case 0x02:	/* 0xE2 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			imm = imm | ( oprom[ pos++ ] << 16 );
			sprintf( buf, "0x%06x", imm );
			break;

		case 0x03:	/* 0xE3 */
			imm = oprom[ pos++ ];
			switch( imm & 0x03 )
			{
			case 0x00:
				sprintf( buf, "%s", s_allreg32[imm] );
				break;

			case 0x01:
				op = imm;
				imm = oprom[ pos++ ];
				imm = imm | ( oprom[ pos++ ] << 8 );
				sprintf( buf, "%s+0x%04x", s_allreg32[op], imm );
				break;

			case 0x02:
				sprintf( buf, "unknown" );
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg8[op1] );
					break;

				case 0x07:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg16[op1] );
					break;

				case 0x13:
					imm = oprom[ pos++ ];
					imm = imm | ( oprom[ pos++ ] << 8 );
					sprintf( buf, "0x%06x", pc + pos + (INT16)imm );
					break;
				}
				break;
			}
			break;

		case 0x04:	/* 0xE4 */
			imm = oprom[ pos++ ];
			sprintf( buf, "-%s", s_allreg32[imm] );
			break;

		case 0x05:	/* 0xE5 */
			imm = oprom[ pos++ ];
			sprintf( buf, "%s+", s_allreg32[imm] );
			break;
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_e0[ op ];
		break;

	case _E8:
		if ( op & 0x08 )
		{
			sprintf( buf, "%s", s_reg32[ op & 0x07 ] );
		}
		else
		{
			imm = oprom[ pos++ ];
			sprintf( buf, "%s", s_allreg32[imm] );
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_e8[ op ];
		break;

	case _F0:
		switch( op & 0x07 )
		{
		case 0x00:	/* 0xF0 */
			imm = oprom[ pos++ ];
			sprintf( buf, "0x%02x", imm );
			break;

		case 0x01:	/* 0xF1 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			sprintf( buf, "0x%04x", imm );
			break;

		case 0x02:	/* 0xF2 */
			imm = oprom[ pos++ ];
			imm = imm | ( oprom[ pos++ ] << 8 );
			imm = imm | ( oprom[ pos++ ] << 16 );
			sprintf( buf, "0x%06x", imm );
			break;

		case 0x03:	/* 0xF3 */
			imm = oprom[ pos++ ];
			switch( imm & 0x03 )
			{
			case 0x00:
				sprintf( buf, "%s", s_allreg32[imm] );
				break;

			case 0x01:
				op = imm;
				imm = oprom[ pos++ ];
				imm = imm | ( oprom[ pos++ ] << 8 );
				sprintf( buf, "%s+0x%04x", s_allreg32[op], imm );
				break;

			case 0x02:
				sprintf( buf, "unknown" );
				break;

			case 0x03:
				switch( imm )
				{
				case 0x03:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg8[op1] );
					break;

				case 0x07:
					op = oprom[ pos++ ];
					op1 = oprom[ pos++ ];
					sprintf( buf, "%s+%s", s_allreg32[op], s_allreg16[op1] );
					break;

				case 0x13:
					imm = oprom[ pos++ ];
					imm = imm | ( oprom[ pos++ ] << 8 );
					sprintf( buf, "0x%06x", pc + pos + (INT16)imm );
					break;
				}
				break;
			}
			break;

		case 0x04:	/* 0xF4 */
			imm = oprom[ pos++ ];
			sprintf( buf, "-%s", s_allreg32[imm] );
			break;

		case 0x05:	/* 0xF5 */
			imm = oprom[ pos++ ];
			sprintf( buf, "%s+", s_allreg32[imm] );
			break;
		}
		op = oprom[ pos++ ];
		dasm = &mnemonic_f0[ op ];
		break;
	}

	dst += sprintf( dst, "%s", s_mnemonic[ dasm->mnemonic ] );

	switch( dasm->mnemonic )
	{
	case _CALL:
	case _CALR:
		flags = DASMFLAG_STEP_OVER;
		break;
	case _RET:
	case _RETD:
	case _RETI:
		flags = DASMFLAG_STEP_OUT;
		break;
	}

	switch( dasm->operand1 )
	{
	case _A:
		dst += sprintf( dst, " A" );
		break;

	case _C8:
		dst += sprintf( dst, " %s", s_reg8[op & 0x07] );
		break;

	case _C16:
		dst += sprintf( dst, " %s", s_reg16[op & 0x07] );
		break;

	case _C32:
		dst += sprintf( dst, " %s", s_reg32[op & 0x07] );
		break;

	case _MC16:
		dst += sprintf( dst, " %s", s_mulreg16[op & 0x07] );
		break;

	case _CC:
		dst += sprintf( dst, " %s", s_cond[op & 0x0F] );
		break;

	case _CR8:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x22:
			dst += sprintf( dst, " DMAM0" );
			break;
		case 0x26:
			dst += sprintf( dst, " DMAM1" );
			break;
		case 0x2a:
			dst += sprintf( dst, " DMAM2" );
			break;
		case 0x2e:
			dst += sprintf( dst, " DMAM3" );
			break;
		default:
			dst += sprintf( dst, " unknown" );
			break;
		}
		break;

	case _CR16:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x20:
			dst += sprintf( dst, " DMAC0" );
			break;
		case 0x24:
			dst += sprintf( dst, " DMAC1" );
			break;
		case 0x28:
			dst += sprintf( dst, " DMAC2" );
			break;
		case 0x2c:
			dst += sprintf( dst, " DMAC3" );
			break;
		default:
			dst += sprintf( dst, " unknown" );
			break;
		}
		break;

	case _CR32:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x00:
			dst += sprintf( dst, " DMAS0" );
			break;
		case 0x04:
			dst += sprintf( dst, " DMAS1" );
			break;
		case 0x08:
			dst += sprintf( dst, " DMAS2" );
			break;
		case 0x0c:
			dst += sprintf( dst, " DMAS3" );
			break;
		case 0x10:
			dst += sprintf( dst, " DMAD0" );
			break;
		case 0x14:
			dst += sprintf( dst, " DMAD1" );
			break;
		case 0x18:
			dst += sprintf( dst, " DMAD2" );
			break;
		case 0x1c:
			dst += sprintf( dst, " DMAD3" );
			break;
		default:
			dst += sprintf( dst, " unknown" );
			break;
		}
		break;

	case _D8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, " 0x%06x", ( pc + pos + (INT8)imm ) & 0xFFFFFF );
		break;

	case _D16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, " 0x%06x", ( pc + pos + (INT16)imm ) & 0xFFFFFF );
		break;

	case _F:
		dst += sprintf( dst, " F" );
		break;

	case _I3:
		dst += sprintf( dst, " %d", op & 0x07 );
		break;

	case _I8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, " 0x%02x", imm );
		break;

	case _I16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, " 0x%04x", imm );
		break;

	case _I24:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		imm = imm | ( oprom[ pos++ ] << 16 );
		dst += sprintf( dst, " 0x%06x", imm );
		break;

	case _I32:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		imm = imm | ( oprom[ pos++ ] << 16 );
		imm = imm | ( oprom[ pos++ ] << 24 );
		dst += sprintf( dst, "0x%08x", imm );
		break;

	case _M:
		switch( dasm->mnemonic )
		{
		case _CALL:
		case _JP:
		case _LDA:
			dst += sprintf( dst, " %s", buf );
			break;
		default:
			dst += sprintf( dst, " (%s)", buf );
			break;
		}
		break;

	case _M8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, " (0x%02x)", imm );
		break;

	case _M16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, " (0x%04x)", imm );
		break;

	case _R:
		dst += sprintf( dst, " %s", buf );
		break;

	case _SR:
		dst += sprintf( dst, " SR" );
		break;
	}

	switch( dasm->operand2 )
	{
	case _A:
		dst += sprintf( dst, ",A" );
		break;

	case _C8:
		dst += sprintf( dst, ",%s", s_reg8[op & 0x07] );
		break;

	case _C16:
		dst += sprintf( dst, ",%s", s_reg16[op & 0x07] );
		break;

	case _C32:
		dst += sprintf( dst, ",%s", s_reg32[op & 0x07] );
		break;

	case _MC16:
		dst += sprintf( dst, ",%s", s_mulreg16[op & 0x07] );
		break;

	case _CC:
		dst += sprintf( dst, ",%s", s_cond[op & 0x0F] );
		break;

	case _CR8:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x22:
			dst += sprintf( dst, ",DMAM0" );
			break;
		case 0x26:
			dst += sprintf( dst, ",DMAM1" );
			break;
		case 0x2a:
			dst += sprintf( dst, ",DMAM2" );
			break;
		case 0x2e:
			dst += sprintf( dst, ",DMAM3" );
			break;
		default:
			dst += sprintf( dst, ",unknown" );
			break;
		}
		break;

	case _CR16:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x20:
			dst += sprintf( dst, ",DMAC0" );
			break;
		case 0x24:
			dst += sprintf( dst, ",DMAC1" );
			break;
		case 0x28:
			dst += sprintf( dst, ",DMAC2" );
			break;
		case 0x2c:
			dst += sprintf( dst, ",DMAC3" );
			break;
		default:
			dst += sprintf( dst, ",unknown" );
			break;
		}
		break;

	case _CR32:
		imm = oprom[ pos++ ];
		switch( imm )
		{
		case 0x00:
			dst += sprintf( dst, ",DMAS0" );
			break;
		case 0x04:
			dst += sprintf( dst, ",DMAS1" );
			break;
		case 0x08:
			dst += sprintf( dst, ",DMAS2" );
			break;
		case 0x0c:
			dst += sprintf( dst, ",DMAS3" );
			break;
		case 0x10:
			dst += sprintf( dst, ",DMAD0" );
			break;
		case 0x14:
			dst += sprintf( dst, ",DMAD1" );
			break;
		case 0x18:
			dst += sprintf( dst, ",DMAD2" );
			break;
		case 0x1c:
			dst += sprintf( dst, ",DMAD3" );
			break;
		default:
			dst += sprintf( dst, ",unknown" );
			break;
		}
		break;

	case _D8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, ",0x%06x", ( pc + pos + (INT8)imm ) & 0xFFFFFF );
		break;

	case _D16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, ",0x%06x", ( pc + pos + (INT16)imm ) & 0xFFFFFF );
		break;

	case _F:
		dst += sprintf( dst, ",F'" );
		break;

	case _I3:
		dst += sprintf( dst, ",%d", op & 0x07 );
		break;

	case _I8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, ",0x%02x", imm );
		break;

	case _I16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, ",0x%04x", imm );
		break;

	case _I24:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		imm = imm | ( oprom[ pos++ ] << 16 );
		dst += sprintf( dst, ",0x%06x", imm );
		break;

	case _I32:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		imm = imm | ( oprom[ pos++ ] << 16 );
		imm = imm | ( oprom[ pos++ ] << 24 );
		dst += sprintf( dst, ",0x%08x", imm );
		break;

	case _M:
		switch( dasm->mnemonic )
		{
		case _CALL:
		case _JP:
		case _LDA:
			dst += sprintf( dst, ",%s", buf );
			break;
		default:
			dst += sprintf( dst, ",(%s)", buf );
			break;
		}
		break;

	case _M8:
		imm = oprom[ pos++ ];
		dst += sprintf( dst, ",(0x%02x)", imm );
		break;

	case _M16:
		imm = oprom[ pos++ ];
		imm = imm | ( oprom[ pos++ ] << 8 );
		dst += sprintf( dst, ",(0x%04x)", imm );
		break;

	case _R:
		dst += sprintf( dst, ",%s", buf );
		break;

	case _SR:
		dst += sprintf( dst, ",SR" );
		break;
	}

	return pos | flags | DASMFLAG_SUPPORTED;
}

