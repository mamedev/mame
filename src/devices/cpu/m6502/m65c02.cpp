// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65c02.c

    Mostek 6502, CMOS variant with some additional instructions (but
    not the bitwise ones)

***************************************************************************/

#include "emu.h"
#include "m65c02.h"

const device_type M65C02 = &device_creator<m65c02_device>;

m65c02_device::m65c02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M65C02, "M65C02", tag, owner, clock, "mc65c02", __FILE__)
{
}

m65c02_device::m65c02_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	m6502_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

offs_t m65c02_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

#include "cpu/m6502/m65c02.inc"
