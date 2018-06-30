// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 disassembler

*/

#ifndef MAME_CPU_E0C6200_E0C6200D_H
#define MAME_CPU_E0C6200_E0C6200D_H

#pragma once

class e0c6200_disassembler : public util::disasm_interface
{
public:
	e0c6200_disassembler() = default;
	virtual ~e0c6200_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 page2_address_bits() const override;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		em_JP, em_RETD, em_CALL, em_CALZ,
		em_LD, em_LBPX, em_ADC, em_CP, em_ADD, em_SUB, em_SBC, em_AND, em_OR, em_XOR,
		em_RLC, em_FAN, em_PSET, em_LDPX, em_LDPY, em_SET, em_RST, em_INC, em_DEC,
		em_RRC, em_ACPX, em_ACPY, em_SCPX, em_SCPY, em_PUSH, em_POP,
		em_RETS, em_RET, em_JPBA, em_HALT, em_SLP, em_NOP5, em_NOP7,
		em_NOT, em_SCF, em_SZF, em_SDF, em_EI, em_DI, em_RDF, em_RZF, em_RCF, em_ILL
	};

	enum e_params
	{
		ep_S, ep_E, ep_I, ep_R0, ep_R2, ep_R4, ep_Q,
		ep_cC, ep_cNC, ep_cZ, ep_cNZ,
		ep_A, ep_B, ep_X, ep_Y, ep_MX, ep_MY, ep_XP, ep_XH, ep_XL, ep_YP, ep_YH, ep_YL,
		ep_P, ep_F, ep_MN, ep_SP, ep_SPH, ep_SPL
	};

	static const char *const em_name[];
	static const u32 em_flags[];
	static const u16 ep_bits[];
	static const u8 ep_redirect_r[4];
	static const char *const ep_name[];

	std::string decode_param(u16 opcode, int param);
};

#endif
