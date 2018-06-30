// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510d.h

    65ce02 with a mmu and a port, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M4510D_H
#define MAME_CPU_M6502_M4510D_H

#pragma once

#include "m6502d.h"

class m4510_disassembler : public m6502_base_disassembler
{
public:
	m4510_disassembler();
	virtual ~m4510_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
