// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6507.c

    Mostek 6502, NMOS variant with reduced address bus

***************************************************************************/

#include "emu.h"
#include "m6507.h"

const device_type M6507 = &device_creator<m6507_device>;

m6507_device::m6507_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M6507, "M6507", tag, owner, clock, "m6507", __FILE__)
{
	program_config.m_addrbus_width = 13;
	sprogram_config.m_addrbus_width = 13;
}

void m6507_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_6507_nd;
	else
		mintf = new mi_6507_normal;

	init();
}

UINT8 m6507_device::mi_6507_normal::read(UINT16 adr)
{
	return program->read_byte(adr & 0x1fff);
}

UINT8 m6507_device::mi_6507_normal::read_sync(UINT16 adr)
{
	return sdirect->read_byte(adr & 0x1fff);
}

UINT8 m6507_device::mi_6507_normal::read_arg(UINT16 adr)
{
	return direct->read_byte(adr & 0x1fff);
}

void m6507_device::mi_6507_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr & 0x1fff, val);
}

UINT8 m6507_device::mi_6507_nd::read_sync(UINT16 adr)
{
	return sprogram->read_byte(adr & 0x1fff);
}

UINT8 m6507_device::mi_6507_nd::read_arg(UINT16 adr)
{
	return program->read_byte(adr & 0x1fff);
}
