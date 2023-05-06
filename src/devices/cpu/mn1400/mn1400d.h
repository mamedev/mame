// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita (Panasonic) MN1400 MCU family disassembler

*/


#ifndef MAME_CPU_MN1400_MN1400D_H
#define MAME_CPU_MN1400_MN1400D_H

#pragma once

class mn1400_disassembler : public util::disasm_interface
{
public:
	mn1400_disassembler() = default;
	virtual ~mn1400_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
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

	static const char *const s_mnemonics[];
	static const u8 s_bits[];
	static const u32 s_flags[];
	static const u8 mn1400_mnemonic[0x100];
};

#endif // MAME_CPU_MN1400_MN1400D_H
