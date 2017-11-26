// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/********************************************
    NEC V810 (upd70732) disassembler
  Tomasz Slanina - analog[at]op.pl
*******************************************/


#ifndef MAME_CPU_V810_V810DASM_H
#define MAME_CPU_V810_V810DASM_H

#pragma once

class v810_disassembler : public util::disasm_interface
{
public:
	v810_disassembler() = default;
	virtual ~v810_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const dRegs[];
};

#endif
