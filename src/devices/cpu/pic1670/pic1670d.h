// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PIC1670 disassembler

#ifndef MAME_CPU_PIC1670_PIC1670D_H
#define MAME_CPU_PIC1670_PIC1670D_H

#pragma once

class pic1670_disassembler : public util::disasm_interface
{
public:
	pic1670_disassembler() = default;
	virtual ~pic1670_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, const pic1670_disassembler *, u16, u16);
	};

	static const instruction instructions[];

	std::string freg(u16 opcode) const;
	static char fw(u16 opcode);
	static std::string imm8(u16 opcode);
};

#endif // MAME_CPU_PIC1670_PIC1670D_H
