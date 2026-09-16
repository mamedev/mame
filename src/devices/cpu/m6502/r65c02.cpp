// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02.cpp

    Rockwell 65c02, CMOS variant with bitwise instructions
    Also licensed to Ricoh, who sold it as RP65C02.

    TODO:
    - are any of the CPU subtype differences meaningful to MAME?

***************************************************************************/

#include "emu.h"
#include "r65c02.h"
#include "r65c02d.h"

DEFINE_DEVICE_TYPE(R65C02, r65c02_device, "r65c02", "Rockwell R65C02")
DEFINE_DEVICE_TYPE(R65C102, r65c102_device, "r65c102", "Rockwell R65C102")
DEFINE_DEVICE_TYPE(R65C112, r65c112_device, "r65c112", "Rockwell R65C112")

r65c02_device::r65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	w65c02_device(mconfig, type, tag, owner, clock)
{
}

r65c02_device::r65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	r65c02_device(mconfig, R65C02, tag, owner, clock)
{
}

r65c102_device::r65c102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	r65c02_device(mconfig, R65C102, tag, owner, clock)
{
}

r65c112_device::r65c112_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	r65c02_device(mconfig, R65C112, tag, owner, clock)
{
}

std::unique_ptr<util::disasm_interface> r65c02_device::create_disassembler()
{
	return std::make_unique<r65c02_disassembler>();
}

#include "cpu/m6502/r65c02.hxx"
