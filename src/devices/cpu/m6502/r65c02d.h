// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02d.h

    Rockwell 65c02, CMOS variant with bitwise instructions, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_R65C02D_H
#define MAME_CPU_M6502_R65C02D_H

#pragma once

#include "m6502d.h"

class r65c02_disassembler : public m6502_base_disassembler
{
public:
	r65c02_disassembler();
	virtual ~r65c02_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
