// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510d.cpp

    6502 with 6 i/o pins, also known as 8500, disassembler

***************************************************************************/

#include "emu.h"
#include "m6510d.h"
#include "cpu/m6502/m6510d.hxx"

m6510_disassembler::m6510_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
