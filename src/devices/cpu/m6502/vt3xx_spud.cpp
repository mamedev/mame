// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    vt3xx_spud.cpp

***************************************************************************/

#include "emu.h"
#include "vt3xx_spud.h"
#include "cpu/m6502/vt3xx_spud.hxx"

vt3xx_spu_disassembler::vt3xx_spu_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
