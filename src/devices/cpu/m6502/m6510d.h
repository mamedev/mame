// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510d.h

    6502 with 6 i/o pins, also known as 8500, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M6510D_H
#define MAME_CPU_M6502_M6510D_H

#pragma once

#include "m6502d.h"

class m6510_disassembler : public m6502_base_disassembler
{
public:
	m6510_disassembler();
	virtual ~m6510_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
