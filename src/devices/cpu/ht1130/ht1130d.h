// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_HT1130_HT1130D_H
#define MAME_CPU_HT1130_HT1130D_H

#pragma once


class ht1130_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	ht1130_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_HT1130_HT1130D_H
