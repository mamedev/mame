// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   A T11 disassembler
 *
 *   Note: this is probably not the most efficient disassembler in the world :-)
 *
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#ifndef MAME_CPU_T11_T11DASM_H
#define MAME_CPU_T11_T11DASM_H

#pragma once

class t11_disassembler : public util::disasm_interface
{
public:
	t11_disassembler() = default;
	virtual ~t11_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const regs[8];
	u16 r16p(offs_t &pc, const data_buffer &opcodes);
	template <int Width> std::string MakeEA (int lo, offs_t &pc, const data_buffer &opcodes);

};

#endif
