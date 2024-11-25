// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_OLMS66K_NX8DASM_H
#define MAME_CPU_OLMS66K_NX8DASM_H

#pragma once

class nx8_500s_disassembler : public util::disasm_interface
{
public:
	nx8_500s_disassembler();
	nx8_500s_disassembler(const u16 &psw);

protected:
	// disasm_interface overrides
	virtual offs_t opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// formatting helpers
	static void format_n8(std::ostream &stream, u8 n8);
	static void format_n16(std::ostream &stream, u16 n16);
	static void format_fix8(std::ostream &stream, u8 n8);
	static void format_off8(std::ostream &stream, u8 n8);
	static void format_dir16(std::ostream &stream, u16 n16);
	static void format_sfr8(std::ostream &stream, u8 n8, bool word);

	offs_t dasm_composite(std::ostream &stream, offs_t pc, offs_t prefix, const data_buffer &opcodes, std::string obj, bool word) const;

	const u16 &m_psw;
};

#endif // MAME_CPU_OLMS66K_NX8DASM_H
