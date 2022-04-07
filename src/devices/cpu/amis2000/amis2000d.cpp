// license:BSD-3-Clause
// copyright-holders:hap
/*

  AMI S2000-family disassembler

*/

#include "emu.h"
#include "amis2000d.h"

const char *const amis2000_disassembler::s_mnemonics[] =
{
	"LAB", "LAE", "LAI", "LBE", "LBEP", "LBF", "LBZ", "XAB", "XABU", "XAE",
	"LAM", "XC", "XCI", "XCD", "STM", "RSM",
	"ADD", "ADCS", "ADIS", "AND", "XOR", "CMA", "STC", "RSC", "SF1", "RF1", "SF2", "RF2",
	"SAM", "SZM", "SBE", "SZC", "SOS", "SZK", "SZI", "TF1", "TF2",
	"PP", "JMP", "JMS", "RT", "RTS", "NOP", "HALT",
	"INP", "OUT", "DISB", "DISN", "MVS", "PSH", "PSL", "EUR"
};

// number of bits per opcode parameter, negative indicates complement
const s8 amis2000_disassembler::s_bits[] =
{
	0, 0, 4, 2, 2, 2, 2, 0, 0, 0,
	-2, -2, -2, -2, 2, 2,
	0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0,
	-4, 6, 6, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

const u32 amis2000_disassembler::s_flags[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, STEP_COND, STEP_COND, 0, 0,
	0, STEP_COND, STEP_COND, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND,
	0, 0, STEP_OVER, STEP_OUT, STEP_OUT, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


const u8 amis2000_disassembler::s2000_mnemonic[0x100] =
{
	/* 0x00 */
	mNOP, mHALT, mRT, mRTS, mPSH, mPSL, mAND, mSOS,
	mSBE, mSZC, mSTC, mRSC, mLAE, mXAE, mINP, mEUR,
	/* 0x10 */
	mCMA, mXABU, mLAB, mXAB, mADCS, mXOR, mADD, mSAM,
	mDISB, mMVS, mOUT, mDISN, mSZM, mSZM, mSZM, mSZM,
	/* 0x20 */
	mSTM, mSTM, mSTM, mSTM, mRSM, mRSM, mRSM, mRSM,
	mSZK, mSZI, mRF1, mSF1, mRF2, mSF2, mTF1, mTF2,
	mXCI, mXCI, mXCI, mXCI, mXCD, mXCD, mXCD, mXCD,
	mXC, mXC, mXC, mXC, mLAM, mLAM, mLAM, mLAM,
	/* 0x40 */
	mLBZ, mLBZ, mLBZ, mLBZ, mLBF, mLBF, mLBF, mLBF,
	mLBE, mLBE, mLBE, mLBE, mLBEP, mLBEP, mLBEP, mLBEP,
	mADIS, mADIS, mADIS, mADIS, mADIS, mADIS, mADIS, mADIS,
	mADIS, mADIS, mADIS, mADIS, mADIS, mADIS, mADIS, mADIS,
	mPP, mPP, mPP, mPP, mPP, mPP, mPP, mPP,
	mPP, mPP, mPP, mPP, mPP, mPP, mPP, mPP,
	mLAI, mLAI, mLAI, mLAI, mLAI, mLAI, mLAI, mLAI,
	mLAI, mLAI, mLAI, mLAI, mLAI, mLAI, mLAI, mLAI,
	/* 0x80 */
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS, mJMS,
	/* 0xc0 */
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP
};

offs_t amis2000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	u8 instr = s2000_mnemonic[op];

	util::stream_format(stream, "%-6s", s_mnemonics[instr]);

	// opcode parameter
	int mask = s_bits[instr];
	bool complement = (mask < 0);
	if (mask < 0)
		mask = -mask;
	mask = (1 << mask) - 1;

	if (mask != 0)
	{
		u8 param = op;
		if (complement)
			param = ~param;
		param &= mask;

		if (mask < 0x10)
			util::stream_format(stream, "%d", param);
		else
			util::stream_format(stream, "$%02X", param);
	}

	return 1 | s_flags[instr] | SUPPORTED;
}

u32 amis2000_disassembler::opcode_alignment() const
{
	return 1;
}
