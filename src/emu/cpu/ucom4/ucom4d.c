// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family disassembler

*/

#include "emu.h"
#include "debugger.h"
#include "ucom4.h"


enum e_mnemonics
{
	mLI, mL, mLM, mLDI, mLDZ, mS, mTAL, mTLA,
	mX, mXI, mXD, mXM, mXMI, mXMD, mAD, mADC, mADS, mDAA, mDAS,
	mEXL, mCLA, mCMA, mCIA, mCLC, mSTC, mTC, mINC, mDEC, mIND, mDED,
	mRMB, mSMB, mREB, mSEB, mRPB, mSPB, mJMP, mJCP, mJPA, mCAL, mCZP, mRT, mRTS,
	mCI, mCM, mCMB, mTAB, mCLI, mTMB, mTPA, mTPB,
	mTIT, mIA, mIP, mOE, mOP, mOCD, mNOP,
	mILL,
	mTAW, mTAZ, mTHX, mTLY, mXAW, mXAZ, mXHR, mXHX, mXLS, mXLY, mXC,
	mSFB, mRFB, mFBT, mFBF, mRAR, mINM, mDEM, mSTM, mTTM, mEI, mDI
};

static const char *const s_mnemonics[] =
{
	"LI", "L", "LM", "LDI", "LDZ", "S", "TAL", "TLA",
	"X", "XI", "XD", "XM", "XMI", "XMD", "AD", "ADC", "ADS", "DAA", "DAS",
	"EXL", "CLA", "CMA", "CIA", "CLC", "STC", "TC", "INC", "DEC", "IND", "DED",
	"RMB", "SMB", "REB", "SEB", "RPB", "SPB", "JMP", "JCP", "JPA", "CAL", "CZP", "RT", "RTS",
	"CI", "CM", "CMB", "TAB", "CLI", "TMB", "TPA", "TPB",
	"TIT", "IA", "IP", "OE", "OP", "OCD", "NOP",
	"?",
	"TAW", "TAZ", "THX", "TLY", "XAW", "XAZ", "XHR", "XHX", "XLS", "XLY", "XC",
	"SFB", "RFB", "FBT", "FBF", "RAR", "INM", "DEM", "STM", "TTM", "EI", "DI"
};

// number of bits per opcode parameter, 2 digits means opcode is 2 bytes
static const UINT8 s_bits[] =
{
	4, 0, 2, 80, 4, 0, 0, 0,
	0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 83, 6, 0, 83, 4, 0, 0,
	40, 0, 2, 2, 40, 2, 2, 2,
	0, 0, 0, 0, 0, 80, 0,
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 0, 0, 0, 80, 0, 0, 0
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, _OVER, _OVER, _OUT, _OUT,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


static const UINT8 ucom4_mnemonic[0x100] =
{
	/* 0x00 */
	mNOP, mDI, mS, mTIT, mTC, mTTM, mDAA, mTAL,
	mAD, mADS, mDAS, mCLC, mCM, mINC, mOP, mDEC,
	mCMA, mCIA, mTLA, mDED, mSTM, mLDI, mCLI, mCI,
	mEXL, mADC, mXC, mSTC, mILL, mINM, mOCD, mDEM,
	/* 0x20 */
	mFBF, mFBF, mFBF, mFBF, mTAB, mTAB, mTAB, mTAB,
	mX, mXM, mXM, mXM, mXD, mXMD, mXMD, mXMD,
	mRAR, mEI, mIP, mIND, mCMB, mCMB, mCMB, mCMB,
	mL, mLM, mLM, mLM, mXI, mXMI, mXMI, mXMI,
	/* 0x40 */
	mIA, mJPA, mTAZ, mTAW, mOE, mILL, mTLY, mTHX,
	mRT, mRTS, mXAZ, mXAW, mXLS, mXHR, mXLY, mXHX,
	mTPB, mTPB, mTPB, mTPB, mTPA, mTPA, mTPA, mTPA,
	mTMB, mTMB, mTMB, mTMB, mFBT, mFBT, mFBT, mFBT,
	/* 0x60 */
	mRPB, mRPB, mRPB, mRPB, mREB, mREB, mREB, mREB,
	mRMB, mRMB, mRMB, mRMB, mRFB, mRFB, mRFB, mRFB,
	mSPB, mSPB, mSPB, mSPB, mSEB, mSEB, mSEB, mSEB,
	mSMB, mSMB, mSMB, mSMB, mSFB, mSFB, mSFB, mSFB,
	/* 0x80 */
	mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ,
	mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ, mLDZ,
	mCLA, mLI, mLI, mLI, mLI, mLI, mLI, mLI,
	mLI, mLI, mLI, mLI, mLI, mLI, mLI, mLI,
	/* 0xa0 */
	mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP, mJMP,
	mCAL, mCAL, mCAL, mCAL, mCAL, mCAL, mCAL, mCAL,
	mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP,
	mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP, mCZP,
	/* 0xc0 */
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP,
	mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP, mJCP
};



CPU_DISASSEMBLE(ucom4)
{
	int pos = 0;
	UINT8 op = oprom[pos++];
	UINT8 instr = ucom4_mnemonic[op];

	char *dst = buffer;
	dst += sprintf(dst, "%-4s ", s_mnemonics[instr]);

	// opcode parameter
	int bits = s_bits[instr];
	if (bits)
	{
		UINT16 param = op & ((1 << (bits % 10)) - 1);
		if (bits / 10)
		{
			UINT8 op2 = oprom[pos++];
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
			dst += sprintf(dst, "%d", param);
		else if (bits <= 8)
			dst += sprintf(dst, "$%02X", param);
		else
			dst += sprintf(dst, "$%03X", param);
	}

	return pos | s_flags[instr] | DASMFLAG_SUPPORTED;
}
