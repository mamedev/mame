// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    w65816d.cpp

    WDC W65C816S, disassembler

***************************************************************************/

#include "emu.h"
#include "w65816d.h"
#include "cpu/m6502/w65816d.hxx"

w65816_disassembler::w65816_disassembler(config *_conf) : m6502_base_disassembler(disasm_entries), m_conf(_conf)
{
}

u32 w65816_disassembler::get_instruction_bank() const
{
	return m_conf->get_state_base();
}
