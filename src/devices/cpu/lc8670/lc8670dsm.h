// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sanyo LC8670 disassembler

******************************************************************************/

#ifndef MAME_CPU_LC8670_LC8670DSM_H
#define MAME_CPU_LC8670_LC8670DSM_H

#pragma once

class lc8670_disassembler : public util::disasm_interface
{
public:
	lc8670_disassembler() = default;
	virtual ~lc8670_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// disassembler
	enum
	{
		OP_NULL,
		OP_R8,
		OP_R8RI,
		OP_R16,
		OP_RI,
		OP_A12,
		OP_A16,
		OP_I8,
		OP_B3,
		OP_D9,
		OP_D9B3,
		OP_RII8
	};

	// disasm table
	struct dasm_entry
	{
		const char *str;
		uint8_t       arg1;
		uint8_t       arg2;
		bool        inv;
	};
	static const dasm_entry s_dasm_table[80];

	void dasm_arg(uint8_t op, char *buffer, offs_t pc, int arg, const data_buffer &opcodes, offs_t &pos);
};

#endif
