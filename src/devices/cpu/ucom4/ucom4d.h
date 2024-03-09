// license:BSD-3-Clause
// copyright-holders:hap
/*

  NEC uCOM-4 MCU family disassembler

*/

#ifndef MAME_CPU_UCOM4_UCOM4D_H
#define MAME_CPU_UCOM4_UCOM4D_H

#pragma once

class ucom4_disassembler : public util::disasm_interface
{
public:
	ucom4_disassembler() = default;
	virtual ~ucom4_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual u32 interface_flags() const override { return PAGED; }
	virtual u32 page_address_bits() const override { return 8; }

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics : unsigned;
	static const char *const s_mnemonics[];
	static const u8 s_bits[];
	static const u32 s_flags[];
	static const u8 ucom4_mnemonic[0x100];
};

#endif // MAME_CPU_UCOM4_UCOM4D_H
