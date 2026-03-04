// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510.cpp

    6502 with 6 i/o pins, also known as 8500

    6508 is 6510 plus 256 bytes of internal RAM, mirrored across pages 0
    and 1.

***************************************************************************/

#include "emu.h"
#include "m6510.h"
#include "m6510d.h"

DEFINE_DEVICE_TYPE(M6510, m6510_device, "m6510", "MOS Technology 6510")
DEFINE_DEVICE_TYPE(M6508, m6508_device, "m6508", "MOS Technology 6508")

m6510_device::m6510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M6510, tag, owner, clock)
{
}

m6510_device::m6510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_read_port(*this, 0),
	m_write_port(*this),
	m_dir(0), m_port(0), m_drive(0)
{
	m_pullup = 0x00;
	m_floating = 0x00;
}

void m6510_device::set_pulls(uint8_t _pullup, uint8_t _floating)
{
	m_pullup = _pullup;
	m_floating = _floating;
}

std::unique_ptr<util::disasm_interface> m6510_device::create_disassembler()
{
	return std::make_unique<m6510_disassembler>();
}

void m6510_device::init_port()
{
	save_item(NAME(m_pullup));
	save_item(NAME(m_floating));
	save_item(NAME(m_dir));
	save_item(NAME(m_port));
	save_item(NAME(m_drive));
}

void m6510_device::device_start()
{
	m_mintf = std::make_unique<mi_6510>(this);

	init();
	init_port();
}

void m6510_device::device_reset()
{
	m6502_device::device_reset();
	m_dir = 0xff;
	m_port = 0xff;
	m_drive = 0xff;
	update_port();
}

void m6510_device::update_port()
{
	m_drive = (m_port & m_dir) | (m_drive & ~m_dir);
	m_write_port((m_port & m_dir) | (m_pullup & ~m_dir));
}

uint8_t m6510_device::get_port()
{
	return (m_port & m_dir) | (m_pullup & ~m_dir);
}

uint8_t m6510_device::dir_r()
{
	return m_dir;
}

uint8_t m6510_device::port_r()
{
	return ((m_read_port() | (m_floating & m_drive)) & ~m_dir) | (m_port & m_dir);
}

void m6510_device::dir_w(uint8_t data)
{
	m_dir = data;
	update_port();
}

void m6510_device::port_w(uint8_t data)
{
	m_port = data;
	update_port();
}


m6510_device::mi_6510::mi_6510(m6510_device *_base)
{
	m_base = _base;
}

uint8_t m6510_device::mi_6510::read(uint16_t adr)
{
	uint8_t res = m_program.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510::read_sync(uint16_t adr)
{
	uint8_t res = m_csprogram.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510::read_arg(uint16_t adr)
{
	uint8_t res = m_cprogram.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	return res;
}

void m6510_device::mi_6510::write(uint16_t adr, uint8_t val)
{
	m_program.write_byte(adr, val);
	if(adr == 0x0000)
		m_base->dir_w(val);
	else if(adr == 0x0001)
		m_base->port_w(val);
}


m6508_device::m6508_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M6508, tag, owner, clock)
{
}

void m6508_device::device_start()
{
	m_mintf = std::make_unique<mi_6508>(this);

	init();
	init_port();

	m_ram_page = make_unique_clear<uint8_t[]>(256);
	save_pointer(NAME(m_ram_page), 256);
}


m6508_device::mi_6508::mi_6508(m6508_device *_base)
{
	m_base = _base;
}

uint8_t m6508_device::mi_6508::read(uint16_t adr)
{
	uint8_t res = m_program.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	else if(adr < 0x0200)
		res = m_base->m_ram_page[adr & 0x00ff];
	return res;
}

uint8_t m6508_device::mi_6508::read_sync(uint16_t adr)
{
	uint8_t res = m_csprogram.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	else if(adr < 0x0200)
		res = m_base->m_ram_page[adr & 0x00ff];
	return res;
}

uint8_t m6508_device::mi_6508::read_arg(uint16_t adr)
{
	uint8_t res = m_cprogram.read_byte(adr);
	if(adr == 0x0000)
		res = m_base->dir_r();
	else if(adr == 0x0001)
		res = m_base->port_r();
	else if(adr < 0x0200)
		res = m_base->m_ram_page[adr & 0x00ff];
	return res;
}

void m6508_device::mi_6508::write(uint16_t adr, uint8_t val)
{
	m_program.write_byte(adr, val);
	if(adr == 0x0000)
		m_base->dir_w(val);
	else if(adr == 0x0001)
		m_base->port_w(val);
	else if(adr < 0x0200)
		m_base->m_ram_page[adr & 0x00ff] = val;
}


#include "cpu/m6502/m6510.hxx"
