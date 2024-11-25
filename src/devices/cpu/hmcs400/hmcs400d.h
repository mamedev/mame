// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hitachi HMCS400 MCU family disassembler

*/

#ifndef MAME_CPU_HMCS400_HMCS400D_H
#define MAME_CPU_HMCS400_HMCS400D_H

#pragma once

class hmcs400_disassembler : public util::disasm_interface
{
public:
	hmcs400_disassembler();
	virtual ~hmcs400_disassembler();

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics : unsigned;
	static const char *const s_mnemonics[];
	static const s8 s_bits[];
	static const u32 s_flags[];
	static const u8 hmcs400_mnemonic[0x400];
};

#endif // MAME_CPU_HMCS400_HMCS400D_H
