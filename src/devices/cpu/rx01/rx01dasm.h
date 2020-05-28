// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_RX01_RX01DASM_H
#define MAME_CPU_RX01_RX01DASM_H

#pragma once

class rx01_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	rx01_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// tables
	static const char *const s_0_or_1[2];
	static const char *const s_conditions[16];
	static const char *const s_flag_control[4];
};

#endif // MAME_CPU_RX01_RX01DASM_H
