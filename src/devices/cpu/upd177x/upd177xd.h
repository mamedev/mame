// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// upd177X disassembler

#ifndef MAME_CPU_UPD177X_UPD177XD_H
#define MAME_CPU_UPD177X_UPD177XD_H

#pragma once

class upd177x_disassembler : public util::disasm_interface
{
public:
	upd177x_disassembler() = default;
	virtual ~upd177x_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		u32 (*cb)(std::ostream &, u16, u16);
	};

	static const instruction instructions[];

	static std::string reg4(u16 opcode);
	static std::string reg8(u16 opcode);
	static std::string imm8(u16 opcode);
	static std::string imm7(u16 opcode);
	static std::string imm6(u16 opcode);
	static std::string imm4(u16 opcode);
	static std::string imm3_5(u16 opcode);
	static std::string abs12(u16 opcode, u16 pc);
	static std::string abs8(u16 opcode, u16 pc);
	static std::string abs4_4(u16 opcode, u16 pc);
};

#endif // MAME_CPU_upd177X_upd177XD_H
