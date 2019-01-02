// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6504.c

    MOS Technology 6502, NMOS variant with reduced address bus

***************************************************************************/

#include "emu.h"
#include "m6504.h"

DEFINE_DEVICE_TYPE(M6504, m6504_device, "m6504", "MOS Technology M6504")

m6504_device::m6504_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6504, tag, owner, clock)
{
	program_config.m_addr_width = 13;
	sprogram_config.m_addr_width = 13;
}

void m6504_device::device_start()
{
	if(cache_disabled)
		mintf = std::make_unique<mi_6504_nd>();
	else
		mintf = std::make_unique<mi_6504_normal>();

	init();
}

uint8_t m6504_device::mi_6504_normal::read(uint16_t adr)
{
	return program->read_byte(adr & 0x1fff);
}

uint8_t m6504_device::mi_6504_normal::read_sync(uint16_t adr)
{
	return scache->read_byte(adr & 0x1fff);
}

uint8_t m6504_device::mi_6504_normal::read_arg(uint16_t adr)
{
	return cache->read_byte(adr & 0x1fff);
}

void m6504_device::mi_6504_normal::write(uint16_t adr, uint8_t val)
{
	program->write_byte(adr & 0x1fff, val);
}

uint8_t m6504_device::mi_6504_nd::read_sync(uint16_t adr)
{
	return sprogram->read_byte(adr & 0x1fff);
}

uint8_t m6504_device::mi_6504_nd::read_arg(uint16_t adr)
{
	return program->read_byte(adr & 0x1fff);
}
