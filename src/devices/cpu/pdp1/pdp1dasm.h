// license:BSD-3-Clause
// copyright-holders:Raphael Nabet

#ifndef MAME_CPU_PDP1_PDP1DASM_H
#define MAME_CPU_PDP1_PDP1DASM_H

#pragma once

class pdp1_disassembler : public util::disasm_interface
{
public:
	pdp1_disassembler() = default;
	virtual ~pdp1_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	void ea();
};

#endif
