// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

	SunPlus micro'nSP disassembler

*****************************************************************************/

#ifndef MAME_CPU_UNSP_UNSPDASM_H
#define MAME_CPU_UNSP_UNSPDASM_H

#pragma once

class unsp_disassembler : public util::disasm_interface
{
public:
	unsp_disassembler() = default;
	virtual ~unsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static char const *const reg[];
	static char const *const jmp[];
	static char const *const alu[];
};

#endif
