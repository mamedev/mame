// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_SIGMA2_SIGMA2D_H
#define MAME_CPU_SIGMA2_SIGMA2D_H

#pragma once

class sigma2_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	sigma2_disassembler();

protected:
	sigma2_disassembler(const char *const *reg_names);

	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// disassembly helpers
	offs_t dasm_memory_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const;
	virtual offs_t dasm_read_direct(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const;

	const char *const *const m_reg_names;
};

class xerox530_disassembler : public sigma2_disassembler
{
public:
	// construction/destruction
	xerox530_disassembler();

protected:
	// sigma2_disassembler overrides
	virtual offs_t dasm_read_direct(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) const override;
};

#endif // MAME_CPU_SIGMA2_SIGMA2D_H
