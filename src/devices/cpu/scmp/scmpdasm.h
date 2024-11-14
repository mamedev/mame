// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_SCMP_SCMPDASM_H
#define MAME_CPU_SCMP_SCMPDASM_H

#pragma once

class scmp_disassembler : public util::disasm_interface
{
public:
	scmp_disassembler();

	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	static void format_disp(std::ostream &stream, offs_t pc, u8 op, u8 disp);
};

#endif // MAME_CPU_SCMP_SCMPDASM_H
