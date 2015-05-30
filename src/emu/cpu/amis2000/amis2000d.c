// license:BSD-3-Clause
// copyright-holders:hap
/*

  AMI S2000-family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "amis2000.h"


enum e_mnemonics
{
	mLAB = 0, mLAE, mLAI, mLBE, mLBEP, mLBF, mLBZ, mXAB, mXABU, mXAE,
	mLAM, mXC, mXCI, mXCD, mSTM, mRSM,
	mADD, mADCS, mADIS, mAND, mXOR, mCMA, mSTC, mRSC, mSF1, mRF1, mSF2, mRF2,
	mSAM, mSZM, mSBE, mSZC, mSOS, mSZK, mSZI, mTF1, mTF2,
	mPP, mJMP, mJMS, mRT, mRTS, mNOP, mHALT,
	mINP, mOUT, mDISB, mDISN, mMVS, mPSH, mPSL, mEUR
};

static const char *const s_mnemonics[] =
{
	"LAB", "LAE", "LAI", "LBE", "LBEP", "LBF", "LBZ", "XAB", "XABU", "XAE",
	"LAM", "XC", "XCI", "XCD", "STM", "RSM",
	"ADD", "ADCS", "ADIS", "AND", "XOR", "CMA", "STC", "RSC", "SF1", "RF1", "SF2", "RF2",
	"SAM", "SZM", "SBE", "SZC", "SOS", "SZK", "SZI", "TF1", "TF2",
	"PP", "JMP", "JMS", "RT", "RTS", "NOP", "HALT",
	"INP", "OUT", "DISB", "DISN", "MVS", "PSH", "PSL", "EUR"
};

// number of bits per opcode parameter, negative indicates complement
static const INT8 s_bits[] =
{
	0, 0, 4, 2, 2, 2, 2, 0, 0, 0,
	-2, -2, -2, -2, 2, 2,
	0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 2, 0, 0, 0, 0, 0, 0, 0,
	-4, 6, 6, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, _OVER, _OUT, _OUT, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};


static const UINT8 s2000_mnemonic[0x100] =
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



CPU_DISASSEMBLE( amis2000 )
{
	int pos = 0;
	UINT8 op = oprom[pos++];
	UINT8 instr = s2000_mnemonic[op];

	char *dst = buffer;
	dst += sprintf(dst, "%-5s ", s_mnemonics[instr]);

	// opcode parameter
	int mask = s_bits[instr];
	bool complement = (mask < 0);
	if (mask < 0)
		mask = -mask;
	mask = (1 << mask) - 1;

	if (mask != 0)
	{
		UINT8 param = op;
		if (complement)
			param = ~param;
		param &= mask;

		if (mask < 0x10)
			dst += sprintf(dst, "%d", param);
		else
			dst += sprintf(dst, "$%02X", param);
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}
