// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

// MN10300 instruction-length table.

#ifndef MAME_CPU_MN10300_MN10300_INSN_LENGTH_H
#define MAME_CPU_MN10300_MN10300_INSN_LENGTH_H

#pragma once

#include <cstdint>

static inline int mn10300_insn_length(uint8_t op, uint8_t op2)
{
	if (op < 0xF0)
	{
		const int lo = op & 3;
		switch (op & 0xF0)
		{
		case 0x00: return (lo == 0) ? 1 : 3;    // clr / store-to-abs16
		case 0x10: return 1;                     // ext*
		case 0x20:                               // add imm8 / mov imm16
			return (op < 0x24) ? 2 : (op < 0x28) ? 3 : (op < 0x2C) ? 2 : 3;
		case 0x30: return (op < 0x3C) ? 3 : 1;   // (abs16),dD / mov sp,aD
		case 0x40: return (lo < 2) ? 1 : 2;      // inc / mov (disp8,sp)
		case 0x50: return (op < 0x58) ? 1 : 2;   // inc4,asl2 / mov (disp8,sp)
		case 0x60: return 1;                     // mov dD,(aM)
		case 0x70: return 1;                     // mov (aM),dD
		case 0x80: case 0x90: case 0xA0: case 0xB0:
			// imm8 forms are the src==dst encodings (bits[3:2]==bits[1:0]).
			return (((op >> 2) & 3) == (op & 3)) ? 2 : 1;
		case 0xC0:
			if (op <= 0xCA) return 2;            // b<cc>, bra
			if (op == 0xCB) return 1;            // nop
			if (op == 0xCC) return 3;            // jmp disp16
			if (op == 0xCD) return 5;            // call disp16
			return 2;                            // movm (CE/CF)
		case 0xD0:
			if (op <= 0xDB) return 1;            // Lcc, lra, setlb
			if (op == 0xDC) return 5;            // jmp disp32
			if (op == 0xDD) return 7;            // call disp32
			return 3;                            // retf/ret (DE/DF)
		case 0xE0: return 1;                     // add dS,dD
		}
		return 1;
	}

	switch (op)
	{
	case 0xF0: case 0xF1: case 0xF2: case 0xF3:
	case 0xF4: case 0xF5: case 0xF6: return 2;   // (F4 really is 2; unidasm bug says 1)
	case 0xF8: case 0xF9: return 3;
	case 0xFA: case 0xFB: return 4;
	case 0xFC: case 0xFD: return 6;
	case 0xFE:
		if (op2 <= 0x02) return 7;               // bset/bclr/btst imm8,(abs32)
		if (op2 >= 0x80 && op2 <= 0x82) return 5; // ... (abs16)
		return 2;                                // illegal -> core traps
	}
	return 1;                                    // 0xF7, 0xFF: illegal
}

// Convenience overload for callers holding a raw pointer to the instruction.
static inline int mn10300_insn_length(const uint8_t *p)
{
	return mn10300_insn_length(p[0], p[1]);
}

#endif // MAME_CPU_MN10300_MN10300_INSN_LENGTH_H
