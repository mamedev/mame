// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510d.cpp

    65ce02 with a mmu and a port, disassembler

***************************************************************************/

#include "emu.h"
#include "m4510d.h"
#include "cpu/m6502/m4510d.hxx"

m4510_disassembler::m4510_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
