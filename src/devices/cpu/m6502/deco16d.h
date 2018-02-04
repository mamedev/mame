// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16d.h

    6502, reverse-engineered DECO variant, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_DECO16D_H
#define MAME_CPU_M6502_DECO16D_H

#pragma once

#include "m6502d.h"

class deco16_disassembler : public m6502_base_disassembler
{
public:
	deco16_disassembler();
	virtual ~deco16_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
