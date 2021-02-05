// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Sanyo LC57 generic disassembler

#ifndef MAME_CPU_LC57_LC57D_H
#define MAME_CPU_LC57_LC57D_H

#pragma once

class lc57_disassembler : public util::disasm_interface
{
public:
	lc57_disassembler() = default;
	virtual ~lc57_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, const data_buffer &, u8, u16);
	};

	static const instruction instructions[];
};

#endif // MAME_CPU_LC57_LC57D_H
