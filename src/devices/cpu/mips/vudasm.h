// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************
*
*   Sony Playstation 2 Vector Unit disassembler
*
*/

#ifndef MAME_CPU_MIPS_VUDASM_H
#define MAME_CPU_MIPS_VUDASM_H

#pragma once

class sonyvu_disassembler : public util::disasm_interface
{
public:
	sonyvu_disassembler() = default;
	virtual ~sonyvu_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	void dasm_upper(uint32_t pc, uint32_t op, std::ostream &stream);
	void dasm_lower(uint32_t pc, uint32_t op, std::ostream &stream);
	std::string signed_5bit(uint16_t val);
	std::string signed_5bit_rd(uint16_t val);
	std::string unsigned_11bit(uint16_t val);
	std::string signed_11bit(uint16_t val);
	std::string signed_11bit_x8(uint16_t val);
	std::string signed_15bit(uint16_t val);

	static char const *const DEST_STR[16];
	static char const *const DEST_COMMA_STR[16];
	static char const *const BC_STR[16];
	static char const *const BC_COMMA_STR[16];
	static char const *const VFREG[32];
	static char const *const VIREG[32];
};

#endif // MAME_CPU_MIPS_VUDASM_H
