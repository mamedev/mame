// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    r65c02.c

    Rockwell 65c02, CMOS variant with bitwise instructions

***************************************************************************/

#include "emu.h"
#include "r65c02.h"

const device_type R65C02 = device_creator<r65c02_device>;

r65c02_device::r65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65c02_device(mconfig, R65C02, "R65C02", tag, owner, clock, "r65c02", __FILE__)
{
}

r65c02_device::r65c02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	m65c02_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

offs_t r65c02_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	return disassemble_generic(stream, pc, oprom, opram, options, disasm_entries);
}

#include "cpu/m6502/r65c02.hxx"
