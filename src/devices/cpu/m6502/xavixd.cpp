// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    xavixd.cpp

***************************************************************************/

#include "emu.h"
#include "xavixd.h"
#include "cpu/m6502/xavixd.hxx"

xavix_disassembler::xavix_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
