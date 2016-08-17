// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
/*
    SPARC disassembler
*/

#include "emu.h"
#include "sparcdasm.h"
#include "sparcdefs.h"

#include <algorithm>
#include <cstdio>


namespace {
	INT32 get_disp16(UINT32 op) { return DISP19; }
	INT32 get_disp19(UINT32 op) { return DISP19; }
	INT32 get_disp22(UINT32 op) { return DISP19; }

	const char *bicc_comment(const sparc_debug_state *state, bool use_cc, offs_t pc, UINT32 op)
	{
		if (!state || (state->get_translated_pc() != pc)) return nullptr;
		auto const cc((use_cc && (BRCC & 0x2)) ? state->get_xcc() : state->get_icc());
		switch (COND)
		{
		case 0x0: return "will fall through";
		case 0x1: return (cc & 0x4) ? "will branch" : "will fall through";
		case 0x2: return ((cc & 0x04) | ((cc ^ (cc >> 2)) & 0x2)) ? "will branch" : "will fall through";
		case 0x3: return ((cc ^ (cc >> 2)) & 0x2) ? "will branch" : "will fall through";
		case 0x4: return (cc & 0x5) ? "will branch" : "will fall through";
		case 0x5: return (cc & 0x1) ? "will branch" : "will fall through";
		case 0x6: return (cc & 0x8) ? "will branch" : "will fall through";
		case 0x7: return (cc & 0x2) ? "will branch" : "will fall through";
		case 0x8: return "will branch";
		case 0x9: return (cc & 0x4) ? "will fall through" : "will branch";
		case 0xa: return ((cc & 0x04) | ((cc ^ (cc >> 2)) & 0x2)) ? "will fall through" : "will branch";
		case 0xb: return ((cc ^ (cc >> 2)) & 0x2) ? "will fall through" : "will branch";
		case 0xc: return (cc & 0x5) ? "will fall through" : "will branch";
		case 0xd: return (cc & 0x1) ? "will fall through" : "will branch";
		case 0xe: return (cc & 0x8) ? "will fall through" : "will branch";
		case 0xf: return (cc & 0x2) ? "will fall through" : "will branch";
		}
		return nullptr;
	}
	const char *bfcc_comment(const sparc_debug_state *state, bool use_cc, offs_t pc, UINT32 op)
	{
		if (!state || (state->get_translated_pc() != pc)) return nullptr;
		auto const fcc(state->get_fcc(use_cc ? BRCC : 0));
		switch (COND)
		{
		case 0x0: return "will fall through";
		case 0x1: return ((fcc == 1) || (fcc == 2) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0x2: return ((fcc == 1) || (fcc == 2)) ? "will branch" : "will fall through";
		case 0x3: return ((fcc == 1) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0x4: return (fcc == 1) ? "will branch" : "will fall through";
		case 0x5: return ((fcc == 2) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0x6: return (fcc == 2) ? "will branch" : "will fall through";
		case 0x7: return (fcc == 3) ? "will branch" : "will fall through";
		case 0x8: return "will branch";
		case 0x9: return (fcc == 0) ? "will branch" : "will fall through";
		case 0xa: return ((fcc == 0) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0xb: return ((fcc == 0) || (fcc == 2)) ? "will branch" : "will fall through";
		case 0xc: return ((fcc == 0) || (fcc == 2) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0xd: return ((fcc == 0) || (fcc == 1)) ? "will branch" : "will fall through";
		case 0xe: return ((fcc == 0) || (fcc == 1) || (fcc == 3)) ? "will branch" : "will fall through";
		case 0xf: return ((fcc == 0) || (fcc == 1) || (fcc == 2)) ? "will branch" : "will fall through";
		}
		return nullptr;
	}
	const char *bpr_comment(const sparc_debug_state *state, bool use_cc, offs_t pc, UINT32 op)
	{
		if (!state || (state->get_translated_pc() != pc)) return nullptr;
		const INT64 reg(state->get_reg_r(RS1));
		switch (COND)
		{
		case 1: return (reg == 0) ? "will branch" : "will fall through";
		case 2: return (reg <= 0) ? "will branch" : "will fall through";
		case 3: return (reg < 0) ? "will branch" : "will fall through";
		case 5: return (reg != 0) ? "will branch" : "will fall through";
		case 6: return (reg > 0) ? "will branch" : "will fall through";
		case 7: return (reg >= 0) ? "will branch" : "will fall through";
		}
		return nullptr;
	}
}

const char * const sparc_disassembler::REG_NAMES[32] = {
	"%g0", "%g1", "%g2", "%g3", "%g4", "%g5", "%g6", "%g7",
	"%o0", "%o1", "%o2", "%o3", "%o4", "%o5", "%o6", "%o7",
	"%l0", "%l1", "%l2", "%l3", "%l4", "%l5", "%l6", "%l7",
	"%i0", "%i1", "%i2", "%i3", "%i4", "%i5", "%i6", "%i7"
};

const sparc_disassembler::branch_desc sparc_disassembler::EMPTY_BRANCH_DESC = {
	nullptr, nullptr, 0, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BPCC_DESC = {
	&get_disp19, &bicc_comment, 6, true, true,
	{ "%icc", nullptr, "%xcc", nullptr },
	{
		"bn",    "be",    "ble",   "bl",    "bleu",  "bcs",   "bneg",  "bvs",
		"ba",    "bne",   "bg",    "bge",   "bgu",   "bcc",   "bpos",  "bvc"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BICC_DESC = {
	&get_disp22, &bicc_comment, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"bn",    "be",    "ble",   "bl",    "bleu",  "bcs",   "bneg",  "bvs",
		"ba",    "bne",   "bg",    "bge",   "bgu",   "bcc",   "bpos",  "bvc"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::BPR_DESC = {
	&get_disp16, &bpr_comment, 5, true, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		nullptr, "brz",   "brlez", "brlz",  nullptr, "brnz",  "brgz",  "brgez",
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::FBPFCC_DESC = {
	&get_disp19, &bfcc_comment, 6, true, true,
	{ "%fcc0", "%fcc1", "%fcc2", "%fcc3" },
	{
		"fbn",   "fbne",  "fblg",  "fbul",  "fbl",   "fbug",  "fbg",   "fbu",
		"fba",   "fbe",   "fbue",  "fbge",  "fbuge", "fble",  "fbule", "fbo"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::FBFCC_DESC = {
	&get_disp22, &bfcc_comment, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"fbn",   "fbne",  "fblg",  "fbul",  "fbl",   "fbug",  "fbg",   "fbu",
		"fba",   "fbe",   "fbue",  "fbge",  "fbuge", "fble",  "fbule", "fbo"
	}
};

const sparc_disassembler::branch_desc sparc_disassembler::CBCCC_DESC = {
	&get_disp22, nullptr, 6, false, false,
	{ nullptr, nullptr, nullptr, nullptr },
	{
		"cbn",   "cb123", "cb12",  "cb13",  "cb1",   "cb23",  "cb2",   "cb3",
		"cba",   "cb0",   "cb03",  "cb02",  "cb023", "cb01",  "cb013", "cb012"
	}
};

const sparc_disassembler::int_op_desc_map::value_type sparc_disassembler::V7_INT_OP_DESC[] = {
	{ 0x00, { false, "add",      nullptr } }, { 0x10, { false, "addcc",    nullptr } },
	{ 0x01, { true,  "and",      nullptr } }, { 0x11, { true,  "andcc",    "btst"  } },
	{ 0x02, { true,  "or",       nullptr } }, { 0x12, { true,  "orcc",     nullptr } },
	{ 0x03, { true,  "xor",      nullptr } }, { 0x13, { true,  "xorcc",    nullptr } },
	{ 0x04, { false, "sub",      nullptr } }, { 0x14, { false, "subcc",    "cmp"   } },
	{ 0x05, { true,  "andn",     nullptr } }, { 0x15, { true,  "andncc",   nullptr } },
	{ 0x06, { true,  "orn",      nullptr } }, { 0x16, { true,  "orncc",    nullptr } },
	{ 0x07, { true,  "xnor",     nullptr } }, { 0x17, { true,  "xnorcc",   nullptr } },
	{ 0x08, { false, "addx",     nullptr } }, { 0x18, { false, "addxcc",   nullptr } },
	{ 0x0c, { false, "subx",     nullptr } }, { 0x1c, { false, "subxcc",   nullptr } },

	{ 0x20, { false, "taddcc",   nullptr } },
	{ 0x21, { false, "tsubcc",   nullptr } },
	{ 0x22, { false, "taddcctv", nullptr } },
	{ 0x23, { false, "tsubcctv", nullptr } },
	{ 0x24, { false, "mulscc",   nullptr } },

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
	{  1, { true,  nullptr, nullptr } },
	{  2, { false, "%ccr",  "%ccr"  } },
	{  3, { false, "%asi",  "%asi"  } },
	{  4, { false, "%tick", nullptr } },
	{  5, { false, "%pc",   nullptr } },
	{  6, { false, "%fprs", "%fprs" } }
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

const sparc_disassembler::prftch_desc_map::value_type sparc_disassembler::V9_PRFTCH_DESC[] = {
	{ 0x00, { "#n_reads"   } },
	{ 0x01, { "#one_read"  } },
	{ 0x02, { "#n_writes"  } },
	{ 0x03, { "#one_write" } },
	{ 0x04, { "#page"      } }
};

const sparc_disassembler::vis_op_desc_map::value_type sparc_disassembler::VIS1_OP_DESC[] = {
	{ 0x000, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge8"       } },
	{ 0x002, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge8l"      } },
	{ 0x004, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge16"      } },
	{ 0x006, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge16l"     } },
	{ 0x008, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge32"      } },
	{ 0x00a, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge32l"     } },

	{ 0x010, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "array8"      } },
	{ 0x012, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "array16"     } },
	{ 0x014, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "array32"     } },
	{ 0x018, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  true,  "alignaddr"   } },
	{ 0x01a, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  true,  "alignaddrl"  } },

	{ 0x020, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmple16"    } },
	{ 0x022, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpne16"    } },
	{ 0x024, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmple32"    } },
	{ 0x026, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpne32"    } },
	{ 0x028, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpgt16"    } },
	{ 0x02a, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpeq16"    } },
	{ 0x02c, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpgt32"    } },
	{ 0x02e, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fcmpeq32"    } },

	{ 0x031, { vis_op_desc::Fs, vis_op_desc::Fd, vis_op_desc::Fd, false, "fmul8x16"    } },
	{ 0x033, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fd, false, "fmul8x16au"  } },
	{ 0x035, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fd, false, "fmul8x16al"  } },
	{ 0x036, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fmul8sux16"  } },
	{ 0x037, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fmul8ulx16"  } },
	{ 0x038, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fd, false, "fmuld8sux16" } },
	{ 0x039, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fd, false, "fmuld8ulx16" } },
	{ 0x03a, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpack32"     } },
	{ 0x03b, { vis_op_desc::X,  vis_op_desc::Fd, vis_op_desc::Fs, false, "fpack16"     } },
	{ 0x03d, { vis_op_desc::X,  vis_op_desc::Fd, vis_op_desc::Fs, false, "fpackfix"    } },
	{ 0x03e, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "pdist"       } },

	{ 0x048, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "faligndata"  } },
	{ 0x04b, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fd, false, "fpmerge"     } },
	{ 0x04d, { vis_op_desc::X,  vis_op_desc::Fs, vis_op_desc::Fd, false, "fexpand"     } },

	{ 0x050, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpadd16"     } },
	{ 0x051, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fpadd16s"    } },
	{ 0x052, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpadd32"     } },
	{ 0x053, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fpadd32s"    } },
	{ 0x054, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpsub16"     } },
	{ 0x055, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fpsub16s"    } },
	{ 0x056, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpsub32"     } },
	{ 0x057, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fpsub32s"    } },

	{ 0x060, { vis_op_desc::X,  vis_op_desc::X,  vis_op_desc::Fd, false, "fzero"       } },
	{ 0x061, { vis_op_desc::X,  vis_op_desc::X,  vis_op_desc::Fs, false, "fzeros"      } },
	{ 0x062, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fnor"        } },
	{ 0x063, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fnors"       } },
	{ 0x064, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fandnot2"    } },
	{ 0x065, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fandnot2s"   } },
	{ 0x066, { vis_op_desc::X,  vis_op_desc::Fd, vis_op_desc::Fd, false, "fnot2"       } },
	{ 0x067, { vis_op_desc::X,  vis_op_desc::Fs, vis_op_desc::Fs, false, "fnot2s"      } },
	{ 0x068, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fandnot1"    } },
	{ 0x069, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fandnot1s"   } },
	{ 0x06a, { vis_op_desc::Fd, vis_op_desc::X,  vis_op_desc::Fd, false, "fnot1"       } },
	{ 0x06b, { vis_op_desc::Fs, vis_op_desc::X,  vis_op_desc::Fs, false, "fnot1s"      } },
	{ 0x06c, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fxor"        } },
	{ 0x06d, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fxors"       } },
	{ 0x06e, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fnand"       } },
	{ 0x06f, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fnands"      } },

	{ 0x070, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fand"        } },
	{ 0x071, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fands"       } },
	{ 0x072, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fxnor"       } },
	{ 0x073, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fxnors"      } },
	{ 0x074, { vis_op_desc::Fd, vis_op_desc::X,  vis_op_desc::Fd, false, "fsrc1"       } },
	{ 0x075, { vis_op_desc::Fs, vis_op_desc::X,  vis_op_desc::Fs, false, "fsrc1s"      } },
	{ 0x076, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fornot2"     } },
	{ 0x077, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fornot2s"    } },
	{ 0x078, { vis_op_desc::X,  vis_op_desc::Fd, vis_op_desc::Fd, false, "fsrc2"       } },
	{ 0x079, { vis_op_desc::X,  vis_op_desc::Fs, vis_op_desc::Fs, false, "fsrc2s"      } },
	{ 0x07a, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fornot1"     } },
	{ 0x07b, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fornot1s"    } },
	{ 0x07c, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "for"         } },
	{ 0x07d, { vis_op_desc::Fs, vis_op_desc::Fs, vis_op_desc::Fs, false, "fors"        } },
	{ 0x07e, { vis_op_desc::X,  vis_op_desc::X,  vis_op_desc::Fd, false, "fone"        } },
	{ 0x07f, { vis_op_desc::X,  vis_op_desc::X,  vis_op_desc::Fs, false, "fones"       } },

	{ 0x080, { vis_op_desc::X,  vis_op_desc::X,  vis_op_desc::X,  false, "shutdown"    } }
};

const sparc_disassembler::state_reg_desc_map::value_type sparc_disassembler::VIS1_STATE_REG_DESC[] = {
	{ 19, { false,  "%gsr",  "%gsr"  } }
};

const sparc_disassembler::asi_desc_map::value_type sparc_disassembler::VIS1_ASI_DESC[] = {
	{ 0x2c, { "#ASI_NUCLEUS_QUAD_LDD_L", nullptr } },
	{ 0x70, { "#ASI_BLK_AIUP",           nullptr } },
	{ 0x71, { "#ASI_BLK_AIUS",           nullptr } },
	{ 0x78, { "#ASI_BLK_AIUPL",          nullptr } },
	{ 0x79, { "#ASI_BLK_AIUSL",          nullptr } },
	{ 0xc0, { "#ASI_PST8_P",             nullptr } },
	{ 0xc1, { "#ASI_PST8_S",             nullptr } },
	{ 0xc2, { "#ASI_PST16_P",            nullptr } },
	{ 0xc3, { "#ASI_PST16_S",            nullptr } },
	{ 0xc4, { "#ASI_PST32_P",            nullptr } },
	{ 0xc5, { "#ASI_PST32_S",            nullptr } },
	{ 0xc8, { "#ASI_PST8_PL",            nullptr } },
	{ 0xc9, { "#ASI_PST8_SL",            nullptr } },
	{ 0xca, { "#ASI_PST16_PL",           nullptr } },
	{ 0xcb, { "#ASI_PST16_SL",           nullptr } },
	{ 0xcc, { "#ASI_PST32_PL",           nullptr } },
	{ 0xcd, { "#ASI_PST32_SL",           nullptr } },
	{ 0xd0, { "#ASI_FL8_P",              nullptr } },
	{ 0xd1, { "#ASI_FL8_S",              nullptr } },
	{ 0xd2, { "#ASI_FL16_P",             nullptr } },
	{ 0xd3, { "#ASI_FL16_S",             nullptr } },
	{ 0xd8, { "#ASI_FL8_PL",             nullptr } },
	{ 0xd9, { "#ASI_FL8_SL",             nullptr } },
	{ 0xda, { "#ASI_FL16_PL",            nullptr } },
	{ 0xdb, { "#ASI_FL16_SL",            nullptr } },
	{ 0xe0, { "#ASI_BLK_COMMIT_P",       nullptr } },
	{ 0xe1, { "#ASI_BLK_COMMIT_S",       nullptr } },
	{ 0xf0, { "#ASI_BLK_P",              nullptr } },
	{ 0xf1, { "#ASI_BLK_S",              nullptr } },
	{ 0xf8, { "#ASI_BLK_PL",             nullptr } },
	{ 0xf9, { "#ASI_BLK_SL",             nullptr } }
};

const sparc_disassembler::vis_op_desc_map::value_type sparc_disassembler::VIS2_OP_DESC[] = {
	{ 0x001, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge8n"      } },
	{ 0x003, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge8ln"     } },
	{ 0x005, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge16n"     } },
	{ 0x007, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge16ln"    } },
	{ 0x009, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge32n"     } },
	{ 0x00b, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "edge32ln"    } },

	{ 0x019, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  true,  "bmask"       } },

	{ 0x04c, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "bshuffle"    } }
};

const sparc_disassembler::asi_desc_map::value_type sparc_disassembler::VIS2P_ASI_DESC[] = {
	{ 0x22, { "#ASI_TWINX_AIUP",         nullptr } },
	{ 0x23, { "#ASI_TWINX_AIUS",         nullptr } },
	{ 0x26, { "#ASI_TWINX_REAL",         nullptr } },
	{ 0x27, { "#ASI_TWINX_N",            nullptr } },
	{ 0x2a, { "#ASI_TWINX_AIUP_L",       nullptr } },
	{ 0x2b, { "#ASI_TWINX_AIUS_L",       nullptr } },
	{ 0x2e, { "#ASI_TWINX_REAL_L",       nullptr } },
	{ 0x2f, { "#ASI_TWINX_NL",           nullptr } },
	{ 0xe2, { "#ASI_TWINX_P",            nullptr } },
	{ 0xe3, { "#ASI_TWINX_S",            nullptr } },
	{ 0xea, { "#ASI_TWINX_PL",           nullptr } },
	{ 0xeb, { "#ASI_TWINX_SL",           nullptr } }
};

const sparc_disassembler::fpop1_desc_map::value_type sparc_disassembler::VIS3_FPOP1_DESC[] = {
	{ 0x051, { true,  false, false, false, "fnadds"  } },
	{ 0x052, { true,  true,  true,  true,  "fnaddd"  } },
	{ 0x059, { true,  false, false, false, "fnmuls"  } },
	{ 0x05a, { true,  true,  true,  true,  "fnmuld"  } },

	{ 0x061, { true,  false, false, false, "fhadds"  } },
	{ 0x062, { true,  true,  true,  true,  "fhaddd"  } },
	{ 0x065, { true,  false, false, false, "fhsubs"  } },
	{ 0x066, { true,  true,  true,  true,  "fhsubd"  } },

	{ 0x071, { true,  false, false, false, "fnhadds" } },
	{ 0x072, { true,  true,  true,  true,  "fnhaddd" } },
	{ 0x079, { true,  false, false, true,  "fnsmuld" } }
};

const sparc_disassembler::vis_op_desc_map::value_type sparc_disassembler::VIS3_OP_DESC[] = {
	{ 0x011, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "addxc"          } },
	{ 0x013, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "addxccc"        } },
	{ 0x016, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "umulxhi"        } },
	{ 0x017, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::R,  false, "lzcnt"          } },
	{ 0x01b, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::X,  false, "cmask8"         } },
	{ 0x01d, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::X,  false, "cmask16"        } },
	{ 0x01f, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::X,  false, "cmask32"        } },

	{ 0x021, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsll16"         } },
	{ 0x023, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsrl16"         } },
	{ 0x025, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsll32"         } },
	{ 0x027, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsrl32"         } },
	{ 0x029, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fslas16"        } },
	{ 0x02b, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsra16"         } },
	{ 0x02d, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fslas32"        } },
	{ 0x02f, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fsra32"         } },

	{ 0x03f, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "pdistn"         } },

	{ 0x040, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fmean16"        } },
	{ 0x044, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fchksm16"       } },

	{ 0x115, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "xmulx"          } },
	{ 0x116, { vis_op_desc::R,  vis_op_desc::R,  vis_op_desc::R,  false, "xmulxhi"        } }
};

const sparc_disassembler::vis_op_desc_map::value_type sparc_disassembler::VIS3B_OP_DESC[] = {
	{ 0x042, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpadd64"        } },
	{ 0x046, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::Fd, false, "fpsub64"        } },

	{ 0x110, { vis_op_desc::X,  vis_op_desc::Fd, vis_op_desc::R,  false, "movdtox"        } },
	{ 0x111, { vis_op_desc::X,  vis_op_desc::Fs, vis_op_desc::R,  false, "movstouw"       } },
	{ 0x113, { vis_op_desc::X,  vis_op_desc::Fs, vis_op_desc::R,  false, "movstosw"       } },
	{ 0x118, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::Fd, false, "movxtod"        } },
	{ 0x119, { vis_op_desc::X,  vis_op_desc::R,  vis_op_desc::Fs, false, "movwtos"        } },

	{ 0x120, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fpcmpule8"      } },
	{ 0x122, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fpcmpune8"      } },
	{ 0x128, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fpcmpugt8"      } },
	{ 0x12a, { vis_op_desc::Fd, vis_op_desc::Fd, vis_op_desc::R,  false, "fpcmpueq8"      } }
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

template <typename T> inline void sparc_disassembler::add_vis_op_desc(const T &desc)
{
	for (const auto &it : desc)
		m_vis_op_desc.insert(it);
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


sparc_disassembler::sparc_disassembler(const sparc_debug_state *state, unsigned version)
	: sparc_disassembler(state, version, vis_none)
{
}

sparc_disassembler::sparc_disassembler(const sparc_debug_state *state, unsigned version, vis_level vis)
	: m_version(version)
	, m_vis_level(vis)
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
	, m_vis_op_desc()
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

	switch (m_vis_level)
	{
	case vis_3b:
		add_vis_op_desc(VIS3B_OP_DESC);
	case vis_3:
		add_fpop1_desc(VIS3_FPOP1_DESC);
		add_vis_op_desc(VIS3_OP_DESC);
	case vis_2p:
		add_asi_desc(VIS2P_ASI_DESC);
	case vis_2:
		add_vis_op_desc(VIS2_OP_DESC);
	case vis_1:
		m_op_field_width = std::max(m_op_field_width, 12);
		add_vis_op_desc(VIS1_OP_DESC);
		add_state_reg_desc(VIS1_STATE_REG_DESC);
		add_asi_desc(VIS1_ASI_DESC);
		if (m_vis_level >= vis_3)
		{
			m_vis_op_desc.find(0x020)->second.mnemonic = "fpcmple16";
			m_vis_op_desc.find(0x022)->second.mnemonic = "fpcmpne16";
			m_vis_op_desc.find(0x024)->second.mnemonic = "fpcmple32";
			m_vis_op_desc.find(0x026)->second.mnemonic = "fpcmpne32";
			m_vis_op_desc.find(0x028)->second.mnemonic = "fpcmpgt16";
			m_vis_op_desc.find(0x02a)->second.mnemonic = "fpcmpeq16";
			m_vis_op_desc.find(0x02c)->second.mnemonic = "fpcmpgt32";
			m_vis_op_desc.find(0x02e)->second.mnemonic = "fpcmpeq32";

			m_vis_op_desc.find(0x060)->second.mnemonic = "fzerod";
			m_vis_op_desc.find(0x062)->second.mnemonic = "fnord";
			m_vis_op_desc.find(0x064)->second.mnemonic = "fandnot2d";
			m_vis_op_desc.find(0x066)->second.mnemonic = "fnot2d";
			m_vis_op_desc.find(0x068)->second.mnemonic = "fandnot1d";
			m_vis_op_desc.find(0x06a)->second.mnemonic = "fnot1d";
			m_vis_op_desc.find(0x06c)->second.mnemonic = "fxord";
			m_vis_op_desc.find(0x06e)->second.mnemonic = "fnandd";

			m_vis_op_desc.find(0x070)->second.mnemonic = "fandd";
			m_vis_op_desc.find(0x072)->second.mnemonic = "fxnord";
			m_vis_op_desc.find(0x074)->second.mnemonic = "fsrc1d";
			m_vis_op_desc.find(0x076)->second.mnemonic = "fornot2d";
			m_vis_op_desc.find(0x078)->second.mnemonic = "fsrc2d";
			m_vis_op_desc.find(0x07a)->second.mnemonic = "fornot1d";
			m_vis_op_desc.find(0x07c)->second.mnemonic = "ford";
			m_vis_op_desc.find(0x07e)->second.mnemonic = "foned";
		}
	case vis_none:
		break;
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
			if (USEIMM)
			{
				if (RS1 == RD)
				{
					if (SIMM13 == 1)    print(buf, "%-*s%s", m_op_field_width, "dec", REG_NAMES[RD]);
					else                print(buf, "%-*s%d,%s", m_op_field_width, "dec", SIMM13, REG_NAMES[RD]);
					return 4 | DASMFLAG_SUPPORTED;
				}
			}
			else
			{
				if (RS1 == 0)
				{
					if (RS2 == RD)  print(buf, "%-*s%s", m_op_field_width, "neg", REG_NAMES[RD]);
					else            print(buf, "%-*s%s,%s", m_op_field_width, "neg", REG_NAMES[RS2], REG_NAMES[RD]);
					return 4 | DASMFLAG_SUPPORTED;
				}
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
		case 0x12:
			if (!USEIMM && (RS1 == 0) && (RD == 0))
			{
				print(buf, "%-*s%s", m_op_field_width, "tst", REG_NAMES[RS2]);
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x14:
			if (USEIMM && (RS1 == RD) && (RD != 0))
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
				if (RS1 == 0)
				{
					if (USEIMM) print(buf, "%-*s0x%08x,%%psr", m_op_field_width, "mov", SIMM13);
					else        print(buf, "%-*s%s,%%psr", m_op_field_width, "mov", REG_NAMES[RS2]);
				}
				else
				{
					if (USEIMM) print(buf, "%-*s%s,0x%08x,%%psr", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
					else        print(buf, "%-*s%s,%s,%%psr", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				}
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
				if (RS1 == 0)
				{
					if (USEIMM) print(buf, "%-*s0x%08x,%%wim", m_op_field_width, "mov", SIMM13);
					else        print(buf, "%-*s%s,%%wim", m_op_field_width, "mov", REG_NAMES[RS2]);
				}
				else
				{
					if (USEIMM) print(buf, "%-*s%s,0x%08x,%%wim", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
					else        print(buf, "%-*s%s,%s,%%wim", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				}
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x33:
			if (m_version <= 8)
			{
				if (RS1 == 0)
				{
					if (USEIMM) print(buf, "%-*s0x%08x,%%tbr", m_op_field_width, "mov", SIMM13);
					else        print(buf, "%-*s%s,%%tbr", m_op_field_width, "mov", REG_NAMES[RS2]);
				}
				else
				{
					if (USEIMM) print(buf, "%-*s%s,0x%08x,%%tbr", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
					else        print(buf, "%-*s%s,%s,%%tbr", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
				}
				return 4 | DASMFLAG_SUPPORTED;
			}
			break;
		case 0x34:
			return dasm_fpop1(buf, pc, op);
		case 0x35:
			return dasm_fpop2(buf, pc, op);
		case 0x36:
			return dasm_impdep1(buf, pc, op);
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
				if (it->second.g0_synth && (RD == 0))
				{
					if (!USEIMM)
						print(buf, "%-*s%s,%s", m_op_field_width, it->second.g0_synth, REG_NAMES[RS1], REG_NAMES[RS2]);
					else if (it->second.hex_imm)
						print(buf, "%-*s%s,0x%08x", m_op_field_width, it->second.g0_synth, REG_NAMES[RS1], SIMM13);
					else
						print(buf, "%-*s%s,%d", m_op_field_width, it->second.g0_synth, REG_NAMES[RS1], SIMM13);
				}
				else
				{
					if (!USEIMM)
						print(buf, "%-*s%s,%s,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], REG_NAMES[RS2], REG_NAMES[RD]);
					else if (it->second.hex_imm)
						print(buf, "%-*s%s,0x%08x,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], SIMM13, REG_NAMES[RD]);
					else
						print(buf, "%-*s%s,%d,%s", m_op_field_width, it->second.mnemonic, REG_NAMES[RS1], SIMM13, REG_NAMES[RD]);
				}
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

	print(ptr, "%s%s%s", mnemonic, ANNUL ? ",a" : "", (desc.use_pred && !PRED) ? ",pn" : "");
	pad_op_field(buf, ptr);
	if (desc.use_cc) print(ptr, "%s,", desc.reg_cc[BRCC]);
	if (OP2 == 3) print(ptr, "%s,", REG_NAMES[RS1]);
	const INT32 disp(desc.get_disp(op));
	print(ptr, "%%pc%c0x%0*x ! 0x%08x", (disp < 0) ? '-' : '+', desc.disp_width, std::abs(disp), pc + disp);
	//const char * const comment(desc.get_comment ? desc.get_comment(m_state, desc.use_cc, pc, op) : nullptr);
	//if (comment) print(ptr, " - %s", comment);

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
		if (RS1 == 0)
		{
			if (USEIMM) print(buf, "%-*s%d,%%y", m_op_field_width, "mov", SIMM13);
			else        print(buf, "%-*s%s,%%y", m_op_field_width, "mov", REG_NAMES[RS2]);
		}
		else
		{
			if (USEIMM) print(buf, "%-*s%s,%08x,%%y", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13);
			else        print(buf, "%-*s%s,%s,%%y", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2]);
		}
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
					if (RS1 == 0)
					{
						if (USEIMM) print(buf, "%-*s%d,%s", m_op_field_width, "mov", SIMM13, it->second.write_name);
						else        print(buf, "%-*s%s,%s", m_op_field_width, "mov", REG_NAMES[RS2], it->second.write_name);
					}
					else
					{
						if (USEIMM) print(buf, "%-*s%s,%08x,%s", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13, it->second.write_name);
						else        print(buf, "%-*s%s,%s,%s", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2], it->second.write_name);
					}
				}
				else
				{
					const char * const comment((RD < 16) ? "reserved" : "implementation-dependent");
					if (RS1 == 0)
					{
						if (USEIMM) print(buf, "%-*s%d,%%asr%d ! %s", m_op_field_width, "mov", SIMM13, RD, comment);
						else        print(buf, "%-*s%s,%%asr%d ! %s", m_op_field_width, "mov", REG_NAMES[RS2], RD, comment);
					}
					else
					{
						if (USEIMM) print(buf, "%-*s%s,%08x,%%asr%d ! %s", m_op_field_width, "wr", REG_NAMES[RS1], SIMM13, RD, comment);
						else        print(buf, "%-*s%s,%s,%%asr%d ! %s", m_op_field_width, "wr", REG_NAMES[RS1], REG_NAMES[RS2], RD, comment);
					}
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
		default: mnemonic = nullptr; shift = false; break;
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


offs_t sparc_disassembler::dasm_impdep1(char *buf, offs_t pc, UINT32 op) const
{
	const auto it(m_vis_op_desc.find(OPF));
	if (it != m_vis_op_desc.end())
	{
		print(buf, "%-*s", m_op_field_width, it->second.mnemonic);
		bool args(false);
		if (it->second.collapse && !RS1)
		{
			dasm_vis_arg(buf, args, it->second.rs2, RS2);
		}
		else if (it->second.collapse && !RS2)
		{
			dasm_vis_arg(buf, args, it->second.rs1, RS1);
		}
		else
		{
			dasm_vis_arg(buf, args, it->second.rs1, RS1);
			dasm_vis_arg(buf, args, it->second.rs2, RS2);
		}
		dasm_vis_arg(buf, args, it->second.rd, RD);
		return 4 | DASMFLAG_SUPPORTED;
	}

	switch (OPF)
	{
	case 0x081:
		if (m_vis_level >= vis_2)
		{
			print(buf, "%-*s0x%x", m_op_field_width, "siam", IAMODE);
			return 4 | DASMFLAG_SUPPORTED;
		}
		break;
	case 0x151:
	case 0x152:
		if (m_vis_level >= vis_3)
		{
			const bool shift(OPF == 0x152);
			print(buf, "%-*s%%fcc%d,%%f%d,%%f%d", m_op_field_width, (shift) ? "flcmpd" : "flcmps", RD & 3, freg(RS1, shift), freg(RS2, shift));
			return 4 | DASMFLAG_SUPPORTED;
		}
		break;
	}

	// TODO: driver hook for other kinds of coprocessor?

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
			else if ((RD == 3) && (m_vis_level >= vis_3b))
			{
				print(buf, "%-*s[", m_op_field_width, "ldx");
				dasm_address(buf, op);
				print(buf, "],%%efsr");
			}
			break;
		case 0x25: // Store floating-point state register
			if ((RD == 0) || (RD == 1))
			{
				print(buf, "%-*s%%fsr,[", m_op_field_width, (RD == 1) ? "stx" : "st");
				dasm_address(buf, op);
				*buf++ = ']';
				*buf = '\0';
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
				else                            print(buf, ",0x%02x", RD);
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
			*buf = '\0';
			return 4 | DASMFLAG_SUPPORTED;
		case 0x26: // Store Floating-point deferred-trap Queue
		case 0x36: // Store Coprocessor deferred-trap Queue
			print(buf, "%-*s%%%cq,[", m_op_field_width, "std", (OP3 == 0x36) ? 'c' : 'f');
			dasm_address(buf, op);
			*buf++ = ']';
			*buf = '\0';
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
		*buf = '\0';
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
		*buf = '\0';
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


void sparc_disassembler::dasm_vis_arg(char *&output, bool &args, vis_op_desc::arg fmt, UINT32 reg) const
{
	switch (fmt)
	{
	case vis_op_desc::X:
		break;
	case vis_op_desc::R:
		print(output, args ? ",%s" : "%s", REG_NAMES[reg]);
		args = true;
		break;
	case vis_op_desc::Fs:
	case vis_op_desc::Fd:
		print(output, args ? ",%%f%d" : "%%f%d", freg(reg, (fmt == vis_op_desc::Fd)));
		args = true;
		break;
	};
}
