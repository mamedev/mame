// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    rp2a03d.cpp

    6502, NES variant, disassembler

***************************************************************************/

#include "emu.h"
#include "rp2a03d.h"
#include "cpu/m6502/rp2a03d.hxx"

rp2a03_disassembler::rp2a03_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
