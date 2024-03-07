// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell PPS-4/1 disassembler

*/

#include "emu.h"
#include "pps41d.h"

// constructor

pps41_common_disassembler::pps41_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t pps41_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

enum pps41_common_disassembler::e_mnemonics : unsigned
{
	// MM76/shared
	mILL /* 0! */,
	mXAB, mLBA, mLB, mEOB2,
	mSB, mRB, mSKBF,
	mXAS, mLSA,
	mL, mX, mXDSK, mXNSK,
	mA, mAC, mACSK, mASK, mCOM, mRC, mSC, mSKNC, mLAI, mAISK,
	mRT, mRTSK, mT, mNOP, mTL, mTM, mTML, mTR,
	mSKMEA, mSKBEI, mSKAEI,
	mSOS, mROS, mSKISL, mIBM, mOB, mIAM, mOA, mIOS, mI1, mI2C, mINT1H, mDIN1, mINT0L, mDIN0, mSEG1, mSEG2,

	// MM78 differences
	mINT0H, mINT1L, mSAG, mEOB3, mTAB,
	mI1SK, mIX, mOX, mLXA, mXAX, mIOA,
	mTLB, mTMLB
};

const char *const pps41_common_disassembler::s_name[] =
{
	// MM76
	"?",
	"XAB", "LBA", "LB", "EOB",
	"SB", "RB", "SKBF",
	"XAS", "LSA",
	"L", "X", "XDSK", "XNSK",
	"A", "AC", "ACSK", "ASK", "COM", "RC", "SC", "SKNC", "LAI", "AISK",
	"RT", "RTSK", "T", "NOP", "TL", "TM", "TML", "TR",
	"SKMEA", "SKBEI", "SKAEI",
	"SOS", "ROS", "SKISL", "IBM", "OB", "IAM", "OA", "IOS", "I1", "I2C", "INT1H", "DIN1", "INT0L", "DIN0", "SEG1", "SEG2",

	// MM78
	"INT0H", "INT1L", "SAG", "EOB", "TAB",
	"I1SK", "IX", "OX", "LXA", "XAX", "IOA",
	"TLB", "TMLB"
};

// number of bits per opcode parameter
// note: d4 means hex, d5 means inverted, d6 means extended, d7 means double extended
const u8 pps41_common_disassembler::s_bits[] =
{
	// MM76
	0,
	0, 0, 4, 2,
	2, 2, 2,
	0, 0,
	2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 4, 4,
	0, 0, 0x36, 0, 0x76, 0x36, 0x76, 0x34,
	0, 0x40, 0x60,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	// MM78
	0, 0, 0, 3, 0,
	0, 0, 0, 0, 0, 0,
	0xf6, 0xf6
};

const u32 pps41_common_disassembler::s_flags[] =
{
	// MM76
	0,
	0, 0, 0, 0,
	0, 0, STEP_COND,
	0, 0,
	0, 0, STEP_COND, STEP_COND,
	0, 0, STEP_COND, STEP_COND, 0, 0, 0, STEP_COND, 0, STEP_COND,
	STEP_OUT, STEP_OUT, 0, 0, 0, STEP_OVER, STEP_OVER, 0,
	STEP_COND, STEP_COND, STEP_COND,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STEP_COND, STEP_COND, STEP_COND, STEP_COND, 0, 0,

	// MM78
	STEP_COND, STEP_COND, 0, 0, 0,
	STEP_COND, 0, 0, 0, 0, 0,
	0, STEP_OVER
};


// common disasm

offs_t pps41_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];

	// get parameter
	u8 bits = s_bits[instr];
	u16 mask = (1 << (bits & 0xf)) - 1;
	bits &= 0xf0;

	u16 param = op & mask;
	if (bits & 0x20)
		param ^= mask;

	// memory bit opcodes are 1,2,3,4
	if (instr == mSB || instr == mRB || instr == mSKBF)
		param++;

	// disassemble it
	util::stream_format(stream, "%-8s", s_name[instr]);
	if (mask > 0)
	{
		if (bits & 0x10)
			util::stream_format(stream, (mask & ~0xf) ? "$%02X" : "$%X", param);
		else
			util::stream_format(stream, "%d", param);
	}

	return 1 | s_flags[instr] | SUPPORTED;
}


// MM76 disasm

const u8 mm76_disassembler::mm76_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mSKNC, mRT,   mRTSK, mINT0L,mINT1H,mDIN1, mDIN0, mSKBF, mSKBF, mSKBF, mSKBF, mSC,   mRC,   mSEG1, mSEG2, // 0
	mSB,   mSB,   mSB,   mSB,   mRB,   mRB,   mRB,   mRB,   mOA,   mOB,   mIAM,  mIBM,  mEOB2, mEOB2, mEOB2, mEOB2, // 1
	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 2
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 3

	mAC,   mACSK, mA,    mASK,  mLBA,  mCOM,  mXAB,  mSKMEA,mILL,  mILL,  mI1,   mI2C,  mLSA,  mIOS,  mXAS,  mILL,  // 4
	mL,    mL,    mL,    mL,    mXNSK, mXNSK, mXNSK, mXNSK, mX,    mX,    mX,    mX,    mXDSK, mXDSK, mXDSK, mXDSK, // 5
	mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, // 6
	mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  // 7

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // 8
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // 9
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // A
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // B

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // C
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // D
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // E
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT     // F
};

offs_t mm76_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm76_opmap, stream, pc, opcodes, params);
}


// MM78 disasm

const u8 mm78_disassembler::mm78_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mSKISL,mSKNC, mINT0H,mINT1L,mRC,   mSC,   mSAG,  mEOB3, mEOB3, mEOB3, mEOB3, mEOB3, mEOB3, mEOB3, mEOB3, // 0
	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 1
	mSB,   mSB,   mSB,   mSB,   mRB,   mRB,   mRB,   mRB,   mSKBF, mSKBF, mSKBF, mSKBF, mTAB,  mIOS,  mRTSK, mRT,   // 2
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 3

	mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  // 4
	mL,    mL,    mL,    mL,    mXNSK, mXNSK, mXNSK, mXNSK, mXDSK, mXDSK, mXDSK, mXDSK, mX,    mX,    mX,    mX,    // 5
	mI1SK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, mAISK, // 6
	mSOS,  mROS,  mIX,   mOX,   mXAS,  mLXA,  mLBA,  mCOM,  mI2C,  mXAX,  mXAB,  mIOA,  mAC,   mACSK, mA,    mSKMEA,// 7

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // 8
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // 9
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // A
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // B

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // C
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // D
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // E
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT     // F
};

offs_t mm78_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm78_opmap, stream, pc, opcodes, params);
}
