/* 68340 SIM module */

#include "emu.h"
#include "m68kcpu.h"


READ32_HANDLER( m68340_internal_sim_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_sim_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE32_HANDLER( m68340_internal_sim_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_sim_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
	}
}

void m68340_sim::reset(void)
{

}
