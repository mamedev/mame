// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02.c

    Mostek 6502, CMOS variant with some additional instructions (but
    not the bitwise ones)

***************************************************************************/

#include "emu.h"
#include "m65c02.h"

DEFINE_DEVICE_TYPE(M65C02, m65c02_device, "m65c02", "M65C02")

m65c02_device::m65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M65C02, tag, owner, clock)
{
}

m65c02_device::m65c02_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock)
{
}

offs_t m65c02_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	return disassemble_generic(stream, pc, oprom, opram, options, disasm_entries);
}

#include "cpu/m6502/m65c02.hxx"
