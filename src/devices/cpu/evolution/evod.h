// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// EVOLUTION disassembler

#ifndef MAME_CPU_EVOLUTION_EVOD_H
#define MAME_CPU_EVOLUTION_EVOD_H

#pragma once

#include <functional>

class evolution_disassembler : public util::disasm_interface
{
public:
	evolution_disassembler() = default;
	virtual ~evolution_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		int size;
		void (*fct)(std::ostream &, u16, u16, u32);
	};

	static const char *const regs[];
	static const instruction instructions[];
};

#endif // MAME_CPU_EVOLUTION_EVOD_H
