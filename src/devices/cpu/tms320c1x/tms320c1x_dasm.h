// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/**************************************************************************\
* Texas Instruments TMS320C1x DSP Disassembler
*
* Copyright Tony La Porta
\**************************************************************************/

#ifndef MAME_CPU_TMS320C1X_TMS320C1X_DASM_H
#define MAME_CPU_TMS320C1X_TMS320C1X_DASM_H

#pragma once

class tms320c1x_disassembler : public util::disasm_interface
{
public:
	tms320c1x_disassembler();
	virtual ~tms320c1x_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct TMS320C1xOpcode  {
		u16 mask;          /* instruction mask */
		u16 bits;          /* constant bits */
		u16 extcode;       /* value that gets extension code */
		const char *parse; /* how to parse bits */
		const char *fmt;   /* instruction format */

		TMS320C1xOpcode(u16 m, u16 b, u16 e, const char *p, const char *f) : mask(m), bits(b), extcode(e), parse(p), fmt(f) {}
	};

	static const char *const arith[4];
	static const char *const nextar[4];
	static const char *const TMS320C1xFormats[];

	std::vector<TMS320C1xOpcode> Op;
};

#endif // MAME_CPU_TMS320C1X_TMS320C1X_DASM_H
