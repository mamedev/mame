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

std::unique_ptr<util::disasm_interface> w65c02_device::create_disassembler()
{
	return std::make_unique<w65c02_disassembler>();
}

#include "cpu/m6502/w65c02.hxx"
