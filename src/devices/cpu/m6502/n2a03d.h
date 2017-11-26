// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    n2a03d.h

    6502, NES variant, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_N2A03D_H
#define MAME_CPU_M6502_N2A03D_H

#pragma once

#include "m6502d.h"

class n2a03_disassembler : public m6502_base_disassembler
{
public:
	n2a03_disassembler();
	virtual ~n2a03_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
