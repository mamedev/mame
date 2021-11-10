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
	m_timer_tick(nullptr),
	m_signal_handler(*this)
{
}

void clock_device::device_start()
{
	m_signal_handler.resolve_safe();

	save_item(NAME(m_signal));
	save_item(NAME(m_output));
	save_item(NAME(m_duty));
	save_item(NAME(m_period));
	save_item(NAME(m_pw));
	save_item(NAME(m_thigh));
	save_item(NAME(m_tlow));

	m_timer_init = timer_alloc(TID_CLOCK_INIT);
	m_timer_tick = timer_alloc(TID_CLOCK_TICK);
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

void clock_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_CLOCK_INIT:
		{
			attotime period = (m_clock > 0) ? attotime::from_hz(m_clock) : m_period;
			assert(!period.is_zero());

			if (period.is_never())
			{
				m_timer_tick->adjust(attotime::never);
				return;
			}

			if (!m_pw.is_never())
			{
				// set timing via pulse width
				attotime pw = m_pw;
				if (pw > period)
					pw = period;

				m_thigh = pw;
				m_tlow = period - pw;
			}
			else
			{
				// set timing via duty cycle
				if (m_duty == 0.5)
				{
					m_thigh = period / 2;
					m_tlow = m_thigh;
				}
				else if (m_duty == 0.0)
				{
					m_thigh = attotime::zero;
					m_tlow = period;
				}
				else if (m_duty == 1.0)
				{
					m_thigh = period;
					m_tlow = attotime::zero;
				}
				else
				{
					double p = period.as_double();
					m_thigh = attotime::from_double(m_duty * p);
					m_tlow = attotime::from_double((1.0 - m_duty) * p);
				}
			}

			attotime next = m_signal ? m_thigh : m_tlow;
			if (next < m_timer_tick->remaining())
				m_timer_tick->adjust(next);

			break;
		}

		case TID_CLOCK_TICK:
			if (m_thigh.is_zero())
				m_signal = 0;
			else if (m_tlow.is_zero())
				m_signal = 1;
			else
				m_signal ^= 1;

			m_timer_tick->adjust(m_signal ? m_thigh : m_tlow);
			output();
			break;

		default:
			break;
	}
}
