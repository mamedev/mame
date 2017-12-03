// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02d.cpp

    Rockwell 65c02, CMOS variant with bitwise instructions, disassembler

***************************************************************************/

#include "emu.h"
#include "r65c02d.h"
#include "cpu/m6502/r65c02d.hxx"

r65c02_disassembler::r65c02_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
