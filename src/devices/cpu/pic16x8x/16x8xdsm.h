// license:BSD-3-Clause
// copyright-holders: Grull Osgo
/*

  Microchip PIC16X8x Emulator

  Based on MAME's PIC16C5x cpu_device dissasm developed by Tony La Porta.
 

*/

#ifndef MAME_CPU_PIC16X8X_16X8XDSM_H
#define MAME_CPU_PIC16X8X_16X8XDSM_H

#pragma once

class pic16x8x_disassembler : public util::disasm_interface
{
public:
	pic16x8x_disassembler();
	virtual ~pic16x8x_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct PIC16X8xOpcode  {
		u16 mask;          // instruction mask
		u16 bits;          // constant bits
		u16 extcode;       // value that gets extension code
		const char *parse; // how to parse bits
		const char *fmt;   // instruction format

		PIC16X8xOpcode(u16 m, u16 b, u16 e, const char *p, const char *f) : mask(m), bits(b), extcode(e), parse(p), fmt(f) {}
	};

	static const u8 sfr_bank0[16];
	static const char *const sfregs[12];
	static const char *const dest[2];
	static const char *const reg_flags[9][8];
	static const char *const PIC16X8xFormats[];

	std::vector<PIC16X8xOpcode> Op;
};

#endif
