// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02.c

    MOS Technology 6502, CMOS variant with some additional instructions
    (but not the bitwise ones)

***************************************************************************/

#include "emu.h"
#include "m65c02.h"
#include "m65c02d.h"

DEFINE_DEVICE_TYPE(M65C02, m65c02_device, "m65c02", "MOS Technology M65C02")

m65c02_device::m65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M65C02, tag, owner, clock)
{
}

m65c02_device::m65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock)
{
}

std::unique_ptr<util::disasm_interface> m65c02_device::create_disassembler()
{
	return std::make_unique<m65c02_disassembler>();
}

#include "cpu/m6502/m65c02.hxx"
