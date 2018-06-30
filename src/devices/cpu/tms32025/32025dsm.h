// license:BSD-3-Clause
// copyright-holders:Tony La Porta, hap
	/**************************************************************************\
	*              Texas Instruments TMS320x25 DSP Disassembler                *
	*                                                                          *
	*                 Copyright Tony La Porta                                  *
	*              To be used with TMS320x25 DSP Emulator engine.              *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*               as the structure here was borrowed from it.                *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	*                                                                          *
	* A Memory Address                                                         *
	* B Opcode Address Argument (Requires next opcode read)                    *
	* C Compare mode                                                           *
	* D Immediate byte load                                                    *
	* K Immediate bit load                                                     *
	* W Immediate word load                                                    *
	* M AR[x] register modification type (for indirect addressing)             *
	* N ARP register to change ARP pointer to (for indirect addressing)        *
	* P I/O port address number                                                *
	* R AR[R] register to use                                                  *
	* S Shift ALU left                                                         *
	* T Shift ALU left (Hex) / Nibble data                                     *
	* X Don't care bit                                                         *
	*                                                                          *
	\**************************************************************************/

#ifndef MAME_CPU_TMS32025_32025DSM_H
#define MAME_CPU_TMS32025_32025DSM_H

#pragma once

class tms32025_disassembler : public util::disasm_interface
{
public:
	tms32025_disassembler();
	virtual ~tms32025_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct TMS32025Opcode  {
		u16 mask;          /* instruction mask */
		u16 bits;          /* constant bits */
		u16 extcode;       /* value that gets extension code */
		const char *parse; /* how to parse bits */
		const char *fmt;   /* instruction format */

		TMS32025Opcode(u16 m, u16 b, u16 e, const char *p, const char *f) : mask(m), bits(b), extcode(e), parse(p), fmt(f) {}
	};

	static const char *const arith[8];
	static const char *const nextar[16];
	static const char *const cmpmode[4];
	static const char *const TMS32025Formats[];

	std::vector<TMS32025Opcode> Op;
};

#endif
