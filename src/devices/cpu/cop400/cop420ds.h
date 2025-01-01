// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop420ds.h

    National Semiconductor COP420 disassembler.

***************************************************************************/

#ifndef MAME_CPU_COP400_COP420DS_H
#define MAME_CPU_COP400_COP420DS_H

#pragma once

class cop420_disassembler : public util::disasm_interface
{
public:
	cop420_disassembler() = default;
	virtual ~cop420_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_COP400_COP420DS_H
