// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02.c

    Rockwell 65c02, CMOS variant with bitwise instructions

***************************************************************************/

#include "emu.h"
#include "r65c02.h"
#include "r65c02d.h"

DEFINE_DEVICE_TYPE(R65C02, r65c02_device, "r65c02", "Rockwell R65C02")

r65c02_device::r65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	r65c02_device(mconfig, R65C02, tag, owner, clock)
{
}

r65c02_device::r65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m65c02_device(mconfig, type, tag, owner, clock)
{
}

std::unique_ptr<util::disasm_interface> r65c02_device::create_disassembler()
{
	return std::make_unique<r65c02_disassembler>();
}

#include "cpu/m6502/r65c02.hxx"
