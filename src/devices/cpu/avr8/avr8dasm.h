// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Atmel 8-bit AVR disassembler

    Written by Ryan Holtz
*/

#ifndef MAME_CPU_AVR8_AVR8DASM_H
#define MAME_CPU_AVR8_AVR8DASM_H

#pragma once

class avr8_disassembler : public util::disasm_interface
{
public:
	avr8_disassembler() = default;
	virtual ~avr8_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
