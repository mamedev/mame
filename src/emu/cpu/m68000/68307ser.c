/* 68307 SERIAL Module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68307_internal_serial_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68307_internal_serial_r %08x, (%04x)\n", pc, offset*2,mem_mask);
	}

	return 0x0000;
}

WRITE16_HANDLER( m68307_internal_serial_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68307_internal_serial_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
	}
}

void m68307_serial::reset(void)
{

}

