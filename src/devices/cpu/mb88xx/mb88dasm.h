// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*******************************************************************************

    mb88dasm.c
    Core implementation for the portable Fujitsu MB88xx series MCU disassembler.

    Written by Ernesto Corvi

*******************************************************************************/


#ifndef MAME_CPU_MB88XX_MB88DASM_H
#define MAME_CPU_MB88XX_MB88DASM_H

#pragma once

class mb88_disassembler : public util::disasm_interface
{
public:
	mb88_disassembler() = default;
	virtual ~mb88_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
