// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02d.h

    MOS Technology 6502, CMOS variant with some additional instructions
    (but not the bitwise ones), disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M65C02D_H
#define MAME_CPU_M6502_M65C02D_H

#pragma once

#include "m6502d.h"

class m65c02_disassembler : public m6502_base_disassembler
{
public:
	m65c02_disassembler();
	virtual ~m65c02_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
