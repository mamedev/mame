// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02d.h

    6502 with Z register and some more stuff, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M65CE02D_H
#define MAME_CPU_M6502_M65CE02D_H

#pragma once

#include "m6502d.h"

class m65ce02_disassembler : public m6502_base_disassembler
{
public:
	m65ce02_disassembler();
	virtual ~m65ce02_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
