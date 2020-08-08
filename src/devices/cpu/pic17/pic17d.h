// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_PIC17_PIC17D_H
#define MAME_CPU_PIC17_PIC17D_H

#pragma once

class pic17_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	pic17_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// register names
	static const char *const s_peripheral_regs[0x20];

	// instruction mnemonics
	static const char *const s_tb_ops[4];
	static const char *const s_bit_ops[4];
	static const char *const s_cp_ops[4];
	static const char *const s_alu_ops[0x30 / 2];

	// internal helpers
	void format_register(std::ostream &stream, u8 reg) const;
	void format_literal(std::ostream &stream, u8 data) const;
	void format_address(std::ostream &stream, u16 dst) const;
};

#endif // MAME_CPU_PIC17_PIC17D_H
