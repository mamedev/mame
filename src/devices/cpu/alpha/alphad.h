// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ALPHA_ALPHAD_H
#define MAME_CPU_ALPHA_ALPHAD_H

#pragma once

class alpha_disassembler : public util::disasm_interface
{
public:
	enum dasm_type : unsigned
	{
		TYPE_UNKNOWN = 0,
		TYPE_NT      = 1,
		TYPE_UNIX    = 2,
		TYPE_VMS     = 3,
	};

	alpha_disassembler(alpha_disassembler::dasm_type type = TYPE_UNKNOWN)
		: m_dasm_type(type)
	{
	}
	virtual ~alpha_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 4; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static char const *const R[];
	static char const *const F[];

	static char const *const PT[];
	static char const *const ABX[];
	static char const *const IBX[];

	dasm_type const m_dasm_type;
};

#endif // MAME_CPU_ALPHA_ALPHAD_H
