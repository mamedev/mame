// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    deco16d.cpp

    6502, reverse-engineered DECO variant, disassembler

***************************************************************************/

#include "emu.h"
#include "deco16d.h"
#include "cpu/m6502/deco16d.hxx"

deco16_disassembler::deco16_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
