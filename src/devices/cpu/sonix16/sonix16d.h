// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, AJR

// Sonix 16-bit DSP disassembler

#ifndef MAME_CPU_SONIX16_SONIX16D_H
#define MAME_CPU_SONIX16_SONIX16D_H

#pragma once

#include <functional>

class sonix16_disassembler : public util::disasm_interface
{
public:
	sonix16_disassembler() = default;
	virtual ~sonix16_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static std::string ioreg(u8 reg);

	struct instruction {
		u16 value;
		u16 mask;
		int size;
		void (*fct)(std::ostream &, u16, u16, u32);
	};

	static const char *const regs[];
	static const char *const immregs[];
	static const char *const indregs[];
	static const char *const yregs[];
	static const char *const macregs[];
	static const char *const conds[];
	static const char *const mods[];
	static const instruction instructions[];
};

#endif // MAME_CPU_SONIX16_SONIX16D_H
