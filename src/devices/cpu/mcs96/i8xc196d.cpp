// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#include "emu.h"
#include "i8xc196d.h"

i8xc196_disassembler::i8xc196_disassembler() : mcs96_disassembler(disasm_entries)
{
}

#include "cpu/mcs96/i8xc196d.hxx"
