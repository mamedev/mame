// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "sm510.h"


enum e_mnemonics
{
	mILL,
	mLB, mLBL, mSBM, mEXBLA, mINCB, mDECB,
	mATPL, mRTN0, mRTN1, mTL, mTML, mTM, mT,
	mEXC, mBDC, mEXCI, mEXCD, mLDA, mLAX, mWR, mWS,
	mKTA, mATBP, mATL, mATFC, mATR,
	mADD, mADD11, mADX, mCOMA, mROT, mRC, mSC,
	mTB, mTC, mTAM, mTMI, mTA0, mTABL, mTIS, mTAL, mTF1, mTF4,
	mRM, mSM, mSKIP, mCEND, mIDIV
};

static const char *const s_mnemonics[] =
{
	"?",
	"LB", "LBL", "SBM", "EXBLA", "INCB", "DECB",
	"ATPL", "RTN0", "RTN1", "TL", "TML", "TM", "T",
	"EXC", "BDC", "EXCI", "EXCD", "LDA", "LAX", "WR", "WS",
	"KTA", "ATBP", "ATL", "ATFC", "ATR",
	"ADD", "ADD11", "ADX", "COMA", "ROT", "RC", "SC",
	"TB", "TC", "TAM", "TMI", "TA0", "TABL", "TIS", "TAL", "TF1", "TF4",
	"RM", "SM", "SKIP", "CEND", "IDIV"
};

// number of bits per opcode parameter, 8 or larger means 2-byte opcode
static const UINT8 s_bits[] =
{
	0,
	4, 8, 0, 0, 0, 0,
	0, 0, 0, 4+8, 2+8, 6+8, 6,
	2, 0, 2, 2, 2, 4, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	2, 2, 0, 0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0,
	0, _OUT, _OUT, 0, _OVER, _OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, _OVER, 0
};

static const UINT8 sm510_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mSKIP, mATBP, mSBM,  mATPL, mRM,   mRM,   mRM,   mRM,   mADD,  mADD11,mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3 - note: $3A has synonym DC(decimal correct)

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	0,     mTB,   mTC,   mTAM,  mTMI,  mTMI,  mTMI,  mTMI,  mTIS,  mATL,  mTA0,  mTABL, 0,     mCEND, mTAL,  mLBL,  // 5
	mATFC, mATR,  mWR,   mWS,   mINCB, mIDIV, mRC,   mSC,   mTF1,  mTF4,  mKTA,  mROT,  mDECB, mBDC,  mRTN0, mRTN1, // 6
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTML,  mTML,  mTML,  mTML,  // 7

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 8
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 9
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // A
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // B

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // C
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // D
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // E
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM    // F
};



CPU_DISASSEMBLE(sm510)
{
	// get raw opcode
	int pos = 0;
	UINT8 op = oprom[pos];
	pos++;
	UINT8 instr = sm510_mnemonic[op];

	int bits = s_bits[instr];
	UINT8 mask = op & ((1 << (bits & 7)) - 1);
	UINT16 param = mask;
	if (bits >= 8)
	{
		param = oprom[pos];
		pos++;
	}

	// disassemble it
	char *dst = buffer;
	dst += sprintf(dst, "%-6s ", s_mnemonics[instr]);
	if (bits > 0)
	{
		if (bits <= 4)
		{
			if (param < 10)
				dst += sprintf(dst, "%d", param);
			else
				dst += sprintf(dst, "$%X", param);
		}
		else if (bits <= 8)
		{
			dst += sprintf(dst, "$%02X", param);
		}
		else if (instr == mTL || instr == mTML)
		{
			UINT16 address = (param << 4 & 0xc00) | (mask << 6 & 0x3c0) | (param & 0x3f);
			dst += sprintf(dst, "$%03X", address);
		}
		else if (instr == mTM)
		{
			//todo
		}
	}
	
	return (pos & DASMFLAG_LENGTHMASK) | s_flags[instr] | DASMFLAG_SUPPORTED;
}
