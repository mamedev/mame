// license:BSD-3-Clause
// copyright-holders:Philip Bennett, AJR
/*
    DSPP disassembler shim
*/

#ifndef MAME_CPU_DSPP_DSPPDASM_H
#define MAME_CPU_DSPP_DSPPDASM_H

#pragma once

class dspp_disassembler : public util::disasm_interface
{
public:
	dspp_disassembler() = default;
	virtual ~dspp_disassembler() = default;

	virtual uint32_t opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static void format_operand(std::ostream &stream, uint16_t operand);

	static offs_t dasm_control(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op);
	static offs_t dasm_super_special(std::ostream &stream, uint16_t op);
	static offs_t dasm_special(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op);
	static offs_t dasm_branch(std::ostream &stream, uint16_t op);
	static offs_t dasm_complex_branch(std::ostream &stream, uint16_t op);
	static offs_t dasm_arithmetic(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op);
};

#endif // MAME_CPU_DSPP_DSPPDASM_H
