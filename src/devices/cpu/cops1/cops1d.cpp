// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor COPS(MM57 MCU family) disassembler

*/

#include "emu.h"
#include "cops1d.h"

// constructor

cops1_common_disassembler::cops1_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t cops1_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

enum cops1_common_disassembler::e_mnemonics : unsigned
{
	mILL,
	mAD, mADD, mSUB, mCOMP, m0TA, mADX, mHXA, mTAM, mSC, mRSC, mTC,
	mTIN, mTF, mTKB, mTIR,
	mBTD, mDSPA, mDSPS, mAXO, mLDF, mREAD,
	mGO, mCALL, mRET, mRETS, mLG, mLGCALL, mNOP,
	mEXC, mEXCM, mEXCP, mMTA, mLM,
	mSM1, mSM2, mSM4, mSM8, mRSM1, mRSM2, mRSM4, mRSM8, mTM,
	mLB, mLBL, mATB, mBTA, mHXBR
};

const char *const cops1_common_disassembler::s_name[] =
{
	"?",
	"AD", "ADD", "SUB", "COMP", "0TA", "ADX", "HXA", "TAM", "SC", "RSC", "TC",
	"TIN", "TF", "TKB", "TIR",
	"BTD", "DSPA", "DSPS", "AXO", "LDF", "READ",
	"GO", "CALL", "RET", "RETS", "LG/GO", "LG/CALL", "NOP",
	"EXC", "EXCM", "EXCP", "MTA", "LM",
	"SM", "SM", "SM", "SM", "RSM", "RSM", "RSM", "RSM", "TM",
	"LB", "LBL", "ATB", "BTA", "HXBR"
};

// bitmask for opcode parameter
const u8 cops1_common_disassembler::s_bits[] =
{
	0,
	0, 0, 0, 0, 0, 0x0f, 0, 0, 0, 0, 0,
	0, 0x30, 0, 0,
	0, 0, 0, 0, 0xff, 0,
	0x3f, 0x3f, 0, 0, 0xff, 0xff, 0,
	0x30, 0x30, 0x30, 0x30, 0x0f,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x3f, 0xff, 0, 0, 0
};

const u32 cops1_common_disassembler::s_flags[] =
{
	0,
	0, STEP_COND, STEP_COND, 0, 0, STEP_COND, 0, STEP_COND, 0, 0, STEP_COND,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND,
	0, 0, 0, 0, 0, 0,
	0, STEP_OVER, STEP_OUT, STEP_OUT, 0, STEP_OVER, 0,
	0, STEP_COND, STEP_COND, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};


// common disasm

offs_t cops1_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];
	int len = 1;

	// get parameter
	u8 mask = s_bits[instr];
	u16 param = op & mask;

	if (mask == 0x30)
	{
		mask >>= 4;
		param >>= 4;
	}

	// 2-byte instructions
	else if (mask == 0xff)
	{
		pc = increment_pc(pc);
		u8 arg = params.r8(pc);
		len++;

		if (instr == mLG)
		{
			// bit 6 indicates GO or CALL
			if (~arg & 0x40)
				instr++;

			param = (~param << 7 & 0x780) | (arg >> 1 & 0x40) | (arg & 0x3f);
		}
		else
			param = arg;
	}

	// disassemble it
	util::stream_format(stream, "%-8s", s_name[instr]);
	if (mask > 0)
	{
		if (mask < 16)
		{
			// SM and RSM param is scrambled
			if (instr >= mSM1 && instr <= mSM8)
				param = instr - mSM1;
			else if (instr >= mRSM1 && instr <= mRSM8)
				param = instr - mRSM1;

			// memory bit opcodes are 1,2,4,8
			if (instr >= mSM1 && instr <= mTM)
				param = 1 << param;

			// TF is 1,2,3,4
			else if (instr == mTF)
				param++;

			// EXC type instructions omit param if it's 0
			if (!(param == 0 && (instr == mEXC || instr == mEXCM || instr == mEXCP || instr == mMTA)))
				util::stream_format(stream, "%d", param);
		}
		else
		{
			// exception for LG
			if (instr == mLG || instr == mLGCALL)
				util::stream_format(stream, "$%02X,$%02X", param >> 6, param & 0x3f);

			// exception for LB/LBL
			else if (instr == mLB || instr == mLBL)
			{
				// LB x,10 is 0 by default
				if (instr == mLB && (param & 0xf) == 10)
					param &= 0x30;

				util::stream_format(stream, "%d,%d", param >> 4, param & 0xf);
			}

			else
				util::stream_format(stream, "$%02X", param);
		}
	}

	return len | s_flags[instr] | SUPPORTED;
}


// MM5799 disasm

const u8 mm5799_disassembler::mm5799_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mHXBR, mADD,  mSC,   mTF,   mTIR,  mMTA,  mEXC,  mEXCM, mEXCP, mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 0
	mDSPA, mDSPS, mAD,   mLBL,  mTF,   mTKB,  mMTA,  mEXC,  mEXCM, mEXCP, mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 1
	mCOMP, mAXO,  mSUB,  mRSC,  mTF,   mBTD,  mMTA,  mEXC,  mEXCM, mEXCP, mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 2
	m0TA,  mHXA,  mTAM,  mLDF,  mREAD, mTIN,  mMTA,  mEXC,  mEXCM, mEXCP, mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 3

	mRET,  mRETS, mRSM8, mBTA,  mTM,   mTM,   mTM,   mTM,   mRSM1, mSM1,  mSM8,  mRSM4, mRSM2, mTC,   mSM2,  mSM4,  // 4
	mATB,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 5
	mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   mLG,   // 6
	mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   mLM,   // 7

	mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, // 8
	mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, // 9
	mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, // A
	mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, // B

	mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   // C
	mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   // D
	mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   // E
	mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO,   mGO    // F
};

offs_t mm5799_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(mm5799_opmap, stream, pc, opcodes, params);
}
