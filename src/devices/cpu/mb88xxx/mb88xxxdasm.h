// license:BSD-3-Clause
// copyright-holders:trwgQ26xxx
/*******************************************************************************

	Fujitsu MB8840x / MB8850xH series 4-bit MCU disassembler.

	Written by trwgQ26xxx, based on Ernesto Corvi's MB88xx series MCU disassembler.

*******************************************************************************/

#ifndef MAME_CPU_MB88XXX_MB88XXXDASM_H
#define MAME_CPU_MB88XXX_MB88XXXDASM_H

#pragma once

class mb88xxx_disassembler : public util::disasm_interface
{
public:
	mb88xxx_disassembler() = default;
	virtual ~mb88xxx_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_MB88XXX_MB88XXXDASM_H
