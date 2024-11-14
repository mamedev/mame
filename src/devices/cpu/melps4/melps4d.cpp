// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family disassembler

  Not counting the extra opcodes for peripherals (eg. timers, A/D),
  each MCU in the series has small differences in the opcode map.

*/

#include "emu.h"
#include "melps4d.h"


// common lookup tables

enum melps4_disassembler::e_mnemonics : unsigned
{
	mILL,
	mTAB, mTBA, mTAY, mTYA, mTEAB, mTABE, mTEPA, mTXA, mTAX,
	mLXY, mLZ, mINY, mDEY, mLCPS, mSADR,
	mTAM, mXAM, mXAMD, mXAMI,
	mLA, mAM, mAMC, mAMCS, mA, mSC, mRC, mSZC, mCMA, mRL, mRR,
	mSB, mRB, mSZB, mSEAM, mSEY,
	mTLA, mTHA, mTAJ, mXAL, mXAH, mLC7, mDEC, mSHL, mRHL, mCPA, mCPAS, mCPAE, mSZJ,
	mT1AB, mTRAB, mT2AB, mTAB1, mTABR, mTAB2, mTVA, mTWA, mSNZ1, mSNZ2,
	mBA, mSP, mB, mBM, mRT, mRTS, mRTI,
	mCLD, mCLS, mCLDS, mSD, mRD, mSZD, mOSAB, mOSPA, mOSE, mIAS, mOFA, mIAF, mOGA, mIAK, mSZK, mSU, mRU,
	mEI, mDI, mINTH, mINTL, mNOP
};

const char *const melps4_disassembler::s_name[] =
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
const u8 melps4_disassembler::s_bits[] =
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

const u32 melps4_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, STEP_COND, STEP_COND, 0, 0,
	0, 0, STEP_COND, STEP_COND,
	0, 0, 0, STEP_COND, STEP_COND, 0, 0, STEP_COND, 0, 0, 0,
	0, 0, STEP_COND, STEP_COND, STEP_COND,
	0, 0, 0, 0, 0, 0, STEP_COND, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, STEP_COND, STEP_COND,
	0, 0, 0, STEP_OVER, STEP_OUT, STEP_OUT, STEP_OUT,
	0, 0, 0, 0, 0, STEP_COND, 0, 0, 0, 0, 0, 0, 0, 0, STEP_COND, 0, 0,
	0, 0, 0, 0, 0
};



// M58846 disasm

const u8 melps4_disassembler::m58846_opmap[0xc0] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mBA,   mINY,  mDEY,  mDI,   mEI,   mRU,   mSU,   0,     mTABE, mAM,   mOSE,  mTYA,  0,     0,     mCMA,  // 0x
	mCLS,  mCLDS, 0,     mCLD,  mRD,   mSD,   mTEPA, mOSPA, mRL,   mRR,   mTEAB, mOSAB, mTBA,  mTAY,  mTAB,  0,     // 1x
	mSZB,  mSZB,  mSZB,  mSZB,  0,     0,     mSEAM, 0,     0,     0,     0,     mSZD,  0,     0,     0,     mSZC,  // 2x
	mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  mSEY,  // 3x

	mLCPS, mLCPS, 0,     mAMC,  mRT,   mRTS,  mRTI,  0,     mRC,   mSC,   mLZ,   mLZ,   mSB,   mSB,   mSB,   mSB,   // 4x
	0,     0,     0,     mAMCS, mIAS,  mIAS,  0,     mIAK,  mSZK,  mSZK,  mSZK,  mSZK,  mRB,   mRB,   mRB,   mRB,   // 5x
	mXAM,  mXAM,  mXAM,  mXAM,  mTAM,  mTAM,  mTAM,  mTAM,  mXAMD, mXAMD, mXAMD, mXAMD, mXAMI, mXAMI, mXAMI, mXAMI, // 6x
	mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   mSP,   // 7x

	0,     mOFA,  mSNZ1, mSNZ2, mOGA,  mT2AB, mTVA,  0,     0,     0,     mTAB2, 0,     mIAF,  0,     0,     0,     // 8x
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     // 9x
	mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    mA,    // Ax
	mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA,   mLA    // Bx
};

offs_t melps4_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 op = opcodes.r16(pc) & 0x1ff;

	// get opcode
	u8 instr;
	if (op >= 0x180)
		instr = mB;
	else if (op >= 0x100)
		instr = mBM;
	else if (op >= 0xc0)
		instr = mLXY;
	else
		instr = m58846_opmap[op];

	u32 flags = s_flags[instr];
	util::stream_format(stream, "%-6s", s_name[instr]);

	// get immediate param
	u8 bits = s_bits[instr];

	// special case for LXY x,y
	if (instr == mLXY)
	{
		u8 x = op >> 4 & 3;
		u8 y = op & 0xf;
		util::stream_format(stream, "%d,%d", x, y);
	}
	else if (bits > 0)
	{
		u8 param = op & ((1 << bits) - 1);

		if (instr == mA && param == 6)
			flags &= ~STEP_COND;

		if (bits > 4)
			util::stream_format(stream, "$%02X", param);
		else
			util::stream_format(stream, "%d", param);
	}

	return 1 | flags | SUPPORTED;
}
