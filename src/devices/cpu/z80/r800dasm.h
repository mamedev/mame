// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 *   r800dasm.h
 *   Disassembler for ASCII R800 based on portable Z80 disassembler.
 *
 *****************************************************************************/

#ifndef MAME_CPU_Z80_R800DASM_H
#define MAME_CPU_Z80_R800DASM_H

#pragma once

class r800_disassembler : public util::disasm_interface
{
public:
	r800_disassembler();
	virtual ~r800_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	enum e_mnemonics : unsigned;
	struct r800dasm;

	static const u32 s_flags[];
	static const r800dasm mnemonic_xx_cb[256];
	static const r800dasm mnemonic_cb[256];
	static const r800dasm mnemonic_ed[256];
	static const r800dasm mnemonic_xx[256];
	static const r800dasm mnemonic_main[256];

	static inline char sign(s8 offset);
	static inline u32 offs(s8 offset);
};

#endif
