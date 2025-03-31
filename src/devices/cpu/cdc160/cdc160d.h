// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_CDC160_CDC160D_H
#define MAME_CPU_CDC160_CDC160D_H

#pragma once

class cdc160_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	cdc160_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

class cdc160a_disassembler : public cdc160_disassembler
{
public:
	// construction/destruction
	cdc160a_disassembler();

protected:
	// util::disasm_interface overrides
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_CDC160_CDC160D_H
