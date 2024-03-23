// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_INTERDATA16_DASM16_H
#define MAME_CPU_INTERDATA16_DASM16_H

#pragma once

class interdata16_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	interdata16_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// formatting helpers
	static void format_s16(std::ostream &stream, s16 halfword);
};

#endif // MAME_CPU_INTERDATA16_DASM16_H
