// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family disassembler

*/

#include "emu.h"
#include "ucom4d.h"


// common lookup tables

enum ucom4_disassembler::e_mnemonics : unsigned
{
	mILL,
	mLI, mL, mLM, mLDI, mLDZ, mS, mTAL, mTLA,
	mX, mXI, mXD, mXM, mXMI, mXMD, mAD, mADC, mADS, mDAA, mDAS,
	mEXL, mCLA, mCMA, mCIA, mCLC, mSTC, mTC, mINC, mDEC, mIND, mDED,
	mRMB, mSMB, mREB, mSEB, mRPB, mSPB, mJMP, mJCP, mJPA, mCAL, mCZP, mRT, mRTS,
	mCI, mCM, mCMB, mTAB, mCLI, mTMB, mTPA, mTPB,
	mTIT, mIA, mIP, mOE, mOP, mOCD, mNOP,
	mTAW, mTAZ, mTHX, mTLY, mXAW, mXAZ, mXHR, mXHX, mXLS, mXLY, mXC,
	mSFB, mRFB, mFBT, mFBF, mRAR, mINM, mDEM, mSTM, mTTM, mEI, mDI
};

const char *const ucom4_disassembler::s_mnemonics[] =
{
	"?",
	"LI", "L", "LM", "LDI", "LDZ", "S", "TAL", "TLA",
	"X", "XI", "XD", "XM", "XMI", "XMD", "AD", "ADC", "ADS", "DAA", "DAS",
	"EXL", "CLA", "CMA", "CIA", "CLC", "STC", "TC", "INC", "DEC", "IND", "DED",
	"RMB", "SMB", "REB", "SEB", "RPB", "SPB", "JMP", "JCP", "JPA", "CAL", "CZP", "RT", "RTS",
	"CI", "CM", "CMB", "TAB", "CLI", "TMB", "TPA", "TPB",
	"TIT", "IA", "IP", "OE", "OP", "OCD", "NOP",
	"TAW", "TAZ", "THX", "TLY", "XAW", "XAZ", "XHR", "XHX", "XLS", "XLY", "XC",
	"SFB", "RFB", "FBT", "FBF", "RAR", "INM", "DEM", "STM", "TTM", "EI", "DI"
};

// number of bits per opcode parameter, 2 digits means opcode is 2 bytes
const u8 ucom4_disassembler::s_bits[] =
{
	0,
	4, 0, 2, 80, 4, 0, 0, 0,
	0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 83, 6, 0, 83, 4, 0, 0,
	40, 0, 2, 2, 40, 2, 2, 2,
	0, 0, 0, 0, 0, 80, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 0, 0, 0, 80, 0, 0, 0
};

const u32 ucom4_disassembler::s_flags[] =
{
	0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, STEP_COND, STEP_COND, 0, STEP_COND, STEP_COND, STEP_COND, 0, STEP_COND, 0, 0,
	0, 0, 0, 0, 0, 0, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND,
	0, 0, 0, 0, 0, 0, 0, 0, 0, STEP_OVER, STEP_OVER, STEP_OUT, STEP_OUT,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND, STEP_COND,
	STEP_COND, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, STEP_COND, STEP_COND, 0, STEP_COND, STEP_COND, 0, STEP_COND, 0, 0
};


const u8 ucom4_disassembler::ucom4_mnemonic[0x100] =
{
//  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	mNOP, mDI,  mS,   mTIT, mTC,  mTTM, mDAA, mTAL, mAD,  mADS, mDAS, mCLC, mCM,  mINC, mOP,  mDEC, // 0
	mCMA, mCIA, mTLA, mDED, mSTM, mLDI, mCLI, mCI,  mEXL, mADC, mXC,  mSTC, 0,    mINM, mOCD, mDEM, // 1
	mFBF, mFBF, mFBF, mFBF, mTAB, mTAB, mTAB, mTAB, mX,   mXM,  mXM,  mXM,  mXD,  mXMD, mXMD, mXMD, // 2
	mRAR, mEI,  mIP,  mIND, mCMB, mCMB, mCMB, mCMB, mL,   mLM,  mLM,  mLM,  mXI,  mXMI, mXMI, mXMI, // 3
	mIA,  mJPA, mTAZ, mTAW, mOE,  0,    mTLY, mTHX, mRT,  mRTS, mXAZ, mXAW, mXLS, mXHR, mXLY, mXHX, // 4

	mTPB, mTPB, mTPB, mTPB, mTPA, mTPA, mTPA, mTPA, mTMB, mTMB, mTMB, mTMB, mFBT, mFBT, mFBT, mFBT, // 5
	mRPB, mRPB, mRPB, mRPB, mREB, mREB, mREB, mREB, mRMB, mRMB, mRMB, mRMB, mRFB, mRFB, mRFB, mRFB, // 6
	mSPB, mSPB, mSPB, mSPB, mSEB, mSEB, mSEB, mSEB, mSMB, mSMB, mSMB, mSMB, mSFB, mSFB, mSFB, mSFB, // 7
	mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, // 8

	mCLA, mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  mLI,  // 9
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mCAL, mCAL, mCAL, mCAL, mCAL, mCAL, mCAL, mCAL, // A
	mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, // B

	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, // C
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, // D
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, // E
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP  // F
};


// disasm

offs_t ucom4_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t pos = pc;
	u8 op = opcodes.r8(pos++);
	u8 instr = ucom4_mnemonic[op];

	util::stream_format(stream,"%-6s", s_mnemonics[instr]);

	// opcode parameter
	int bits = s_bits[instr];
	if (bits)
	{
		u16 param = op & ((1 << (bits % 10)) - 1);
		if (bits / 10)
		{
			u8 op2 = opcodes.r8(pos++);
			param = (param << (bits / 10)) | (op2 & ((1 << (bits / 10)) - 1));
			bits = (bits % 10) + (bits / 10);
		}

		// special case for CZP
		if (instr == mCZP)
		{
			param <<= 2;
			bits += 2;
		}

		if (bits <= 4)
			util::stream_format(stream, "%d", param);
		else if (bits <= 8)
			util::stream_format(stream, "$%02X", param);
		else
			util::stream_format(stream, "$%03X", param);
	}

	return (pos - pc) | s_flags[instr] | SUPPORTED;
}
