// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 SERIAL module */

#include "emu.h"
#include "68340.h"


READ32_MEMBER( m68340cpu_device::m68340_internal_serial_r )
{
	m68340cpu_device *m68k = this;
	m68340_serial* serial = m68k->m68340SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = space.device().safe_pc();
		logerror("%08x m68340_internal_serial_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE32_MEMBER( m68340cpu_device::m68340_internal_serial_w )
{
	m68340cpu_device *m68k = this;
	m68340_serial* serial = m68k->m68340SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = space.device().safe_pc();
		logerror("%08x m68340_internal_serial_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
	}

}

void m68340_serial::reset(void)
{
}
