// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#include "emu.h"
#include "i8xc196.h"
#include "i8xc196d.h"

i8xc196_device::i8xc196_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	mcs96_device(mconfig, type, tag, owner, clock, 16)
{
}

std::unique_ptr<util::disasm_interface> i8xc196_device::create_disassembler()
{
	return std::make_unique<i8xc196_disassembler>();
}

void i8xc196_device::io_w8(uint8_t adr, uint8_t data)
{
	switch(adr) {
	case 0:
		break;
	case 1:
		break;
	default:
		logerror("%s: io_w8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	}
	return;
}

void i8xc196_device::io_w16(uint8_t adr, uint16_t data)
{
	switch(adr) {
	case 0:
		break;
	default:
		io_w8(adr, data);
		io_w8(adr+1, data>>8);
		break;
	}
	return;
}

uint8_t i8xc196_device::io_r8(uint8_t adr)
{
	switch(adr) {
	case 0x00:
		return 0x00;
	case 0x01:
		return 0x00;
	}
	uint8_t data = 0x00;
	logerror("%s: io_r8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

uint16_t i8xc196_device::io_r16(uint8_t adr)
{
	if(adr < 2)
		return 0x0000;
	uint16_t data = 0x0000;
	logerror("%s: io_r16 %02x, %04x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

void i8xc196_device::do_exec_partial()
{
}

#include "cpu/mcs96/i8xc196.hxx"
