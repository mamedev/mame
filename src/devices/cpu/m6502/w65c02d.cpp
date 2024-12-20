// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    w65c02d.cpp

    WDC W65C02, CMOS variant with some additional instructions
    (but not the bitwise ones), disassembler

***************************************************************************/

#include "emu.h"
#include "w65c02d.h"
#include "cpu/m6502/w65c02d.hxx"

w65c02_disassembler::w65c02_disassembler() : m6502_base_disassembler(disasm_entries)
{
}
