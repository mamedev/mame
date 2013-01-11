/* SH3/4 Timer Unit */

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"

static const int tcnt_div[8] = { 4, 16, 64, 256, 1024, 1, 1, 1 };

/*-------------------------------------------------
    sh4_scale_up_mame_time - multiply a attotime by
    a (constant+1) where 0 <= constant < 2^32
-------------------------------------------------*/

INLINE attotime sh4_scale_up_mame_time(attotime _time1, UINT32 factor1)
{
	return _time1 * factor1 + _time1;
}

static UINT32 compute_ticks_timer(emu_timer *timer, int hertz, int divisor)
{
	double ret;

	ret=((timer->remaining().as_double() * (double)hertz) / (double)divisor) - 1;
	return (UINT32)ret;
}

static void sh4_timer_recompute(sh4_state *sh4, int which)
{
	double ticks;

	UINT32 tcnt = 0;
	UINT32 tcr = 0;
	switch (which)
	{
		case 0:
			tcr = sh4->SH4_TCR0;
			tcnt = sh4->SH4_TCNT0;
			break;

		case 1:
			tcr = sh4->SH4_TCR1;
			tcnt = sh4->SH4_TCNT1;
			break;

		case 2:
			tcr = sh4->SH4_TCR2;
			tcnt = sh4->SH4_TCNT2;
			break;
	}

	ticks = tcnt;
	sh4->timer[which]->adjust(sh4_scale_up_mame_time(attotime::from_hz(sh4->pm_clock) * tcnt_div[tcr & 7], ticks), which);
}


TIMER_CALLBACK( sh4_timer_callback )
{
	sh4_state *sh4 = (sh4_state *)ptr;
	int which = param;

	switch (which)
	{
		case 0:
			sh4->SH4_TCNT0 = sh4->SH4_TCOR0;
			break;

		case 1:
			sh4->SH4_TCNT1 = sh4->SH4_TCOR1;
			break;

		case 2:
			sh4->SH4_TCNT2 = sh4->SH4_TCOR2;
			break;

	}

	sh4_timer_recompute(sh4, which);

	switch (which)
	{
		case 0:
			sh4->SH4_TCR0 |= 0x100;
			break;

		case 1:
			sh4->SH4_TCR1 |= 0x100;
			break;

		case 2:
			sh4->SH4_TCR2 |= 0x100;
			break;

	}

	switch (which)
	{
		case 0:
			if (sh4->SH4_TCR0 & 0x20)
			{
				sh4_exception_request(sh4, SH4_INTC_TUNI0);
			//  logerror("SH4_INTC_TUNI0 requested\n");
			}
			break;

		case 1:
			if (sh4->SH4_TCR1 & 0x20)
			{
				sh4_exception_request(sh4, SH4_INTC_TUNI1);
			//  logerror("SH4_INTC_TUNI1 requested\n");
			}
			break;

		case 2:
			if (sh4->SH4_TCR2 & 0x20)
			{
				sh4_exception_request(sh4, SH4_INTC_TUNI2);
			//  logerror("SH4_INTC_TUNI2 requested\n");
			}
			break;

	}
}


UINT32 sh4_handle_tcnt0_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	if (sh4->SH4_TSTR & 1)
		return compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->SH4_TCR0 & 7]);
	else
		return sh4->SH4_TCNT0;
}

UINT32 sh4_handle_tcnt1_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	if (sh4->SH4_TSTR & 2)
		return compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->SH4_TCR1 & 7]);
	else
		return sh4->SH4_TCNT1;
}

UINT32 sh4_handle_tcnt2_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	if (sh4->SH4_TSTR & 4)
		return compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->SH4_TCR2 & 7]);
	else
		return sh4->SH4_TCNT2;
}

UINT32 sh4_handle_tcor0_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCOR0;
}

UINT32 sh4_handle_tcor1_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCOR1;
}

UINT32 sh4_handle_tcor2_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCOR2;
}

UINT32 sh4_handle_tcr0_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCR0;
}

UINT32 sh4_handle_tcr1_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCR1;
}

UINT32 sh4_handle_tcr2_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCR2;
}

UINT32 sh4_handle_tstr_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TSTR;
}

UINT32 sh4_handle_tocr_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TOCR;
}

UINT32 sh4_handle_tcpr2_addr_r(sh4_state *sh4, UINT32 mem_mask)
{
	return sh4->SH4_TCPR2;
}


void sh4_handle_tstr_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	UINT32 old2 = sh4->SH4_TSTR;
	COMBINE_DATA(&sh4->SH4_TSTR);

	if (old2 & 1)
		sh4->SH4_TCNT0 = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->SH4_TCR0 & 7]);
	if ((sh4->SH4_TSTR & 1) == 0) {
		sh4->timer[0]->adjust(attotime::never);
	} else
		sh4_timer_recompute(sh4, 0);

	if (old2 & 2)
		sh4->SH4_TCNT1 = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->SH4_TCR1 & 7]);
	if ((sh4->SH4_TSTR & 2) == 0) {
		sh4->timer[1]->adjust(attotime::never);
	} else
		sh4_timer_recompute(sh4, 1);

	if (old2 & 4)
		sh4->SH4_TCNT2 = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->SH4_TCR2 & 7]);
	if ((sh4->SH4_TSTR & 4) == 0) {
		sh4->timer[2]->adjust(attotime::never);
	} else
		sh4_timer_recompute(sh4, 2);
}

void sh4_handle_tcr0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	UINT32 old2 = sh4->SH4_TCR0;
	COMBINE_DATA(&sh4->SH4_TCR0);
	if (sh4->SH4_TSTR & 1)
	{
		sh4->SH4_TCNT0 = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(sh4, 0);
	}
	if (!(sh4->SH4_TCR0 & 0x20) || !(sh4->SH4_TCR0 & 0x100))
		sh4_exception_unrequest(sh4, SH4_INTC_TUNI0);
}

void sh4_handle_tcr1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	UINT32 old2 = sh4->SH4_TCR1;
	COMBINE_DATA(&sh4->SH4_TCR1);
	if (sh4->SH4_TSTR & 2)
	{
		sh4->SH4_TCNT1 = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(sh4, 1);
	}
	if (!(sh4->SH4_TCR1 & 0x20) || !(sh4->SH4_TCR1 & 0x100))
		sh4_exception_unrequest(sh4, SH4_INTC_TUNI1);
}

void sh4_handle_tcr2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	UINT32 old2 = sh4->SH4_TCR2;
	COMBINE_DATA(&sh4->SH4_TCR2);
	if (sh4->SH4_TSTR & 4)
	{
		sh4->SH4_TCNT2 = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(sh4, 2);
	}
	if (!(sh4->SH4_TCR2 & 0x20) || !(sh4->SH4_TCR2 & 0x100))
		sh4_exception_unrequest(sh4, SH4_INTC_TUNI2);
}

void sh4_handle_tcor0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCOR0);
	if (sh4->SH4_TSTR & 1)
	{
		sh4->SH4_TCNT0 = compute_ticks_timer(sh4->timer[0], sh4->pm_clock, tcnt_div[sh4->SH4_TCR0 & 7]);
		sh4_timer_recompute(sh4, 0);
	}
}

void sh4_handle_tcor1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCOR1);
	if (sh4->SH4_TSTR & 2)
	{
		sh4->SH4_TCNT1 = compute_ticks_timer(sh4->timer[1], sh4->pm_clock, tcnt_div[sh4->SH4_TCR1 & 7]);
		sh4_timer_recompute(sh4, 1);
	}
}

void sh4_handle_tcor2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCOR2);
	if (sh4->SH4_TSTR & 4)
	{
		sh4->SH4_TCNT2 = compute_ticks_timer(sh4->timer[2], sh4->pm_clock, tcnt_div[sh4->SH4_TCR2 & 7]);
		sh4_timer_recompute(sh4, 2);
	}
}

void sh4_handle_tcnt0_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCNT0);
	if (sh4->SH4_TSTR & 1)
		sh4_timer_recompute(sh4, 0);
}

void sh4_handle_tcnt1_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCNT1);
	if (sh4->SH4_TSTR & 2)
		sh4_timer_recompute(sh4, 1);
}

void sh4_handle_tcnt2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCNT2);
	if (sh4->SH4_TSTR & 4)
		sh4_timer_recompute(sh4, 2);
}

void sh4_handle_tocr_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TOCR);
}

void sh4_handle_tcpr2_addr_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_TCPR2);
}
