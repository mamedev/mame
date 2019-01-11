// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8x9x.h

    MCS96, 8x9x branch, the original version

***************************************************************************/

#include "emu.h"
#include "i8x9xd.h"

i8x9x_disassembler::i8x9x_disassembler() : mcs96_disassembler(disasm_entries)
{
}

#include "cpu/mcs96/i8x9xd.hxx"

