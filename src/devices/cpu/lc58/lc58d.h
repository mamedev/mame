// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Sanyo LC58 generic disassembler

#ifndef MAME_CPU_LC58_LC58D_H
#define MAME_CPU_LC58_LC58D_H

#pragma once

class lc58_disassembler : public util::disasm_interface
{
public:
	lc58_disassembler() = default;
	virtual ~lc58_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, const lc58_disassembler *, u16);
	};

	static const instruction instructions[];
};

#endif // MAME_CPU_LC58_LC58D_H
