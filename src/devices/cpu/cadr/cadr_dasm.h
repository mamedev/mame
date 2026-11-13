// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
// ********************************************************************************
// * MIT CADR processor microcode disassembler
// ********************************************************************************

#ifndef MAME_CPU_CADR_CADR_DASM_H
#define MAME_CPU_CADR_CADR_DASM_H

#pragma once

class cadr_disassembler : public util::disasm_interface
{
public:
	cadr_disassembler() = default;
	virtual ~cadr_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_CADR_CADR_DASM_H
