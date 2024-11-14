// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_MN1610_MN1610D_H
#define MAME_CPU_MN1610_MN1610D_H

#pragma once

#include <optional>

class mn1610_disassembler : public util::disasm_interface
{
public:
	mn1610_disassembler() = default;
	virtual ~mn1610_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

protected:
	virtual std::optional<std::string> operand(unsigned t, u16 pc, u16 data);
};

class mn1613_disassembler : public mn1610_disassembler
{
public:
	mn1613_disassembler() = default;
	virtual ~mn1613_disassembler() = default;

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

protected:
	virtual std::optional<std::string> operand(unsigned t, u16 pc, u16 data) override;
};

#endif // MAME_CPU_MN1610_MN1610D_H
