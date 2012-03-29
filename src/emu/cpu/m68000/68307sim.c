/* 68307 SIM module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68307_internal_sim_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68307_internal_sim_r %08x, (%04x)\n", pc, offset*2,mem_mask);

	if (sim)
	{
		switch (offset<<1)
		{
			case m68307SIM_BR0:	return (sim->m_br[0]);
			case m68307SIM_OR0:	return (sim->m_or[0]);
			case m68307SIM_BR1:	return (sim->m_br[1]);
			case m68307SIM_OR1:	return (sim->m_or[1]);
			case m68307SIM_BR2:	return (sim->m_br[2]);
			case m68307SIM_OR2:	return (sim->m_or[2]);
			case m68307SIM_BR3:	return (sim->m_br[3]);
			case m68307SIM_OR3:	return (sim->m_or[3]);
		}
	}


	return 0x0000;
}

WRITE16_HANDLER( m68307_internal_sim_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68307_internal_sim_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);

	if (sim)
	{
		switch (offset<<1)
		{
			
			case m68307SIM_BR0:
				COMBINE_DATA(&sim->m_br[0]);
				break;
			case m68307SIM_OR0:
				COMBINE_DATA(&sim->m_or[0]);
				break;
			case m68307SIM_BR1:
				COMBINE_DATA(&sim->m_br[1]);
				break;
			case m68307SIM_OR1:
				COMBINE_DATA(&sim->m_or[1]);
				break;
			case m68307SIM_BR2:
				COMBINE_DATA(&sim->m_br[2]);
				break;
			case m68307SIM_OR2:
				COMBINE_DATA(&sim->m_or[2]);
				break;
			case m68307SIM_BR3:
				COMBINE_DATA(&sim->m_br[3]);
				break;
			case m68307SIM_OR3:
				COMBINE_DATA(&sim->m_or[3]);
				break;
		}
	}
}


void m68307_sim::reset(void)
{
	for (int i=0;i<4;i++)
	{
		m_br[i] = 0xc001;
		m_or[i] = 0xdffd;
	}
}
