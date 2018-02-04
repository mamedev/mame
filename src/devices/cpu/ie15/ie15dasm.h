// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#ifndef MAME_CPU_IE15_IE15DASM_H
#define MAME_CPU_IE15_IE15DASM_H

#pragma once

class ie15_disassembler : public util::disasm_interface
{
public:
	ie15_disassembler() = default;
	virtual ~ie15_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
