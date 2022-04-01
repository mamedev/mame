// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M32C_M32CDASM_H
#define MAME_CPU_M32C_M32CDASM_H

#pragma once

class m32c_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	m32c_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// static tables
	static const char *const s_aregs[4];
	static const char *const s_sregs_16bit[8];
	static const char *const s_sregs_24bit[8];
	static const char *const s_sregs_dma[8];
	static const char *const s_cnds[16];
	static const char *const s_dadd_ops[6];
	static const char *const s_bit_ops[8];
	static const char *const s_long_ops[4];
	static const char *const s_long_shift_ops[5];
	static const char *const s_imm1111_ops[8];
	static const char *const s_str_ops[4];
	static const char *const s_sstr_ops[4];
	static const char *const s_btst_ops[8];
	static const u8 s_short_code[4];

	// formatting helpers
	void format_imm8(std::ostream &stream, u8 imm) const;
	void format_imm16(std::ostream &stream, u16 imm) const;
	void format_imm24(std::ostream &stream, u32 imm) const;
	void format_imm32(std::ostream &stream, u32 imm) const;
	void format_label(std::ostream &stream, u32 label) const;
	void format_imm_signed(std::ostream &stream, s32 imm) const;
	void format_relative(std::ostream &stream, const char *reg, s32 disp) const;

	// internal helpers
	void dasm_abs16(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const;
	void dasm_abs24(std::ostream &stream, offs_t &pc, const data_buffer &opcodes) const;
	void dasm_operand(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 code, u8 size, bool indirect) const;
	void dasm_immediate_mode(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 code, u8 size, bool indirect, int count, bool signd) const;
	void dasm_00000001(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, bool indirect_src, bool indirect_dest) const;
	void dasm_general(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes, u8 op1, bool indirect_src, bool indirect_dest) const;
	void dasm_1101(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 op1, bool indirect_src, bool indirect_dest) const;
	void dasm_111x(std::ostream &stream, offs_t &pc, offs_t &flags, const data_buffer &opcodes, u8 op1, bool indirect_dest) const;
};

#endif // MAME_CPU_M32C_M32CDASM_H
