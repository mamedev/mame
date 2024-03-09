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

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return PAGED2LEVEL; }
	virtual u32 page_address_bits() const override { return 8; }
	virtual u32 page2_address_bits() const override { return 4; }

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics : unsigned;
	enum e_params : unsigned;
	static const char *const em_name[];
	static const u32 em_flags[];
	static const u16 ep_bits[];
	static const u8 ep_redirect_r[4];
	static const char *const ep_name[];

	std::string decode_param(u16 opcode, int param);
};

#endif
