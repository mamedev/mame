// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300dasm.c
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 Microcontroller
 *
 *  Created on: 18/12/2013
 */

#ifndef MAME_CPU_8X300_8X300DASM_H
#define MAME_CPU_8X300_8X300DASM_H

#pragma once

class n8x300_disassembler : public util::disasm_interface
{
public:
	n8x300_disassembler() = default;
	virtual ~n8x300_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const reg_names[32];

	bool is_rot(uint16_t opcode);
	bool is_src_rot(uint16_t opcode);
};

#endif
