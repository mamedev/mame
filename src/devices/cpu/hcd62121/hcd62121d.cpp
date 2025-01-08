// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "hcd62121d.h"

const hcd62121_disassembler::dasm hcd62121_disassembler::ops[256] =
{
	/* 0x00 */
	{ "sh?b",    ARG_REG,    ARG_S8 },   { "sh?w",    ARG_REG,    ARG_S8   },
	{ "sh?q",    ARG_REG,    ARG_S8 },   { "sh?t",    ARG_REG,    ARG_S8   },
	{ "mskb",    ARG_REGREG, ARG_NONE }, { "mskw",    ARG_REGREG, ARG_NONE },
	{ "mskq",    ARG_REGREG, ARG_NONE }, { "mskt",    ARG_REGREG, ARG_NONE },
	{ "sh?b",    ARG_REG,    ARG_S4 },   { "sh?w",    ARG_REG,    ARG_S4   },
	{ "sh?q",    ARG_REG,    ARG_S4 },   { "sh?t",    ARG_REG,    ARG_S4   },
	{ "tstb",    ARG_REGREG, ARG_NONE }, { "tstw",    ARG_REGREG, ARG_NONE },
	{ "tstq",    ARG_REGREG, ARG_NONE }, { "tstt",    ARG_REGREG, ARG_NONE },

	/* 0x10 */
	{ "xorb",    ARG_REGREG, ARG_NONE }, { "xorw",    ARG_REGREG, ARG_NONE },
	{ "xorq",    ARG_REGREG, ARG_NONE }, { "xort",    ARG_REGREG, ARG_NONE },
	{ "cmpb",    ARG_REGREG, ARG_NONE }, { "cmpw",    ARG_REGREG, ARG_NONE },
	{ "cmpq",    ARG_REGREG, ARG_NONE }, { "cmpt",    ARG_REGREG, ARG_NONE },
	{ "movb",    ARG_REGREG, ARG_NONE }, { "movw",    ARG_REGREG, ARG_NONE },
	{ "movq",    ARG_REGREG, ARG_NONE }, { "movt",    ARG_REGREG, ARG_NONE },
	{ "cmpaddb", ARG_REGREG, ARG_NONE }, { "cmpaddw", ARG_REGREG, ARG_NONE },
	{ "cmpaddq", ARG_REGREG, ARG_NONE }, { "cmpaddt", ARG_REGREG, ARG_NONE },

	/* 0x20 */
	{ "shrb",    ARG_REG,    ARG_S1   }, { "shrw",    ARG_REG,    ARG_S1   },
	{ "shrq",    ARG_REG,    ARG_S1   }, { "shrt",    ARG_REG,    ARG_S1   },
	{ "orb",     ARG_REGREG, ARG_NONE }, { "orw",     ARG_REGREG, ARG_NONE },
	{ "orq",     ARG_REGREG, ARG_NONE }, { "ort",     ARG_REGREG, ARG_NONE },
	{ "shlb",    ARG_REG,    ARG_S1   }, { "shlw",    ARG_REG,    ARG_S1   },
	{ "shlq",    ARG_REG,    ARG_S1   }, { "shlt",    ARG_REG,    ARG_S1   },
	{ "andb",    ARG_REGREG, ARG_NONE }, { "andw",    ARG_REGREG, ARG_NONE },
	{ "andq",    ARG_REGREG, ARG_NONE }, { "andt",    ARG_REGREG, ARG_NONE },

	/* 0x30 */
	{ "sbbb",    ARG_REGREG, ARG_NONE }, { "sbbw",    ARG_REGREG, ARG_NONE }, /* BCD SUB */
	{ "sbbq",    ARG_REGREG, ARG_NONE }, { "sbbt",    ARG_REGREG, ARG_NONE }, /* BCD SUB */
	{ "subb",    ARG_REGREG, ARG_NONE }, { "subw",    ARG_REGREG, ARG_NONE },
	{ "subq",    ARG_REGREG, ARG_NONE }, { "subt",    ARG_REGREG, ARG_NONE },
	{ "adbb",    ARG_REGREG, ARG_NONE }, { "adbw",    ARG_REGREG, ARG_NONE }, /* BCD ADD */
	{ "adbq",    ARG_REGREG, ARG_NONE }, { "adbt",    ARG_REGREG, ARG_NONE }, /* BCD ADD */
	{ "addb",    ARG_REGREG, ARG_NONE }, { "addw",    ARG_REGREG, ARG_NONE },
	{ "addq",    ARG_REGREG, ARG_NONE }, { "addt",    ARG_REGREG, ARG_NONE },

	/* 0x40 */
	{ "sh?b",    ARG_IRG,    ARG_S8   }, { "sh?w",    ARG_IRG,    ARG_S8   },
	{ "sh?q",    ARG_IRG,    ARG_S8   }, { "sh?t",    ARG_IRG,    ARG_S8   },
	{ "mskb",    ARG_IRGREG, ARG_NONE }, { "mskw",    ARG_IRGREG, ARG_NONE },
	{ "mskq",    ARG_IRGREG, ARG_NONE }, { "mskt",    ARG_IRGREG, ARG_NONE },
	{ "sh?b",    ARG_IRG,    ARG_S4   }, { "sh?w",    ARG_IRG,    ARG_S4   },
	{ "sh?q",    ARG_IRG,    ARG_S4   }, { "sh?t",    ARG_IRG,    ARG_S4   },
	{ "tstb",    ARG_IRGREG, ARG_NONE }, { "tstw",    ARG_IRGREG, ARG_NONE },
	{ "tstq",    ARG_IRGREG, ARG_NONE }, { "tstt",    ARG_IRGREG, ARG_NONE },

	/* 0x50 */
	{ "xorb",    ARG_IRGREG, ARG_NONE }, { "xorw",    ARG_IRGREG, ARG_NONE },
	{ "xorq",    ARG_IRGREG, ARG_NONE }, { "xort",    ARG_IRGREG, ARG_NONE },
	{ "cmpb",    ARG_IRGREG, ARG_NONE }, { "cmpw",    ARG_IRGREG, ARG_NONE },
	{ "cmpq",    ARG_IRGREG, ARG_NONE }, { "cmpt",    ARG_IRGREG, ARG_NONE },
	{ "movb",    ARG_IRGREG, ARG_NONE }, { "movw",    ARG_IRGREG, ARG_NONE },
	{ "movq",    ARG_IRGREG, ARG_NONE }, { "movt",    ARG_IRGREG, ARG_NONE },
	{ "cmpaddb", ARG_IRGREG, ARG_NONE }, { "cmpaddw", ARG_IRGREG, ARG_NONE },
	{ "cmpaddq", ARG_IRGREG, ARG_NONE }, { "cmpaddt", ARG_IRGREG, ARG_NONE },

	/* 0x60 */
	{ "shrb",    ARG_IRG,    ARG_S1   }, { "shrw",    ARG_IRG,    ARG_S1   },
	{ "shrq",    ARG_IRG,    ARG_S1   }, { "shrt",    ARG_IRG,    ARG_S1   },
	{ "orb",     ARG_IRGREG, ARG_NONE }, { "orw",     ARG_IRGREG, ARG_NONE },
	{ "orq",     ARG_IRGREG, ARG_NONE }, { "ort",     ARG_IRGREG, ARG_NONE },
	{ "shlb",    ARG_IRG,    ARG_S1   }, { "shlw",    ARG_IRG,    ARG_S1   },
	{ "shlq",    ARG_IRG,    ARG_S1   }, { "shlt",    ARG_IRG,    ARG_S1   },
	{ "andb",    ARG_IRGREG, ARG_NONE }, { "andw",    ARG_IRGREG, ARG_NONE },
	{ "andq",    ARG_IRGREG, ARG_NONE }, { "andt",    ARG_IRGREG, ARG_NONE },

	/* 0x70 */
	{ "sbbb",    ARG_IRGREG, ARG_NONE }, { "sbbw",    ARG_IRGREG, ARG_NONE }, /* BCD SUB */
	{ "sbbq",    ARG_IRGREG, ARG_NONE }, { "sbbt",    ARG_IRGREG, ARG_NONE }, /* BCD SUB */
	{ "subb",    ARG_IRGREG, ARG_NONE }, { "subw",    ARG_IRGREG, ARG_NONE },
	{ "subq",    ARG_IRGREG, ARG_NONE }, { "subt",    ARG_IRGREG, ARG_NONE },
	{ "adbb",    ARG_IRGREG, ARG_NONE }, { "adbw",    ARG_IRGREG, ARG_NONE }, /* BCD ADD */
	{ "adbq",    ARG_IRGREG, ARG_NONE }, { "adbt",    ARG_IRGREG, ARG_NONE }, /* BCD ADD */
	{ "addb",    ARG_IRGREG, ARG_NONE }, { "addw",    ARG_IRGREG, ARG_NONE },
	{ "addq",    ARG_IRGREG, ARG_NONE }, { "addt",    ARG_IRGREG, ARG_NONE },

	/* 0x80 */
	{ "un80?",   ARG_NONE,   ARG_NONE }, { "un81?",   ARG_NONE,   ARG_NONE },
	{ "un82?",   ARG_NONE,   ARG_NONE }, { "un83?",   ARG_NONE,   ARG_NONE },
	{ "un84?",   ARG_NONE,   ARG_NONE }, { "un85?",   ARG_NONE,   ARG_NONE },
	{ "un86?",   ARG_NONE,   ARG_NONE }, { "un87?",   ARG_NONE,   ARG_NONE },
	{ "jump",    ARG_A16,    ARG_NONE }, { "jump",    ARG_A24,    ARG_NONE },
	{ "call",    ARG_A16,    ARG_NONE }, { "un8b?",   ARG_NONE,   ARG_NONE },
	{ "bstack_to_dmem", ARG_NONE, ARG_NONE }, { "fstack_to_dmem", ARG_NONE, ARG_NONE },
	{ "un8E?",   ARG_NONE,   ARG_NONE }, { "un8F?",   ARG_NONE,   ARG_NONE },

	/* 0x90 */
	{ "retzh",   ARG_NONE,   ARG_NONE }, { "retzl",   ARG_NONE,   ARG_NONE },
	{ "retc",    ARG_NONE,   ARG_NONE }, { "retz",    ARG_NONE,   ARG_NONE },
	{ "retnzh",  ARG_NONE,   ARG_NONE }, { "retnzl",  ARG_NONE,   ARG_NONE },
	{ "retnc",   ARG_NONE,   ARG_NONE }, { "retnz",   ARG_NONE,   ARG_NONE },
	{ "jump",    ARG_IRG,    ARG_NONE }, { "un99?",   ARG_NONE,   ARG_NONE },
	{ "un9A?",   ARG_NONE,   ARG_NONE }, { "un9b?",   ARG_NONE,   ARG_NONE },
	{ "un9C?",   ARG_NONE,   ARG_NONE }, { "un9D?",   ARG_NONE,   ARG_NONE },
	{ "reti",    ARG_NONE,   ARG_NONE }, { "ret",     ARG_NONE,   ARG_NONE },

	/* 0xa0 */
	{ "jmpzh",   ARG_A16,    ARG_NONE }, { "jmpzl",   ARG_A16,    ARG_NONE },
	{ "jmpc",    ARG_A16,    ARG_NONE }, { "jmpz",    ARG_A16,    ARG_NONE },
	{ "jmpnzh",  ARG_A16,    ARG_NONE }, { "jmpnzl",  ARG_A16,    ARG_NONE },
	{ "jmpnc",   ARG_A16,    ARG_NONE }, { "jmpnz",   ARG_A16,    ARG_NONE },
	{ "callzh",  ARG_A16,    ARG_NONE }, { "callzl",  ARG_A16,    ARG_NONE },
	{ "callc",   ARG_A16,    ARG_NONE }, { "callz",   ARG_A16,    ARG_NONE },
	{ "callnzh", ARG_A16,    ARG_NONE }, { "callnzl", ARG_A16,    ARG_NONE },
	{ "callnc",  ARG_A16,    ARG_NONE }, { "callnz",  ARG_A16,    ARG_NONE },

	/* 0xb0 */
	{ "unB0?",   ARG_I8,     ARG_NONE }, { "unB1?",     ARG_I8,     ARG_NONE },
	{ "unB2?",   ARG_I8,     ARG_NONE }, { "timer_set", ARG_I8,     ARG_NONE },
	{ "out",     ARG_KHI,    ARG_REG  }, { "out",       ARG_KHI,    ARG_I8   },
	{ "out",     ARG_KLO,    ARG_REG  }, { "out",       ARG_KLO,    ARG_I8   },
	{ "unB8?",   ARG_NONE,   ARG_NONE }, { "unB9?",     ARG_I8,     ARG_NONE },
	{ "unBA?",   ARG_NONE,   ARG_NONE }, { "jmpcl",     ARG_A16,    ARG_NONE },
	{ "unBC?",   ARG_I8,     ARG_NONE }, { "unBD?",     ARG_NONE,   ARG_NONE },
	{ "unBE?",   ARG_NONE,   ARG_NONE }, { "jmpncl",    ARG_A16,    ARG_NONE },

	/* 0xc0 */
	{ "movb",    ARG_REG,    ARG_I8   }, { "movw",    ARG_REG,    ARG_I16  },
	{ "movq",    ARG_REG,    ARG_I8   }, { "movt",    ARG_REG,    ARG_I8   },
	{ "movb",    ARG_ILR,    ARG_ILR  }, { "movw",    ARG_ILR,    ARG_ILR  },
	{ "movq",    ARG_ILR,    ARG_ILR  }, { "movt",    ARG_ILR,    ARG_ILR  },
	{ "unC8?",   ARG_NONE,   ARG_NONE }, { "unC9?",   ARG_NONE,   ARG_NONE },
	{ "unCA?",   ARG_NONE,   ARG_NONE }, { "unCb?",   ARG_NONE,   ARG_NONE },
	{ "swapb",   ARG_IRGREG, ARG_NONE }, { "swapw",   ARG_IRGREG, ARG_NONE },
	{ "swapq",   ARG_IRGREG, ARG_NONE }, { "swapt",   ARG_IRGREG, ARG_NONE },

	/* 0xd0 */
	{ "movb",    ARG_CS,     ARG_REG  }, { "movb",    ARG_CS,     ARG_I8   },
	{ "movb",    ARG_DSZ,    ARG_REG  }, { "movb",    ARG_DSZ,    ARG_I8   },
	{ "movb",    ARG_SS,     ARG_REG  }, { "movb",    ARG_SS,     ARG_I8   },
	{ "movw",    ARG_SP,     ARG_REG  }, { "movw",    ARG_SP,     ARG_I16  },
	{ "movb",    ARG_F,      ARG_REG  }, { "movb",    ARG_F,      ARG_I8   },
	{ "unDA?",   ARG_NONE,   ARG_NONE }, { "unDb?",   ARG_NONE,   ARG_NONE },
	{ "movb",    ARG_DS,     ARG_REG  }, { "movb",    ARG_DS,     ARG_I8   },
	{ "movw",    ARG_LAR,    ARG_REG  }, { "movb",    ARG_LAR,    ARG_I8   },

	/* 0xe0 */
	{ "in0",     ARG_REG,    ARG_NONE }, { "movb",    ARG_REG,    ARG_OPT  },
	{ "in",      ARG_REG,    ARG_KI   }, { "movb",    ARG_REG,    ARG_DSZ  },
	{ "movb",    ARG_REG,    ARG_F    }, { "movb",    ARG_REG,    ARG_TIME },
	{ "movb",    ARG_REG,    ARG_PORT }, { "unE7?",   ARG_I8,     ARG_NONE },
	{ "movw",    ARG_REG,    ARG_LAR  }, { "movw?",   ARG_REG,    ARG_LAR  },
	{ "movw",    ARG_REG,    ARG_PC   }, { "movw",    ARG_REG,    ARG_SP   },
	{ "unEC?",   ARG_NONE,   ARG_NONE }, { "movb",    ARG_REG,    ARG_DS   },
	{ "movb",    ARG_REG,    ARG_CS   }, { "movb",    ARG_REG,    ARG_SS   },

	/* 0xf0 */
	{ "movb",        ARG_OPT,    ARG_REG  }, { "unF1?",          ARG_I8,     ARG_NONE },
	{ "movb",        ARG_PORT,   ARG_REG  }, { "unF3?",          ARG_I8,     ARG_NONE },
	{ "unF4?",       ARG_I8,     ARG_NONE }, { "unF5?",          ARG_I8,     ARG_NONE },
	{ "unF6?",       ARG_I8,     ARG_NONE }, { "timer_ctrl",     ARG_I8,     ARG_NONE },
	{ "unF8?",       ARG_I8,     ARG_NONE }, { "unF9?",          ARG_NONE,   ARG_NONE },
	{ "unFA?",       ARG_NONE,   ARG_NONE }, { "unFb?",          ARG_NONE,   ARG_NONE },
	{ "timer_clear", ARG_NONE,   ARG_NONE }, { "timer_wait_low", ARG_NONE,   ARG_NONE },
	{ "timer_wait",  ARG_NONE,   ARG_NONE }, { "nop",            ARG_NONE,   ARG_NONE }
};

u32 hcd62121_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t hcd62121_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 op;
	u8 op1;
	u8 op2;
	offs_t base_pc = pc;
	const dasm *inst;

	op = opcodes.r8(pc++);

	inst = &ops[op];

	/* Special cases for shift and rotate instructions */
	if (inst->arg2 == ARG_S4 || inst->arg2 == ARG_S8)
		util::stream_format(stream, "%c%c%c%c        ", inst->str[0], inst->str[1], (opcodes.r8(pc) & 0x80) ? 'r' : 'l', inst->str[3]);
	else
		util::stream_format(stream, "%-12s", inst->str);

	switch(inst->arg1)
	{
	case ARG_REGREG:
		op1 = opcodes.r8(pc++);
		op2 = opcodes.r8(pc++);
		if (op1 & 0x80)
		{
			util::stream_format(stream, "r%02x,0x%02x", op1 & 0x7f, op2);
		}
		else
		{
			if (op2 & 0x80)
				util::stream_format(stream, "r%02x,r%02x", op1 & 0x7f, op2 & 0x7f);
			else
				util::stream_format(stream, "r%02x,r%02x", op2 & 0x7f, op1 & 0x7f);
		}
		break;
	case ARG_REG:
		util::stream_format(stream, "r%02x", opcodes.r8(pc++) & 0x7f);
		break;
	case ARG_IRGREG:
		/* bit 6 = direction. 0 - regular, 1 - reverse */
		op1 = opcodes.r8(pc++);
		op2 = opcodes.r8(pc++);
		if (op1 & 0x80)
		{
			util::stream_format(stream, "(r%02x),0x%02x", 0x40 | (op1 & 0x3f), op2);
		}
		else
		{
			if (op2 & 0x80)
				util::stream_format(stream, "(r%02x%s),r%02x", 0x40 | (op1 & 0x3f), (op1 & 0x40) ? ".r" : "", op2 & 0x7f);
			else
				util::stream_format(stream, "r%02x,(r%02x%s)", op2 & 0x7f, 0x40 | (op1 & 0x3f), (op1 & 0x40) ? ".r" : "");
		}
		break;
	case ARG_IRG:
		/* bit 6 = direction. 0 - regular, 1 - reverse */
		op1 = opcodes.r8(pc++);
		util::stream_format(stream, "(r%02x%s)", 0x40 | (op1 & 0x3f), (op1 & 0x40) ? ".r" : "");
		break;
	case ARG_F:
		util::stream_format(stream, "F");
		break;
	case ARG_CS:
		util::stream_format(stream, "CS");
		break;
	case ARG_DS:
		util::stream_format(stream, "DS");
		break;
	case ARG_SS:
		util::stream_format(stream, "SS");
		break;
	case ARG_PC:
		util::stream_format(stream, "PC");
		break;
	case ARG_SP:
		util::stream_format(stream, "SP");
		break;
	case ARG_I8:
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		break;
	case ARG_I16:
	case ARG_A16:
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_I64:
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_I80:
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_A24:
		util::stream_format(stream, "0x%02x:", opcodes.r8(pc++));
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_ILR:
		op1 = opcodes.r8(pc++);
		op2 = opcodes.r8(pc++);
		if ((op1 & 0x80) || (op2 & 0x80))
		{
			if (op1 & 0x80)
			{
				/* (lar),imm */
				util::stream_format(stream, "(%slar%s), %02x", (op1 & 0x20) ? ((op1 & 0x40) ? "--" : "++") : "", (op1 & 0x20) ? "" : ((op1 & 0x40) ? "--" : "++"), op2);
			}
			else
			{
				/* (lar),reg */
				util::stream_format(stream, "(%slar%s),r%02x", (op1 & 0x20) ? ((op1 & 0x40) ? "--" : "++") : "", (op1 & 0x20) ? "" : ((op1 & 0x40) ? "--" : "++"), op2 & 0x7f);
			}
		}
		else
		{
			/* reg,(lar) */
			util::stream_format(stream, "r%02x,(%slar%s)", op2 & 0x7f, (op1 & 0x20) ? ((op1 & 0x40) ? "--" : "++") : "", (op1 & 0x20) ? "" : ((op1 & 0x40) ? "--" : "++"));
		}
		break;
	case ARG_LAR:
		util::stream_format(stream, "lar");
		break;
	case ARG_DSZ:
		util::stream_format(stream, "dsize");
		break;
	case ARG_OPT:
		util::stream_format(stream, "OPT");
		break;
	case ARG_PORT:
		util::stream_format(stream, "PORT");
		break;
	case ARG_TIME:
		util::stream_format(stream, "TIME");
		break;
	case ARG_KLO:
		util::stream_format(stream, "KOL");
		break;
	case ARG_KHI:
		util::stream_format(stream, "KOH");
		break;
	default:
		break;
	}

	switch(inst->arg2)
	{
	case ARG_REG:
		util::stream_format(stream, ",r%02x", opcodes.r8(pc++) & 0x7f);
		break;
	case ARG_F:
		util::stream_format(stream, ",F");
		break;
	case ARG_CS:
		util::stream_format(stream, ",CS");
		break;
	case ARG_DS:
		util::stream_format(stream, ",DS");
		break;
	case ARG_SS:
		util::stream_format(stream, ",SS");
		break;
	case ARG_PC:
		util::stream_format(stream, ",PC");
		break;
	case ARG_SP:
		util::stream_format(stream, ",SP");
		break;
	case ARG_I8:
		util::stream_format(stream, ",0x%02x", opcodes.r8(pc++));
		break;
	case ARG_I16:
		util::stream_format(stream, ",0x%02x", opcodes.r8(pc+1));
		util::stream_format(stream, "%02x", opcodes.r8(pc));
		pc += 2;
		break;
	case ARG_A16:
		util::stream_format(stream, ",0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_I64:
		util::stream_format(stream, ",0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_I80:
		util::stream_format(stream, ",0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_A24:
		util::stream_format(stream, ",0x%02x:", opcodes.r8(pc++));
		util::stream_format(stream, "0x%02x", opcodes.r8(pc++));
		util::stream_format(stream, "%02x", opcodes.r8(pc++));
		break;
	case ARG_ILR:
		/* Implemented by ARG_ILR section for arg1 */
		break;
	case ARG_LAR:
		util::stream_format(stream, ",lar");
		break;
	case ARG_DSZ:
		util::stream_format(stream, ",dsize");
		break;
	case ARG_OPT:
		util::stream_format(stream, ",OPT");
		break;
	case ARG_PORT:
		util::stream_format(stream, ",PORT");
		break;
	case ARG_TIME:
		util::stream_format(stream, ",TIME");
		break;
	case ARG_KI:
		util::stream_format(stream, ",KI");
		break;
	case ARG_S4:
		util::stream_format(stream, ",4");
		break;
	case ARG_S8:
		util::stream_format(stream, ",8");
		break;
	default:
		break;
	}

	return (pc - base_pc) | SUPPORTED;
}

