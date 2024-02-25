// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family disassembler

*/

#ifndef MAME_CPU_MELPS4_MELPS4D_H
#define MAME_CPU_MELPS4_MELPS4D_H

#pragma once

class melps4_disassembler : public util::disasm_interface
{
public:
	melps4_disassembler() = default;
	virtual ~melps4_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// opcode mnemonics
	enum e_mnemonics
	{
		mILL,
		mTAB, mTBA, mTAY, mTYA, mTEAB, mTABE, mTEPA, mTXA, mTAX,
		mLXY, mLZ, mINY, mDEY, mLCPS, mSADR,
		mTAM, mXAM, mXAMD, mXAMI,
		mLA, mAM, mAMC, mAMCS, mA, mSC, mRC, mSZC, mCMA, mRL, mRR,
		mSB, mRB, mSZB, mSEAM, mSEY,
		mTLA, mTHA, mTAJ, mXAL, mXAH, mLC7, mDEC, mSHL, mRHL, mCPA, mCPAS, mCPAE, mSZJ,
		mT1AB, mTRAB, mT2AB, mTAB1, mTABR, mTAB2, mTVA, mTWA, mSNZ1, mSNZ2,
		mBA, mSP, mB, mBM, mRT, mRTS, mRTI,
		mCLD, mCLS, mCLDS, mSD, mRD, mSZD, mOSAB, mOSPA, mOSE, mIAS, mOFA, mIAF, mOGA, mIAK, mSZK, mSU, mRU,
		mEI, mDI, mINTH, mINTL, mNOP
	};

	static const char *const s_name[];
	static const u8 s_bits[];
	static const u32 s_flags[];
	static const u8 m58846_opmap[0xc0];
};

#endif // MAME_CPU_MELPS4_MELPS4D_H
