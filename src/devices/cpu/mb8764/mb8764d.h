// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

    Fujitsu MB8764 family DSP disassembler

***************************************************************************/

#ifndef MAME_CPU_MB8764_MB8764D_H
#define MAME_CPU_MB8764_MB8764D_H

#pragma once

class mb8764_disassembler : public util::disasm_interface
{
public:
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static std::string alu(u32 op);
	static std::string regs(u32 field, bool group);
	static std::string ram(u32 op);
	static std::string mask(u32 op);
	static std::string condition(u32 op);
};

#endif // MAME_CPU_MB8764_MB8764D_H
