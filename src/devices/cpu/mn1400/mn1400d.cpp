// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita (Panasonic) MN1400 MCU family disassembler

*/

#include "emu.h"
#include "mn1400d.h"


// common lookup tables

enum mn1400_disassembler::e_mnemonics : unsigned
{
	mILL, mILL2,
	mL, mLD, mLI, mLIC, mLDC, mST, mSTD, mSTIC, mSTDC,
	mLX, mLY, mTAX, mTAY, mTYA, mTACU, mTACL, mTCAU, mTCAL,
	mNOP, mAND, mANDI, mOR, mXOR, mA, mAI, mCPL, mC, mCI, mCY,
	mSL, mICY, mDCY, mICM, mDCM, mSM, mRM, mTB,
	mINA, mINB, mOTD, mOTMD, mOTE, mOTIE, mRCO, mSCO, mCCO,
	mRC, mRP, mSC, mSP,
	mBS0, mBS1, mBS01, mBSN0, mBSN1, mBSN01,
	mBP, mBC, mBZ, mBPC, mBPZ, mBCZ, mBPCZ,
	mBNP, mBNC, mBNZ, mBNPC, mBNPZ, mBNCZ, mBNPCZ,
	mJMP, mCAL, mRET, mEC, mDC
};

const char *const mn1400_disassembler::s_mnemonics[] =
{
	"?", "?",
	"L", "LD", "LI", "LIC", "LDC", "ST", "STD", "STIC", "STDC",
	"LX", "LY", "TAX", "TAY", "TYA", "TACU", "TACL", "TCAU", "TCAL",
	"NOP", "AND", "ANDI", "OR", "XOR", "A", "AI", "CPL", "C", "CI", "CY",
	"SL", "ICY", "DCY", "ICM", "DCM", "SM", "RM", "TB",
	"INA", "INB", "OTD", "OTMD", "OTE", "OTIE", "RCO", "SCO", "CCO",
	"RC", "RP", "SC", "SP",
	"BS0", "BS1", "BS01", "BSN0", "BSN1", "BSN01",
	"BP", "BC", "BZ", "BPC", "BPZ", "BCZ", "BPCZ",
	"BNP", "BNC", "BNZ", "BNPC", "BNPZ", "BNCZ", "BNPCZ",
	"JMP", "CAL", "RET", "EC", "DC"
};

// number of bits per opcode parameter
const u8 mn1400_disassembler::s_bits[] =
{
	0, 8,
	0, 2, 4, 0, 0, 0, 2, 0, 0,
	3, 4, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4,
	0, 0, 0, 0, 0, 4, 4, 4,
	0, 0, 0, 0, 0, 4, 0, 0, 0,
	0, 0, 0, 0,
	8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8,
	11, 11, 0, 0, 0
};

const u32 mn1400_disassembler::s_flags[] =
{
	0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, STEP_OVER, STEP_OUT, 0, 0
};


const u8 mn1400_disassembler::mn1400_mnemonic[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTAX,  mTYA,  mTAY,  mAND,  mOR,   mXOR,  mA,    mCPL,  mC,    mST,   mSTIC, mSTDC, mL,    mLIC,  mLDC,  // 0
	mOTE,  mOTMD, mOTD,  mCCO,  mINA,  mINB,  mRCO,  mSCO,  mTACL, mTACU, mTCAL, mTCAU, mDC,   mEC,   mSL,   mRET,  // 1
	mLD,   mLD,   mLD,   mLD,   mSTD,  mSTD,  mSTD,  mSTD,  mRC,   mRP,   mSC,   mSP,   mICY,  mDCY,  mICM,  mDCM,  // 2
	mLX,   mLX,   mLX,   mLX,   mLX,   mLX,   mLX,   mLX,   1,     1,     mBSN0, mBS0,  mBSN1, mBS1,  mBSN01,mBS01, // 3

	mJMP,  mJMP,  mJMP,  mJMP,  mJMP,  mJMP,  mJMP,  mJMP,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  mCAL,  // 4
	mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   mLI,   // 5
	mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   mLY,   // 6
	mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, mANDI, // 7

	mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   mAI,   // 8
	mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   mCI,   // 9
	mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   mCY,   // A
	mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   mSM,   // B

	mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   mRM,   // C
	mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   mTB,   // D
	1,     1,     mBNZ,  mBZ,   mBNC,  mBC,   mBNCZ, mBCZ,  mBNP,  mBP,   mBNPZ, mBPZ,  mBNPC, mBPC,  mBNPCZ,mBPCZ, // E
	mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE, mOTIE  // F
};


// disasm

offs_t mn1400_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t pos = pc;
	u8 op = opcodes.r8(pos++);
	u8 instr = mn1400_mnemonic[op];
	u8 bits = s_bits[instr];

	util::stream_format(stream,"%-8s", s_mnemonics[instr]);

	// opcode parameter
	if (bits != 0)
	{
		if (bits >= 8)
		{
			u16 param = opcodes.r8(pos++);
			if (bits > 8)
				param |= (op & ((1 << (bits - 8)) - 1)) << 8;
			else
				param |= pc & 0x700;

			if (instr > 1)
				util::stream_format(stream, "$%03X", param);
		}
		else
		{
			u8 param = op & ((1 << bits) - 1);
			util::stream_format(stream, "%d", param);
		}
	}

	return (pos - pc) | s_flags[instr] | SUPPORTED;
}
