// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable I8085A disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_I8085_8085DASM_H
#define MAME_CPU_I8085_8085DASM_H

#pragma once

class i8085_disassembler : public util::disasm_interface
{
public:
	i8085_disassembler() = default;
	virtual ~i8085_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
