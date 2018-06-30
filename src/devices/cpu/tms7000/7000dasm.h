// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/*

  TMS7000 disassembler

*/

#ifndef MAME_CPU_TMS7000_7000DASM_H
#define MAME_CPU_TMS7000_7000DASM_H

#pragma once

class tms7000_disassembler : public util::disasm_interface
{
public:
	tms7000_disassembler() = default;
	virtual ~tms7000_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum operandtype { DONE, NONE, UI8, I8, UI16, I16, PCREL, PCABS, TRAP };

	struct oprandinfo {
		char opstr[4][12];
		operandtype decode[4];
	};

	struct tms7000_opcodeinfo {
		int opcode;
		char name[8];
		int operand;
		uint32_t s_flag;
	};

	static const oprandinfo of[];
	static const tms7000_opcodeinfo opcs[];
};

#endif
