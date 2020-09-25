// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H16_H16DASM_H
#define MAME_CPU_H16_H16DASM_H

#pragma once

class h16_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	h16_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum class ea_mode {
		SIGNED,
		UNSIGNED,
		DESTINATION,
		MEMORY,
		PLAIN_MEMORY,
		ADDRESS,
		BITFIELD
	};

	static const char *const s_conditions[16];
	static const char *const s_term_conditions[16];
	static const char *const s_basic_ops[4];
	static const char *const s_logical_ops[3];
	static const char *const s_sft_ops[8];
	static const char *const s_bit_ops[4];
	static const char *const s_bf_ops[3];
	static const char *const s_cr_ops[5];

	// formatting/disassembly helpers
	void format_register(std::ostream &stream, char bank, u8 n) const;
	void format_register_to_register(std::ostream &stream, u8 rr) const;
	void format_register_list(std::ostream &stream, u16 rl) const;
	u8 get_cr_size(u8 cr) const;
	void format_cr(std::ostream &stream, u8 cr) const;
	void format_s8(std::ostream &stream, u8 data) const;
	void format_s16(std::ostream &stream, u16 data) const;
	void format_s32(std::ostream &stream, u32 data) const;
	void dasm_displacement(std::ostream &stream, offs_t &pc, const data_buffer &opcodes, u8 sd) const;
	void dasm_ea(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, u8 ea, char bank, ea_mode mode, u8 sz) const;
	void dasm_eas_ead(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, ea_mode smode, ea_mode dmode, u8 sz) const;
	void dasm_eas_ead(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, ea_mode smode, ea_mode dmode, u8 szs, u8 szd) const;
	void dasm_branch_disp(std::ostream &stream, offs_t &pc, const h16_disassembler::data_buffer &opcodes, u8 sz) const;
};

#endif // MAME_CPU_H16_H16DASM_H
