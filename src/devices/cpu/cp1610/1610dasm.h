// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo

#ifndef MAME_CPU_CP1610_CP1610DASM_H
#define MAME_CPU_CP1610_CP1610DASM_H

#pragma once

class cp1610_disassembler : public util::disasm_interface
{
public:
	cp1610_disassembler() = default;
	virtual ~cp1610_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
