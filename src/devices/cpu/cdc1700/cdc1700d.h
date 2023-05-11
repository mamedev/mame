// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_CDC1700_CDC1700D_H
#define MAME_CPU_CDC1700_CDC1700D_H

#pragma once

class cdc1700_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	cdc1700_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	virtual offs_t dasm_register_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst);

	// formatting helpers
	void format_s8(std::ostream &stream, u8 value);
};

class cyber18_disassembler : public cdc1700_disassembler
{
public:
	// construction/destruction
	cyber18_disassembler();

protected:
	// cdc1700_disassembler overrides
	virtual offs_t dasm_register_reference(std::ostream &stream, offs_t pc, const data_buffer &opcodes, u16 inst) override;
};

#endif // MAME_CPU_CDC1700_CDC1700D_H
