// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02d.cpp

    MOS Technology 6502, CMOS variant with some additional instructions
    (but not the bitwise ones), disassembler

***************************************************************************/

#include "emu.h"
#include "m65c02d.h"
#include "cpu/m6502/m65c02d.hxx"

m65c02_disassembler::m65c02_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
