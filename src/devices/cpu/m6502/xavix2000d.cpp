// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavix2000d.cpp (Super XaviX)

***************************************************************************/

#include "emu.h"
#include "xavix2000d.h"
#include "cpu/m6502/xavix2000d.hxx"

xavix2000_disassembler::xavix2000_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
