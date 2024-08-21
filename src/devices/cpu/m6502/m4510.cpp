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

DEFINE_DEVICE_TYPE(M4510, m4510_device, "m4510", "CSG 4510")

m4510_device::m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m65ce02_device(mconfig, M4510, tag, owner, clock),
	map_enable(0),
	nomap(false),
	read_port(*this, 0),
	write_port(*this),
	dir(0), port(0), drive(0)
{
	program_config.m_addr_width = 20;
	program_config.m_logaddr_width = 16;
	program_config.m_page_shift = 13;
	sprogram_config.m_addr_width = 20;
	sprogram_config.m_logaddr_width = 16;
	sprogram_config.m_page_shift = 13;
	pullup = 0x00;
	floating = 0x00;
}

void m4510_device::set_pulls(uint8_t _pullup, uint8_t _floating)
{
	pullup = _pullup;
	floating = _floating;
}

std::unique_ptr<util::disasm_interface> m4510_device::create_disassembler()
{
	return std::make_unique<m4510_disassembler>();
}

void m4510_device::init_port()
{
	save_item(NAME(pullup));
	save_item(NAME(floating));
	save_item(NAME(dir));
	save_item(NAME(port));
	save_item(NAME(drive));
}

void m4510_device::device_start()
{
	mintf = std::make_unique<mi_4510>(this);

	m65ce02_device::init();
	init_port();

	save_item(NAME(map_offset));
	save_item(NAME(map_enable));
}

void m4510_device::device_reset()
{
	map_offset[0] = map_offset[1] = 0;
	map_enable = 0;
	nomap = true;

	m65ce02_device::device_reset();
	dir = 0xff;
	port = 0xff;
	drive = 0xff;
	update_port();
}

void m4510_device::update_port()
{
	drive = (port & dir) | (drive & ~dir);
	write_port((port & dir) | (pullup & ~dir));
}

uint8_t m4510_device::get_port()
{
	return (port & dir) | (pullup & ~dir);
}

uint8_t m4510_device::dir_r()
{
	return dir;
}

uint8_t m4510_device::port_r()
{
	return ((read_port() | (floating & drive)) & ~dir) | (port & dir);
}

void m4510_device::dir_w(uint8_t data)
{
	dir = data;
	update_port();
}

void m4510_device::port_w(uint8_t data)
{
	port = data;
	update_port();
}

bool m4510_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	target_space = &space(spacenum);
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
	uint8_t res = program.read_byte(base->map(adr));
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m4510_device::mi_4510::read_sync(uint16_t adr)
{
	uint8_t res = csprogram.read_byte(base->map(adr));
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m4510_device::mi_4510::read_arg(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(base->map(adr));
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

void m4510_device::mi_4510::write(uint16_t adr, uint8_t val)
{
	program.write_byte(base->map(adr), val);
	if(adr == 0x0000)
		base->dir_w(val);
	else if(adr == 0x0001)
		base->port_w(val);
}

#include "cpu/m6502/m4510.hxx"
