// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6509d.cpp

    6502 with banking and extended address bus, disassembler

***************************************************************************/

#include "emu.h"
#include "m6509d.h"
#include "cpu/m6502/m6509d.hxx"

m6509_disassembler::m6509_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
