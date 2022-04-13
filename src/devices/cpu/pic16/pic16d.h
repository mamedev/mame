// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// PIC16 generic disassembler, with extended opcodes

#ifndef MAME_CPU_PIC16_PIC16D_H
#define MAME_CPU_PIC16_PIC16D_H

#pragma once

class pic16_disassembler : public util::disasm_interface
{
public:
	pic16_disassembler() = default;
	virtual ~pic16_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, const pic16_disassembler *, u16, u16);
	};

	static const instruction instructions[];

	std::string freg(u16 opcode) const;
	static char fw(u16 opcode);
	static std::string imm8(u16 opcode);
	static std::string imm7(u16 opcode);
	static std::string imm6(u16 opcode);
	static std::string imm6s(u16 opcode);
	static std::string imm5(u16 opcode);
	static std::string rel9(u16 opcode, u16 pc);
	std::string abs11(u16 opcode, u16 pc) const;
};

#endif // MAME_CPU_PIC16_PIC16D_H
