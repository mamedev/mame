// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MN10300_MN103DASM_H
#define MAME_CPU_MN10300_MN103DASM_H

#pragma once

class mn10300_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	mn10300_disassembler();

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	void format_immediate(std::ostream &stream, u32 imm) const;
	void format_immediate_unsigned(std::ostream &stream, u32 imm) const;
	void format_an_disp(std::ostream &stream, u32 imm, int n) const;
	void format_sp_disp(std::ostream &stream, u32 imm) const;
	void format_regs(std::ostream &stream, u8 regs) const;

	offs_t disassemble_f0(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f1(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f2(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f3(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f4(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f5(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f6(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f8(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_f9(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_fa(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_fb(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_fc(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_fd(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
	offs_t disassemble_fe(std::ostream &stream, offs_t pc, const data_buffer &opcodes) const;
};

#endif // MAME_CPU_MN10300_MN103DASM_H
