// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    w65c02s.cpp

    WDC W65C02S, CMOS variant with bitwise instructions, BE, ML, VP pins
    and cleaner fetch patterns

***************************************************************************/

#include "emu.h"
#include "w65c02s.h"
#include "r65c02d.h"

DEFINE_DEVICE_TYPE(W65C02S, w65c02s_device, "w65c02s", "WDC W65C02S")

w65c02s_device::w65c02s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	w65c02s_device(mconfig, W65C02S, tag, owner, clock)
{
}

w65c02s_device::w65c02s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m65c02_device(mconfig, type, tag, owner, clock)
{
}

std::unique_ptr<util::disasm_interface> w65c02s_device::create_disassembler()
{
	return std::make_unique<r65c02_disassembler>();
}

#include "cpu/m6502/w65c02s.hxx"
