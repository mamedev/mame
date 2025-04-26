// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   scdasm.c
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner
 *
 *****************************************************************************/

#ifndef MAME_CPU_SC61860_SCDASM_H
#define MAME_CPU_SC61860_SCDASM_H

#pragma once

class sc61860_disassembler : public util::disasm_interface
{
public:
	sc61860_disassembler() = default;
	virtual ~sc61860_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum Adr
	{
		Ill,
		Imp,
		Imm, ImmW,
		RelP, RelM,
		Abs,
		Ptc,
		Etc,
		Cal,
		Lp
	};

	struct opcode {
		const char *mnemonic;
		Adr adr;
	};

	static const opcode table[];

};

#endif
