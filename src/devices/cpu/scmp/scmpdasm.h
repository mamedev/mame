// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   scmpdasm.c
 *
 *   National Semiconductor SC/MP CPU Disassembly
 *
 *****************************************************************************/

#ifndef MAME_CPU_SCMP_SCMPDASM_H
#define MAME_CPU_SCMP_SCMPDASM_H

#pragma once

class scmp_disassembler : public util::disasm_interface
{
public:
	scmp_disassembler() = default;
	virtual ~scmp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif
