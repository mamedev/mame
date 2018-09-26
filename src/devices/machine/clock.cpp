// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "clock.h"

DEFINE_DEVICE_TYPE(CLOCK, clock_device, "clock", "Clock")

clock_device::clock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CLOCK, tag, owner, clock),
	m_signal(0),
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
	if (!m_signal_handler.isnull() && m_clock > 0)
	{
		if (m_timer == nullptr)
			m_timer = timer_alloc(0);

		const attotime period(attotime::from_hz(m_clock * 2));

		attotime next = period - m_timer->elapsed();
		if (next < attotime::zero)
			next = attotime::zero;

		m_timer->adjust(next, 0, period);
	}
	else if (m_timer != nullptr)
		m_timer->adjust(attotime::never);
}

void clock_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_signal = !m_signal;
	m_signal_handler(m_signal);
}
