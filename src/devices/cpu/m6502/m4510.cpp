// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510.cpp

    65ce02 with a mmu and a cia integrated

    differences between the standard 65ce02 and this CPU:
    http://www.zimmers.net/anonftp/pub/cbm/c65/65ce02.txt

***************************************************************************/

#include "emu.h"
#include "m4510.h"
#include "m4510d.h"

DEFINE_DEVICE_TYPE(M4510, m4510_device, "m4510", "CSG M4510")

m4510_device::m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65ce02_device(mconfig, M4510, tag, owner, clock),
	map_enable(0),
	nomap(false)
{
	program_config.m_addr_width = 20;
	program_config.m_logaddr_width = 16;
	program_config.m_page_shift = 13;
	sprogram_config.m_addr_width = 20;
	sprogram_config.m_logaddr_width = 16;
	sprogram_config.m_page_shift = 13;
}

std::unique_ptr<util::disasm_interface> m4510_device::create_disassembler()
{
	return std::make_unique<m4510_disassembler>();
}

void m4510_device::device_start()
{
	mintf = std::make_unique<mi_4510>(this);

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

bool m4510_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM)
	{
		address = map(address);
	}

	return true;
}

m4510_device::mi_4510::mi_4510(m4510_device *_base)
{
	base = _base;
}

uint8_t m4510_device::mi_4510::read(uint16_t adr)
{
	return program->read_byte(base->map(adr));
}

uint8_t m4510_device::mi_4510::read_sync(uint16_t adr)
{
	return scache->read_byte(base->map(adr));
}

uint8_t m4510_device::mi_4510::read_arg(uint16_t adr)
{
	return cache->read_byte(base->map(adr));
}

void m4510_device::mi_4510::write(uint16_t adr, uint8_t val)
{
	program->write_byte(base->map(adr), val);
}

#include "cpu/m6502/m4510.hxx"
