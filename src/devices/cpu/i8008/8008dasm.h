// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   8008dasm.c
 *
 *   Intel 8008 CPU Disassembly
 *
 *****************************************************************************/

#ifndef MAME_CPU_I8008_8008DASM_H
#define MAME_CPU_I8008_8008DASM_H

#pragma once

class i8008_disassembler : public util::disasm_interface
{
public:
	i8008_disassembler() = default;
	virtual ~i8008_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char reg[];
	static const char flag_names[];

};

#endif
