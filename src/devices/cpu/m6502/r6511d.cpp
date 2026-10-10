// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    r6511d.cpp

    Rockwell R6511Q one-chip microprocessor, disassembler

***************************************************************************/

#include "emu.h"
#include "r6511d.h"
#include "cpu/m6502/r6511d.hxx"

r6511_disassembler::r6511_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
