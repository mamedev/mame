// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8hd.cpp

    H8-300H base cpu emulation, disassembler

***************************************************************************/

#include "emu.h"
#include "h8hd.h"
#include "emu/cpu/h8/h8hd.hxx"

h8h_disassembler::h8h_disassembler() : h8_disassembler(disasm_entries, true)
{
}
