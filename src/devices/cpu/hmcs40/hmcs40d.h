// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS40 MCU family disassembler

*/

#ifndef MAME_CPU_HMCS40_HMCS40D_H
#define MAME_CPU_HMCS40_HMCS40D_H

#pragma once

class hmcs40_disassembler : public util::disasm_interface
{
public:
	hmcs40_disassembler();
	virtual ~hmcs40_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return NONLINEAR_PC | PAGED; }
	virtual u32 page_address_bits() const override { return 6; }
	virtual offs_t pc_linear_to_real(offs_t pc) const override { return (pc & ~0x3f) | m_l2r[pc & 0x3f]; }
	virtual offs_t pc_real_to_linear(offs_t pc) const override { return (pc & ~0x3f) | m_r2l[pc & 0x3f]; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
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

	static const char *const s_mnemonics[];
	static const s8 s_bits[];
	static const u32 s_flags[];
	static const u8 hmcs40_mnemonic[0x400];

	u8 m_l2r[0x40];
	u8 m_r2l[0x40];
};

#endif
