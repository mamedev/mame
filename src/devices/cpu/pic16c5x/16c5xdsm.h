// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                      Microchip PIC16C5x Emulator                         *
	*                                                                          *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*                        as this was based on it.                          *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	* A Address to jump to.                                                    *
	* B Bit address within an 8-bit file register.                             *
	* D Destination select (0 = store result in W (accumulator))               *
	*                      (1 = store result in file register)                 *
	* F Register file address (00-1F).                                         *
	* K Literal field, constant data.                                          *
	*                                                                          *
	\**************************************************************************/

#ifndef MAME_CPU_PIC16C5X_16C5XDSM_H
#define MAME_CPU_PIC16C5X_16C5XDSM_H

#pragma once

class pic16c5x_disassembler : public util::disasm_interface
{
public:
	pic16c5x_disassembler();
	virtual ~pic16c5x_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct PIC16C5xOpcode  {
		u16 mask;          /* instruction mask */
		u16 bits;          /* constant bits */
		u16 extcode;       /* value that gets extension code */
		const char *parse;  /* how to parse bits */
		const char *fmt;    /* instruction format */

		PIC16C5xOpcode(u16 m, u16 b, u16 e, const char *p, const char *f) : mask(m), bits(b), extcode(e), parse(p), fmt(f) {}
	};

	static const char *const regfile[32];
	static const char *const dest[2];
	static const char *const PIC16C5xFormats[];

	std::vector<PIC16C5xOpcode> Op;
};

#endif
