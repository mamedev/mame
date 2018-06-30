// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02d.cpp

    6502 with Z register and some more stuff, disassembler

***************************************************************************/

#include "emu.h"
#include "m65ce02d.h"
#include "cpu/m6502/m65ce02d.hxx"

m65ce02_disassembler::m65ce02_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
