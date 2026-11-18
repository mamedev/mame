// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
 *
 *   es5510d.h - Ensoniq ES5510 (ESP) disassembler
 *   by Christian Brunschen
 *
 ***************************************************************************/

#ifndef MAME_CPU_ES5510_ES5510D_H
#define MAME_CPU_ES5510_ES5510D_H

#pragma once

class es5510_disassembler : public util::disasm_interface
{
public:
	enum class format : u8
	{
		BYTES,      // six instruction bytes
		PARAMS,     // one instruction word plus a GPR word in the params buffer
		GPR_BYTES   // three GPR bytes followed by six instruction bytes
	};

	struct operand_values
	{
		u32 a = 0;
		u32 b = 0;
		u32 c = 0;
		u32 d = 0;
		u32 dil = 0;
	};

	es5510_disassembler(format fmt = format::BYTES);
	virtual ~es5510_disassembler();

	static void describe(std::ostream &stream, u64 instr, u32 gpr, bool have_gpr, operand_values const *values = nullptr);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	format const m_format;
};

#endif // MAME_CPU_ES5510_ES5510D_H
