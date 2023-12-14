// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_XTENSA_XTENSAD_H
#define MAME_CPU_XTENSA_XTENSAD_H

#pragma once


class xtensa_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	xtensa_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_XTENSA_XTENSAD_H
