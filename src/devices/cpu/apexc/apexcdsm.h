// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*
    cpu/apexc/apexcsm.c : APE(X)C CPU disassembler

    By Raphael Nabet

    see cpu/apexc.c for background and tech info
*/

#ifndef MAME_CPU_APEXC_APEXCDSM_H
#define MAME_CPU_APEXC_APEXCDSM_H

#pragma once

class apexc_disassembler : public util::disasm_interface
{
public:
	apexc_disassembler() = default;
	virtual ~apexc_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum format_type {branch, shiftl, shiftr, multiply, store, swap, one_address, two_address};

	struct instr_desc
	{
		const char *mnemonic;
		format_type format; /* -> X and Y are format */
	};

	static const instr_desc instructions[16];
};

#endif
