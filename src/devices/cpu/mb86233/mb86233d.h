// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi

#ifndef MAME_CPU_MB86233_MB86233D_H
#define MAME_CPU_MB86233_MB86233D_H

#pragma once

class mb86233_disassembler : public util::disasm_interface
{
public:
	mb86233_disassembler() = default;
	virtual ~mb86233_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const regnames[0x40];
	static std::string condition(unsigned int cond, bool invert);
	static std::string regs(u32 reg);
	static std::string memory(u32 reg, bool x1, bool bank);
	static std::string alu0_func(u32 alu);
};

#endif
