// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_V620_V620_H
#define MAME_CPU_V620_V620_H

#pragma once

class v620_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	v620_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	virtual offs_t dasm_004xxx(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_misc(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const;
	virtual offs_t dasm_io(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const;

	// internal helpers
	void format_number(std::ostream &stream, u16 n) const;
	void format_address(std::ostream &stream, u16 addr) const;
};

class v75_disassembler : public v620_disassembler
{
public:
	// construction/destruction
	v75_disassembler();

protected:
	virtual offs_t dasm_004xxx(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_misc(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const override;
	virtual offs_t dasm_io(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const override;
};

#endif // MAME_CPU_V620_V620_H
