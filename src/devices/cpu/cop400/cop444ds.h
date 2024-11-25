// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop444ds.cpp

    National Semiconductor COP444 disassembler.

***************************************************************************/

#ifndef MAME_CPU_COP400_COP444DS_H
#define MAME_CPU_COP400_COP444DS_H

#pragma once

class cop444_disassembler : public util::disasm_interface
{
public:
	cop444_disassembler() = default;
	virtual ~cop444_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_COP400_COP444DS_H
