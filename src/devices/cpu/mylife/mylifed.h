// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MYLIFE_MYLIFED_H
#define MAME_CPU_MYLIFE_MYLIFED_H

#pragma once

class mylife_disassembler : public util::disasm_interface
{
public:
	mylife_disassembler();

protected:
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_MYLIFE_MYLIFED_H
