// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Shared infrared report generator for XaviX controllers. Latches input
    reports and emits timed receiver events and data levels using framed
    serial, sampled data or pulse-distance formats, with configurable report
    lengths, framing and bit timing.

    TODO:
    - Verify physical transmitter timing and repetition for the
      configured controllers.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ir.h"

DEFINE_DEVICE_TYPE(XAVIX_IR_REPORT, xavix_ir_report_device, "xavix_ir_report", "XaviX Infrared Controller Reports")

xavix_ir_report_device::xavix_ir_report_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_IR_REPORT, tag, owner, clock)
	, m_report_cb(*this, 0xffff)
	, m_report_enable_cb(*this, 1)
	, m_sequence_cb(*this, NO_SEQUENCE)
	, m_enable_cb(*this, 1)
	, m_event_out_cb(*this)
	, m_report_bits{ 8, 8, 8, 8, 8, 8, 8, 8 }
	, m_sample_ticks{}
	, m_data(1)
	, m_bit_ticks(0x47 * 64)
	, m_pulse_one_ticks(0)
	, m_pulse_zero_ticks(0)
	, m_first_pulse_extension(0)
	, m_start_ticks{}
	, m_first_data_ticks{}
	, m_report_count(2)
	, m_sample_tick_count(0)
	, m_mode(mode::MSB_FIRST)
{
}

void xavix_ir_report_device::device_start()
{
	assert(m_bit_ticks);
	assert(m_mode != mode::PULSE_DISTANCE || (m_pulse_one_ticks && m_pulse_zero_ticks));
	assert(m_report_count && m_report_count <= m_latched_report.size());
	for (unsigned report = 0; report != m_report_count; ++report)
		assert(m_report_bits[report] && m_report_bits[report] <= (m_mode == mode::SAMPLE_SEQUENCE ? 32 : 16));
	for (unsigned bit = 0; bit != m_sample_tick_count; ++bit)
		assert(m_sample_ticks[bit]);

	m_frame_timer = timer_alloc(FUNC(xavix_ir_report_device::frame_start), this);
	m_bit_timer = timer_alloc(FUNC(xavix_ir_report_device::bit_tick), this);

	save_item(NAME(m_latched_report));
	save_item(NAME(m_shift));
	save_item(NAME(m_report_index));
	save_item(NAME(m_bits_remaining));
	save_item(NAME(m_data));
	save_item(NAME(m_frame_pending));
	save_item(NAME(m_between_reports));
	save_item(NAME(m_start_cell));
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
	m_start_cell = false;
}

u8 xavix_ir_report_device::motion_report(unsigned player, bool button, u8 strength)
{
	assert(player < 2);
	return MOTION_REPORTS[player][button][strength & 3];
}

u16 xavix_ir_report_device::baseball_bat_report(bool bunt, bool swing)
{
	// Accepted state 3 has no known physical meaning; a swing temporarily takes priority over a held bunt button.
	u8 const state = swing ? 2 : bunt;
	return 1 | (u16(BASEBALL_BAT_PAYLOADS[state]) << 1);
}

void xavix_ir_report_device::vblank_w(int state)
{
	if (state && m_enable_cb() && !m_frame_pending && !m_bits_remaining && !m_between_reports)
	{
		for (unsigned report = 0; report != m_report_count; ++report)
			m_latched_report[report] = m_report_enable_cb[report]()
				? (m_mode == mode::SAMPLE_SEQUENCE ? m_sequence_cb[report]() : m_report_cb[report]())
				: NO_SEQUENCE;

		// Defer the burst until the video interrupt has returned, avoiding serial-edge latency at vblank.
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
	m_start_cell = m_mode == mode::FRAMED_LSB;
	if (m_mode == mode::PULSE_DISTANCE)
		m_data = 1;
	else
		m_data = m_mode == mode::FRAMED_LSB ? 0 : BIT(m_shift, m_bits_remaining - 1);

	// The first event marks framing; pulse-distance reports encode every bit before the next event.
	m_event_out_cb(1);
	if (m_mode == mode::PULSE_DISTANCE)
		m_bit_timer->adjust(clocks_to_attotime(current_pulse_ticks() + m_first_pulse_extension));
	else
	{
		u32 ticks = current_bit_ticks();
		if (m_start_cell && m_start_ticks[report])
			ticks = m_start_ticks[report];
		else if (!m_start_cell && m_first_data_ticks[report])
			ticks = m_first_data_ticks[report];
		m_bit_timer->adjust(clocks_to_attotime(ticks));
	}
}

unsigned xavix_ir_report_device::next_report(unsigned report) const
{
	while (report < m_report_count && m_latched_report[report] == NO_SEQUENCE)
		++report;
	return report;
}

u32 xavix_ir_report_device::current_bit_ticks() const
{
	if (m_mode == mode::SAMPLE_SEQUENCE)
	{
		unsigned const bit = m_report_bits[m_report_index] - m_bits_remaining;
		if (bit < m_sample_tick_count)
			return m_sample_ticks[bit];
	}

	return m_bit_ticks;
}

u32 xavix_ir_report_device::current_pulse_ticks() const
{
	return BIT(m_shift, 0) ? m_pulse_one_ticks : m_pulse_zero_ticks;
}

TIMER_CALLBACK_MEMBER(xavix_ir_report_device::bit_tick)
{
	if (m_mode == mode::PULSE_DISTANCE)
	{
		m_event_out_cb(1);
		if (--m_bits_remaining)
		{
			m_shift >>= 1;
			m_bit_timer->adjust(clocks_to_attotime(current_pulse_ticks()));
		}
		else
		{
			m_bit_timer->adjust(attotime::never);
		}
	}
	else if (m_between_reports)
	{
		m_between_reports = false;
		unsigned const report = next_report(m_report_index + 1);
		if (report < m_report_count)
			start_report(report);
		else
			m_bit_timer->adjust(attotime::never);
	}
	else if (m_start_cell)
	{
		m_start_cell = false;
		m_data = BIT(m_shift, 0);
		m_bit_timer->adjust(clocks_to_attotime(m_first_data_ticks[m_report_index] ? m_first_data_ticks[m_report_index] : current_bit_ticks()));
	}
	else if (--m_bits_remaining)
	{
		if (m_mode == mode::FRAMED_LSB)
		{
			m_shift >>= 1;
			m_data = BIT(m_shift, 0);
		}
		else
		{
			m_data = BIT(m_shift, m_bits_remaining - 1);
		}
		m_bit_timer->adjust(clocks_to_attotime(current_bit_ticks()));
	}
	else if ((next_report(m_report_index + 1) < m_report_count) || m_mode == mode::FRAMED_LSB)
	{
		m_data = 1;
		m_between_reports = true;
		m_bit_timer->adjust(clocks_to_attotime(m_bit_ticks));
	}
	else
	{
		m_data = 1;
		m_bit_timer->adjust(attotime::never);
	}
}
