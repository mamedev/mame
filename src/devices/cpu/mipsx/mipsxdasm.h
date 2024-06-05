// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_MIPSX_MIPSXDASM_H
#define MAME_CPU_MIPSX_MIPSXDASM_H

#pragma once

class mipsx_disassembler : public util::disasm_interface
{
public:
	mipsx_disassembler() = default;
	virtual ~mipsx_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_MIPSX_MIPSXDASM_H
