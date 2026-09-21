// license:BSD-3-Clause
// copyright-holders:Andrea Bogazzi

// Jaleco "FPU" math coprocessor disassembler

#ifndef MAME_CPU_JALFPU_JALFPU_DASM_H
#define MAME_CPU_JALFPU_JALFPU_DASM_H

#pragma once

class jaleco_fpu_disassembler : public util::disasm_interface
{
public:
	jaleco_fpu_disassembler() = default;
	virtual ~jaleco_fpu_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_JALFPU_JALFPU_DASM_H
