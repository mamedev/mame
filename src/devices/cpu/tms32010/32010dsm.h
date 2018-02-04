// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                Texas Instruments TMS32010 DSP Disassembler               *
	*                                                                          *
	*                  Copyright Tony La Porta                                 *
	*               To be used with TMS32010 DSP Emulator engine.              *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*                        as this was based on it.                          *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	* A Memory address                                                         *
	* B Branch Address for Branch instructions (Requires next opcode read)     *
	* D Immediate byte load                                                    *
	* K Immediate bit  load                                                    *
	* W Immediate word load (Actually 13 bit)                                  *
	* M AR[x] register modification type (for indirect addressing)             *
	* N ARP register to change ARP pointer to (for indirect addressing)        *
	* P I/O port address number                                                *
	* R AR[R] register to use                                                  *
	* S Shift ALU left                                                         *
	*                                                                          *
	\**************************************************************************/

#ifndef MAME_CPU_TMS32010_DIS32010_H
#define MAME_CPU_TMS32010_DIS32010_H

#pragma once

class tms32010_disassembler : public util::disasm_interface
{
public:
	tms32010_disassembler();
	virtual ~tms32010_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct TMS32010Opcode  {
		u16 mask;          /* instruction mask */
		u16 bits;          /* constant bits */
		u16 extcode;       /* value that gets extension code */
		const char *parse; /* how to parse bits */
		const char *fmt;   /* instruction format */

		TMS32010Opcode(u16 m, u16 b, u16 e, const char *p, const char *f) : mask(m), bits(b), extcode(e), parse(p), fmt(f) {}
	};

	static const char *const arith[4];
	static const char *const nextar[4];
	static const char *const TMS32010Formats[];

	std::vector<TMS32010Opcode> Op;
};

#endif
