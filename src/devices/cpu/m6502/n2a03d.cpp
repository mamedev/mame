// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    n2a03d.cpp

    6502, NES variant, disassembler

***************************************************************************/

#include "emu.h"
#include "n2a03d.h"
#include "cpu/m6502/n2a03d.hxx"

n2a03_disassembler::n2a03_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
