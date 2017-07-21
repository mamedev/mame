// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m4510.c

    65ce02 with a mmu and a cia integrated

***************************************************************************/

#include "emu.h"
#include "m4510.h"

DEFINE_DEVICE_TYPE(M4510, m4510_device, "m4510", "M4510")

m4510_device::m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65ce02_device(mconfig, M4510, tag, owner, clock),
	map_enable(0),
	nomap(false)
{
	program_config.m_addrbus_width = 20;
	program_config.m_logaddr_width = 16;
	program_config.m_page_shift = 13;
	sprogram_config.m_addrbus_width = 20;
	sprogram_config.m_logaddr_width = 16;
	sprogram_config.m_page_shift = 13;
}

offs_t m4510_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	return disassemble_generic(stream, pc, oprom, opram, options, disasm_entries);
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

bool m4510_device::memory_translate(int spacenum, int intention, offs_t &address)
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

uint8_t m4510_device::mi_4510_normal::read(uint16_t adr)
{
	return program->read_byte(base->map(adr));
}

uint8_t m4510_device::mi_4510_normal::read_sync(uint16_t adr)
{
	return sdirect->read_byte(base->map(adr));
}

uint8_t m4510_device::mi_4510_normal::read_arg(uint16_t adr)
{
	return direct->read_byte(base->map(adr));
}

void m4510_device::mi_4510_normal::write(uint16_t adr, uint8_t val)
{
	program->write_byte(base->map(adr), val);
}

m4510_device::mi_4510_nd::mi_4510_nd(m4510_device *_base) : mi_4510_normal(_base)
{
}

uint8_t m4510_device::mi_4510_nd::read_sync(uint16_t adr)
{
	return sprogram->read_byte(base->map(adr));
}

uint8_t m4510_device::mi_4510_nd::read_arg(uint16_t adr)
{
	return program->read_byte(base->map(adr));
}

#include "cpu/m6502/m4510.hxx"
