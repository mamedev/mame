// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6510.c

    6502 with 6 i/o pins, also known as 8500

***************************************************************************/

#include "emu.h"
#include "m6510.h"

const device_type M6510 = &device_creator<m6510_device>;

m6510_device::m6510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M6510, "M6510", tag, owner, clock, "m6510", __FILE__),
	read_port(*this),
	write_port(*this), dir(0), port(0), drive(0)
{
	pullup = 0x00;
	floating = 0x00;
}

m6510_device::m6510_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	m6502_device(mconfig, type, name, tag, owner, clock, shortname, source),
	read_port(*this),
	write_port(*this), dir(0), port(0), drive(0)
{
	pullup = 0x00;
	floating = 0x00;
}

void m6510_device::set_pulls(UINT8 _pullup, UINT8 _floating)
{
	pullup = _pullup;
	floating = _floating;
}

offs_t m6510_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void m6510_device::device_start()
{
	read_port.resolve_safe(0);
	write_port.resolve_safe();

	if(direct_disabled)
		mintf = new mi_6510_nd(this);
	else
		mintf = new mi_6510_normal(this);

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

UINT8 m6510_device::get_port()
{
	return (port & dir) | (pullup & ~dir);
}

UINT8 m6510_device::dir_r()
{
	return dir;
}

UINT8 m6510_device::port_r()
{
	return ((read_port() | (floating & drive)) & ~dir) | (port & dir);
}

void m6510_device::dir_w(UINT8 data)
{
	dir = data;
	update_port();
}

void m6510_device::port_w(UINT8 data)
{
	port = data;
	update_port();
}


m6510_device::mi_6510_normal::mi_6510_normal(m6510_device *_base)
{
	base = _base;
}

UINT8 m6510_device::mi_6510_normal::read(UINT16 adr)
{
	UINT8 res = program->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

UINT8 m6510_device::mi_6510_normal::read_sync(UINT16 adr)
{
	UINT8 res = sdirect->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

UINT8 m6510_device::mi_6510_normal::read_arg(UINT16 adr)
{
	UINT8 res = direct->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

void m6510_device::mi_6510_normal::write(UINT16 adr, UINT8 val)
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

UINT8 m6510_device::mi_6510_nd::read_sync(UINT16 adr)
{
	UINT8 res = sprogram->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

UINT8 m6510_device::mi_6510_nd::read_arg(UINT16 adr)
{
	UINT8 res = program->read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

#include "cpu/m6502/m6510.inc"
