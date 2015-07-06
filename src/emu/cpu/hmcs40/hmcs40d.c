// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family disassembler

  NOTE: start offset(basepc) is $3F, not 0

*/

#include "emu.h"
#include "debugger.h"
#include "hmcs40.h"


enum e_mnemonics
{
	mILL,
	mLAB, mLBA, mLAY, mLASPX, mLASPY, mXAMR,
	mLXA, mLYA, mLXI, mLYI, mIY, mDY, mAYY, mSYY, mXSP,
	mLAM, mLBM, mXMA, mXMB, mLMAIY, mLMADY,
	mLMIIY, mLAI, mLBI,
	mAI, mIB, mDB, mAMC, mSMC, mAM, mDAA, mDAS, mNEGA, mCOMB, mSEC, mREC, mTC, mROTL, mROTR, mOR,
	mMNEI, mYNEI, mANEM, mBNEM, mALEI, mALEM, mBLEM,
	mSEM, mREM, mTM,
	mBR, mCAL, mLPU, mTBR, mRTN,
	mSEIE, mSEIF0, mSEIF1, mSETF, mSECF, mREIE, mREIF0, mREIF1, mRETF, mRECF, mTI0, mTI1, mTIF0, mTIF1, mTTF, mLTI, mLTA, mLAT, mRTNI,
	mSED, mRED, mTD, mSEDD, mREDD, mLAR, mLBR, mLRA, mLRB, mP,
	mNOP
};

static const char *const s_mnemonics[] =
{
	"?",
	"LAB", "LBA", "LAY", "LASPX", "LASPY", "XAMR",
	"LXA", "LYA", "LXI", "LYI", "IY", "DY", "AYY", "SYY", "XSP",
	"LAM", "LBM", "XMA", "XMB", "LMAIY", "LMADY",
	"LMIIY", "LAI", "LBI",
	"AI", "IB", "DB", "AMC", "SMC", "AM", "DAA", "DAS", "NEGA", "COMB", "SEC", "REC", "TC", "ROTL", "ROTR", "OR",
	"MNEI", "YNEI", "ANEM", "BNEM", "ALEI", "ALEM", "BLEM",
	"SEM", "REM", "TM",
	"BR", "CAL", "LPU", "TBR", "RTN",
	"SEIE", "SEIF0", "SEIF1", "SETF", "SECF", "REIE", "REIF0", "REIF1", "RETF", "RECF", "TI0", "TI1", "TIF0", "TIF1", "TTF", "LTI", "LTA", "LAT", "RTNI",
	"SED", "RED", "TD", "SEDD", "REDD", "LAR", "LBR", "LRA", "LRB", "P",
	"NOP"
};

// number of bits per opcode parameter, 99 means (XY) parameter, negative means reversed bit-order
static const INT8 s_bits[] =
{
	0,
	0, 0, 0, 0, 0, 4,
	0, 0, -4, -4, 0, 0, 0, 0, 99,
	99, 99, 99, 99, 99, 99,
	-4, -4, -4,
	-4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	-4, -4, 0, 0, -4, 0, 0,
	2, 2, 2,
	6, 6, 5, 3, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -4, 0, 0, 0,
	0, 0, 0, 4, 4, 3, 3, 3, 3, 3,
	0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0,
	0, _OVER, 0, 0, _OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, _OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

// next program counter in sequence (relative)
static const INT8 s_next_pc[0x40] =
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32+0x40 /* rollback */,
	-32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17,
	-15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, -1
};


static const UINT8 hmcs40_mnemonic[0x400] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	/* 0x000 */
	mNOP,  mXSP,  mXSP,  mXSP,  mSEM,  mSEM,  mSEM,  mSEM,  mLAM,  mLAM,  mLAM,  mLAM,  0,     0,     0,     0,
	mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,
	mLBM,  mLBM,  mLBM,  mLBM,  mBLEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mAMC,  0,     0,     0,     mAM,   0,     0,     0,     0,     0,     0,     0,     mLTA,  0,     0,     0,
	/* 0x040 */
	mLXA,  0,     0,     0,     0,     mDAS,  mDAA,  0,     0,     0,     0,     0,     mREC,  0,     0,     mSEC,
	mLYA,  0,     0,     0,     mIY,   0,     0,     0,     mAYY,  0,     0,     0,     0,     0,     0,     0,
	mLBA,  0,     0,     0,     mIB,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,
	/* 0x080 */
	mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,
	mSED,  0,     0,     0,     mTD,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mSEIF1,mSECF, mSEIF0,0,     mSEIE, mSETF, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x0c0 */
	mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  0,     0,     0,     0,     0,     0,     0,     0,
	mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD,
	mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  0,     0,     0,     0,     0,     0,     0,     0,
	mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR,

/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	/* 0x100 */
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLMAIY,mLMAIY,0,     0,     mLMADY,mLMADY,0,     0,     mLAY,  0,     0,     0,     0,     0,     0,     0,
	mOR,   0,     0,     0,     mANEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x140 */
	mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,
	mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,
	mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,
	mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,
	/* 0x180 */
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mTIF1, mTI1,  mTIF0, mTI0,  0,     mTTF,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x1c0 */
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,

/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	/* 0x200 */
	mTM,   mTM,   mTM,   mTM,   mREM,  mREM,  mREM,  mREM,  mXMA,  mXMA,  mXMA,  mXMA,  0,     0,     0,     0,
	mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI,
	mXMB,  mXMB,  mXMB,  mXMB,  mROTR, mROTL, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mSMC,  0,     0,     0,     mALEM, 0,     0,     0,     0,     0,     0,     0,     mLAT,  0,     0,     0,
	/* 0x240 */
	mLASPX,0,     0,     0,     mNEGA, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     mTC,
	mLASPY,0,     0,     0,     mDY,   0,     0,     0,     mSYY,  0,     0,     0,     0,     0,     0,     0,
	mLAB,  0,     0,     0,     0,     0,     0,     mDB,   0,     0,     0,     0,     0,     0,     0,     0,
	mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI,
	/* 0x280 */
	mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI,
	mRED,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mREIF1,mRECF, mREIF0,0,     mREIE, mRETF, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x2c0 */
	mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  0,     0,     0,     0,     0,     0,     0,     0,
	mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD,
	mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,

/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	/* 0x300 */
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mCOMB, 0,     0,     0,     mBNEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x340 */
	mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,
	mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,
	mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x380 */
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     mRTNI, 0,     0,     mRTN,  0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	/* 0x3c0 */
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL
};



CPU_DISASSEMBLE(hmcs40)
{
	UINT16 op = (oprom[0] | oprom[1] << 8) & 0x3ff;
	char *dst = buffer;
	UINT8 instr = hmcs40_mnemonic[op];
	INT8 bits = s_bits[instr];

	// special case for (XY) opcode
	if (bits == 99)
	{
		dst += sprintf(dst, "%s", s_mnemonics[instr]);

		if (op & 1)
			dst += sprintf(dst, "X");
		if (op & 2)
			dst += sprintf(dst, "Y");
	}
	else
	{
		dst += sprintf(dst, "%-6s ", s_mnemonics[instr]);

		// opcode parameter
		if (bits != 0)
		{
			UINT8 param = op;

			// reverse bits
			if (bits < 0)
			{
				param = BITSWAP8(param,0,1,2,3,4,5,6,7);
				param >>= (8 + bits);
				bits = -bits;
			}

			param &= ((1 << bits) - 1);

			if (bits > 5)
				dst += sprintf(dst, "$%02X", param);
			else
				dst += sprintf(dst, "%d", param);
		}
	}

	int pos = s_next_pc[pc & 0x3f] & DASMFLAG_LENGTHMASK;
	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}
