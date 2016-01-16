// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

***************************************************************************/

#include "emu.h"
#include "i8xc196.h"

i8xc196_device::i8xc196_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	mcs96_device(mconfig, type, name, tag, owner, clock, 16, shortname, source)
{
}

offs_t i8xc196_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disasm_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void i8xc196_device::io_w8(UINT8 adr, UINT8 data)
{
	switch(adr) {
	case 0:
		break;
	case 1:
		break;
	default:
		logerror("%s: io_w8 %02x, %02x (%04x)\n", tag().c_str(), adr, data, PPC);
	}
	return;
}

void i8xc196_device::io_w16(UINT8 adr, UINT16 data)
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

UINT8 i8xc196_device::io_r8(UINT8 adr)
{
	switch(adr) {
	case 0x00:
		return 0x00;
	case 0x01:
		return 0x00;
	}
	UINT8 data = 0x00;
	logerror("%s: io_r8 %02x, %02x (%04x)\n", tag().c_str(), adr, data, PPC);
	return data;
}

UINT16 i8xc196_device::io_r16(UINT8 adr)
{
	if(adr < 2)
		return 0x0000;
	UINT16 data = 0x0000;
	logerror("%s: io_r16 %02x, %04x (%04x)\n", tag().c_str(), adr, data, PPC);
	return data;
}

void i8xc196_device::do_exec_partial()
{
}

#include "cpu/mcs96/i8xc196.inc"
