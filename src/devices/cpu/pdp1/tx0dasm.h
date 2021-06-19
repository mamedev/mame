// license:BSD-3-Clause
// copyright-holders:Raphael Nabet

#ifndef MAME_CPU_PDP1_TX0DASM_H
#define MAME_CPU_PDP1_TX0DASM_H

#pragma once

class tx0_64kw_disassembler : public util::disasm_interface
{
public:
	tx0_64kw_disassembler() = default;
	virtual ~tx0_64kw_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

class tx0_8kw_disassembler : public util::disasm_interface
{
public:
	tx0_8kw_disassembler() = default;
	virtual ~tx0_8kw_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
