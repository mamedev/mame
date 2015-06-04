// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510.c

    65ce02 with a mmu and a cia integrated

***************************************************************************/

#include "emu.h"
#include "m4510.h"

const device_type M4510 = &device_creator<m4510_device>;

m4510_device::m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m65ce02_device(mconfig, M4510, "M4510", tag, owner, clock, "m4510", __FILE__)
{
	program_config.m_addrbus_width = 20;
	program_config.m_logaddr_width = 16;
	program_config.m_page_shift = 13;
	sprogram_config.m_addrbus_width = 20;
	sprogram_config.m_logaddr_width = 16;
	sprogram_config.m_page_shift = 13;
}

offs_t m4510_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void m4510_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_4510_nd(this);
	else
		mintf = new mi_4510_normal(this);

	m65ce02_device::init();

	save_item(NAME(map_offset));
	save_item(NAME(map_enable));
}

void m4510_device::device_reset()
{
	map_offset[0] = map_offset[1] = 0;
	map_enable = 0;
	nomap = true;

	// Wild guess, this setting makes the cpu start executing some code in the c65 driver
	//map_offset[1] = 0x2e000;
	//map_enable = 0x80;
	m65ce02_device::device_reset();
}

bool m4510_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM)
	{
		address = map(address);
	}

	return true;
}

m4510_device::mi_4510_normal::mi_4510_normal(m4510_device *_base)
{
	base = _base;
}

UINT8 m4510_device::mi_4510_normal::read(UINT16 adr)
{
	return program->read_byte(base->map(adr));
}

UINT8 m4510_device::mi_4510_normal::read_sync(UINT16 adr)
{
	return sdirect->read_byte(base->map(adr));
}

UINT8 m4510_device::mi_4510_normal::read_arg(UINT16 adr)
{
	return direct->read_byte(base->map(adr));
}

void m4510_device::mi_4510_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(base->map(adr), val);
}

m4510_device::mi_4510_nd::mi_4510_nd(m4510_device *_base) : mi_4510_normal(_base)
{
}

UINT8 m4510_device::mi_4510_nd::read_sync(UINT16 adr)
{
	return program->read_byte(base->map(adr));
}

UINT8 m4510_device::mi_4510_nd::read_arg(UINT16 adr)
{
	return sprogram->read_byte(base->map(adr));
}

#include "cpu/m6502/m4510.inc"
