// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Shared infrared transmission and carrier reception for XaviX controllers.
    Controllers own their inputs and report contents; encoder bases emit
    receiver events and data levels with the appropriate framing and bit timing.
    The receiver interface collects carrier edges and measures marks and spaces
    using the controller clock; packet framing remains controller-owned.

    TODO:
    - Replace vblank-synchronised bursts with measured controller clocks,
      repetition periods and independently phased transmitters.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ir.h"

#include <bit>

xavix_ir_report_device::xavix_ir_report_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_data(1)
	, m_bit_ticks(0x47 * 64)
	, m_event_out_cb(*this)
	, m_report_bits{ 8, 8, 8, 8, 8, 8, 8, 8 }
	, m_report_count(2)
{
}

void xavix_ir_report_device::device_start()
{
	m_event_out_cb.resolve_safe();
	assert(m_bit_ticks);
	assert(m_report_count && m_report_count <= m_latched_report.size());
	for (unsigned report = 0; report != m_report_count; ++report)
		assert(m_report_bits[report] && m_report_bits[report] <= 32);

	m_frame_timer = timer_alloc(FUNC(xavix_ir_report_device::frame_start), this);
	m_bit_timer = timer_alloc(FUNC(xavix_ir_report_device::bit_tick), this);

	save_item(NAME(m_latched_report));
	save_item(NAME(m_shift));
	save_item(NAME(m_report_index));
	save_item(NAME(m_bits_remaining));
	save_item(NAME(m_data));
	save_item(NAME(m_frame_pending));
	save_item(NAME(m_between_reports));
}

void xavix_ir_report_device::device_reset()
{
	m_frame_timer->adjust(attotime::never);
	m_bit_timer->adjust(attotime::never);
	m_latched_report.fill(NO_SEQUENCE);
	m_shift = 0;
	m_report_index = 0;
	m_bits_remaining = 0;
	m_data = 1;
	m_frame_pending = false;
	m_between_reports = false;
}

void xavix_ir_msb_device::device_start()
{
	xavix_ir_report_device::device_start();
	for (unsigned bit = 0; bit != m_sample_tick_count; ++bit)
		assert(m_sample_ticks[bit]);
}

void xavix_ir_framed_device::device_start()
{
	xavix_ir_report_device::device_start();
	save_item(NAME(m_start_cell));
}

void xavix_ir_framed_device::device_reset()
{
	xavix_ir_report_device::device_reset();
	m_start_cell = false;
}

void xavix_ir_pulse_distance_device::device_start()
{
	xavix_ir_report_device::device_start();
	assert(m_pulse_one_ticks && m_pulse_zero_ticks);
}

void xavix_ir_report_device::vblank_w(int state)
{
	if (state && reports_enabled() && !m_frame_pending && !m_bits_remaining && !m_between_reports)
	{
		for (unsigned report = 0; report != m_report_count; ++report)
			m_latched_report[report] = report_enabled(report) ? read_report(report) : NO_SEQUENCE;

		// Compatibility approximation: avoid receive latency and packets the ROM rejects across its video handler.
		// Physical controllers transmit independently of console vblank; their repetition timing is unmeasured.
		m_frame_pending = true;
		m_frame_timer->adjust(attotime::from_msec(2));
	}
}

TIMER_CALLBACK_MEMBER(xavix_ir_report_device::frame_start)
{
	m_frame_pending = false;
	unsigned const report = next_report(0);
	if (report < m_report_count)
		start_report(report);
}

void xavix_ir_report_device::start_report(unsigned report)
{
	m_report_index = report;
	m_shift = m_latched_report[report];
	m_bits_remaining = m_report_bits[report];
	begin_report();
}

unsigned xavix_ir_report_device::next_report(unsigned report) const
{
	while (report < m_report_count && m_latched_report[report] == NO_SEQUENCE)
		++report;
	return report;
}

void xavix_ir_report_device::finish_report(bool stop_cell, u32 gap_ticks)
{
	m_data = 1;
	if (stop_cell || next_report(m_report_index + 1) < m_report_count)
	{
		m_between_reports = true;
		schedule_bit(gap_ticks ? gap_ticks : m_bit_ticks);
	}
	else
		m_bit_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(xavix_ir_report_device::bit_tick)
{
	if (m_between_reports)
	{
		m_between_reports = false;
		unsigned const report = next_report(m_report_index + 1);
		if (report < m_report_count)
			start_report(report);
		else
			m_bit_timer->adjust(attotime::never);
	}
	else
		advance_report();
}

void xavix_ir_msb_device::begin_report()
{
	m_data = BIT(m_shift, m_bits_remaining - 1);
	receiver_event();
	schedule_bit(current_bit_ticks());
}

u32 xavix_ir_msb_device::current_bit_ticks() const
{
	unsigned const bit = report_bits() - m_bits_remaining;
	return bit < m_sample_tick_count ? m_sample_ticks[bit] : m_bit_ticks;
}

void xavix_ir_msb_device::advance_report()
{
	if (--m_bits_remaining)
	{
		m_data = BIT(m_shift, m_bits_remaining - 1);
		schedule_bit(current_bit_ticks());
	}
	else
		finish_report();
}

void xavix_ir_framed_device::begin_report()
{
	m_start_cell = true;
	m_data = 0;
	receiver_event();
	schedule_bit(m_start_ticks[m_report_index] ? m_start_ticks[m_report_index] : m_bit_ticks);
}

void xavix_ir_framed_device::advance_report()
{
	if (m_start_cell)
	{
		m_start_cell = false;
		m_data = BIT(m_shift, 0);
		schedule_bit(m_bit_ticks);
	}
	else if (--m_bits_remaining)
	{
		m_shift >>= 1;
		m_data = BIT(m_shift, 0);
		schedule_bit(m_bit_ticks);
	}
	else
		finish_report(true);
}

void xavix_ir_pulse_distance_device::set_pulse_distance_ticks(u32 one_ticks, u32 zero_ticks, u32 first_extension)
{
	m_pulse_one_ticks = one_ticks;
	m_pulse_zero_ticks = zero_ticks;
	m_first_pulse_extension = first_extension;
}

u32 xavix_ir_pulse_distance_device::current_pulse_ticks() const
{
	return BIT(m_shift, 0) ? m_pulse_one_ticks : m_pulse_zero_ticks;
}

void xavix_ir_pulse_distance_device::begin_report()
{
	m_data = 1;
	receiver_event();
	schedule_bit(current_pulse_ticks() + m_first_pulse_extension);
}

void xavix_ir_pulse_distance_device::advance_report()
{
	receiver_event();
	if (--m_bits_remaining)
	{
		m_shift >>= 1;
		schedule_bit(current_pulse_ticks());
	}
	else
		finish_report();
}

void xavix_ir_run_length_device::begin_report()
{
	assert(BIT(m_shift, 0));
	m_data = 1;
	receiver_event();
	schedule_bit(current_run_bits() * m_bit_ticks);
}

unsigned xavix_ir_run_length_device::current_run_bits() const
{
	return std::min<unsigned>(m_bits_remaining, 1 + std::countr_zero(m_shift >> 1));
}

void xavix_ir_run_length_device::advance_report()
{
	unsigned const bits = current_run_bits();
	m_shift = bits < 32 ? m_shift >> bits : 0;
	m_bits_remaining -= bits;
	if (m_bits_remaining)
	{
		receiver_event();
		schedule_bit(current_run_bits() * m_bit_ticks);
	}
	else
		finish_report(false, m_report_gap_ticks);
}

xavix_ir_receiver_interface::xavix_ir_receiver_interface(device_t &device, u32 quiet_ticks, bool rising_start)
	: device_interface(device, "xavix_ir_receiver")
	, m_quiet_ticks(quiet_ticks)
	, m_rising_start(rising_start)
	, m_ir_level(false)
{
}

void xavix_ir_receiver_interface::interface_post_start()
{
	assert(device().clock() && m_quiet_ticks);
	m_ir_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(xavix_ir_receiver_interface::quiet_tick), this));
	device().save_item(NAME(m_ir_burst_start));
	device().save_item(NAME(m_ir_last_edge));
	device().save_item(NAME(m_ir_level));
	device().save_item(NAME(m_ir_carrier));
}

void xavix_ir_receiver_interface::ir_receiver_reset(bool reset_level)
{
	m_ir_timer->adjust(attotime::never);
	m_ir_burst_start = attotime::zero;
	m_ir_last_edge = attotime::zero;
	m_ir_carrier = false;
	if (reset_level)
		m_ir_level = false;
}

void xavix_ir_receiver_interface::ir_receiver_wait(u32 ticks)
{
	m_ir_timer->adjust(device().clocks_to_attotime(ticks));
}

void xavix_ir_receiver_interface::ir_receiver_w(int state)
{
	bool const level = bool(state);
	if (level == m_ir_level)
		return;

	m_ir_level = level;
	attotime const now = device().machine().time();
	if (!m_ir_carrier && (level || !m_rising_start))
	{
		m_ir_carrier = true;
		m_ir_burst_start = now;
		ir_burst_started((now - m_ir_last_edge).as_ticks(device().clock()));
	}
	m_ir_last_edge = now;
	// Join carrier cycles without joining successive data marks.
	ir_receiver_wait(m_quiet_ticks);
}

TIMER_CALLBACK_MEMBER(xavix_ir_receiver_interface::quiet_tick)
{
	if (m_ir_carrier)
	{
		m_ir_carrier = false;
		ir_burst_finished((m_ir_last_edge - m_ir_burst_start).as_ticks(device().clock()));
	}
	else
		ir_idle_timeout();
}
