// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS400 MCU family disassembler

*/

#include "emu.h"
#include "hmcs400d.h"

// constructor

hmcs400_disassembler::hmcs400_disassembler()
{
}

hmcs400_disassembler::~hmcs400_disassembler()
{
}


// common lookup tables

enum hmcs400_disassembler::e_mnemonics : unsigned
{
	mILL1, mILL2,
	mLAI, mLBI, mLMID, mLMIIY,
	mLAB, mLBA, mLAW, mLAY, mLASPX, mLASPY, mLAMR, mXMRA,
	mLWI, mLXI, mLYI, mLWA, mLXA, mLYA, mIY, mDY, mAYY, mSYY, mXSP,
	mLAM, mLAMD, mLBM, mLMA, mLMAD, mLMAIY, mLMADY, mXMA, mXMAD, mXMB,
	mAI, mIB, mDB, mDAA, mDAS, mNEGA, mCOMB, mROTR, mROTL, mSEC, mREC, mTC,
	mAM, mAMD, mAMC, mAMCD, mSMC, mSMCD, mOR, mANM, mANMD, mORM, mORMD, mEORM, mEORMD,
	mINEM, mINEMD, mANEM, mANEMD, mBNEM, mYNEI, mILEM, mILEMD, mALEM, mALEMD, mBLEM, mALEI,
	mSEM, mSEMD, mREM, mREMD, mTM, mTMD,
	mBR, mBRL, mJMPL, mCAL, mCALL, mTBR, mRTN, mRTNI,
	mSED, mSEDD, mRED, mREDD, mTD, mTDD, mLAR, mLBR, mLRA, mLRB, mP,
	mNOP, mSTS, mSBY, mSTOP
};

const char *const hmcs400_disassembler::s_mnemonics[] =
{
	"?", "?",
	"LAI", "LBI", "LMID", "LMIIY",
	"LAB", "LBA", "LAW", "LAY", "LASPX", "LASPY", "LAMR", "XMRA",
	"LWI", "LXI", "LYI", "LWA", "LXA", "LYA", "IY", "DY", "AYY", "SYY", "XSP",
	"LAM", "LAMD", "LBM", "LMA", "LMAD", "LMAIY", "LMADY", "XMA", "XMAD", "XMB",
	"AI", "IB", "DB", "DAA", "DAS", "NEGA", "COMB", "ROTR", "ROTL", "SEC", "REC", "TC",
	"AM", "AMD", "AMC", "AMCD", "SMC", "SMCD", "OR", "ANM", "ANMD", "ORM", "ORMD", "EORM", "EORMD",
	"INEM", "INEMD", "ANEM", "ANEMD", "BNEM", "YNEI", "ILEM", "ILEMD", "ALEM", "ALEMD", "BLEM", "ALEI",
	"SEM", "SEMD", "REM", "REMD", "TM", "TMD",
	"BR", "BRL", "JMPL", "CAL", "CALL", "TBR", "RTN", "RTNI",
	"SED", "SEDD", "RED", "REDD", "TD", "TDD", "LAR", "LBR", "LRA", "LRB", "P",
	"NOP", "STS", "SBY", "STOP"
};

// number of bits per opcode parameter, 99 means (XY) parameter
const s8 hmcs400_disassembler::s_bits[] =
{
	0, 10,
	4, 4, 14, 4,
	0, 0, 10, 0, 0, 0, 4, 4,
	2, 4, 4, 10, 0, 0, 0, 0, 0, 0, 99,
	99, 10, 99, 99, 10, 99, 99, 99, 10, 99,
	4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 10, 0, 10, 0, 10, 0, 0, 10, 0, 10, 0, 10,
	4, 14, 0, 10, 0, 4, 4, 14, 0, 10, 0, 4,
	2, 12, 2, 12, 2, 12,
	8, 14, 14, 6, 14, 4, 0, 0,
	0, 4, 0, 4, 0, 4, 4, 4, 4, 4, 4,
	0, 0, 0, 0
};

const u32 hmcs400_disassembler::s_flags[] =
{
	0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	STEP_COND, STEP_COND, 0, STEP_OVER | STEP_COND, STEP_OVER | STEP_COND, 0, STEP_OUT, STEP_OUT,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

const u8 hmcs400_disassembler::hmcs400_mnemonic[0x400] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x000
	mNOP,  mXSP,  mXSP,  mXSP,  mANEM, 0,     0,     0,     mAM,   0,     0,     0,     mORM,  0,     0,     0,
	mRTN,  mRTNI, 0,     0,     mALEM, 0,     0,     0,     mAMC,  0,     0,     0,     mEORM, 0,     0,     0,
	mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM, mINEM,
	mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM, mILEM,
	// 0x040
	mLBM,  mLBM,  mLBM,  mLBM,  mBNEM, 0,     0,     0,     mLAB,  0,     0,     0,     mIB,   0,     0,     0,
	mLMAIY,mLMAIY,0,     0,     mAYY,  0,     0,     0,     mLASPY,0,     0,     0,     mIY,   0,     0,     0,
	mNEGA, 0,     0,     0,     mRED,  0,     0,     0,     mLASPX,0,     0,     0,     0,     0,     0,     mTC,
	mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI, mYNEI,
	// 0x080
	mXMA,  mXMA,  mXMA,  mXMA,  mSEM,  mSEM,  mSEM,  mSEM,  mREM,  mREM,  mREM,  mREM,  mTM,   mTM,   mTM,   mTM,
	mLAM,  mLAM,  mLAM,  mLAM,  mLMA,  mLMA,  mLMA,  mLMA,  mSMC,  0,     0,     0,     mANM,  0,     0,     0,
	mROTR, mROTL, 0,     0,     0,     0,     mDAA,  0,     0,     0,     mDAS,  0,     0,     0,     0,     mLAY,
	mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,  mTBR,
	// 0x0c0
	mXMB,  mXMB,  mXMB,  mXMB,  mBLEM, 0,     0,     0,     mLBA,  0,     0,     0,     0,     0,     0,     mDB,
	mLMADY,mLMADY,0,     0,     mSYY,  0,     0,     0,     mLYA,  0,     0,     0,     0,     0,     0,     mDY,
	mTD,   0,     0,     0,     mSED,  0,     0,     0,     mLXA,  0,     0,     0,     mREC,  0,     0,     mSEC,
	mLWI,  mLWI,  mLWI,  mLWI,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x100
	mLAW,  1,     1,     1,     mANEMD,1,     1,     1,     mAMD,  1,     1,     1,     mORMD, 1,     1,     1,
	mLWA,  1,     1,     1,     mALEMD,1,     1,     1,     mAMCD, 1,     1,     1,     mEORMD,1,     1,     1,
	mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,mINEMD,
	mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,mILEMD,
	// 0x140
	mCOMB, 0,     0,     0,     mOR,   0,     0,     0,     mSTS,  0,     0,     0,     mSBY,  mSTOP, 0,     0,
	mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL, mJMPL,
	mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL, mCALL,
	mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,  mBRL,
	// 0x180
	mXMAD, 1,     1,     1,     mSEMD, mSEMD, mSEMD, mSEMD, mREMD, mREMD, mREMD, mREMD, mTMD,  mTMD,  mTMD,  mTMD,
	mLAMD, 1,     1,     1,     mLMAD, 1,     1,     1,     mSMCD, 1,     1,     1,     mANMD, 1,     1,     1,
	mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID, mLMID,
	mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,    mP,
	// 0x1c0
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,
	mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x200
	mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,  mLBI,
	mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,  mLYI,
	mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,  mLXI,
	mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,  mLAI,
	// 0x240
	mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,  mLBR,
	mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,  mLAR,
	mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD, mREDD,
	mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR, mLAMR,
	// 0x280
	mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,
	mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,mLMIIY,
	mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,  mTDD,
	mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI, mALEI,
	// 0x2c0
	mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,  mLRB,
	mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,  mLRA,
	mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD, mSEDD,
	mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA, mXMRA,

//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	// 0x300
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	// 0x340
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	// 0x380
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	// 0x3c0
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,
	mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR,   mBR
};


// disasm

offs_t hmcs400_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 op = opcodes.r16(pc++) & 0x3ff;
	u8 instr = hmcs400_mnemonic[op];
	u8 bits = s_bits[instr];
	int len = 1;

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
			u16 param1 = op & ((1 << (bits % 10)) - 1);
			u16 param2 = 0;
			if (bits >= 10)
			{
				len++;
				param2 = params.r16(pc++) & 0x3ff;
			}

			if (instr >= mBR && instr <= mCALL)
			{
				if (instr == mBR)
					param1 |= pc & 0x3f00;
				else if (instr != mCAL)
					param1 = param1 << 10 | param2;
				util::stream_format(stream, "$%04X", param1); // ROM address
			}
			else if (instr != mILL2 && instr != mLAW && instr != mLWA)
			{
				if (bits != 10)
					util::stream_format(stream, (param1 < 10) ? "%d" : "$%X", param1);
				if (bits > 10)
					stream << ",";
				if (bits >= 10)
					util::stream_format(stream, "$%03X", param2); // RAM address
			}
		}
	}

	return len | s_flags[instr] | SUPPORTED;
}
