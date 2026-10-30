// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m740d.cpp

    Mitsubishi M740 series (M507xx/M509xx), disassembler

***************************************************************************/

#include "emu.h"
#include "m740d.h"
#include "cpu/m6502/m740d.hxx"

m740_disassembler::m740_disassembler(config *_conf) : m6502_base_disassembler(disasm_entries), m_conf(_conf)
{
}

u32 m740_disassembler::get_instruction_bank() const
{
	return m_conf->get_state_base();
}
