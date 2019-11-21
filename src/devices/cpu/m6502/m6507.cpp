// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6507.c

    MOS Technology 6502, NMOS variant with reduced address bus

***************************************************************************/

#include "emu.h"
#include "m6507.h"

DEFINE_DEVICE_TYPE(M6507, m6507_device, "m6507", "MOS Technology M6507")

m6507_device::m6507_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, M6507, tag, owner, clock)
{
	program_config.m_addr_width = 13;
	sprogram_config.m_addr_width = 13;
}

void m6507_device::device_start()
{
	mintf = std::make_unique<mi_6507>();

	init();
}

uint8_t m6507_device::mi_6507::read(uint16_t adr)
{
	return program->read_byte(adr & 0x1fff);
}

uint8_t m6507_device::mi_6507::read_sync(uint16_t adr)
{
	return scache->read_byte(adr & 0x1fff);
}

uint8_t m6507_device::mi_6507::read_arg(uint16_t adr)
{
	return cache->read_byte(adr & 0x1fff);
}

void m6507_device::mi_6507::write(uint16_t adr, uint8_t val)
{
	program->write_byte(adr & 0x1fff, val);
}
