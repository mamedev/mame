// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 timers

***************************************************************************/

#include "emu.h"
#include "gt913_timer.h"
#include "h8_cpu_base.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT913_TIMER8,  gt913_timer8_device, "gt913_timer8", "Casio GT913 8-bit timer")
DEFINE_DEVICE_TYPE(GT913_TIMER16, gt913_timer16_device, "gt913_timer16", "Casio GT913 16-bit timer")


gt913_timer_device::gt913_timer_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_intc(*this, finder_base::DUMMY_TAG)
{
}

void gt913_timer_device::device_start()
{
	save_item(NAME(m_tcr));
	save_item(NAME(m_tcor));
	save_item(NAME(m_tcnt));
	save_item(NAME(m_clock_divider));
	save_item(NAME(m_last_clock_update));
	save_item(NAME(m_event_time));
}

void gt913_timer_device::device_reset()
{
	m_tcr = 0;
	m_tcor = 0;
	m_tcnt = 0;
	m_last_clock_update = 0;
	m_event_time = 0;
	update_tcr();
}

u64 gt913_timer_device::internal_update(u64 current_time)
{
	while(m_event_time && current_time >= m_event_time)
	{
		update_counter(m_event_time);
		recalc_event(m_event_time);
	}

	return m_event_time;
}

void gt913_timer_device::notify_standby(int state)
{
	if(!state && m_event_time)
	{
		u64 delta = m_cpu->total_cycles() - m_cpu->standby_time();
		m_event_time += delta;
		m_last_clock_update += delta;
	}
}

void gt913_timer_device::update_counter(u64 cur_time)
{
	if(!cur_time)
		cur_time = m_cpu->total_cycles();

	const u64 base_time = (m_last_clock_update + m_clock_divider/2) / m_clock_divider;
	m_last_clock_update = cur_time;
	const u64 new_time = (cur_time + m_clock_divider/2) / m_clock_divider;
	const u64 delta = new_time - base_time;

	if (!delta)
		return;

	u64 cnt = m_tcnt + delta;
	// TODO: how should a compare value of 0 actually be handled?
	if (m_tcor && cnt >= m_tcor)
	{
		cnt %= m_tcor;
		m_tcr |= 0x10;
		update_irq();
	}
	m_tcnt = cnt;
}

void gt913_timer_device::recalc_event(u64 cur_time)
{
	const bool update_cpu = cur_time == 0;
	const u64 old_event_time = m_event_time;

	if (m_tcnt)
	{
		if(!cur_time)
			cur_time = m_cpu->total_cycles();

		const u64 base_time = (cur_time + m_clock_divider/2) / m_clock_divider;
		const u16 event_delay = m_tcor - m_tcnt;
		m_event_time = ((base_time + event_delay) * m_clock_divider) + m_clock_divider/2;
	}
	else
	{
		m_event_time = 0;
	}

	if(old_event_time != m_event_time && update_cpu)
		m_cpu->internal_update();
}

void gt913_timer_device::tcr_w(u8 data)
{
	update_counter();

	// bit 4 is probably the timer irq flag
	m_tcr = (data & ~0x10) | ((data & m_tcr) & 0x10);
	update_irq();
	update_tcr();
	recalc_event();
}

void gt913_timer_device::update_irq()
{
	if (BIT(m_tcr, 4) && BIT(m_tcr, 3))
		m_intc->internal_interrupt(m_intc_vector);
}

void gt913_timer_device::update_tcr()
{
	/*
	On the CTK-551, this behavior provides the expected rate for timer 0, which is the MIDI PPQN timer.
	For timer 1, this is less certain, but it seems to provide an auto power off delay only a little
	longer than the "about six minutes" mentioned in the user manual.
	*/
	switch (m_tcr & 0x7)
	{
	default:
		logerror("%s: unknown timer divider %u\n", machine().describe_context(), m_tcr & 0x7);
		[[fallthrough]];
	case 0:
		m_clock_divider = (1 << 0);
		break;
	case 2:
		m_clock_divider = (1 << 9);
		break;
	}
}

void gt913_timer8_device::tcor_w(u8 data)
{
	update_counter();
	m_tcor = data;
	recalc_event();
}

u8 gt913_timer8_device::tcnt_r()
{
	if (!machine().side_effects_disabled())
	{
		update_counter();
		recalc_event();
	}
	return m_tcnt;
}

void gt913_timer8_device::tcnt_w(u8 data)
{
	update_counter();
	m_tcnt = data;
	recalc_event();
}

void gt913_timer16_device::tcor_w(offs_t offset, u16 data, u16 mem_mask)
{
	update_counter();
	COMBINE_DATA(&m_tcor);
	recalc_event();
}

u16 gt913_timer16_device::tcnt_r()
{
	if (!machine().side_effects_disabled())
	{
		update_counter();
		recalc_event();
	}
	return m_tcnt;
}

void gt913_timer16_device::tcnt_w(offs_t offset, u16 data, u16 mem_mask)
{
	update_counter();
	COMBINE_DATA(&m_tcnt);
	recalc_event();
}
