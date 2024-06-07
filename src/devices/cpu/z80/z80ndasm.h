// license:BSD-3-Clause
/*****************************************************************************
 *
 *   Z80N disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_Z80_Z80NDASM_H
#define MAME_CPU_Z80_Z80NDASM_H

#pragma once

#include "z80dasm.h"

class z80n_disassembler : public z80_disassembler
{
protected:
	static const z80dasm mnemonic_ed_n[256];

	virtual const z80dasm &get_mnemonic_ed(u8 opcode) override { return mnemonic_ed_n[opcode]; }
};

#endif
