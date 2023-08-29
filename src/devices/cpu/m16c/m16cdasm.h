// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M16C_M16CDASM_H
#define MAME_CPU_M16C_M16CDASM_H

#pragma once

class m16c_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	m16c_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// static tables
	static const char *const s_regs[2][6];
	static const char *const s_cregs[8];
	static const char *const s_byte_ops[8];
	static const char *const s_bit_ops[14];
	static const char *const s_cnds[16];
	static const char *const s_imm76_ops[2][9];
	static const char *const s_nibmov_ops[4];
	static const char *const s_decimal_ops[2][4];

	// formatting helpers
	void format_imm8(std::ostream &stream, u8 imm) const;
	void format_imm16(std::ostream &stream, u16 imm) const;
	void format_label(std::ostream &stream, u32 label) const;
	void format_imm_signed(std::ostream &stream, s16 imm) const;
	void format_relative(std::ostream &stream, const char *reg, s32 disp) const;

	// internal helpers
	void dasm_ea(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 mode, bool size) const;
	void dasm_general(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_quick(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_shift(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_74(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_76(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_7a(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool size) const;
	void dasm_7c(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const;
	void dasm_7d(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes) const;
	void dasm_7e(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const;
	void dasm_eb(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes) const;
};

#endif // MAME_CPU_M16C_M16CDASM_H
