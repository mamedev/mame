// license:BSD-3-Clause
// copyright-holders:smf
#include "clock.h"

const device_type CLOCK = &device_creator<clock_device>;

clock_device::clock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CLOCK, "Clock", tag, owner, clock, "clock", __FILE__),
	m_signal(false),
	m_timer(nullptr),
	m_signal_handler(*this)
{
}

void clock_device::device_start()
{
	m_signal_handler.resolve();

	save_item(NAME(m_signal));
}

void clock_device::device_clock_changed()
{
	update_timer();
}

attotime clock_device::period()
{
	if (m_clock > 0)
		return attotime::from_hz(m_clock * 2);

	return attotime::never;
}

void clock_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_signal = !m_signal;
	m_signal_handler(m_signal);
}

void clock_device::update_timer()
{
	if (!m_signal_handler.isnull() && m_clock > 0)
	{
		if (m_timer == nullptr)
		{
			m_timer = timer_alloc(0);
			m_timer->adjust(period(), 0, period());
		}
		else
		{
			attotime next = period() - m_timer->elapsed();

			if (next < attotime::zero)
			{
				next = attotime::zero;
			}

			m_timer->adjust(next, 0, period());
		}
	}
	else if (m_timer != nullptr)
	{
		m_timer->adjust(attotime::never);
	}
}


//-------------------------------------------------
//  hold_w - stop or resume clock
//-------------------------------------------------

WRITE_LINE_MEMBER(clock_device::hold_w)
{
	if (m_timer != nullptr)
		m_timer->enable(!state);
}


//-------------------------------------------------
//  reset_w - reset clock timer and output
//-------------------------------------------------

WRITE_LINE_MEMBER(clock_device::reset_w)
{
	// active high
	if (state)
	{
		// clear output if previously set
		if (m_signal)
		{
			m_signal = false;
			if (!m_signal_handler.isnull())
				m_signal_handler(0);
		}

		// if timer is enabled, start a new period
		if (m_timer != nullptr && m_timer->enabled())
			m_timer->adjust(period(), 0, period());
	}
}
