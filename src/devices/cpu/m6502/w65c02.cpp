// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    w65c02.cpp

    WDC W65C02, CMOS variant with some additional instructions
    (but not the bitwise ones)

***************************************************************************/

#include "emu.h"
#include "w65c02.h"
#include "w65c02d.h"

DEFINE_DEVICE_TYPE(W65C02, w65c02_device, "w65c02", "WDC W65C02")

w65c02_device::w65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, W65C02, tag, owner, clock)
{
}

w65c02_device::w65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock)
{
}

void w65c02_device::do_sbc_cd(uint8_t val)
{
	// SBC allows interdigit carry from decimal adjustment on 65C02
	uint8_t c = m_P & F_C ? 0 : 1;
	m_P &= ~(F_N|F_V|F_Z|F_C);
	uint16_t diff = m_A - val - c;
	uint8_t al = (m_A & 15) - (val & 15) - c;
	uint8_t ah = (m_A >> 4) - (val >> 4) - (int8_t(al) < 0);
	// N and Z flags will be set in the next cycle
	if((m_A^val) & (m_A^diff) & 0x80)
		m_P |= F_V;
	if(!(diff & 0xff00))
		m_P |= F_C;
	m_A = (ah << 4) | (al & 15);
	if(int8_t(al) < 0)
		m_A -= 6;
	if(int8_t(ah) < 0)
		m_A -= 0x60;
}

void w65c02_device::do_sbc_c(uint8_t val)
{
	if(m_P & F_D)
		do_sbc_cd(val);
	else
		do_sbc_nd(val);
}

std::unique_ptr<util::disasm_interface> w65c02_device::create_disassembler()
{
	return std::make_unique<w65c02_disassembler>();
}

#include "cpu/m6502/w65c02.hxx"
