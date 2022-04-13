// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8d.h

    H8S-2000 base cpu emulation, disassembler

***************************************************************************/

#include "emu.h"
#include "h8s2000d.h"
#include "cpu/h8/h8s2000d.hxx"

h8s2000_disassembler::h8s2000_disassembler() : h8_disassembler(disasm_entries, true)
{
}
