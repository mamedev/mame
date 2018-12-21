// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*******************************************************************

sm8500d.c
Sharp sm8500 CPU disassembly



*******************************************************************/

#ifndef MAME_CPU_SM8500_SM8500D_H
#define MAME_CPU_SM8500_SM8500D_H

#pragma once

class sm8500_disassembler : public util::disasm_interface
{
public:
	sm8500_disassembler() = default;
	virtual ~sm8500_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		zADC=0, zADCW, zADD, zADDW, zAND, zANDW, zBAND, zBBC, zBBS,
		zBCLR, zBCMP, zBMOV, zBOR, zBR, zBTST, zBSET, zBXOR, zCALL, zCALS, zCLR,
		zCLRC, zCMP, zCMPW, zCOM, zCOMC, zDA, zDBNZ, zDEC,
		zDECW, zDI, zDIV, zEI, zEXTS, zHALT, zINC, zINCW,
		zIRET, zJMP, zMOV, zMOVM, zMOVW, zMULT, zNEG, zNOP, zOR,
		zORW, zPOP, zPOPW, zPUSH, zPUSHW, zRET, zRL, zRLC,
		zRR, zRRC, zSBC, zSBCW, zSETC, zSLL, zSRA, zSRL, zSTOP,
		zSUB, zSUBW, zSWAP, zXOR, zXORW, zMOVPS0, zINVLD, zDM,
		/* unknowns */
		z5A, z5B,

		/* more complicated instructions */
		z1A, z1B, z4F
	};

	/* instructions not found:
	   5A, 5B,
	*/

	enum e_addrmodes {
		AM_R=1, AM_rr, AM_r1, AM_S, AM_rmb, AM_mbr, AM_Ri, AM_rmw, AM_mwr, AM_smw, AM_mws,
		AM_Sw, AM_iR, AM_rbr, AM_riw, AM_cjp, AM_rib, AM_pi, AM_cbr, AM_i, AM_ii,
		AM_ss, AM_RR, AM_2, AM_SS, AM_bR, AM_Rbr, AM_Rb, AM_rR, AM_Rr, AM_Rii, AM_RiR,
		AM_riB, AM_iS, AM_CALS, AM_bid, AM_1A, AM_1B, AM_4F, AM_5A, AM_5B
	};

	struct sm8500dasm
	{
		uint8_t   mnemonic;
		uint8_t   arguments;
	};

	static const char *const s_mnemonic[];
	static const uint32_t s_flags[];
	static const char *const sm8500_cond[16];
	static const uint8_t sm8500_b2w[8];
	static const sm8500dasm mnemonic[256];
};

#endif
