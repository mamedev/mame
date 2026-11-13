// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    r6511d.h

    Rockwell R6511Q one-chip microprocessor, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_R6511D_H
#define MAME_CPU_M6502_R6511D_H

#pragma once

#include "m6502d.h"

class r6511_disassembler : public m6502_base_disassembler
{
public:
	r6511_disassembler();
	virtual ~r6511_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
