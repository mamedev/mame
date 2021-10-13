// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gt913d.cpp

    GT913 base cpu emulation, disassembler

***************************************************************************/

#include "emu.h"
#include "gt913d.h"
#include "cpu/h8/gt913d.hxx"

gt913_disassembler::gt913_disassembler() : h8_disassembler(disasm_entries, false)
{
}
