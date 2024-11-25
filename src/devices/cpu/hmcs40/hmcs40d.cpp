// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family disassembler

  NOTE: start offset(basepc) is $3F, not 0. In other words, if you want a full
  disasm from MAME's debugger: dasm x.asm,3f,1000

*/

#include "emu.h"
#include "hmcs40d.h"

// constructor

hmcs40_disassembler::hmcs40_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0x3f; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;

		// see hmcs40_cpu_device::increment_pc()
		u32 mask = 0x3f;
		u32 low = pc & mask;
		u32 fb = (low << 1 & 0x20) == (low & 0x20);

		if (low == (mask >> 1))
			fb = 1;
		else if (low == mask)
			fb = 0;

		pc = (pc & ~mask) | ((pc << 1 | fb) & mask);
	}
}

hmcs40_disassembler::~hmcs40_disassembler()
{
}


// common lookup tables

enum hmcs40_disassembler::e_mnemonics : unsigned
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

const char *const hmcs40_disassembler::s_mnemonics[] =
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
const s8 hmcs40_disassembler::s_bits[] =
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

const u32 hmcs40_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0,
	STEP_COND, STEP_OVER | STEP_COND, 0, 0, STEP_OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STEP_OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

const u8 hmcs40_disassembler::hmcs40_mnemonic[0x400] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x000
	mNOP,  mXSP,  mXSP,  mXSP,  mSEM,  mSEM,  mSEM,  mSEM,  mLAM,  mLAM,  mLAM,  mLAM,  0,     0,     0,     0,
	mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,
	mLBM,  mLBM,  mLBM,  mLBM,  mBLEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mAMC,  0,     0,     0,     mAM,   0,     0,     0,     0,     0,     0,     0,     mLTA,  0,     0,     0,
	// 0x040
	mLXA,  0,     0,     0,     0,     mDAS,  mDAA,  0,     0,     0,     0,     0,     mREC,  0,     0,     mSEC,
	mLYA,  0,     0,     0,     mIY,   0,     0,     0,     mAYY,  0,     0,     0,     0,     0,     0,     0,
	mLBA,  0,     0,     0,     mIB,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,
	// 0x080
	mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,
	mSED,  0,     0,     0,     mTD,   0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mSEIF1,mSECF, mSEIF0,0,     mSEIE, mSETF, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x0c0
	mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  0,     0,     0,     0,     0,     0,     0,     0,
	mSEDD, mSEDD, mSEDD, mSEDD, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  0,     0,     0,     0,     0,     0,     0,     0,
	mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR, mXAMR,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x100
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLMAIY,mLMAIY,0,     0,     mLMADY,mLMADY,0,     0,     mLAY,  0,     0,     0,     0,     0,     0,     0,
	mOR,   0,     0,     0,     mANEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x140
	mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,
	mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,
	mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,
	mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,  mLTI,
	// 0x180
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mTIF1, mTI1,  mTIF0, mTI0,  0,     mTTF,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x1c0
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x200
	mTM,   mTM,   mTM,   mTM,   mREM,  mREM,  mREM,  mREM,  mXMA,  mXMA,  mXMA,  mXMA,  0,     0,     0,     0,
	mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI, mMNEI,
	mXMB,  mXMB,  mXMB,  mXMB,  mROTR, mROTL, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mSMC,  0,     0,     0,     mALEM, 0,     0,     0,     0,     0,     0,     0,     mLAT,  0,     0,     0,
	// 0x240
	mLASPX,0,     0,     0,     mNEGA, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     mTC,
	mLASPY,0,     0,     0,     mDY,   0,     0,     0,     mSYY,  0,     0,     0,     0,     0,     0,     0,
	mLAB,  0,     0,     0,     0,     0,     0,     mDB,   0,     0,     0,     0,     0,     0,     0,     0,
	mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI,
	// 0x280
	mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI,
	mRED,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mREIF1,mRECF, mREIF0,0,     mREIE, mRETF, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x2c0
	mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  0,     0,     0,     0,     0,     0,     0,     0,
	mREDD, mREDD, mREDD, mREDD, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x300
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	mCOMB, 0,     0,     0,     mBNEM, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x340
	mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,
	mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,  mLPU,
	mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x380
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     mRTNI, 0,     0,     mRTN,  0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
	// 0x3c0
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL
};


// disasm

offs_t hmcs40_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 op = opcodes.r16(pc) & 0x3ff;
	u8 instr = hmcs40_mnemonic[op];
	s8 bits = s_bits[instr];

	// special case for (XY) opcode
	if (bits == 99)
	{
		util::stream_format(stream, "%s", s_mnemonics[instr]);

		if (op & 1)
			stream << "X";
		if (op & 2)
			stream << "Y";
	}
	else
	{
		util::stream_format(stream, "%-8s", s_mnemonics[instr]);

		// opcode parameter
		if (bits != 0)
		{
			u8 param = op;

			// reverse bits
			if (bits < 0)
			{
				param = bitswap<8>(param,0,1,2,3,4,5,6,7);
				param >>= (8 + bits);
				bits = -bits;
			}

			param &= ((1 << bits) - 1);
			util::stream_format(stream, (bits > 4) ? "$%02X" : (param < 10) ? "%d" : "$%X", param);
		}
	}

	return 1 | s_flags[instr] | SUPPORTED;
}
