// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_CR16C_CR16CDASM_H
#define MAME_CPU_CR16C_CR16CDASM_H 1

#pragma once

class cr16c_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	cr16c_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	// formatting helpers
	void format_reg(std::ostream &stream, u8 value);
	void format_rpair(std::ostream &stream, u8 value);
	void format_rrpair(std::ostream &stream, u8 value);
	void format_rproc(std::ostream &stream, u8 value, bool d);
	void format_imm20(std::ostream &stream, u32 imm);
	void format_imm32(std::ostream &stream, u32 imm);
	void format_abs20(std::ostream &stream, u32 addr);
	void format_abs24(std::ostream &stream, u32 addr);
	void format_disp4(std::ostream &stream, u8 disp);
	void format_disp4_x2(std::ostream &stream, u8 disp);
	void format_disp14(std::ostream &stream, u16 disp);
	void format_disp16(std::ostream &stream, u16 disp);
	void format_disp20(std::ostream &stream, u32 disp);
	void format_disp20_neg(std::ostream &stream, u32 disp);
	void format_pc_disp4(std::ostream &stream, offs_t pc, u8 disp);
	void format_pc_disp8(std::ostream &stream, offs_t pc, u8 disp);
	void format_pc_disp16(std::ostream &stream, offs_t pc, u16 disp);
	void format_pc_disp24(std::ostream &stream, offs_t pc, u32 disp);
	void format_excp_vect(std::ostream &stream, u8 value);

	// disassembly helpers
	offs_t dasm_imm4_16_reg(std::ostream &stream, offs_t pc, u16 opcode, bool i, const data_buffer &opcodes);
	offs_t dasm_imm4_16_rpair(std::ostream &stream, offs_t pc, u16 opcode, const data_buffer &opcodes);

private:
	// static tables
	static const char *const s_cc[14];
};

#endif // MAME_CPU_CR16C_CR16CDASM_H
