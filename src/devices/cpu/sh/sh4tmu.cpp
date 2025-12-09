// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* SH3/4 Timer Unit */

#include "emu.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"

static const int tcnt_div[8] = { 4, 16, 64, 256, 1024, 1, 1, 1 };

/*-------------------------------------------------
    sh4_scale_up_mame_time - multiply a attotime by
    a (constant+1) where 0 <= constant < 2^32
-------------------------------------------------*/

static inline attotime sh4_scale_up_mame_time(const attotime &_time1, uint32_t factor1)
{
	return _time1 * factor1 + _time1;
}

static uint32_t compute_ticks_timer(emu_timer *timer, int hertz, int divisor)
{
	double ret;

	ret=((timer->remaining().as_double() * (double)hertz) / (double)divisor) - 1;
	return (uint32_t)ret;
}

void sh34_base_device::sh4_timer_recompute(int which)
{
	double ticks;

	uint32_t tcnt = 0;
	uint32_t tcr = 0;
	switch (which)
	{
		case 0:
			tcr = m_tcr0;
			tcnt = m_tcnt0;
			break;

		case 1:
			tcr = m_tcr1;
			tcnt = m_tcnt1;
			break;

		case 2:
			tcr = m_tcr2;
			tcnt = m_tcnt2;
			break;
	}

	ticks = tcnt;
	m_timer[which]->adjust(sh4_scale_up_mame_time(attotime::from_hz(m_pm_clock) * tcnt_div[tcr & 7], ticks), which);
}


TIMER_CALLBACK_MEMBER( sh34_base_device::sh4_timer_callback )
{
	int which = param;

	switch (which)
	{
		case 0:
			m_tcnt0 = m_tcor0;
			break;

		case 1:
			m_tcnt1 = m_tcor1;
			break;

		case 2:
			m_tcnt2 = m_tcor2;
			break;

	}

	sh4_timer_recompute(which);

	switch (which)
	{
		case 0:
			m_tcr0 |= 0x100;
			break;

		case 1:
			m_tcr1 |= 0x100;
			break;

		case 2:
			m_tcr2 |= 0x100;
			break;

	}

	switch (which)
	{
		case 0:
			if (m_tcr0 & 0x20)
			{
				sh4_exception_request(SH4_INTC_TUNI0);
			//  logerror("SH4_INTC_TUNI0 requested\n");
			}
			break;

		case 1:
			if (m_tcr1 & 0x20)
			{
				sh4_exception_request(SH4_INTC_TUNI1);
			//  logerror("SH4_INTC_TUNI1 requested\n");
			}
			break;

		case 2:
			if (m_tcr2 & 0x20)
			{
				sh4_exception_request(SH4_INTC_TUNI2);
			//  logerror("SH4_INTC_TUNI2 requested\n");
			}
			break;

	}
}

void sh34_base_device::tocr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_tocr);
}

uint8_t sh34_base_device::tocr_r(offs_t offset, uint8_t mem_mask)
{
	return m_tocr;
}

uint8_t sh34_base_device::tstr_r(offs_t offset, uint8_t mem_mask)
{
	return m_tstr;
}

void sh34_base_device::tstr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint32_t chg = m_tstr;
	COMBINE_DATA(&m_tstr);
	chg ^= m_tstr;

	if (chg & 1)
	{
		if ((m_tstr & 1) == 0)
		{
			m_tcnt0 = compute_ticks_timer(m_timer[0], m_pm_clock, tcnt_div[m_tcr0 & 7]);
			m_timer[0]->adjust(attotime::never);
		}
		else
			sh4_timer_recompute(0);
	}

	if (chg & 2)
	{
		if ((m_tstr & 2) == 0)
		{
			m_tcnt1 = compute_ticks_timer(m_timer[1], m_pm_clock, tcnt_div[m_tcr1 & 7]);
			m_timer[1]->adjust(attotime::never);
		}
		else
			sh4_timer_recompute(1);
	}

	if (chg & 4)
	{
		if ((m_tstr & 4) == 0)
		{
			m_tcnt2 = compute_ticks_timer(m_timer[2], m_pm_clock, tcnt_div[m_tcr2 & 7]);
			m_timer[2]->adjust(attotime::never);
		}
		else
			sh4_timer_recompute(2);
	}
}

uint32_t sh34_base_device::tcor0_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcor0;
}

void sh34_base_device::tcor0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcor0);
}

uint32_t sh34_base_device::tcnt0_r(offs_t offset, uint32_t mem_mask)
{
	if (m_tstr & 1)
		return compute_ticks_timer(m_timer[0], m_pm_clock, tcnt_div[m_tcr0 & 7]);
	else
		return m_tcnt0;
}

void sh34_base_device::tcnt0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcnt0);
	if (m_tstr & 1)
		sh4_timer_recompute(0);
}

uint16_t sh34_base_device::tcr0_r(offs_t offset, uint16_t mem_mask)
{
	return m_tcr0;
}

void sh34_base_device::tcr0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t old2 = m_tcr0;
	COMBINE_DATA(&m_tcr0);
	if (m_tstr & 1)
	{
		m_tcnt0 = compute_ticks_timer(m_timer[0], m_pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(0);
	}
	if (!(m_tcr0 & 0x20) || !(m_tcr0 & 0x100))
		sh4_exception_unrequest(SH4_INTC_TUNI0);
}

uint32_t sh34_base_device::tcor1_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcor1;
}

void sh34_base_device::tcor1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcor1);
}

uint32_t sh34_base_device::tcnt1_r(offs_t offset, uint32_t mem_mask)
{
	if (m_tstr & 2)
		return compute_ticks_timer(m_timer[1], m_pm_clock, tcnt_div[m_tcr1 & 7]);
	else
		return m_tcnt1;
}

void sh34_base_device::tcnt1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcnt1);
	if (m_tstr & 2)
		sh4_timer_recompute(1);
}

uint16_t sh34_base_device::tcr1_r(offs_t offset, uint16_t mem_mask)
{
	return m_tcr1;
}

void sh34_base_device::tcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t old2 = m_tcr1;
	COMBINE_DATA(&m_tcr1);
	if (m_tstr & 2)
	{
		m_tcnt1 = compute_ticks_timer(m_timer[1], m_pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(1);
	}
	if (!(m_tcr1 & 0x20) || !(m_tcr1 & 0x100))
		sh4_exception_unrequest(SH4_INTC_TUNI1);
}

uint32_t sh34_base_device::tcor2_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcor2;
}

void sh34_base_device::tcor2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcor2);
}

uint32_t sh34_base_device::tcnt2_r(offs_t offset, uint32_t mem_mask)
{
	if (m_tstr & 4)
		return compute_ticks_timer(m_timer[2], m_pm_clock, tcnt_div[m_tcr2 & 7]);
	else
		return m_tcnt2;
}

void sh34_base_device::tcnt2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcnt2);
	if (m_tstr & 4)
		sh4_timer_recompute(2);
}

uint16_t sh34_base_device::tcr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_tcr2;
}

void sh34_base_device::tcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t old2 = m_tcr2;
	COMBINE_DATA(&m_tcr2);
	if (m_tstr & 4)
	{
		m_tcnt2 = compute_ticks_timer(m_timer[2], m_pm_clock, tcnt_div[old2 & 7]);
		sh4_timer_recompute(2);
	}
	if (!(m_tcr2 & 0x20) || !(m_tcr2 & 0x100))
		sh4_exception_unrequest(SH4_INTC_TUNI2);
}

uint32_t sh34_base_device::tcpr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcpr2;
}

void sh34_base_device::tcpr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcpr2);
}
