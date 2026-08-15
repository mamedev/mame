// license:BSD-3-Clause
// copyright-holders:smf, hap
/*

Generic clock signal device

Set the period either with device_t m_clock, or with set_period if it needs
to be more fine-tuned (m_clock has higher priority).

The duty cycle can be changed with set_duty_cycle (default is 50%), or the
pulse width (active time) can be set directly with set_pulse_width.

Output signal at machine start is right after falling edge.

*/

#include "emu.h"
#include "clock.h"

DEFINE_DEVICE_TYPE(CLOCK, clock_device, "clock", "Clock")

clock_device::clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CLOCK, tag, owner, clock),
	m_signal(0),
	m_output(-1),
	m_duty(0.5),
	m_period(attotime::never),
	m_pw(attotime::never),
	m_timer_init(nullptr),
	m_timer_tick_low(nullptr),
	m_timer_tick_high(nullptr),
	m_signal_handler(*this)
{
}

void clock_device::device_start()
{
	save_item(NAME(m_signal));
	save_item(NAME(m_output));
	save_item(NAME(m_duty));
	save_item(NAME(m_period));
	save_item(NAME(m_pw));

	m_timer_init = timer_alloc(FUNC(clock_device::clock_init), this);
	m_timer_tick_low  = timer_alloc(FUNC(clock_device::clock_tick<false>), this);
	m_timer_tick_high = timer_alloc(FUNC(clock_device::clock_tick<true>), this);
	reinit();
}

void clock_device::reinit()
{
	if (!m_timer_init)
		return;

	// not using synchronize(), that may retrigger more than once
	m_timer_init->adjust(attotime::zero);
}

void clock_device::output()
{
	if (m_signal != m_output)
	{
		m_output = m_signal;
		m_signal_handler(m_output);
	}
}

TIMER_CALLBACK_MEMBER(clock_device::clock_init)
{
	attotime thigh, tlow;
	attotime period = (m_clock > 0) ? attotime::from_hz(m_clock) : m_period;
	assert(!period.is_zero());

	if (period.is_never())
	{
		m_timer_tick_low->adjust(attotime::never);
		m_timer_tick_high->adjust(attotime::never);
		return;
	}

	if (!m_pw.is_never())
	{
		// set timing via pulse width
		attotime pw = m_pw;
		if (pw > period)
			pw = period;

		thigh = pw;
		tlow = period - pw;
	}
	else
	{
		// set timing via duty cycle
		if (m_duty == 0.5)
		{
			thigh = period / 2;
			tlow = thigh;
		}
		else if (m_duty == 0.0)
		{
			thigh = attotime::zero;
			tlow = period;
		}
		else if (m_duty == 1.0)
		{
			thigh = period;
			tlow = attotime::zero;
		}
		else
		{
			double p = period.as_double();
			thigh = attotime::from_double(m_duty * p);
			tlow = attotime::from_double((1.0 - m_duty) * p);
		}
	}

	attotime tcycle = thigh + tlow;
	if (m_signal)
	{
		m_timer_tick_high->adjust(attotime::zero, 0, tcycle);
		m_timer_tick_low->adjust(thigh, 0, tcycle);
	}
	else
	{
		m_timer_tick_low->adjust(attotime::zero, 0, tcycle);
		m_timer_tick_high->adjust(tlow, 0, tcycle);
	}

	if (tlow.is_zero())
		m_timer_tick_low->adjust(attotime::never);

	if (thigh.is_zero())
		m_timer_tick_high->adjust(attotime::never);
}

template<bool High>
TIMER_CALLBACK_MEMBER(clock_device::clock_tick)
{
	m_signal = High ? 1 : 0;
	output();
}
