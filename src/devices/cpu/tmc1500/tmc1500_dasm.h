// license:BSD-3-Clause
// copyright-holders:baco, Antigravity

#ifndef MAME_CPU_TMC1500_TMC1500_DASM_H
#define MAME_CPU_TMC1500_TMC1500_DASM_H

#pragma once

class tmc1500_disassembler : public util::disasm_interface
{
public:
	tmc1500_disassembler();
	virtual ~tmc1500_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_TMC1500_TMC1500_DASM_H
