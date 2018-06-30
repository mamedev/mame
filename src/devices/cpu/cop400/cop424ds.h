// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop424ds.c

    National Semiconductor COP424 Emulator.

***************************************************************************/

#ifndef MAME_CPU_COP424_COP424DS_H
#define MAME_CPU_COP424_COP424DS_H

#pragma once

class cop424_disassembler : public util::disasm_interface
{
public:
	cop424_disassembler() = default;
	virtual ~cop424_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
