// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 disassembler

    Written by Ryan Holtz
*/

#ifndef MAME_CPU_PDP8_PDP8DASM_H
#define MAME_CPU_PDP8_PDP8DASM_H

#pragma once

class pdp8_disassembler : public util::disasm_interface
{
public:
	pdp8_disassembler() = default;
	virtual ~pdp8_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
