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

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return PAGED; }
	virtual u32 page_address_bits() const override { return 7; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics : unsigned;
	static const char *const s_name[];
	static const u8 s_bits[];
	static const u32 s_flags[];
	static const u8 m58846_opmap[0xc0];
};

#endif // MAME_CPU_MELPS4_MELPS4D_H
