// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family disassembler

*/


#ifndef MAME_CPU_UCOM4_UCOM4DASM_H
#define MAME_CPU_UCOM4_UCOM4DASM_H

#pragma once

class ucom4_disassembler : public util::disasm_interface
{
public:
	ucom4_disassembler() = default;
	virtual ~ucom4_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 page2_address_bits() const override;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
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

	static const char *const s_mnemonics[];
	static const u8 s_bits[];
	static const u32 s_flags[];
	static const u8 ucom4_mnemonic[0x100];
};

#endif
