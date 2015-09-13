// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502.c

    6502, NES variant

***************************************************************************/

#include "emu.h"
#include "n2a03.h"

const device_type N2A03 = &device_creator<n2a03_device>;

n2a03_device::n2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, N2A03, "N2A03", tag, owner, clock, "n2a03", __FILE__)
{
}

offs_t n2a03_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void n2a03_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_2a03_nd;
	else
		mintf = new mi_2a03_normal;

	init();
}

UINT8 n2a03_device::mi_2a03_normal::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_sync(UINT16 adr)
{
	return sdirect->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_arg(UINT16 adr)
{
	return direct->read_byte(adr);
}

void n2a03_device::mi_2a03_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

UINT8 n2a03_device::mi_2a03_nd::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_sync(UINT16 adr)
{
	return sprogram->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_arg(UINT16 adr)
{
	return program->read_byte(adr);
}

void n2a03_device::mi_2a03_nd::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

#include "cpu/m6502/n2a03.inc"
