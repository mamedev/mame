// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_CPU_Z8_Z8DASM_H
#define MAME_CPU_Z8_Z8DASM_H

#pragma once

class z8_disassembler : public util::disasm_interface
{
public:
	z8_disassembler() = default;
	virtual ~z8_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const REGISTER_NAME[256];
	static const char *const CONDITION_CODE[16];

};

#endif
