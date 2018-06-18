// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
  Diablo Printer TTL CPU disassembler
*/

#ifndef MAME_CPU_DIABLO_DIABLO1300DASM_H
#define MAME_CPU_DIABLO_DIABLO1300DASM_H

#pragma once

class diablo1300_disassembler : public util::disasm_interface
{
public:
	diablo1300_disassembler() = default;
	virtual ~diablo1300_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
