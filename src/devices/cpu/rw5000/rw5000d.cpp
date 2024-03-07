// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell A/B5000 family MCU disassembler

*/

#include "emu.h"
#include "rw5000d.h"

// constructor

rw5000_common_disassembler::rw5000_common_disassembler()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x40; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		pc = increment_pc(pc);
	}
}

offs_t rw5000_common_disassembler::increment_pc(offs_t pc)
{
	int feed = ((pc & 0x3e) == 0) ? 1 : 0;
	feed ^= (pc >> 1 ^ pc) & 1;
	return (pc & ~0x3f) | (pc >> 1 & 0x1f) | (feed << 5);
}


// common lookup tables

enum rw5000_common_disassembler::e_mnemonics : unsigned
{
	mILL,
	mNOP, mRSC, mSC, mTC, mTAM,
	mLAX, mADX, mCOMP, mATB, mATBZ,
	mLDA, mEXC0, mEXCP, mEXCM, mADD,
	mLB0, mLB7, mLB8, mLB9, mLB10, mLB11,
	mRSM, mSM, mTM,
	mTL, mTRA0, mTRA1, mRET,
	mTKB, mTKBS, mTDIN, mREAD, mKSEG, mMTD
};

const char *const rw5000_common_disassembler::s_name[] =
{
	"?",
	"NOP", "RSC", "SC", "TC", "TAM",
	"LAX", "ADX", "COMP", "ATB", "ATBZ",
	"LDA", "EXC", "EXC", "EXC", "ADD",
	"LB", "LB", "LB", "LB", "LB", "LB",
	"RSM", "SM", "TM",
	"TL", "TRA", "TRA", "RET",
	"TKB", "TKBS", "TDIN", "READ", "KSEG", "MTD"
};

// number of bits per opcode parameter
// note: d4 means bitmask param, d5 means inverted
const u8 rw5000_common_disassembler::s_bits[] =
{
	0,
	0, 0, 0, 0, 0,
	0x24, 0x24, 0, 0, 0,
	2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2,
	0x12, 0x12, 0x12,
	4, 6, 6, 0,
	0, 0, 2, 0, 0, 0
};

const u32 rw5000_common_disassembler::s_flags[] =
{
	0,
	0, 0, 0, STEP_COND, STEP_COND,
	0, STEP_COND, 0, 0, 0,
	0, 0, STEP_COND, STEP_COND, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, STEP_COND,
	0, STEP_OVER, 0, STEP_OUT,
	STEP_COND, STEP_COND, STEP_COND, STEP_COND, 0, 0
};


// common disasm

offs_t rw5000_common_disassembler::common_disasm(const u8 *lut_opmap, std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// get raw opcode
	u8 op = opcodes.r8(pc);
	u8 instr = lut_opmap[op];
	u32 flags = s_flags[instr];

	// get parameter
	u8 bits = s_bits[instr];
	u8 mask = (1 << (bits & 0xf)) - 1;
	u8 param = (bits & 0x20) ? (~op & mask) : (op & mask);
	if (bits & 0x10)
		param = 1 << param;

	// TDIN 0 is 4
	if (instr == mTDIN && param == 0)
		param = 4;

	// disassemble it
	util::stream_format(stream, "%-6s", s_name[instr]);

	if (bits > 0)
	{
		// exceptions for opcodes with 2 params
		if (instr >= mEXC0 && instr <= mEXCM)
		{
			switch (instr)
			{
				case mEXC0: util::stream_format(stream, "%d,0", param); break;
				case mEXCP: util::stream_format(stream, "%d,+1", param); break;
				case mEXCM: util::stream_format(stream, "%d,-1", param); break;
				default: break;
			}
		}
		else if (instr == mADD)
		{
			if (param & 1)
				flags |= STEP_COND;

			switch (param ^ 2)
			{
				case 1: stream << "S"; break; // 0,1
				case 2: stream << "C"; break; // 1,0
				case 3: stream << "C,S"; break; // 1,1
				default: break;
			}
		}
		else if (instr >= mLB0 && instr <= mLB11)
		{
			int param2 = (instr == mLB0) ? 0 : (6 + instr - mLB0);
			util::stream_format(stream, "%d,%d", param, param2);
		}
		else if (instr == mTRA0 || instr == mTRA1)
		{
			int param2 = (instr == mTRA1) ? 1 : 0;
			util::stream_format(stream, "%d,$%02X", param2, param);
		}
		else
			util::stream_format(stream, "%d", param);
	}

	return 1 | flags | SUPPORTED;
}


// A5000/A5900 disasm

const u8 a5000_disassembler::a5000_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   0,     mTKB,  mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   0,     0,     0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  0,     0,     0,     0,     // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   0,     mRSC,  0,     mSC,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     mMTD,  mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t a5000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(a5000_opmap, stream, pc, opcodes, params);
}


// A5500 disasm (A5000 + LB x,11, SC/RSC moved to make room for more TL)

const u8 a5500_disassembler::a5500_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   0,     mTKB,  mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   mSC,   mRSC,  0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  mLB11, mLB11, mLB11, mLB11, // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     mMTD,  mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t a5500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(a5500_opmap, stream, pc, opcodes, params);
}


// B5000 disasm (A5000 + TKBS added, MTD removed)

const u8 b5000_disassembler::b5000_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   mTKB,  mTKBS, mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   0,     0,     0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  0,     0,     0,     0,     // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   0,     mRSC,  0,     mSC,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     0,     mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t b5000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b5000_opmap, stream, pc, opcodes, params);
}


// B5500 disasm (B5000 + LB x,11, SC/RSC moved to make room for more TL)

const u8 b5500_disassembler::b5500_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   mTKB,  mTKBS, mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   mSC,   mRSC,  0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  mLB11, mLB11, mLB11, mLB11, // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     0,     mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t b5500_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b5500_opmap, stream, pc, opcodes, params);
}


// B6000 disasm (B5000 + ATBZ added)

const u8 b6000_disassembler::b6000_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   mTKB,  mTKBS, mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   0,     0,     0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  0,     0,     0,     0,     // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   0,     mRSC,  0,     mSC,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     mATBZ, mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t b6000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b6000_opmap, stream, pc, opcodes, params);
}


// B6100 disasm (B5500 + B6000)

const u8 b6100_disassembler::b6100_opmap[0x100] =
{
//  0      1      2      3      4      5      6      7      8      9      A      B      C      D      E      F
	mNOP,  mTC,   mTKB,  mTKBS, mTDIN, mTDIN, mTDIN, mTDIN, mTM,   mTM,   mTM,   mTM,   mSC,   mRSC,  0,     0,     // 0
	mSM,   mSM,   mSM,   mSM,   mRSM,  mRSM,  mRSM,  mRSM,  mRET,  mRET,  mRET,  mRET,  mLB11, mLB11, mLB11, mLB11, // 1
	mLB7,  mLB7,  mLB7,  mLB7,  mLB10, mLB10, mLB10, mLB10, mLB9,  mLB9,  mLB9,  mLB9,  mLB8,  mLB8,  mLB8,  mLB8,  // 2
	mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mTL,   mLB0,  mLB0,  mLB0,  mLB0,  // 3

	mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  mLAX,  // 4
	mLDA,  mLDA,  mLDA,  mLDA,  mEXCP, mEXCP, mEXCP, mEXCP, mEXC0, mEXC0, mEXC0, mEXC0, mEXCM, mEXCM, mEXCM, mEXCM, // 5
	mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mADX,  mREAD, // 6
	mADD,  mADD,  mADD,  mADD,  mKSEG, 0,     mATBZ, mATB,  mCOMP, mCOMP, mCOMP, mCOMP, mTAM,  mTAM,  mTAM,  mTAM,  // 7

	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 8
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // 9
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // A
	mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, mTRA0, // B

	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // C
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // D
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // E
	mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, mTRA1, // F
};

offs_t b6100_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	return common_disasm(b6100_opmap, stream, pc, opcodes, params);
}
