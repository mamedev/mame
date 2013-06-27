/* 68340 SERIAL module */

#include "emu.h"
#include "m68kcpu.h"


READ32_HANDLER( m68000_base_device::m68340_internal_serial_r )
{
	m68000_base_device *m68k = this;
	m68340_serial* serial = m68k->m68340SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = space.device().safe_pc();
		logerror("%08x m68340_internal_serial_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE32_MEMBER( m68000_base_device::m68340_internal_serial_w )
{
	m68000_base_device *m68k = this;
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
