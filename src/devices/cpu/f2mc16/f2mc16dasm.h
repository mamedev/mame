// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_CPU_F2MC16_F2MC16DASM_H
#define MAME_CPU_F2MC16_F2MC16DASM_H

#pragma once

class f2mc16_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	f2mc16_disassembler() = default;
	virtual ~f2mc16_disassembler() = default;

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	void branch_helper(std::ostream &stream, const char *insName, u16 PC, s8 offset);
	offs_t ea_form1_helper(std::ostream &stream, const char *opName, u16 pc, u8 operand, u16 imm16, bool bAIsDest);
	offs_t ea_form1_helper_noA(std::ostream &stream, const char *opName, u16 pc, u8 operand, u16 imm16);
};

#endif // MAME_CPU_F2MC16_F2MC16DASM_H
