// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family disassembler

*/

#include "emu.h"
#include "tms1k_dasm.h"

// constructor

tms1000_base_disassembler::tms1000_base_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits) :
	m_lut_mnemonic(lut_mnemonic),
	m_opcode_9bits(opcode_9bits),
	m_pc_bits(pc_bits)
{
	// init lfsr pc lut
	const u32 len = 1 << pc_bits;
	m_l2r = std::make_unique<u8[]>(len);
	m_r2l = std::make_unique<u8[]>(len);

	for (u32 i = 0, pc = 0; i < len; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;

		// see tms1k_base_device::next_pc()
		u32 mask = (1 << pc_bits) - 1;
		u32 high = 1 << (pc_bits - 1);
		u32 fb = (pc << 1 & high) == (pc & high);

		if (pc == (mask >> 1))
			fb = 1;
		else if (pc == mask)
			fb = 0;

		pc = (pc << 1 | fb) & mask;
	}
}

tms1100_disassembler::tms1100_disassembler(const u8 *lut_mnemonic, bool opcode_9bits, int pc_bits) :
	tms1000_base_disassembler(lut_mnemonic, opcode_9bits, pc_bits)
{ }

tms1000_disassembler::tms1000_disassembler() : tms1000_base_disassembler(tms1000_mnemonic, false, 6)
{ }

tms1100_disassembler::tms1100_disassembler() : tms1100_disassembler(tms1100_mnemonic, false, 6)
{ }

tms1400_disassembler::tms1400_disassembler() : tms1100_disassembler(tms1400_mnemonic, false, 6)
{ }

tms2100_disassembler::tms2100_disassembler() : tms1100_disassembler(tms2100_mnemonic, false, 6)
{ }

tms2400_disassembler::tms2400_disassembler() : tms1100_disassembler(tms2400_mnemonic, false, 6)
{ }

smc1102_disassembler::smc1102_disassembler() : tms1100_disassembler(smc1102_mnemonic, false, 6)
{ }

tms0980_disassembler::tms0980_disassembler() : tms1000_base_disassembler(tms0980_mnemonic, true, 7)
{ }

tp0320_disassembler::tp0320_disassembler() : tms1000_base_disassembler(tp0320_mnemonic, true, 7)
{ }


offs_t tms1000_base_disassembler::pc_linear_to_real(offs_t pc) const
{
	const u32 mask = (1 << m_pc_bits) - 1;
	return (pc & ~mask) | m_l2r[pc & mask];
}

offs_t tms1000_base_disassembler::pc_real_to_linear(offs_t pc) const
{
	const u32 mask = (1 << m_pc_bits) - 1;
	return (pc & ~mask) | m_r2l[pc & mask];
}


// common lookup tables

enum tms1000_base_disassembler::e_mnemonics : unsigned
{
	mILL = 0,
	mAC0AC, mAC1AC, mACACC, mACNAA, mALEC, mALEM, mAMAAC, mBRANCH,
	mCALL, mCCLA, mCLA, mCLO, mCOMC, mCOMX, mCOMX8, mCPAIZ, mCTMDYN,
	mDAN, mDMAN, mDMEA, mDNAA, mDYN, mHALT,
	mIA, mIMAC, mINTDIS, mINTEN, mINTRTN, mIYC,
	mKNEZ, mLDP, mLDX2, mLDX3, mLDX4, mMNEA, mMNEZ,
	mNDMEA, mOFF, mRBIT, mREAC, mRETN, mRSTR,
	mSAL, mSAMAN, mSBIT, mSBL, mSEAC, mSELIN, mSETR,
	mTAC, mTADM, mTAM, mTAMACS, mTAMDYN, mTAMIY, mTAMIYC, mTAMZA,
	mTASR, mTAX, mTAY, mTBIT, mTCA, mTCMIY, mTCY, mTDO, mTKA,
	mTKM, mTMA, mTMSET, mTMY, mTPC, mTRA, mTSG, mTXA, mTYA,
	mXDA, mXMA, mYMCY, mYNEA, mYNEC
};

const char *const tms1000_base_disassembler::s_mnemonic[] =
{
	"?",
	"AC0AC", "AC1AC", "ACACC", "ACNAA", "ALEC", "ALEM", "AMAAC", "BRANCH",
	"CALL", "CCLA", "CLA", "CLO", "COMC", "COMX", "COMX8", "CPAIZ", "CTMDYN",
	"DAN", "DMAN", "DMEA", "DNAA", "DYN", "HALT",
	"IA", "IMAC", "INTDIS", "INTEN", "INTRTN", "IYC",
	"KNEZ", "LDP", "LDX", "LDX", "LDX", "MNEA", "MNEZ",
	"NDMEA", "OFF", "RBIT", "REAC", "RETN", "RSTR",
	"SAL", "SAMAN", "SBIT", "SBL", "SEAC", "SELIN", "SETR",
	"TAC", "TADM", "TAM", "TAMACS", "TAMDYN", "TAMIY", "TAMIYC", "TAMZA",
	"TASR", "TAX", "TAY", "TBIT", "TCA", "TCMIY", "TCY", "TDO", "TKA",
	"TKM", "TMA", "TMSET", "TMY", "TPC", "TRA", "TSG", "TXA", "TYA",
	"XDA", "XMA", "YMCY", "YNEA", "YNEC"
};

const u32 tms1000_base_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, STEP_COND,
	STEP_OVER | STEP_COND, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, STEP_OUT, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, STEP_OUT, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};

const u8 tms1000_base_disassembler::s_bits[] =
{
	0,
	4, 4, 4, 4, 4, 0, 0, 6,
	6, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 4, 2, 3, 4, 0, 0,
	0, 0, 2, 0, 0, 0,
	0, 0, 2, 0, 0, 0, 0,
	0, 0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 2, 0, 4, 4, 0, 0,
	0, 0, 0, 0, 0, 0, 2, 0, 0,
	0, 0, 4, 0, 4
};


// opcode luts

const u8 tms1000_disassembler::tms1000_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mCOMX,   mAC0AC,  mYNEA,   mTAM,    mTAMZA,  mAC0AC,  mAC0AC,  mDAN,    mTKA,    mKNEZ,   mTDO,    mCLO,    mRSTR,   mSETR,   mIA,     mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAMIY,  mTMA,    mTMY,    mTYA,    mTAY,    mAMAAC,  mMNEZ,   mSAMAN,  mIMAC,   mALEM,   mDMAN,   mIYC,    mDYN,    mCPAIZ,  mXMA,    mCLA,    // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mLDX2,   mLDX2,   mLDX2,   mLDX2,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tms1100_disassembler::tms1100_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mMNEA,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mAMAAC,  mDMAN,   mTKA,    mCOMX,   mTDO,    mCOMC,   mRSTR,   mSETR,   mKNEZ,   mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAMZA,  mTAM,    mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mCLA,    // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tms1400_disassembler::tms1400_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mMNEA,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mAMAAC,  mDMAN,   mTKA,    mCOMX,   mTDO,    mTPC,    mRSTR,   mSETR,   mKNEZ,   mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAMZA,  mTAM,    mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mCLA,    // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tms2100_disassembler::tms2100_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mMNEA,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mAMAAC,  mDMAN,   mTKA,    mTAX,    mTDO,    mCOMC,   mRSTR,   mSETR,   mTADM,   mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAC,    mTAM,    mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mAC1AC,  mAC1AC,  mAC1AC,  mTCA,    mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mTRA,    mAC1AC,  mAC1AC,  mAC1AC,  mCLA,    // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tms2400_disassembler::tms2400_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mMNEA,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mAMAAC,  mDMAN,   mTKA,    mTAX,    mTDO,    mTPC,    mRSTR,   mSETR,   mTADM,   mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAC,    mTAM,    mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mAC1AC,  mAC1AC,  mAC1AC,  mTCA,    mAC1AC,  mCOMX,   mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mAC1AC,  mTRA,    mAC1AC,  mTXA,    mAC1AC,  mCLA,    // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 smc1102_disassembler::smc1102_mnemonic[256] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x00
	mMNEA,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mAMAAC,  mDMAN,   mTKA,    mCOMX,   mTASR,   mCOMC,   mRSTR,   mSETR,   mKNEZ,   mRETN,   // 0
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 1
	mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAMZA,  mTAM,    mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   mLDX3,   // 2
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   mTBIT,   mTBIT,   mTBIT,   mTBIT,   mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mAC1AC,  mHALT,   mTSG,    mTSG,    mINTEN,  mINTDIS, mINTRTN, mAC1AC,  mSELIN,  mAC1AC,  mAC1AC,  mTMSET,  mTSG,    mTSG,    mAC1AC,  mCLA,    // 7
	// 0x80
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 8
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 9
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // A
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tms0980_disassembler::tms0980_mnemonic[512] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x000
	mCOMX,   mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mCLA,    mDMAN,   mTKA,    mMNEA,   mTKM,    0,       0,       mSETR,   mKNEZ,   0,       // 0
	mDMEA,   mDNAA,   mCCLA,   mNDMEA,  0,       mAMAAC,  0,       0,       mCTMDYN, mXDA,    0,       0,       0,       0,       0,       0,       // 1
	mTBIT,   mTBIT,   mTBIT,   mTBIT,   0,       0,       0,       0,       mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAMZA,  mTAM,    // 2
	mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   0,       0,       0,       0,       mTCY,    mYNEC,   mTCMIY,  mACACC,  mACNAA,  mTAMACS, mALEC,   mYMCY,   // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  // 7
	// 0x080
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 8
	mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   // 9
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   0,       0,       0,       0,       0,       0,       0,       0,       // A
	mTDO,    mSAL,    mCOMX8,  mSBL,    mREAC,   mSEAC,   mOFF,    0,       0,       0,       0,       0,       0,       0,       0,       mRETN,   // B
	mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  // C
	mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, // D
	mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   // E
	mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   // F
	// 0x100
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 0
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 1
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 2
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 3
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 4
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 5
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 6
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 7
	// 0x180
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // 8
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // 9
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // A
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};

const u8 tp0320_disassembler::tp0320_mnemonic[512] =
{
//  0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F
	// 0x000
	0,       mALEM,   mYNEA,   mXMA,    mDYN,    mIYC,    mCLA,    mDMAN,   mTKA,    mMNEA,   mTKM,    0,       0,       mSETR,   mKNEZ,   0,       // 0
	mDMEA,   mDNAA,   mCCLA,   mNDMEA,  0,       mAMAAC,  0,       0,       mCTMDYN, mXDA,    0,       0,       0,       0,       0,       0,       // 1
	mTBIT,   mTBIT,   mTBIT,   mTBIT,   0,       0,       0,       0,       mTAY,    mTMA,    mTMY,    mTYA,    mTAMDYN, mTAMIYC, mTAMZA,  mTAM,    // 2
	mSAMAN,  mCPAIZ,  mIMAC,   mMNEZ,   0,       0,       mRSTR,   mYMCY,   mTCY,    mYNEC,   mTCMIY,  mACACC,  mACNAA,  mTAMACS, mALEC,   0,       // 3
	mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    mTCY,    // 4
	mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   mYNEC,   // 5
	mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  mTCMIY,  // 6
	mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  mACACC,  // 7
	// 0x080
	mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    mLDP,    // 8
	mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   mLDX4,   // 9
	mSBIT,   mSBIT,   mSBIT,   mSBIT,   mRBIT,   mRBIT,   mRBIT,   mRBIT,   0,       0,       0,       0,       0,       0,       0,       0,       // A
	mTDO,    mSAL,    mCOMX8,  mSBL,    mREAC,   mSEAC,   mOFF,    0,       0,       0,       0,       0,       0,       0,       0,       mRETN,   // B
	mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  mACNAA,  // C
	mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, mTAMACS, // D
	mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   mALEC,   // E
	mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   mYMCY,   // F
	// 0x100
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 0
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 1
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 2
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 3
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 4
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 5
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 6
	mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, mBRANCH, // 7
	// 0x180
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // 8
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // 9
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // A
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // B
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // C
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // D
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   // E
	mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL,   mCALL    // F
};


// disasm

const u8 tms1000_base_disassembler::i2_value[4] =
{
	0, 2, 1, 3
};

const u8 tms1000_base_disassembler::i3_value[8] =
{
	0, 4, 2, 6, 1, 5, 3, 7
};

const u8 tms1000_base_disassembler::i4_value[16] =
{
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
};

offs_t tms1000_base_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u16 op = m_opcode_9bits ? opcodes.r16(pc) & 0x1ff : opcodes.r8(pc);
	u16 instr = m_lut_mnemonic[op];

	if (instr == mAC0AC || instr == mAC1AC)
	{
		// special case for AcAAC
		u8 c = (i4_value[op & 0x0f] + ((instr == mAC1AC) ? 1 : 0)) & 0xf;
		util::stream_format(stream, "A%dACC", c);
	}
	else
	{
		// convert to mnemonic/param
		util::stream_format(stream, "%-8s", s_mnemonic[instr]);

		switch (s_bits[instr])
		{
			case 2:
				util::stream_format(stream, "%d", i2_value[op & 0x03]);
				break;
			case 3:
				util::stream_format(stream, "%d", i3_value[op & 0x07]);
				break;
			case 4:
				util::stream_format(stream, "%d", i4_value[op & 0x0f]);
				break;
			case 6:
				util::stream_format(stream, "$%02X", op & (m_opcode_9bits ? 0x7f : 0x3f));
				break;
			default:
				break;
		}
	}

	return 1 | s_flags[instr] | SUPPORTED;
}
