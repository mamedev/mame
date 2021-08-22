// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M68HC16_CPU16DASM_H
#define MAME_CPU_M68HC16_CPU16DASM_H

#pragma once

#include <string_view>

class cpu16_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	cpu16_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum class mode
	{
		UND,
		INH,
		IMM,
		IMMS,
		REGM,
		XYO,
		IND,
		IND20,
		EXT,
		EXT20,
		E,
		REL,
		BIT,
		BIT16,
		IXP
	};

	struct opcode_info
	{
		std::string_view m_name;
		mode m_mode;
		offs_t m_flags;
	};

	// internal helpers
	void format_signed(std::ostream &stream, u16 value);
	void format_index8(std::ostream &stream, u8 offset, char reg);
	void format_index16(std::ostream &stream, u16 offset, char reg);

	// internal tables
	static const opcode_info s_opinfo[4][256];
	static const std::string_view s_regset[7];
};

#endif // MAME_CPU_M68HC16_CPU16DASM_H
