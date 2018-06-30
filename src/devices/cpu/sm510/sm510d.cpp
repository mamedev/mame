// license:BSD-3-Clause
// copyright-holders:hap, Jonathan Gevaryahu
/*

  Sharp SM5xx MCU family disassembler

*/

#include "emu.h"
#include "sm510d.h"


// common lookup tables

const char *const sm510_common_disassembler::s_mnemonics[] =
{
	// SM510
	"?", "",
	"LB", "LBL", "SBM", "EXBLA", "INCB", "DECB",
	"ATPL", "RTN0", "RTN1", "TL", "TML", "TM", "T",
	"EXC", "BDC", "EXCI", "EXCD", "LDA", "LAX", "PTW", "WR", "WS",
	"KTA", "ATBP", "ATX", "ATL", "ATFC", "ATR",
	"ADD", "ADD11", "ADX", "COMA", "ROT", "RC", "SC",
	"TB", "TC", "TAM", "TMI", "TA0", "TABL", "TIS", "TAL", "TF1", "TF4",
	"RM", "SM",
	"PRE", "SME", "RME", "TMEL",
	"SKIP", "CEND", "IDIV", "DR", "DTA", "CLKLO", "CLKHI",

	// SM500
	"COMCB", "RTN", "RTNS", "SSR", "TR", "TRS", "RBM",
	"ADDC", "PDTW", "TW", "DTW",
	"ATS", "EXKSA", "EXKFA",
	"RMF", "SMF", "COMCN",
	"TA", "TM", "TG",

	// SM530
	"SABM", "SABL", "EXBL",
	"TG", "TBA",
	"KETA", "ATF", "SDS", "RDS",
	"INIS",

	// SM590
	"NOP", "CCTRL", "INBL", "DEBL", "XBLA", "ADCS", "TR",
	// "
	"TAX", "LBLX", "MTR", "STR", "INBM", "DEBM", "RTA", "BLTA", "EXAX", "TBA", "ADS", "ADC", "LBMX", "TLS"
};

// number of bits per opcode parameter, 8 or larger means 2-byte opcode
const u8 sm510_common_disassembler::s_bits[] =
{
	// SM510
	0, 8,
	4, 8, 0, 0, 0, 0,
	0, 0, 0, 4+8, 2+8, 6, 6,
	2, 0, 2, 2, 2, 4, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 4, 0, 0, 0, 0,
	0, 0, 0, 2, 0, 0, 0, 0, 0, 0,
	2, 2,
	8, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,

	// SM500
	0, 0, 0, 4, 6, 6, 0,
	0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 2, 0,

	// SM530
	0, 0, 0,
	2, 0,
	0, 0, 0, 0,
	0,

	// SM590
	0, 0, 0, 0, 0, 0, 7,
	// "
	4, 4, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2+8
};

const u32 sm510_common_disassembler::s_flags[] =
{
	// SM510
	0, 0,
	0, 0, 0, 0, 0, 0,
	0, STEP_OUT, STEP_OUT, 0, STEP_OVER, STEP_OVER, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	0, 0, 0, 0,
	0, STEP_OVER, 0, 0, 0, 0, 0,

	// SM500
	0, STEP_OUT, STEP_OUT, 0, 0, STEP_OVER, 0,
	0, 0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,

	// SM530
	0, 0, 0,
	0, 0,
	0, 0, 0, 0,
	0,

	// SM590
	0, 0, 0, 0, 0, 0, STEP_OVER,
	// "
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STEP_OVER
};


// common disasm

offs_t sm510_common_disassembler::common_disasm(const u8 *lut_mnemonic, const u8 *lut_extended, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params, const u8 pclen)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_mnemonic[op];
	int len = 1;

	int bits = s_bits[instr];
	u8 mask = op & ((1 << (bits & 7)) - 1);
	u16 param = mask;
	if (bits >= 8)
	{
		if (pclen == 6)
		{
			int feed = ((pc >> 1 ^ pc) & 1) ? 0 : 0x20;
			pc = feed | (pc >> 1 & 0x1f) | (pc & ~0x3f);
		}
		else if (pclen == 7)
		{
			int feed = ((pc >> 1 ^ pc) & 1) ? 0 : 0x40;
			pc = feed | (pc >> 1 & 0x3f) | (pc & ~0x7f);
		}
		else
			abort();
		param = params.r8(pc);
		len++;
	}

	// extended opcode
	bool is_extended = (instr == mEXT);
	if (is_extended)
		instr = lut_extended[param];

	// disassemble it
	util::stream_format(stream, "%-6s ", s_mnemonics[instr]);
	if (bits > 0)
	{
		if (bits <= 4)
		{
			if (param < 10)
				util::stream_format(stream, "%d", param);
			else
				util::stream_format(stream, "$%X", param);
		}
		else if (bits <= 8)
		{
			if (!is_extended)
				util::stream_format(stream, "$%02X", param);
		}
		else
		{
			u16 address = (param << 4 & 0xc00) | (mask << 6 & 0x3c0) | (param & 0x03f);
			util::stream_format(stream, "$%03X", address);
		}
	}

	return len | s_flags[instr] | SUPPORTED;
}


// SM510 disasm

const u8 sm510_disassembler::sm510_mnemonic[0x100] =
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

offs_t sm510_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(sm510_mnemonic, nullptr, stream, pc, opcodes, params, 6);
}


// SM511 disasm

const u8 sm511_disassembler::sm511_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mROT,  mDTA,  mSBM,  mATPL, mRM,   mRM,   mRM,   mRM,   mADD,  mADD11,mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	mKTA,  mTB,   mTC,   mTAM,  mTMI,  mTMI,  mTMI,  mTMI,  mTIS,  mATL,  mTA0,  mTABL, mATX,  mCEND, mTAL,  mLBL,  // 5
	mEXT,  mPRE,  mWR,   mWS,   mINCB, mDR,   mRC,   mSC,   mTML,  mTML,  mTML,  mTML,  mDECB, mPTW,  mRTN0, mRTN1, // 6
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   // 7

	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 8
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // 9
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // A
	mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    mT,    // B

	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // C
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // D
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   // E
	mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM,   mTM    // F
};

const u8 sm511_disassembler::sm511_extended[0x10] =
{
	mRME,  mSME,  mTMEL, mATFC, mBDC,  mATBP, mCLKHI,mCLKLO,0,     0,     0,     0,     0,     0,     0,     0      // 60 3
};

offs_t sm511_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// create extended opcode table
	u8 ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x30, sm511_extended, 0x10);

	return common_disasm(sm511_mnemonic, ext, stream, pc, opcodes, params, 6);
}


// SM500 disasm

const u8 sm500_disassembler::sm500_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mSKIP, mATR,  mEXKSA,mATBP, mRM,   mRM,   mRM,   mRM,   mADD,  mADDC, mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mATS,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	mTA,   mTB,   mTC,   mTAM,  mTM2,  mTM2,  mTM2,  mTM2,  mTG,   mPTW,  mTA0,  mTABL, mTW,   mDTW,  mEXT,  mLBL,  // 5
	mCOMCN,mPDTW, mWR,   mWS,   mINCB, mIDIV, mRC,   mSC,   mRMF,  mSMF,  mKTA,  mEXKFA,mDECB, mCOMCB,mRTN,  mRTNS, // 6
	mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  // 7

	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 8
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 9
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // A
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // B

	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // C
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // D
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // E
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS   // F
};

const u8 sm500_disassembler::sm500_extended[0x10] =
{
	mCEND, 0,     0,     0,     mDTA,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0      // 5E 0
};

offs_t sm500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// create extended opcode table
	u8 ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x00, sm500_extended, 0x10);

	return common_disasm(sm500_mnemonic, ext, stream, pc, opcodes, params, 6);
}


// SM5A disasm

const u8 sm5a_disassembler::sm5a_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mSKIP, mATR,  mSBM,  mATBP, mRM,   mRM,   mRM,   mRM,   mADD,  mADDC, mCOMA, mEXBLA,mSM,   mSM,   mSM,   mSM,   // 0
	mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mLDA,  mLDA,  mLDA,  mLDA,  mEXCD, mEXCD, mEXCD, mEXCD, // 1
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 2
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 3

	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 4
	mTA,   mTB,   mTC,   mTAM,  mTM2,  mTM2,  mTM2,  mTM2,  mTG,   mPTW,  mTA0,  mTABL, mTW,   mDTW,  mEXT,  mLBL,  // 5
	mCOMCN,mPDTW, mWR,   mWS,   mINCB, mIDIV, mRC,   mSC,   mRMF,  mSMF,  mKTA,  mRBM,  mDECB, mCOMCB,mRTN,  mRTNS, // 6
	mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  mSSR,  // 7

	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 8
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 9
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // A
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // B

	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // C
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // D
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // E
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS   // F
};

const u8 sm5a_disassembler::sm5a_extended[0x10] =
{
	mCEND, 0,     0,     0,     mDTA,  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0      // 5E 0
};

offs_t sm5a_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// create extended opcode table
	u8 ext[0x100];
	memset(ext, 0, 0x100);
	memcpy(ext + 0x00, sm5a_extended, 0x10);

	return common_disasm(sm5a_mnemonic, ext, stream, pc, opcodes, params, 6);
}


// SM530 disasm

const u8 sm530_disassembler::sm530_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 0 - note: $00 has synonym SKIP
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 1
	mLDA,  mLDA,  mLDA,  mLDA,  mEXC,  mEXC,  mEXC,  mEXC,  mEXCI, mEXCI, mEXCI, mEXCI, mEXCD, mEXCD, mEXCD, mEXCD, // 2
	mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   mLB,   // 3

	mRM,   mRM,   mRM,   mRM,   mSM,   mSM,   mSM,   mSM,   mTM2,  mTM2,  mTM2,  mTM2,  mINCB, mDECB, mRDS,  mSDS,  // 4
	mKTA,  mKETA, mDTA,  mCOMA, mADD,  mADDC, mRC,   mSC,   mTABL, mTAM,  mEXBL, mTC,   mATS,  mATF,  mATBP, 0,     // 5
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mRTN,  mRTNS, mATPL, mLBL,  mTG2,  mTG2,  mTG2,  mTG2,  // 6
	mIDIV, mINIS, mSABM, mSABL, mCEND, mTMEL, mRME,  mSME,  mPRE,  mTBA,  0,     0,     0,     0,     0,     0,     // 7

	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 8
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // 9
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // A
	mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   mTR,   // B

	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // C
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // D
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  // E
	mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS,  mTRS   // F
};

offs_t sm530_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(sm530_mnemonic, nullptr, stream, pc, opcodes, params, 6);
}


// SM590 disasm

const u8 sm590_disassembler::sm590_mnemonic[0x100] =
{
/*  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F  */
	mNOP,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  // 0
	mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  mTAX,  // 1
	mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, mLBLX, // 2
	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 3

	mLDA,  mEXC,  mEXCI, mEXCD, mCOMA, mTAM,  mATR,  mMTR,  mRC,   mSC,   mSTR,  mCCTRL,mRTN,  mRTNS, 0,     0,     // 4
	mINBM, mDEBM, mINBL, mDEBL, mTC,   mRTA,  mBLTA, mXBLA, 0,     0,     0,     0,     mATX,  mEXAX, 0,     0,     // 5
	mTMI,  mTMI,  mTMI,  mTMI,  mTBA2, mTBA2, mTBA2, mTBA2, mRM,   mRM,   mRM,   mRM,   mSM,   mSM,   mSM,   mSM,   // 6
	mADD,  mADS,  mADC,  mADCS, mLBMX, mLBMX, mLBMX, mLBMX, mTL,   mTL,   mTL,   mTL,   mTLS,  mTLS,  mTLS,  mTLS,  // 7

	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // 8
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // 9
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // A
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // B

	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // C
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // D
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  // E
	mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7,  mTR7   // F
};


offs_t sm590_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(sm590_mnemonic, nullptr, stream, pc, opcodes, params, 7);
}


// Sarayan's little helpers

offs_t sm510_common_disassembler::pc_linear_to_real(offs_t pc) const
{
	static const u8 l2r[64] = {
		0x00, 0x20, 0x30, 0x38, 0x3c, 0x3e, 0x1f, 0x2f, 0x37, 0x3b, 0x3d, 0x1e, 0x0f, 0x27, 0x33, 0x39,
		0x1c, 0x2e, 0x17, 0x2b, 0x35, 0x1a, 0x0d, 0x06, 0x03, 0x21, 0x10, 0x28, 0x34, 0x3a, 0x1d, 0x0e,
		0x07, 0x23, 0x31, 0x18, 0x2c, 0x36, 0x1b, 0x2d, 0x16, 0x0b, 0x25, 0x12, 0x09, 0x04, 0x22, 0x11,
		0x08, 0x24, 0x32, 0x19, 0x0c, 0x26, 0x13, 0x29, 0x14, 0x2a, 0x15, 0x0a, 0x05, 0x02, 0x01, 0x3f,
	};
	return (pc & ~0x3f) | l2r[pc & 0x3f];
}

offs_t sm510_common_disassembler::pc_real_to_linear(offs_t pc) const
{
	static const u8 r2l[64] = {
		0x00, 0x3e, 0x3d, 0x18, 0x2d, 0x3c, 0x17, 0x20, 0x30, 0x2c, 0x3b, 0x29, 0x34, 0x16, 0x1f, 0x0c,
		0x1a, 0x2f, 0x2b, 0x36, 0x38, 0x3a, 0x28, 0x12, 0x23, 0x33, 0x15, 0x26, 0x10, 0x1e, 0x0b, 0x06,
		0x01, 0x19, 0x2e, 0x21, 0x31, 0x2a, 0x35, 0x0d, 0x1b, 0x37, 0x39, 0x13, 0x24, 0x27, 0x11, 0x07,
		0x02, 0x22, 0x32, 0x0e, 0x1c, 0x14, 0x25, 0x08, 0x03, 0x0f, 0x1d, 0x09, 0x04, 0x0a, 0x05, 0x3f,
	};
	return (pc & ~0x3f) | r2l[pc & 0x3f];
}

offs_t sm590_disassembler::pc_linear_to_real(offs_t pc) const
{
	static const u8 l2r[128] = {
		0x00, 0x40, 0x60, 0x70, 0x78, 0x7c, 0x7e, 0x3f, 0x5f, 0x6f, 0x77, 0x7b, 0x7d, 0x3e, 0x1f, 0x4f,
		0x67, 0x73, 0x79, 0x3c, 0x5e, 0x2f, 0x57, 0x6b, 0x75, 0x3a, 0x1d, 0x0e, 0x07, 0x43, 0x61, 0x30,
		0x58, 0x6c, 0x76, 0x3b, 0x5d, 0x2e, 0x17, 0x4b, 0x65, 0x32, 0x19, 0x0c, 0x46, 0x23, 0x51, 0x28,
		0x54, 0x6a, 0x35, 0x1a, 0x0d, 0x06, 0x03, 0x41, 0x20, 0x50, 0x68, 0x74, 0x7a, 0x3d, 0x1e, 0x0f,
		0x47, 0x63, 0x71, 0x38, 0x5c, 0x6e, 0x37, 0x5b, 0x6d, 0x36, 0x1b, 0x4d, 0x26, 0x13, 0x49, 0x24,
		0x52, 0x29, 0x14, 0x4a, 0x25, 0x12, 0x09, 0x04, 0x42, 0x21, 0x10, 0x48, 0x64, 0x72, 0x39, 0x1c,
		0x4e, 0x27, 0x53, 0x69, 0x34, 0x5a, 0x2d, 0x16, 0x0b, 0x45, 0x22, 0x11, 0x08, 0x44, 0x62, 0x31,
		0x18, 0x4c, 0x66, 0x33, 0x59, 0x2c, 0x56, 0x2b, 0x55, 0x2a, 0x15, 0x0a, 0x05, 0x02, 0x01, 0x7f,
	};
	return (pc & ~0x7f) | l2r[pc & 0x7f];
}

offs_t sm590_disassembler::pc_real_to_linear(offs_t pc) const
{
	static const u8 r2l[128] = {
		0x00, 0x7e, 0x7d, 0x36, 0x57, 0x7c, 0x35, 0x1c, 0x6c, 0x56, 0x7b, 0x68, 0x2b, 0x34, 0x1b, 0x3f,
		0x5a, 0x6b, 0x55, 0x4d, 0x52, 0x7a, 0x67, 0x26, 0x70, 0x2a, 0x33, 0x4a, 0x5f, 0x1a, 0x3e, 0x0e,
		0x38, 0x59, 0x6a, 0x2d, 0x4f, 0x54, 0x4c, 0x61, 0x2f, 0x51, 0x79, 0x77, 0x75, 0x66, 0x25, 0x15,
		0x1f, 0x6f, 0x29, 0x73, 0x64, 0x32, 0x49, 0x46, 0x43, 0x5e, 0x19, 0x23, 0x13, 0x3d, 0x0d, 0x07,
		0x01, 0x37, 0x58, 0x1d, 0x6d, 0x69, 0x2c, 0x40, 0x5b, 0x4e, 0x53, 0x27, 0x71, 0x4b, 0x60, 0x0f,
		0x39, 0x2e, 0x50, 0x62, 0x30, 0x78, 0x76, 0x16, 0x20, 0x74, 0x65, 0x47, 0x44, 0x24, 0x14, 0x08,
		0x02, 0x1e, 0x6e, 0x41, 0x5c, 0x28, 0x72, 0x10, 0x3a, 0x63, 0x31, 0x17, 0x21, 0x48, 0x45, 0x09,
		0x03, 0x42, 0x5d, 0x11, 0x3b, 0x18, 0x22, 0x0a, 0x04, 0x12, 0x3c, 0x0b, 0x05, 0x0c, 0x06, 0x7f,
	};
	return (pc & ~0x7f) | r2l[pc & 0x7f];
}
