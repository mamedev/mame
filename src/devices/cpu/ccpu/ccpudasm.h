// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ccpudasm.c
    Core implementation for the portable Cinematronics CPU disassembler.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#ifndef MAME_CPU_CCPU_CCPUDASM_H
#define MAME_CPU_CCPU_CCPUDASM_H

#pragma once

class ccpu_disassembler : public util::disasm_interface
{
public:
	ccpu_disassembler() = default;
	virtual ~ccpu_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
