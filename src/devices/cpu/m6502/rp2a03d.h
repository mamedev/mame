// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    rp2a03d.h

    6502, NES variant, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_RP2A03D_H
#define MAME_CPU_M6502_RP2A03D_H

#pragma once

#include "m6502d.h"

class rp2a03_disassembler : public m6502_base_disassembler
{
public:
	rp2a03_disassembler();
	virtual ~rp2a03_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
