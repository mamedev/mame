// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_NOVA_NOVADASM_H
#define MAME_CPU_NOVA_NOVADASM_H

#pragma once

class nova_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	nova_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// formatting helpers
	static void format_effective_address(std::ostream &stream, offs_t pc, u16 inst);
	static void format_device_code(std::ostream &stream, u8 device);
};

#endif // MAME_CPU_NOVA_NOVADASM_H
