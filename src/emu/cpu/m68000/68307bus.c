/* 68307 MBUS module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68307_internal_mbus_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_mbus* mbus = m68k->m68307MBUS;
	assert(mbus != NULL);

	if (mbus)
	{
		int pc = cpu_get_pc(&space->device());


		switch (offset<<1)
		{
			case m68307BUS_MBSR:
				logerror("%08x m68307_internal_mbus_r %08x, (%04x) (MBSR - Status Register)\n", pc, offset*2,mem_mask);
				return space->machine().rand();

			default:
				logerror("%08x m68307_internal_mbus_r %08x, (%04x)\n", pc, offset*2,mem_mask);
				return 0x0000;
		}
	}

	return 0xffff;
}

WRITE16_HANDLER( m68307_internal_mbus_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_mbus* mbus = m68k->m68307MBUS;
	assert(mbus != NULL);

	if (mbus)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68307_internal_mbus_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
	}
}

void m68307_mbus::reset(void)
{

}
