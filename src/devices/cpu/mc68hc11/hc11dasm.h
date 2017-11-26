// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
   Motorola M68HC11 disassembler

   Written by Ville Linde
*/

#ifndef MAME_CPU_MC68HC11_HC11DASM_H
#define MAME_CPU_MC68HC11_HC11DASM_H

#pragma once

class hc11_disassembler : public util::disasm_interface
{
public:
	hc11_disassembler() = default;
	virtual ~hc11_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum
	{
		EA_IMM8 = 1,
		EA_IMM16,
		EA_EXT,
		EA_REL,
		EA_DIRECT,
		EA_DIRECT_IMM8,
		EA_DIRECT_IMM8_REL,
		EA_IND_X,
		EA_IND_X_IMM8,
		EA_IND_X_IMM8_REL,
		EA_IND_Y,
		EA_IND_Y_IMM8,
		EA_IND_Y_IMM8_REL,
		PAGE2,
		PAGE3,
		PAGE4
	};

	struct M68HC11_OPCODE {
		char mnemonic[32];
		int address_mode;
	};

	static const M68HC11_OPCODE opcode_table[256];
	static const M68HC11_OPCODE opcode_table_page2[256];
	static const M68HC11_OPCODE opcode_table_page3[256];
	static const M68HC11_OPCODE opcode_table_page4[256];
};

#endif
