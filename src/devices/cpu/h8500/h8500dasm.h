// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8500DASM_H
#define MAME_CPU_H8500_H8500DASM_H

#pragma once

class h8500_disassembler : public util::disasm_interface
{
public:
	h8500_disassembler(bool expanded = true);

protected:
	// disasm_interface overrides
	virtual offs_t opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// instruction mnemonic tables
	static const char *const s_general_ops[0x20];
	static const char *const s_bit_ops[4];
	static const char *const s_unary_ops[0x10];
	static const char *const s_branches[0x10];

	// formatting helpers
	void format_reg(std::ostream &stream, u8 n, bool w);
	void format_creg(std::ostream &stream, u8 n, bool w);
	void format_reglist(std::ostream &stream, u8 x);
	void format_imm8(std::ostream &stream, u8 x);
	void format_imm16(std::ostream &stream, u16 x);
	void format_bdisp(std::ostream &stream, s16 disp, offs_t pc);
	void format_ea(std::ostream &stream, u8 ea, u16 disp);

	// disassembly helpers
	offs_t dasm_illegal(std::ostream &stream, u8 op);
	offs_t dasm_general(std::ostream &stream, offs_t pc, u8 ea, const data_buffer &opcodes);
	offs_t dasm_misc(std::ostream &stream, offs_t pc, u8 ea, const data_buffer &opcodes);
	offs_t dasm_immop(std::ostream &stream, offs_t pc, bool w, const data_buffer &opcodes);

	const bool m_expanded;
};

#endif // MAME_CPU_H8500_H8500DASM_H
