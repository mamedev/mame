/* 68307 TIMER module */
// 2x timers

#include "emu.h"
#include "m68kcpu.h"

READ16_HANDLER( m68307_internal_timer_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_timer* timer = m68k->m68307TIMER;
	assert(timer != NULL);

	if (timer)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68307_internal_timer_r %08x, (%04x)\n", pc, offset*2,mem_mask);
	}
		
	return 0x0000;
}

WRITE16_HANDLER( m68307_internal_timer_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_timer* timer = m68k->m68307TIMER;
	assert(timer != NULL);

	if (timer)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68307_internal_timer_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
	}
}

void m68307_timer::reset(void)
{
	for (int i=0;i<2;i++)
	{
		m68307_single_timer* tptr = &timer[i];

		tptr->regs[m68307TIMER_TMR] = 0x0000;
		tptr->regs[m68307TIMER_TRR] = 0xffff;
		tptr->regs[m68307TIMER_TCR] = 0x0000;
		tptr->regs[m68307TIMER_TCN] = 0x0000;
		tptr->regs[m68307TIMER_TER] = 0x0000;
		tptr->regs[m68307TIMER_WRR] = 0xffff;
		tptr->regs[m68307TIMER_WCR] = 0xffff;
		tptr->regs[m68307TIMER_XXX] = 0;
		tptr->enabled = false;
	}
}

