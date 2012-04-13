/* 68340 SIM module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68340_internal_sim_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_sim_r %04x, (%04x)\n", pc, offset*2,mem_mask);
	}

	return 0x0000;
}

WRITE16_HANDLER( m68340_internal_sim_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_sim_w %04x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
	}
}

void m68340_sim::reset(void)
{

}
