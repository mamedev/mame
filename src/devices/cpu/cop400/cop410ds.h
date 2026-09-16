// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop410ds.h

    National Semiconductor COP410 disassembler.

***************************************************************************/

#ifndef MAME_CPU_COP400_COP410DS_H
#define MAME_CPU_COP400_COP410DS_H

#pragma once

class cop410_disassembler : public util::disasm_interface
{
public:
	cop410_disassembler() = default;
	virtual ~cop410_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_COP400_COP410DS_H
