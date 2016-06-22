// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
/*
    SPARC disassembler
*/

#include "emu.h"
#include "sparcdasm.h"
#include "sparcdefs.h"

#include <cstdio>

namespace {
	const sparc_disassembler DASM_V7(7);
	const sparc_disassembler DASM_V8(8);
	const sparc_disassembler DASM_V9(9);

	INT32 get_disp16(UINT32 op) { return DISP19; }
	INT32 get_disp19(UINT32 op) { return DISP19; }
	INT32 get_disp22(UINT32 op) { return DISP19; }
}

const char * const sparc_disassembler::REG_NAMES[32] = {
	"%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7",
	"%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%o6", "%o7",
	"%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7",
	"%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%i6", "%i7"
};

const sparc_disassembler::branch_desc sparc_disassembler::EMPTY_BRANCH_DESC = {
	nullptr, 0, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BPCC_DESC = {
	&get_disp19, 6, true, true,
	{ "%icc", nullptr, "%xcc", nullptr },
	{
		"bn",    "be",    "ble",   "bl",    "bleu",  "bcs",   "bneg",  "bvs",
		"ba",    "bne",   "bg",    "bge",   "bgu",   "bcc",   "bpos",  "bvc"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BICC_DESC = {
	&get_disp22, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"bn",    "be",    "ble",   "bl",    "bleu",  "bcs",   "bneg",  "bvs",
		"ba",    "bne",   "bg",    "bge",   "bgu",   "bcc",   "bpos",  "bvc"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BPR_DESC = {
	&get_disp16, 5, true, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		nullptr, "brz",   "brlez", "brlz",  nullptr, "brnz",  "brgz",  "brgez",
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::FBPFCC_DESC = {
	&get_disp19, 6, true, true,
	{ "%fcc0", "%fcc1", "%fcc2", "%fcc3" },
	{
		"fbn",   "fbne",  "fblg",  "fbul",  "fbl",   "fbug",  "fbg",   "fbu",
		"fba",   "fbe",   "fbue",  "fbge",  "fbuge", "fble",  "fbule", "fbo"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::FBFCC_DESC = {
	&get_disp22, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"fbn",   "fbne",  "fblg",  "fbul",  "fbl",   "fbug",  "fbg",   "fbu",
		"fba",   "fbe",   "fbue",  "fbge",  "fbuge", "fble",  "fbule", "fbo"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::CBCCC_DESC = {
	&get_disp22, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"cbn",   "cb123", "cb12",  "cb13",  "cb1",   "cb23",  "cb2",   "cb3",
		"cba",   "cb0",   "cb03",  "cb02",  "cb023", "cb01",  "cb013", "cb012"
	}
};

const sparc_disassembler::int_op_desc_map::value_type sparc_disassembler::V7_INT_OP_DESC[] = {
	{ 0x00, { false, "add"      } }, { 0x10, { false, "addcc"    } },
	{ 0x01, { true,  "and"      } }, { 0x11, { true,  "andcc"    } },
	{ 0x02, { true,  "or"       } }, { 0x12, { true,  "orcc"     } },
	{ 0x03, { true,  "xor"      } }, { 0x13, { true,  "xorcc"    } },
	{ 0x04, { false, "sub"      } }, { 0x14, { false, "subcc"    } },
	{ 0x05, { true,  "andn"     } }, { 0x15, { true,  "andncc"   } },
	{ 0x06, { true,  "orn"      } }, { 0x16, { true,  "orncc"    } },
	{ 0x07, { true,  "xnor"     } }, { 0x17, { true,  "xnorcc"   } },
	{ 0x08, { false, "addx"     } }, { 0x18, { false, "addxcc"   } },
	{ 0x0c, { false, "subx"     } }, { 0x1c, { false, "subxcc"   } },

	{ 0x20, { false, "taddcc"   } },
	{ 0x21, { false, "tsubcc"   } },
	{ 0x22, { false, "taddcctv" } },
	{ 0x23, { false, "tsubcctv" } },
	{ 0x24, { false, "mulscc"   } },

	{ 0x3c, { false, "save"     } },
	{ 0x3d, { false, "restore"  } }
};

const sparc_disassembler::int_op_desc_map::value_type sparc_disassembler::V8_INT_OP_DESC[] = {
	{ 0x0a, { false, "umul"     } }, { 0x1a, { false, "umulcc"   } },
	{ 0x0b, { false, "smul"     } }, { 0x1b, { false, "smulcc"   } },
	{ 0x0e, { false, "udiv"     } }, { 0x1e, { false, "udivcc"   } },
	{ 0x0f, { false, "sdiv"     } }, { 0x1f, { false, "sdivcc"   } }
};

const sparc_disassembler::int_op_desc_map::value_type sparc_disassembler::V9_INT_OP_DESC[] = {
	{ 0x09, { false, "mulx"     } },
	{ 0x0d, { false, "udivx"    } },

	{ 0x2d, { false, "sdivx"    } }
};

const sparc_disassembler::state_reg_desc_map::value_type sparc_disassembler::V9_STATE_REG_DESC[] = {
	{ 1, { true,  nullptr, nullptr } },
	{ 2, { false, "%ccr",  "%ccr"  } },
	{ 3, { false, "%asi",  "%asi"  } },
	{ 4, { false, "%tick", nullptr } },
	{ 5, { false, "%pc",   nullptr } },
	{ 6, { false, "%fprs", "%fprs" } }
};

const char * const sparc_disassembler::MOVCC_CC_NAMES[8] = {
	"%fcc0", "%fcc1", "%fcc2", "%fcc3", "%icc", nullptr, "%xcc", nullptr
};

const char * const sparc_disassembler::MOVCC_COND_NAMES[32] = {
	"n",   "ne",  "lg",  "ul",  "l",   "ug",  "g",   "u",
	"a",   "e",   "ue",  "ge",  "uge", "le",  "ule", "o",
	"n",   "e",   "le",  "l",   "leu", "cs",  "neg", "vs",
	"a",   "ne",  "g",   "ge",  "gu",  "cc",  "pos", "vc"
};

const char * const sparc_disassembler::MOVE_INT_COND_MNEMONICS[8] = {
	nullptr, "movrz", "movrlez", "movrlz", nullptr, "movrnz", "movrgz", "movrgez"
};

const char * const sparc_disassembler::V9_PRIV_REG_NAMES[32] = {
	"%tpc",         "%tnpc",        "%tstate",      "%tt",          "%tick",        "%tba",         "%pstate",      "%tl",
	"%pil",         "%cwp",         "%cansave",     "%canrestore",  "%cleanwin",    "%otherwin",    "%wstate",      "%fq",
	nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,
	nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        "%ver"
};

const sparc_disassembler::fpop1_desc_map::value_type sparc_disassembler::V7_FPOP1_DESC[] = {
	{ 0x001, { false, false, false, false, "fmovs"  } },
	{ 0x005, { false, false, false, false, "fnegs"  } },
	{ 0x009, { false, false, false, false, "fabss"  } },
	{ 0x029, { false, false, false, false, "fsqrts" } },
	{ 0x02a, { false, false, true,  true,  "fsqrtd" } },
	{ 0x02b, { false, false, true,  true,  "fsqrtq" } },
	{ 0x041, { true,  false, false, false, "fadds"  } },
	{ 0x042, { true,  true,  true,  true,  "faddd"  } },
	{ 0x043, { true,  true,  true,  true,  "faddq"  } },
	{ 0x045, { true,  false, false, false, "fsubs"  } },
	{ 0x046, { true,  true,  true,  true,  "fsubd"  } },
	{ 0x047, { true,  true,  true,  true,  "fsubq"  } },
	{ 0x049, { true,  false, false, false, "fmuls"  } },
	{ 0x04a, { true,  true,  true,  true,  "fmuld"  } },
	{ 0x04b, { true,  true,  true,  true,  "fmulq"  } },
	{ 0x04d, { true,  false, false, false, "fdivs"  } },
	{ 0x04e, { true,  true,  true,  true,  "fdivd"  } },
	{ 0x04f, { true,  true,  true,  true,  "fdivq"  } },
	{ 0x069, { true,  false, false, true,  "fsmuld" } },
	{ 0x06e, { true,  true,  true,  true,  "fdmulq" } },
	{ 0x0c4, { false, false, false, false, "fitos"  } },
	{ 0x0c6, { false, false, true,  false, "fdtos"  } },
	{ 0x0c7, { false, false, true,  false, "fqtos"  } },
	{ 0x0c8, { false, false, false, true,  "fitod"  } },
	{ 0x0c9, { false, false, false, true,  "fstod"  } },
	{ 0x0cb, { false, false, true,  true,  "fqtod"  } },
	{ 0x0cc, { false, false, false, true,  "fitoq"  } },
	{ 0x0cd, { false, false, false, true,  "fstoq"  } },
	{ 0x0ce, { false, false, true,  true,  "fdtoq"  } },
	{ 0x0d1, { false, false, false, false, "fstoi"  } },
	{ 0x0d2, { false, false, true,  false, "fdtoi"  } },
	{ 0x0d3, { false, false, true,  false, "fqtoi"  } }
};

const sparc_disassembler::fpop1_desc_map::value_type sparc_disassembler::V9_FPOP1_DESC[] = {
	{ 0x002, { false, false, true,  true,  "fmovd"  } },
	{ 0x003, { false, false, true,  true,  "fmovq"  } },
	{ 0x006, { false, false, true,  true,  "fnegd"  } },
	{ 0x007, { false, false, true,  true,  "fnegq"  } },
	{ 0x00a, { false, false, true,  true,  "fabsd"  } },
	{ 0x00b, { false, false, true,  true,  "fabsq"  } },
	{ 0x081, { false, false, false, true,  "fstox"  } },
	{ 0x082, { false, false, true,  true,  "fdtox"  } },
	{ 0x083, { false, false, true,  true,  "fqtox"  } },
	{ 0x084, { false, false, true,  false, "fxtos"  } },
	{ 0x088, { false, false, true,  true,  "fxtod"  } },
	{ 0x08c, { false, false, true,  true,  "fxtoq"  } }
};

const sparc_disassembler::fpop2_desc_map::value_type sparc_disassembler::V7_FPOP2_DESC[] = {
	{ 0x051, { false, false, "fcmps"     } },
	{ 0x052, { false, true,  "fcmpd"     } },
	{ 0x053, { false, true,  "fcmpq"     } },
	{ 0x055, { false, false, "fcmpes"    } },
	{ 0x056, { false, true,  "fcmped"    } },
	{ 0x057, { false, true,  "fcmpeq"    } }
};

const sparc_disassembler::fpop2_desc_map::value_type sparc_disassembler::V9_FPOP2_DESC[] = {
	{ 0x025, { true,  false, "fmovrse"   } },
	{ 0x026, { true,  true,  "fmovrde"   } },
	{ 0x027, { true,  true,  "fmovrqe"   } },
	{ 0x045, { true,  false, "fmovrslez" } },
	{ 0x046, { true,  true,  "fmovrdlez" } },
	{ 0x047, { true,  true,  "fmovrqlez" } },
	{ 0x065, { true,  false, "fmovrslz"  } },
	{ 0x066, { true,  true,  "fmovrdlz"  } },
	{ 0x067, { true,  true,  "fmovrqlz"  } },
	{ 0x0a5, { true,  false, "fmovrsne"  } },
	{ 0x0a6, { true,  true,  "fmovrdne"  } },
	{ 0x0a7, { true,  true,  "fmovrqne"  } },
	{ 0x0c5, { true,  false, "fmovrsgz"  } },
	{ 0x0c6, { true,  true,  "fmovrdgz"  } },
	{ 0x0c7, { true,  true,  "fmovrqgz"  } },
	{ 0x0e5, { true,  false, "fmovrsgez" } },
	{ 0x0e6, { true,  true,  "fmovrdgez" } },
	{ 0x0e7, { true,  true,  "fmovrqgez" } }
};

const sparc_disassembler::ldst_desc_map::value_type sparc_disassembler::V7_LDST_DESC[] = {
	{ 0x00, { false, false, '\0', false, "ld",     nullptr } }, { 0x10, { false, true,  '\0', false, "lda",     nullptr } },
	{ 0x01, { false, false, '\0', false, "ldub",   nullptr } }, { 0x11, { false, true,  '\0', false, "lduba",   nullptr } },
	{ 0x02, { false, false, '\0', false, "lduh",   nullptr } }, { 0x12, { false, true,  '\0', false, "lduha",   nullptr } },
	{ 0x03, { false, false, '\0', false, "ldd",    nullptr } }, { 0x13, { false, true,  '\0', false, "ldda",    nullptr } },
	{ 0x04, { true,  false, '\0', false, "st",     "clr"   } }, { 0x14, { true,  true,  '\0', false, "sta",     nullptr } },
	{ 0x05, { true,  false, '\0', false, "stb",    "clrb"  } }, { 0x15, { true,  true,  '\0', false, "stba",    nullptr } },
	{ 0x06, { true,  false, '\0', false, "sth",    "clrh"  } }, { 0x16, { true,  true,  '\0', false, "stha",    nullptr } },
	{ 0x07, { true,  false, '\0', false, "std",    nullptr } }, { 0x17, { true,  true,  '\0', false, "stda",    nullptr } },
	{ 0x09, { false, false, '\0', false, "ldsb",   nullptr } }, { 0x19, { false, true,  '\0', false, "ldsba",   nullptr } },
	{ 0x0a, { false, false, '\0', false, "ldsh",   nullptr } }, { 0x1a, { false, true,  '\0', false, "ldsha",   nullptr } },
	{ 0x0d, { false, false, '\0', false, "ldstub", nullptr } }, { 0x1d, { false, true,  '\0', false, "ldstuba", nullptr } },
	{ 0x0f, { false, false, '\0', false, "swap",   nullptr } }, { 0x1f, { false, true,  '\0', false, "swapa",   nullptr } },

	{ 0x20, { false, false, 'f',  false, "ld",     nullptr } }, { 0x30, { false, false, 'c',  false, "ld",      nullptr } },
	{ 0x23, { false, false, 'f',  true,  "ldd",    nullptr } }, { 0x33, { false, false, 'c',  false, "ldd",     nullptr } },
	{ 0x24, { true,  false, 'f',  false, "st",     nullptr } }, { 0x34, { true,  false, 'c',  false, "st",      nullptr } },
	{ 0x27, { true,  false, 'f',  true,  "std",    nullptr } }, { 0x37, { true,  false, 'c',  false, "std",     nullptr } }
};

const sparc_disassembler::ldst_desc_map::value_type sparc_disassembler::V9_LDST_DESC[] = {
	{ 0x08, { false, false, '\0', false, "ldsw",   nullptr } }, { 0x18, { false, true,  '\0', false, "ldswa",   nullptr } },
	{ 0x0b, { false, false, '\0', false, "ldx",    nullptr } }, { 0x1b, { false, true,  '\0', false, "ldxa",    nullptr } },
	{ 0x0e, { true,  false, '\0', false, "stx",    "clrx"  } }, { 0x1e, { true,  true,  '\0', false, "stxa",    nullptr } },

																{ 0x30, { false, true,  'f',  false, "lda",     nullptr } },
	{ 0x22, { false, false, 'f',  true,  "ldq",    nullptr } }, { 0x32, { false, true,  'f',  true,  "ldqa",    nullptr } },
																{ 0x33, { false, true,  'f',  true,  "ldda",    nullptr } },
																{ 0x34, { true,  true,  'f',  false, "sta",     nullptr } },
	{ 0x26, { true,  false, 'f',  true,  "stq",    nullptr } }, { 0x36, { true,  true,  'f',  true,  "stqa",    nullptr } },
																{ 0x37, { true,  true,  'f',  true,  "stda",    nullptr } }
};

const sparc_disassembler::asi_desc_map::value_type sparc_disassembler::V9_ASI_DESC[] = {
	{ 0x10, { "#ASI_AIUP",   nullptr } },
	{ 0x11, { "#ASI_AIUS",   nullptr } },
	{ 0x18, { "#ASI_AIUP_L", nullptr } },
	{ 0x19, { "#ASI_AIUS_L", nullptr } },
	{ 0x80, { "#ASI_P",      nullptr } },
	{ 0x81, { "#ASI_S",      nullptr } },
	{ 0x82, { "#ASI_PNF",    nullptr } },
	{ 0x83, { "#ASI_SNF",    nullptr } },
	{ 0x88, { "#ASI_P_L",    nullptr } },
	{ 0x89, { "#ASI_S_L",    nullptr } },
	{ 0x8a, { "#ASI_PNF_L",  nullptr } },
	{ 0x8b, { "#ASI_SNF_L",  nullptr } }
};

const sparc_disassembler::prftch_desc_map::value_type sparc_disassembler::V9_PRFTCH_DESC[] =
{
	{ 0x00, { "#n_reads"   } },
	{ 0x01, { "#one_read"  } },
	{ 0x02, { "#n_writes"  } },
	{ 0x03, { "#one_write" } },
	{ 0x04, { "#page"      } }
};


inline UINT32 sparc_disassembler::freg(UINT32 val, bool shift) const
{
	return (shift && (m_version >= 9)) ? ((val & 0x1e) | ((val << 5) & 0x20)) : val;
}

template <typename T> inline void sparc_disassembler::add_int_op_desc(const T &desc)
{
	for (const auto &it : desc)
		m_int_op_desc.insert(it);
}

template <typename T> inline void sparc_disassembler::add_fpop1_desc(const T &desc)
{
	for (const auto &it : desc)
		m_fpop1_desc.insert(it);
}

template <typename T> inline void sparc_disassembler::add_fpop2_desc(const T &desc)
{
	for (const auto &it : desc)
		m_fpop2_desc.insert(it);
}

template <typename T> inline void sparc_disassembler::add_ldst_desc(const T &desc)
{
	for (const auto &it : desc)
		m_ldst_desc.insert(it);
}

inline void sparc_disassembler::pad_op_field(char *buf, char *&output) const
{
	while ((output - buf) < m_op_field_width) *output++ = ' ';
}

inline void sparc_disassembler::print(char *&output, const char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	output += std::vsprintf(output, fmt, vl);
	va_end(vl);
}


sparc_disassembler::sparc_disassembler(unsigned version)
	: m_version(version)
	, m_op_field_width(9)
	, m_branch_desc{
		EMPTY_BRANCH_DESC,
		(version >= 9) ? BPCC_DESC : EMPTY_BRANCH_DESC,     // branch on integer condition codes with prediction, SPARCv9
		BICC_DESC,                                          // branch on integer condition codes
		(version >= 9) ? BPR_DESC : EMPTY_BRANCH_DESC,      // branch on integer register with prediction, SPARCv9
		EMPTY_BRANCH_DESC,
		(version >= 9) ? FBPFCC_DESC : EMPTY_BRANCH_DESC,   // branch on floating-point condition codes with prediction, SPARCv9
		FBFCC_DESC,                                         // branch on floating-point condition codes
		(version == 8) ? CBCCC_DESC : EMPTY_BRANCH_DESC     // branch on coprocessor condition codes, SPARCv8
	}
	, m_int_op_desc(std::begin(V7_INT_OP_DESC), std::end(V7_INT_OP_DESC))
	, m_state_reg_desc()
	, m_fpop1_desc(std::begin(V7_FPOP1_DESC), std::end(V7_FPOP1_DESC))
	, m_fpop2_desc(std::begin(V7_FPOP2_DESC), std::end(V7_FPOP2_DESC))
	, m_ldst_desc(std::begin(V7_LDST_DESC), std::end(V7_LDST_DESC))
	, m_asi_desc()
	, m_prftch_desc()
{
	if (m_version >= 8)
	{
		add_int_op_desc(V8_INT_OP_DESC);
	}

	if (m_version >= 9)
	{
		m_op_field_width = 11;

		m_int_op_desc.find(0x08)->second.mnemonic = "addc";
		m_int_op_desc.find(0x0c)->second.mnemonic = "subc";
		m_int_op_desc.find(0x18)->second.mnemonic = "addccc";
		m_int_op_desc.find(0x1c)->second.mnemonic = "subccc";
		add_int_op_desc(V9_INT_OP_DESC);

		add_state_reg_desc(V9_STATE_REG_DESC),
		add_fpop1_desc(V9_FPOP1_DESC);
		add_fpop2_desc(V9_FPOP2_DESC);

		m_ldst_desc.find(0x00)->second.mnemonic = "lduw";
		m_ldst_desc.find(0x04)->second.mnemonic = "stw";
		m_ldst_desc.find(0x10)->second.mnemonic = "lduwa";
		m_ldst_desc.find(0x14)->second.mnemonic = "stwa";
		m_ldst_desc.erase(0x30); // LDC
		m_ldst_desc.erase(0x33); // LDDC
		m_ldst_desc.erase(0x34); // STC
		m_ldst_desc.erase(0x37); // STDC
		add_ldst_desc(V9_LDST_DESC);

		add_asi_desc(V9_ASI_DESC);

		add_prftch_desc(V9_PRFTCH_DESC);
	}
}


offs_t sparc_disassembler::dasm(char *buf, offs_t pc, UINT32 op) const
{
	switch (OP)
	{
	case 0: // Branches & SETHI
		switch (OP2)
		{
		case 0:
			print(buf, "%-*s0x%06x", m_op_field_width, (m_version >= 9) ? "illtrap" : "unimp", CONST22);
			break;
		case 4:
			if (IMM22 == 0 && RD == 0)
				print(buf, "nop");
			else
				print(buf, "%-*s%%hi(0x%08x),%s", m_op_field_width, "sethi", IMM22, REG_NAMES[RD]);
			break;
		default:
			return dasm_branch(buf, pc, op);
		}
		return 4 | DASMFLAG_SUPPORTED;
	case 1:
		print(buf, "%-*s%%pc%c0x%08x ! 0x%08x", m_op_field_width, "call", (DISP30 < 0) ? '-' : '+', std::abs(DISP30), pc + DISP30);
		return 4 | DASMFLAG_SUPPORTED;
	case 2:
		switch (OP3)
		{
		case 0x00:
			if (USEIMM && (RS1 == RD))
			{
				if (SIMM13 == 1)    print(buf, "%-*s%s", m_op_field_width, "inc", REG_NAMES[RD]);
				else                print(buf, "%-*s%d,%s", m_op_field_width, "inc", SIMM13, REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x02:
			if (RS1 == 0)
			{
				if (USEIMM)         print(buf, "%-*s%d,%s", m_op_field_width, "mov", SIMM13, REG_NAMES[RD]);
				else if (RS2 == 0)  print(buf, "%-*s%s", m_op_field_width, "clr", REG_NAMES[RD]);
				else                print(buf, "%-*s%s,%s", m_op_field_width, "mov", REG_NAMES[RS2], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			else if (RS1 == RD)
			{
				if (USEIMM) print(buf, "%-*s0x%08x,%s", m_op_field_width, "bset", SIMM13, REG_NAMES[RD]);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "bset", REG_NAMES[RS2], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x03:
			if (RS1 == RD)
			{
				if (USEIMM) print(buf, "%-*s0x%08x,%s", m_op_field_width, "btog", SIMM13, REG_NAMES[RD]);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "btog", REG_NAMES[RS2], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x04:
			if (USEIMM && (RS1 == RD))
			{
				if (SIMM13 == 1)    print(buf, "%-*s%s", m_op_field_width, "dec", REG_NAMES[RD]);
				else                print(buf, "%-*s%d,%s", m_op_field_width, "dec", SIMM13, REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x05:
			if (RS1 == RD)
			{
				if (USEIMM) print(buf, "%-*s0x%08x,%s", m_op_field_width, "bclr", SIMM13, REG_NAMES[RD]);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "bclr", REG_NAMES[RS2], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x07:
			if ((USEIMM && (SIMM13 == 0)) || (!USEIMM && (RS2 == 0)))
			{
				if (RS1 == RD)  print(buf, "%-*s%s", m_op_field_width, "not", REG_NAMES[RD]);
				else            print(buf, "%-*s%s,%s", m_op_field_width, "not", REG_NAMES[RS1], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x10:
			if (USEIMM && (RS1 == RD))
			{
				if (SIMM13 == 1)    print(buf, "%-*s%s", m_op_field_width, "inccc", REG_NAMES[RD]);
				else                print(buf, "%-*s%d,%s", m_op_field_width, "inccc", SIMM13, REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x11:
			if (RD == 0)
			{
				if (USEIMM) print(buf, "%-*s0x%08x,%s", m_op_field_width, "btst", SIMM13, REG_NAMES[RS1]);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "btst", REG_NAMES[RS2], REG_NAMES[RS1]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x14:
			if (RD == 0)
			{
				if (USEIMM) print(buf, "%-*s%s,%d", m_op_field_width, "cmp", REG_NAMES[RS1], SIMM13);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "cmp", REG_NAMES[RS1], REG_NAMES[RS2]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			else if (USEIMM && (RS1 == RD))
			{
				if (SIMM13 == 1)    print(buf, "%-*s%s", m_op_field_width, "deccc", REG_NAMES[RD]);
				else                print(buf, "%-*s%d,%s", m_op_field_width, "deccc", SIMM13, REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x25:
			return dasm_shift(buf, pc, op, "sll", "sllx", nullptr);
		case 0x26:
			return dasm_shift(buf, pc, op, "srl", "srlx", "clruw");
		case 0x27:
			return dasm_shift(buf, pc, op, "sra", "srax", "signx");
		case 0x28:
			return dasm_read_state_reg(buf, pc, op);
		case 0x29:
			if (m_version <= 8)
			{
				print(buf, "%-*s%%psr,%s", m_op_field_width, "rd", REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x2a:
			if (m_version >= 9)
			{
				if (V9_PRIV_REG_NAMES[RS1])
				{
					print(buf, "%-*s%s,%s", m_op_field_width, "rdpr", V9_PRIV_REG_NAMES[RS1], REG_NAMES[RD]);
					return 4 | DASMFLAG_SUPPORTED;
				}
			}
			else
			{
				print(buf, "%-*s%%wim,%s", m_op_field_width, "rd", REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x2b:
			if (m_version >= 9)
			{
				if (!USEIMM)
				{
					print(buf, "flushw");
					return 4 | DASMFLAG_SUPPORTED;
				}
			}
			else
			{
				print(buf, "%-*s%%tbr,%s", m_op_field_width, "rd", REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x2c:
			return dasm_move_cond(buf, pc, op);
		case 0x2e:
			if ((m_version >= 9) && (RS1 == 0))
			{
				if (USEIMM) print(buf, "%-*s%d,%s", m_op_field_width, "popc", SIMM13, REG_NAMES[RD]);
				else        print(buf, "%-*s%s,%s", m_op_field_width, "popc", REG_NAMES[RS2], REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x2f:
			return dasm_move_reg_cond(buf, pc, op);
		case 0x30:
			return dasm_write_state_reg(buf, pc, op);
		case 0x31:
			if (m_version >= 9)
			{
				switch (RD)
				{
				case 0:
					print(buf, "saved");
					return 4 | DASMFLAG_SUPPORTED;
				case 1:
					print(buf, "restored");
					return 4 | DASMFLAG_SUPPORTED;
				}
			}
			else
			{
				if (!USEIMM)        print(buf, "%-*s%s,%s,%%psr", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				else if (RS1 == 0)  print(buf, "%-*s0x%08x,%%psr", m_op_field_width, "wr", SIMM13);
				else                print(buf, "%-*s%s,0x%08x,%%psr", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x32:
			if (m_version >= 9)
			{
				if (V9_PRIV_REG_NAMES[RD])
				{
					// FIXME: this disassembles wrpr to %fq and %ver which are actually illegal
					if (!USEIMM)        print(buf, "%-*s%s,%s,%s", m_op_field_width, "wrpr", REG_NAMES[RS1], REG_NAMES[RS2], V9_PRIV_REG_NAMES[RD]);
					else if (RS1 == 0)  print(buf, "%-*s0x%08x,%s", m_op_field_width, "wrpr", SIMM13, V9_PRIV_REG_NAMES[RD]);
					else                print(buf, "%-*s%s,0x%08x,%s", m_op_field_width, "wrpr", REG_NAMES[RS1], SIMM13, V9_PRIV_REG_NAMES[RD]);
					return 4 | DASMFLAG_SUPPORTED;
				}
			}
			else
			{
				if (!USEIMM)        print(buf, "%-*s%s,%s,%%wim", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				else if (RS1 == 0)  print(buf, "%-*s0x%08x,%%wim", m_op_field_width, "wr", SIMM13);
				else                print(buf, "%-*s%s,0x%08x,%%wim", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x33:
			if (m_version <= 8)
			{
				if (!USEIMM)        print(buf, "%-*s%s,%s,%%tbr", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				else if (RS1 == 0)  print(buf, "%-*s0x%08x,%%tbr", m_op_field_width, "wr", SIMM13);
				else                print(buf, "%-*s%s,0x%08x,%%tbr", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x34:
			return dasm_fpop1(buf, pc, op);
		case 0x35:
			return dasm_fpop2(buf, pc, op);
		case 0x36:
			// TODO: hooks for IMPDEP1/CPop1
			break;
		case 0x37:
			// TODO: hooks for IMPDEP2/CPop2
			break;
		case 0x38:
			return dasm_jmpl(buf, pc, op);
		case 0x39:
			return dasm_return(buf, pc, op);
		case 0x3a:
			return dasm_tcc(buf, pc, op);
		case 0x3b:
			if (m_version >= 8)
			{
				print(buf, "%-*s", m_op_field_width, "flush");
				dasm_address(buf, op);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x3c:
			if (!USEIMM && (RS1 == RS2) && (RS2 == RD) && (RD == 0))
			{
				print(buf, "save");
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x3d:
			if (!USEIMM && (RS1 == RS2) && (RS2 == RD) && (RD == 0))
			{
				print(buf, "restore");
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x3e:
			if ((m_version >= 9) & ((op & 0x7ffff) == 0))
			{
				switch (RD)
				{
				case 0: print(buf, "done"); return 4 | DASMFLAG_SUPPORTED;
				case 1: print(buf, "retry"); return 4 | DASMFLAG_SUPPORTED;
				}
			}
			break;
		}
		{
			const auto it(m_int_op_desc.find(OP3));
			if (it != m_int_op_desc.end())
			{
				if (!USEIMM)
					print(buf, "%-*s%s,%s,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], REG_NAMES[RS2], REG_NAMES[RD]);
				else if (it->second.hex_imm)
					print(buf, "%-*s%s,0x%08x,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], SIMM13, REG_NAMES[RD]);
				else
					print(buf, "%-*s%s,%d,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], SIMM13, REG_NAMES[RD]);
				return 4 | DASMFLAG_SUPPORTED;
			}
		}
		break;
	case 3:
		return dasm_ldst(buf, pc, op);
	}
	return dasm_invalid(buf, pc, op);
}


offs_t sparc_disassembler::dasm_invalid(char *buf, offs_t pc, UINT32 op) const
{
	print(buf, "%-*s0x%08x ! ", m_op_field_width, ".word", op);
	if (OP == 0)
	{
		print(buf, "op=%x op2=%01x a=%01x cond=%01x", OP, OP2, ANNUL, COND);
	}
	else if ((OP == 2) && ((OP3 == 0x34) || (OP3 == 0x35)))
	{
		print(buf, "FPop%d opf=%03x rd=%d rs1=%d rs2=%d", 1 + (OP3 & 1), OPF, RD, RS1, RS2);
	}
	else if ((OP == 2) && ((OP3 == 0x36) || (OP3 == 0x37)))
	{
		if (m_version >= 9)
			print(buf, "IMPDEP%d impl-dep=%02x impl-dep=%05x", 1 + (OP3 & 1), RD, op & 0x7ffff);
		else
			print(buf, "CPop%d opf=%03x rd=%d rs1=%d rs2=%d", 1 + (OP3 & 1), OPC, RD, RS1, RS2);
	}
	else
	{
		print(buf, "op=%x op3=%02x i=%01x rd=%d", OP, OP3, USEIMM, RD);
	}
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_branch(char *buf, offs_t pc, UINT32 op) const
{
	char *ptr(buf);
	const branch_desc &desc(m_branch_desc[OP2]);
	const char * const mnemonic(desc.mnemonic[COND]);
	if (!mnemonic || (desc.use_cc && !desc.reg_cc[BRCC])) return dasm_invalid(buf, pc, op);

	print(ptr, "%s%s%s", mnemonic, ANNUL ? ",a" : "", (m_branch_desc[OP2].use_pred && !PRED) ? ",pn" : "");
	pad_op_field(buf, ptr);
	if (desc.use_cc) print(ptr, "%s,", desc.reg_cc[BRCC]);
	if (OP2 == 3) print(ptr, "%s,", REG_NAMES[RS1]);
	const INT32 disp(desc.get_disp(op));
	print(ptr, "%%pc%c0x%0*x ! 0x%08x", (disp < 0) ? '-' : '+', desc.disp_width, std::abs(disp), pc + disp);

	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_shift(char *buf, offs_t pc, UINT32 op, const char *mnemonic, const char *mnemonicx, const char *mnemonicx0) const
{
	if ((m_version >= 9) && USEEXT)
	{
		if (USEIMM)
			print(buf, "%-*s%s,%d,%s", m_op_field_width, mnemonicx, REG_NAMES[RS1], SHCNT64, REG_NAMES[RD]);
		else if (!mnemonicx0 || (RS2 != 0))
			print(buf, "%-*s%s,%s,%s", m_op_field_width, mnemonicx, REG_NAMES[RS1], REG_NAMES[RS2], REG_NAMES[RD]);
		else if (RS1 == RD)
			print(buf, "%-*s%s", m_op_field_width, mnemonicx0, REG_NAMES[RD]);
		else
			print(buf, "%-*s%s,%s", m_op_field_width, mnemonicx0, REG_NAMES[RS1], REG_NAMES[RD]);
	}
	else if (USEIMM)
	{
		print(buf, "%-*s%s,%d,%s", m_op_field_width, mnemonic, REG_NAMES[RS1], SHCNT32, REG_NAMES[RD]);
	}
	else
	{
		print(buf, "%-*s%s,%s,%s", m_op_field_width, mnemonic, REG_NAMES[RS1], REG_NAMES[RS2], REG_NAMES[RD]);
	}
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_read_state_reg(char *buf, offs_t pc, UINT32 op) const
{
	if (RS1 == 0)
	{
		print(buf, "%-*s%%y,%s", m_op_field_width, "rd", REG_NAMES[RD]);
		return 4 | DASMFLAG_SUPPORTED;
	}
	else if ((m_version == 8) || ((m_version >= 9) && !USEIMM))
	{
		if (!USEIMM && (RS1 == 15) && (RD == 0))
		{
			print(buf, "stbar");
			return 4 | DASMFLAG_SUPPORTED;
		}
		else
		{
			const auto it(m_state_reg_desc.find(RS1));
			if ((it == m_state_reg_desc.end()) || !it->second.reserved)
			{
				if ((it != m_state_reg_desc.end()) && it->second.read_name)
					print(buf, "%-*s%s,%s", m_op_field_width, "rd", it->second.read_name, REG_NAMES[RD]);
				else
					print(buf, "%-*s%%asr%d,%s ! %s", m_op_field_width, "rd", RS1, REG_NAMES[RD], (RS1 < 16) ? "reserved" : "implementation-dependent");
				return 4 | DASMFLAG_SUPPORTED;
			}
		}
	}
	else if ((m_version >= 9) && USEIMM && (RS1 == 15) && (RD == 0))
	{
		print(buf, "%-*s", m_op_field_width, "membar");
		UINT32 mask(MMASK | (CMASK << 4));
		if (mask == 0) print(buf, "0");
		if (mask & 1) print(buf, "#LoadLoad%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#StoreLoad%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#LoadStore%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#StoreStore%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#Lookaside%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#MemIssue%s", (mask >> 1) ? "|" : "");
		mask >>= 1;
		if (mask & 1) print(buf, "#Sync");
		return 4 | DASMFLAG_SUPPORTED;
	}
	return dasm_invalid(buf, pc, op);
}


offs_t sparc_disassembler::dasm_write_state_reg(char *buf, offs_t pc, UINT32 op) const
{
	if (RD == 0)
	{
		if (USEIMM) print(buf, "%-*s%s,%d,%%y", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
		else        print(buf, "%-*s%s,%s,%%y", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
		return 4 | DASMFLAG_SUPPORTED;
	}
	else if (m_version >= 8)
	{
		if ((m_version >= 9) && USEIMM && (RS1 == 0) && (RD == 15))
		{
			print(buf, "%-*s%d", m_op_field_width, "sir", SIMM13);
			return 4 | DASMFLAG_SUPPORTED;
		}
		else
		{
			const auto it(m_state_reg_desc.find(RD));
			if ((it == m_state_reg_desc.end()) || !it->second.reserved)
			{
				if ((it != m_state_reg_desc.end()) && it->second.write_name)
				{
					if (USEIMM) print(buf, "%-*s%s,%d,%s", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13, it->second.write_name);
					else        print(buf, "%-*s%s,%s,%s", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2], it->second.write_name);
				}
				else
				{
					const char *const comment((RD < 16) ? "reserved" : "implementation-dependent");
					if (USEIMM) print(buf, "%-*s%s,%d,%%asr%d ! %s", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13, RD, comment);
					else        print(buf, "%-*s%s,%s,%%asr%d ! %s", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2], RD, comment);
				}
				return 4 | DASMFLAG_SUPPORTED;
			}
		}
	}
	return dasm_invalid(buf, pc, op);
}


offs_t sparc_disassembler::dasm_move_cond(char *buf, offs_t pc, UINT32 op) const
{
	if ((m_version < 9) || !MOVCC_CC_NAMES[MOVCC]) return dasm_invalid(buf, pc, op);

	char *ptr(buf);
	print(ptr, "mov%s", MOVCC_COND_NAMES[MOVCOND | ((MOVCC << 2) & 16)]);
	pad_op_field(buf, ptr);
	if (USEIMM)
		print(ptr, "%s,%d,%s", MOVCC_CC_NAMES[MOVCC], SIMM11, REG_NAMES[RD]);
	else
		print(ptr, "%s,%s,%s", MOVCC_CC_NAMES[MOVCC], REG_NAMES[RS2], REG_NAMES[RD]);

	return 4 | DASMFLAG_SUPPORTED;
}

offs_t sparc_disassembler::dasm_move_reg_cond(char *buf, offs_t pc, UINT32 op) const
{
	if ((m_version < 9) || !MOVE_INT_COND_MNEMONICS[RCOND]) return dasm_invalid(buf, pc, op);

	if (USEIMM)
		print(buf, "%-*s%s,%d,%s", m_op_field_width, MOVE_INT_COND_MNEMONICS[RCOND], REG_NAMES[RS1], SIMM10, REG_NAMES[RD]);
	else
		print(buf, "%-*s%s,%s,%s", m_op_field_width, MOVE_INT_COND_MNEMONICS[RCOND], REG_NAMES[RS1], REG_NAMES[RS2], REG_NAMES[RD]);

	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_fpop1(char *buf, offs_t pc, UINT32 op) const
{
	const auto it(m_fpop1_desc.find(OPF));
	if (it == m_fpop1_desc.end()) return dasm_invalid(buf, pc, op);

	if (it->second.three_op)
		print(buf, "%-*s%%f%d,%%f%d,%%f%d", m_op_field_width, it->second.mnemonic, freg(RS1, it->second.rs1_shift), freg(RS2, it->second.rs2_shift), freg(RD, it->second.rd_shift));
	else
		print(buf, "%-*s%%f%d,%%f%d", m_op_field_width, it->second.mnemonic, freg(RS2, it->second.rs2_shift), freg(RD, it->second.rd_shift));
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_fpop2(char *buf, offs_t pc, UINT32 op) const
{
	// Move Floating-Point Register on Condition
	if ((m_version >= 9) && (((op >> 18) & 1) == 0) && MOVCC_CC_NAMES[OPFCC])
	{
		const char *mnemonic;
		bool shift;
		switch (OPFLOW)
		{
		case 1:  mnemonic = "fmovs"; shift = false; break;
		case 2:  mnemonic = "fmovd"; shift = true; break;
		case 3:  mnemonic = "fmovq"; shift = true; break;
		default: mnemonic = nullptr;
		}
		if (mnemonic)
		{
			char *ptr(buf);
			print(ptr, "%s%s", mnemonic, MOVCC_COND_NAMES[MOVCOND | ((OPFCC << 2) & 16)]);
			pad_op_field(buf, ptr);
			print(ptr, "%s,%%f%d,%%f%d", MOVCC_CC_NAMES[OPFCC], freg(RS2, shift), freg(RD, shift));
			return 4 | DASMFLAG_SUPPORTED;
		}
	}

	const auto it(m_fpop2_desc.find(OPF));
	if (it != m_fpop2_desc.end())
	{
		if (m_version >= 9)
		{
			if (it->second.int_rs1)
			{
				print(buf, "%-*s%s,%%f%d,%%f%d", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], freg(RS2, it->second.shift), freg(RD, it->second.shift));
				return 4 | DASMFLAG_SUPPORTED;
			}
			else if (RD < 4)
			{
				print(buf, "%-*s%%fcc%d,%%f%d,%%f%d", m_op_field_width, it->second.mnemonic, RD, freg(RS1, it->second.shift), freg(RS2, it->second.shift));
				return 4 | DASMFLAG_SUPPORTED;
			}
		}
		else if (!it->second.int_rs1)
		{
			print(buf, "%-*s%%f%d,%%f%d", m_op_field_width, it->second.mnemonic, freg(RS1, it->second.shift), freg(RS2, it->second.shift));
			return 4 | DASMFLAG_SUPPORTED;
		}
	}

	return dasm_invalid(buf, pc, op);
}


offs_t sparc_disassembler::dasm_jmpl(char *buf, offs_t pc, UINT32 op) const
{
	if (USEIMM && (RD == 0) && ((RS1 == 15) || (RS1 == 31)) && (SIMM13 == 8))
	{
		print(buf, (RS1 == 31) ? "ret" : "retl");
	}
	else
	{
		print(buf, "%-*s", m_op_field_width, (RD == 0) ? "jmp" : (RD == 15) ? "call" : "jmpl");
		dasm_address(buf, op);
		if ((RD != 0) && (RD != 15))
			print(buf, ",%s", REG_NAMES[RD]);
	}
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_return(char *buf, offs_t pc, UINT32 op) const
{
	print(buf, "%-*s", m_op_field_width, (m_version >= 9) ? "return" : "rett");
	dasm_address(buf, op);
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_tcc(char *buf, offs_t pc, UINT32 op) const
{
	static const char *const tcc_names[16] = {
		"tn",   "te",   "tle",  "tl",   "tleu", "tcs",  "tneg", "tvs",
		"ta",   "tne",  "tg",   "tge",  "tgu",  "tcc",  "tpos", "tvc"
	};
	static const char *const cc_names[4] = { "%icc", nullptr, "%xcc", nullptr };
	const char *const mnemonic(tcc_names[COND]);
	if (m_version >= 9)
	{
		const char *const cc(cc_names[TCCCC]);
		if (!cc) return dasm_invalid(buf, pc, op);
		print(buf, "%-*s%s,", m_op_field_width, mnemonic, cc);
	}
	else
	{
		print(buf, "%-*s", m_op_field_width, mnemonic);
	}
	if (USEIMM)
	{
		if (RS1 == 0)       print(buf, "%d", IMM7);
		else                print(buf, "%s,%d", REG_NAMES[RS1], IMM7);
	}
	else
	{
		if (RS1 == 0)       print(buf, "%s", REG_NAMES[RS2]);
		else if (RS2 == 0)  print(buf, "%s", REG_NAMES[RS1]);
		else                print(buf, "%s,%s", REG_NAMES[RS1], REG_NAMES[RS2]);
	}
	return 4 | DASMFLAG_SUPPORTED;
}


offs_t sparc_disassembler::dasm_ldst(char *buf, offs_t pc, UINT32 op) const
{
	if (m_version >= 9)
	{
		switch (OP3)
		{
		case 0x21: // Load floating-point state register
			if ((RD == 0) || (RD == 1))
			{
				print(buf, "%-*s[", m_op_field_width, (RD == 1) ? "ldx" : "ld");
				dasm_address(buf, op);
				print(buf, "],%%fsr");
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x25: // Store floating-point state register
			if ((RD == 0) || (RD == 1))
			{
				print(buf, "%-*s%%fsr,[", m_op_field_width, (RD == 1) ? "stx" : "st");
				dasm_address(buf, op);
				*buf++ = ']';
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x3c: // Compare and swap word in alternate space
		case 0x3e: // Compare and swap doubleword in alternate space
			{
				bool print_asi(true);
				const char *mnemonic((OP3 == 0x3e) ? "casxa" : "casa");
				if (!USEIMM)
				{
					if (ASI == 0x80)
					{
						print_asi = false;
						mnemonic = (OP3 == 0x3e) ? "casx" : "cas";
					}
					else if (ASI == 0x88)
					{
						print_asi = false;
						mnemonic = (OP3 == 0x3e) ? "casxl" : "casl";
					}
				}
				print(buf, "%-*s[%s]", m_op_field_width, mnemonic, REG_NAMES[RS1]);
				if (print_asi) dasm_asi(buf, op);
				print(buf, ",%s,%s", REG_NAMES[RS2], REG_NAMES[RD]);
				if (print_asi) dasm_asi_comment(buf, op);
			}
			return 4 | DASMFLAG_SUPPORTED;
		case 0x2d: // Prefetch data
		case 0x3d: // Prefetch data from alternate space
			{
				print(buf, "%-*s[", m_op_field_width, (OP3 == 0x3d) ? "prefetcha" : "prefetch");
				dasm_address(buf, op);
				*buf++ = ']';
				if (OP3 == 0x3d) dasm_asi(buf, op);
				const auto it(m_prftch_desc.find(RD));
				if (it != m_prftch_desc.end())  print(buf, ",%s", it->second.name);
				else                                print(buf, ",0x%02x", RD);
				if (OP3 == 0x3d) dasm_asi_comment(buf, op);
			}
			return 4 | DASMFLAG_SUPPORTED;
		}
	}
	else
	{
		switch (OP3)
		{
		case 0x21: // Load Floating-point State Register
		case 0x31: // Load Coprocessor State Register
			print(buf, "%-*s[", m_op_field_width, "ld");
			dasm_address(buf, op);
			print(buf, "],%%%csr", (OP3 == 0x31) ? 'c' : 'f');
			return 4 | DASMFLAG_SUPPORTED;
		case 0x25: // Store Floating-point State Register
		case 0x35: // Store Coprocessor State Register
			print(buf, "%-*s%%%csr,[", m_op_field_width, "st", (OP3 == 0x35) ? 'c' : 'f');
			dasm_address(buf, op);
			*buf++ = ']';
			return 4 | DASMFLAG_SUPPORTED;
		case 0x26: // Store Floating-point deferred-trap Queue
		case 0x36: // Store Coprocessor deferred-trap Queue
			print(buf, "%-*s%%%cq,[", m_op_field_width, "std", (OP3 == 0x36) ? 'c' : 'f');
			dasm_address(buf, op);
			*buf++ = ']';
			return 4 | DASMFLAG_SUPPORTED;
		}
	}

	const auto it(m_ldst_desc.find(OP3));
	if (it == m_ldst_desc.end())
		return dasm_invalid(buf, pc, op);

	if (it->second.alternate && USEIMM && (m_version < 9))
		return dasm_invalid(buf, pc, op);

	if (it->second.g0_synth && (RD == 0))
	{
		print(buf, "%-*s[", m_op_field_width, it->second.g0_synth);
		dasm_address(buf, op);
		*buf++ = ']';
		if (it->second.alternate)
		{
			dasm_asi(buf, op);
			dasm_asi_comment(buf, op);
		}
	}
	else
	{
		print(buf, "%-*s", m_op_field_width, it->second.mnemonic);
		if (it->second.rd_first)
		{
			if (it->second.rd_alt_reg)  print(buf, "%%%c%d,", it->second.rd_alt_reg, freg(RD, it->second.rd_shift));
			else                        print(buf, "%s,", REG_NAMES[RD]);
		}
		*buf++ = '[';
		dasm_address(buf, op);
		*buf++ = ']';
		if (it->second.alternate) dasm_asi(buf, op);
		if (!it->second.rd_first)
		{
			if (it->second.rd_alt_reg)  print(buf, ",%%%c%d", it->second.rd_alt_reg, freg(RD, it->second.rd_shift));
			else                        print(buf, ",%s", REG_NAMES[RD]);
		}
		if (it->second.alternate) dasm_asi_comment(buf, op);
	}
	return 4 | DASMFLAG_SUPPORTED;
}


void sparc_disassembler::dasm_address(char *&output, UINT32 op) const
{
	if (USEIMM)
	{
		if (RS1 == 0)       print(output, "0x%08x", SIMM13);
		else                print(output, "%s%c0x%04x", REG_NAMES[RS1], (SIMM13 < 0) ? '-' : '+', std::abs(SIMM13));
	}
	else
	{
		if (RS1 == 0)       print(output, "%s", REG_NAMES[RS2]);
		else if (RS2 == 0)  print(output, "%s", REG_NAMES[RS1]);
		else                print(output, "%s+%s", REG_NAMES[RS1], REG_NAMES[RS2]);
	}
}


void sparc_disassembler::dasm_asi(char *&output, UINT32 op) const
{
	if (USEIMM)
	{
		print(output, "%%asi");
	}
	else
	{
		const auto it(m_asi_desc.find(ASI));
		if ((it != m_asi_desc.end()) && it->second.name)
			print(output, "%s", it->second.name);
		else
			print(output, "0x%02x", ASI);
	}
}


void sparc_disassembler::dasm_asi_comment(char *&output, UINT32 op) const
{
	if (!USEIMM)
	{
		const auto it(m_asi_desc.find(ASI));
		if ((it != m_asi_desc.end()) && it->second.desc)
			print(output, " ! %s", it->second.desc);
	}
}


CPU_DISASSEMBLE( sparcv7 )
{
	UINT32 op = *reinterpret_cast<const UINT32 *>(oprom);
	return DASM_V7.dasm(buffer, pc, BIG_ENDIANIZE_INT32(op));
}

CPU_DISASSEMBLE( sparcv8 )
{
	UINT32 op = *reinterpret_cast<const UINT32 *>(oprom);
	return DASM_V8.dasm(buffer, pc, BIG_ENDIANIZE_INT32(op));
}

CPU_DISASSEMBLE( sparcv9 )
{
	UINT32 op = *reinterpret_cast<const UINT32 *>(oprom);
	return DASM_V9.dasm(buffer, pc, BIG_ENDIANIZE_INT32(op));
}
