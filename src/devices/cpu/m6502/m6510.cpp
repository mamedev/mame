// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510.c

    6502 with 6 i/o pins, also known as 8500

***************************************************************************/

#include "emu.h"
#include "m6510.h"
#include "m6510d.h"

DEFINE_DEVICE_TYPE(M6510, m6510_device, "m6510", "MOS Technology M6510")

m6510_device::m6510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M6510, tag, owner, clock)
{
}

m6510_device::m6510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	read_port(*this),
	write_port(*this), dir(0), port(0), drive(0)
{
	pullup = 0x00;
	floating = 0x00;
}

void m6510_device::set_pulls(uint8_t _pullup, uint8_t _floating)
{
	pullup = _pullup;
	floating = _floating;
}

std::unique_ptr<util::disasm_interface> m6510_device::create_disassembler()
{
	return std::make_unique<m6510_disassembler>();
}

void m6510_device::device_start()
{
	read_port.resolve_safe(0);
	write_port.resolve_safe();

	if(cache_disabled)
		mintf = std::make_unique<mi_6510_nd>(this);
	else
		mintf = std::make_unique<mi_6510_normal>(this);

	init();

	save_item(NAME(pullup));
	save_item(NAME(floating));
	save_item(NAME(dir));
	save_item(NAME(port));
	save_item(NAME(drive));
}

void m6510_device::device_reset()
{
	m6502_device::device_reset();
	dir = 0x00;
	port = 0x00;
	drive = 0x00;
	update_port();
}

void m6510_device::update_port()
{
	drive = (port & dir) | (drive & ~dir);
	write_port((port & dir) | (pullup & ~dir));
}

uint8_t m6510_device::get_port()
{
	return (port & dir) | (pullup & ~dir);
}

uint8_t m6510_device::dir_r()
{
	return dir;
}

uint8_t m6510_device::port_r()
{
	return ((read_port() | (floating & drive)) & ~dir) | (port & dir);
}

void m6510_device::dir_w(uint8_t data)
{
	dir = data;
	update_port();
}

void m6510_device::port_w(uint8_t data)
{
	port = data;
	update_port();
}


m6510_device::mi_6510_normal::mi_6510_normal(m6510_device *_base)
{
	base = _base;
}

uint8_t m6510_device::mi_6510_normal::read(uint16_t adr)
{
	uint8_t res = program->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510_normal::read_sync(uint16_t adr)
{
	uint8_t res = scache->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510_normal::read_arg(uint16_t adr)
{
	uint8_t res = cache->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

void m6510_device::mi_6510_normal::write(uint16_t adr, uint8_t val)
{
	program->write_byte(adr, val);
	if(adr == 0x0000)
		base->dir_w(val);
	else if(adr == 0x0001)
		base->port_w(val);
}

m6510_device::mi_6510_nd::mi_6510_nd(m6510_device *_base) : mi_6510_normal(_base)
{
}

uint8_t m6510_device::mi_6510_nd::read_sync(uint16_t adr)
{
	uint8_t res = sprogram->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510_nd::read_arg(uint16_t adr)
{
	uint8_t res = program->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

#include "cpu/m6502/m6510.hxx"
