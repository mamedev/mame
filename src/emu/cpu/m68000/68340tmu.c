/* 68340 TIMER module */

#include "emu.h"
#include "m68kcpu.h"



READ32_HANDLER( m68340_internal_timer_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_timer* timer = m68k->m68340TIMER;
	assert(timer != NULL);

	if (timer)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_timer_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE32_HANDLER( m68340_internal_timer_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68340_timer* timer = m68k->m68340TIMER;
	assert(timer != NULL);

	if (timer)
	{
		int pc = cpu_get_pc(&space->device());
		logerror("%08x m68340_internal_timer_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
	}
}

void m68340_timer::reset(void)
{

}
