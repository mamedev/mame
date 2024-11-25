// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8s2600d.h

    H8S-2600 base cpu emulation, disassembler

***************************************************************************/

#include "emu.h"
#include "h8s2600d.h"
#include "cpu/h8/h8s2600d.hxx"

h8s2600_disassembler::h8s2600_disassembler() : h8_disassembler(disasm_entries, true)
{
}
