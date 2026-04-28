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
	m_map_enable(0),
	m_nomap(false),
	m_read_port(*this, 0),
	m_write_port(*this),
	m_dir(0), m_port(0), m_drive(0)
{
	m_program_config.m_addr_width = 20;
	m_program_config.m_logaddr_width = 16;
	m_program_config.m_page_shift = 13;
	m_sprogram_config.m_addr_width = 20;
	m_sprogram_config.m_logaddr_width = 16;
	m_sprogram_config.m_page_shift = 13;
	m_pullup = 0x00;
	m_floating = 0x00;
}

void m4510_device::set_pulls(uint8_t _pullup, uint8_t _floating)
{
	m_pullup = _pullup;
	m_floating = _floating;
}

std::unique_ptr<util::disasm_interface> m4510_device::create_disassembler()
{
	return std::make_unique<m4510_disassembler>();
}

void m4510_device::init_port()
{
	save_item(NAME(m_pullup));
	save_item(NAME(m_floating));
	save_item(NAME(m_dir));
	save_item(NAME(m_port));
	save_item(NAME(m_drive));
}

void m4510_device::device_start()
{
	m_mintf = std::make_unique<mi_4510>(this);

	m65ce02_device::init();
	init_port();

	save_item(NAME(m_map_offset));
	save_item(NAME(m_map_enable));
}

void m4510_device::device_reset()
{
	m_map_offset[0] = m_map_offset[1] = 0;
	m_map_enable = 0;
	m_nomap = true;

	m65ce02_device::device_reset();
	m_dir = 0xff;
	m_port = 0xff;
	m_drive = 0xff;
	update_port();
}

void m4510_device::update_port()
{
	m_drive = (m_port & m_dir) | (m_drive & ~m_dir);
	m_write_port((m_port & m_dir) | (m_pullup & ~m_dir));
}

uint8_t m4510_device::get_port()
{
	return (m_port & m_dir) | (m_pullup & ~m_dir);
}

uint8_t m4510_device::dir_r()
{
	return m_dir;
}

uint8_t m4510_device::port_r()
{
	return ((m_read_port() | (m_floating & m_drive)) & ~m_dir) | (m_port & m_dir);
}

void m4510_device::dir_w(uint8_t data)
{
	m_dir = data;
	update_port();
}

void m4510_device::port_w(uint8_t data)
{
	m_port = data;
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
	m_base = _base;
}

uint8_t m4510_device::mi_4510::read(uint16_t adr)
{
	uint8_t res = m_program.read_interruptible(m_base->map(adr));
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

uint8_t m4510_device::mi_4510::read_sync(uint16_t adr)
{
	uint8_t res = m_csprogram.read_interruptible(m_base->map(adr));
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

uint8_t m4510_device::mi_4510::read_arg(uint16_t adr)
{
	uint8_t res = m_cprogram.read_interruptible(m_base->map(adr));
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

void m4510_device::mi_4510::write(uint16_t adr, uint8_t val)
{
	m_program.write_interruptible(m_base->map(adr), val);
	if(adr == 0x0000)
		m_base->dir_w(val);
	else if(adr == 0x0001)
		m_base->port_w(val);
}

#include "cpu/m6502/m4510.hxx"
