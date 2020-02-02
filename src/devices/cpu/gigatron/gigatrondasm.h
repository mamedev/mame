// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_GIGATRON_GIGATRONDASM_H
#define MAME_CPU_GIGATRON_GIGATRONDASM_H

#pragma once

class gigatron_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	gigatron_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// tables
	static const char *const s_ops[7];
	static const char *const s_jumps[8];
};

#endif // MAME_CPU_GIGATRON_GIGATRONDASM_H
