// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68307 TIMER module */
// 2x timers

#include "emu.h"
#include "68307.h"

READ16_MEMBER( m68307cpu_device::m68307_internal_timer_r )
{
	m68307cpu_device *m68k = this;
	m68307_timer* timer = m68k->m68307TIMER;
	assert(timer != nullptr);

	if (timer)
	{
		int pc = space.device().safe_pc();
		int which = offset & 0x8;

		switch (offset&0x7)
		{
			case m68307TIMER_TCN: /* 0x3 (0x126 / 0x136) */
				//if (pc!=0x2182e) logerror("%08x m68307_internal_timer_r %08x (%04x) (TCN - Timer Counter for timer %d)\n", pc, offset*2,mem_mask, which);
				return timer->read_tcn(mem_mask, which);

			default:
				logerror("%08x m68307_internal_timer_r %08x, (%04x)\n", pc, offset*2,mem_mask);
				break;
		}
	}

	return 0x0000;
}

WRITE16_MEMBER( m68307cpu_device::m68307_internal_timer_w )
{
	m68307cpu_device *m68k = this;
	m68307_timer* timer = m68k->m68307TIMER;
	assert(timer != nullptr);

	if (timer)
	{
		int pc = space.device().safe_pc();
		int which = offset & 0x8;

		switch (offset&0x7)
		{
			case m68307TIMER_TMR: /* 0x0 (0x120 / 0x130) */
				logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (TMR - Timer Mode Register for timer %d)\n", pc, offset*2,data,mem_mask, which);
				timer->write_tmr(data, mem_mask, which);
				break;

			case m68307TIMER_TRR: /* 0x1 (0x122 / 0x132) */
				logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (TRR - Timer Reference Register for timer %d)\n", pc, offset*2,data,mem_mask, which);
				timer->write_trr(data, mem_mask, which);
				break;

			case m68307TIMER_TCR: /* 0x2 (0x124 / 0x134) */
				logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (TCR - Timer Capture Register for timer %d) (illegal, read-only)\n", pc, offset*2,data,mem_mask, which);
				break;

			case m68307TIMER_TCN: /* 0x3 (0x126 / 0x136) */
				logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (TCN - Timer Counter for timer %d)\n", pc, offset*2,data,mem_mask, which);
				break;

			case m68307TIMER_TER: /* 0x4 (0x128 / 0x138) */
				/* 8-bit only!! */
				//logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (TER - Timer Event Register for timer %d)\n", pc, offset*2,data,mem_mask, which);
				timer->write_ter(data, mem_mask, which);
				break;

			case m68307TIMER_WRR: /* 0x5 (0x12a / 0x13a) */
				if (which==0)
				{
					logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (WRR - Watchdog Reference Register)\n", pc, offset*2,data,mem_mask);
				}
				else
				{
					logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (illegal)\n", pc, offset*2,data,mem_mask);
				}
				break;

			case m68307TIMER_WCR: /* 0x6 (0x12c / 0x13c) */
				if (which==0)
				{
					logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (WRR - Watchdog Counter Register)\n", pc, offset*2,data,mem_mask);
				}
				else
				{
					logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (illegal)\n", pc, offset*2,data,mem_mask);
				}
				break;

			case m68307TIMER_XXX: /* 0x7 (0x12e / 0x13e) */
				logerror("%08x m68307_internal_timer_w %08x, %04x (%04x) (illegal)\n", pc, offset*2,data,mem_mask);
				break;

		}
	}
}

static TIMER_CALLBACK( m68307_timer0_callback )
{
	m68307cpu_device* m68k = (m68307cpu_device *)ptr;
	m68307_single_timer* tptr = &m68k->m68307TIMER->singletimer[0];
	tptr->regs[m68307TIMER_TMR] |= 0x2;

	m68k->timer0_interrupt();

	tptr->mametimer->adjust(m68k->cycles_to_attotime(20000));
}

static TIMER_CALLBACK( m68307_timer1_callback )
{
	m68307cpu_device* m68k = (m68307cpu_device *)ptr;
	m68307_single_timer* tptr = &m68k->m68307TIMER->singletimer[1];
	tptr->regs[m68307TIMER_TMR] |= 0x2;

	m68k->timer1_interrupt();

	tptr->mametimer->adjust(m68k->cycles_to_attotime(20000));

}

static TIMER_CALLBACK( m68307_wd_timer_callback )
{
	printf("wd timer\n");
}

void m68307_timer::init(m68307cpu_device *device)
{
	parent = device;

	m68307_single_timer* tptr;

	tptr = &singletimer[0];
	tptr->mametimer = device->machine().scheduler().timer_alloc(FUNC(m68307_timer0_callback), parent);

	tptr = &singletimer[1];
	tptr->mametimer = device->machine().scheduler().timer_alloc(FUNC(m68307_timer1_callback), parent);


	wd_mametimer = device->machine().scheduler().timer_alloc(FUNC(m68307_wd_timer_callback), parent);


}

UINT16 m68307_timer::read_tcn(UINT16 mem_mask, int which)
{
	// we should return the current timer value by
	// calculating what it should be based on the time
	// since it was last set
	return 0x3a98;
}

void m68307_timer::write_ter(UINT16 data, UINT16 mem_mask, int which)
{
	assert(which >= 0 && which < ARRAY_LENGTH(singletimer));
	m68307_single_timer* tptr = &singletimer[which];
	if (data & 0x2) tptr->regs[m68307TIMER_TMR] &= ~0x2;
}

void m68307_timer::write_tmr(UINT16 data, UINT16 mem_mask, int which)
{
	m68307cpu_device* m68k = parent;
	assert(which >= 0 && which < ARRAY_LENGTH(singletimer));
	m68307_single_timer* tptr = &singletimer[which];

	COMBINE_DATA(&tptr->regs[m68307TIMER_TMR]);

	data = tptr->regs[m68307TIMER_TMR];

	int ps   = data & (0xff00)>>8;
	int ce   = data & (0x00c0)>>6;
	int om   = data & (0x0020)>>5;
	int ori  = data & (0x0010)>>4;
	int frr  = data & (0x0008)>>3;
	int iclk = data & (0x0006)>>1;
	int rst  = data & (0x0001)>>0;


	m68k->logerror("tmr value %04x : Details :\n", data);
	m68k->logerror("prescale %d\n", ps);
	m68k->logerror("(clock divided by %d)\n", ps+1);
	m68k->logerror("capture edge / enable interrupt %d\n", ce);
	if (ce==0x0) m68k->logerror("(disable interrupt on capture event)\n");
	if (ce==0x1) m68k->logerror("(capture on rising edge only + enable capture interrupt)\n");
	if (ce==0x2) m68k->logerror("(capture on falling edge only + enable capture interrupt)\n");
	if (ce==0x3) m68k->logerror("(capture on any edge + enable capture interrupt)\n");
	m68k->logerror("output mode %d\n", om);
	if (om==0x0) m68k->logerror("(active-low pulse for one cycle))\n");
	if (om==0x1) m68k->logerror("(toggle output)\n");
	m68k->logerror("output reference interrupt %d\n", ori);
	if (ori==0x0) m68k->logerror("(disable reference interrupt)\n");
	if (ori==0x1) m68k->logerror("(enable interrupt on reaching reference value))\n");
	m68k->logerror("free running %d\n", frr);
	if (frr==0x0) m68k->logerror("(free running mode, counter continues after value reached)\n");
	if (frr==0x1) m68k->logerror("(restart mode, counter resets after value reached)\n");
	m68k->logerror("interrupt clock source %d\n", iclk);
	if (iclk==0x0) m68k->logerror("(stop count)\n");
	if (iclk==0x1) m68k->logerror("(master system clock)\n");
	if (iclk==0x2) m68k->logerror("(master system clock divided by 16)\n");
	if (iclk==0x3) m68k->logerror("(TIN Pin)\n");
	m68k->logerror("reset %d\n", rst);
	if (rst==0x0) m68k->logerror("(timer is reset)\n");
	if (rst==0x1) m68k->logerror("(timer is running)\n");

	tptr->mametimer->adjust(m68k->cycles_to_attotime(100000));

	m68k->logerror("\n");

}

void m68307_timer::write_trr(UINT16 data, UINT16 mem_mask, int which)
{
	assert(which >= 0 && which < ARRAY_LENGTH(singletimer));
	m68307_single_timer* tptr = &singletimer[which];

	COMBINE_DATA(&tptr->regs[m68307TIMER_TRR]);
}



void m68307_timer::reset(void)
{
	for (int i=0;i<2;i++)
	{
		m68307_single_timer* tptr = &singletimer[i];

		tptr->regs[m68307TIMER_TMR] = 0x0000;
		tptr->regs[m68307TIMER_TRR] = 0xffff;
		tptr->regs[m68307TIMER_TCR] = 0x0000;
		tptr->regs[m68307TIMER_TCN] = 0x0000;
		tptr->regs[m68307TIMER_TER] = 0x0000;
		tptr->regs[m68307TIMER_WRR] = 0xffff;
		tptr->regs[m68307TIMER_WCR] = 0xffff;
		tptr->regs[m68307TIMER_XXX] = 0;
		tptr->enabled = false;
		tptr->mametimer->adjust(attotime::never);
	}

	wd_mametimer->adjust(attotime::never);


}
