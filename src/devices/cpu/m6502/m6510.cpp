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
	read_port(*this, 0),
	write_port(*this),
	dir(0), port(0), drive(0)
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

void m6510_device::init_port()
{
	save_item(NAME(pullup));
	save_item(NAME(floating));
	save_item(NAME(dir));
	save_item(NAME(port));
	save_item(NAME(drive));
}

void m6510_device::device_start()
{
	mintf = std::make_unique<mi_6510>(this);

	init();
	init_port();
}

void m6510_device::device_reset()
{
	m6502_device::device_reset();
	dir = 0xff;
	port = 0xff;
	drive = 0xff;
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


m6510_device::mi_6510::mi_6510(m6510_device *_base)
{
	base = _base;
}

uint8_t m6510_device::mi_6510::read(uint16_t adr)
{
	uint8_t res = program.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510::read_sync(uint16_t adr)
{
	uint8_t res = csprogram.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

uint8_t m6510_device::mi_6510::read_arg(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	return res;
}

void m6510_device::mi_6510::write(uint16_t adr, uint8_t val)
{
	program.write_byte(adr, val);
	if(adr == 0x0000)
		base->dir_w(val);
	else if(adr == 0x0001)
		base->port_w(val);
}


m6508_device::m6508_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	m6510_device(mconfig, M6508, tag, owner, clock)
{
}

void m6508_device::device_start()
{
	mintf = std::make_unique<mi_6508>(this);

	init();
	init_port();

	ram_page = make_unique_clear<uint8_t[]>(256);
	save_pointer(NAME(ram_page), 256);
}


m6508_device::mi_6508::mi_6508(m6508_device *_base)
{
	base = _base;
}

uint8_t m6508_device::mi_6508::read(uint16_t adr)
{
	uint8_t res = program.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	else if(adr < 0x0200)
		res = base->ram_page[adr & 0x00ff];
	return res;
}

uint8_t m6508_device::mi_6508::read_sync(uint16_t adr)
{
	uint8_t res = csprogram.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	else if(adr < 0x0200)
		res = base->ram_page[adr & 0x00ff];
	return res;
}

uint8_t m6508_device::mi_6508::read_arg(uint16_t adr)
{
	uint8_t res = cprogram.read_byte(adr);
	if(adr == 0x0000)
		res = base->dir_r();
	else if(adr == 0x0001)
		res = base->port_r();
	else if(adr < 0x0200)
		res = base->ram_page[adr & 0x00ff];
	return res;
}

void m6508_device::mi_6508::write(uint16_t adr, uint8_t val)
{
	program.write_byte(adr, val);
	if(adr == 0x0000)
		base->dir_w(val);
	else if(adr == 0x0001)
		base->port_w(val);
	else if(adr < 0x0200)
		base->ram_page[adr & 0x00ff] = val;
}


#include "cpu/m6502/m6510.hxx"
