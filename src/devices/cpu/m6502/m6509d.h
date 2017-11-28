// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6509d.h

    6502 with banking and extended address bus, disassembler

***************************************************************************/

#ifndef MAME_CPU_M6502_M6509D_H
#define MAME_CPU_M6502_M6509D_H

#pragma once

#include "m6502d.h"

class m6509_disassembler : public m6502_base_disassembler
{
public:
	m6509_disassembler();
	virtual ~m6509_disassembler() = default;

private:
	static const disasm_entry disasm_entries[0x100];
};

#endif
