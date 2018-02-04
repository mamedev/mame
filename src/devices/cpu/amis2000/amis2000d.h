// license:BSD-3-Clause
// copyright-holders:hap
/*

  AMI S2000-family disassembler

*/

#ifndef MAME_CPU_AMIS2000_AMIS2000D_H
#define MAME_CPU_AMIS2000_AMIS2000D_H

#pragma once

class amis2000_disassembler : public util::disasm_interface
{
public:
	amis2000_disassembler() = default;
	virtual ~amis2000_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		mLAB = 0, mLAE, mLAI, mLBE, mLBEP, mLBF, mLBZ, mXAB, mXABU, mXAE,
		mLAM, mXC, mXCI, mXCD, mSTM, mRSM,
		mADD, mADCS, mADIS, mAND, mXOR, mCMA, mSTC, mRSC, mSF1, mRF1, mSF2, mRF2,
		mSAM, mSZM, mSBE, mSZC, mSOS, mSZK, mSZI, mTF1, mTF2,
		mPP, mJMP, mJMS, mRT, mRTS, mNOP, mHALT,
		mINP, mOUT, mDISB, mDISN, mMVS, mPSH, mPSL, mEUR
	};

	static const char *const s_mnemonics[];
	static const s8 s_bits[];
	static const u32 s_flags[];
	static const u8 s2000_mnemonic[0x100];
};

#endif
