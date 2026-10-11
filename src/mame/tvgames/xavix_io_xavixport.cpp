// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviXPORT panel and sensor receiver interface. Latches a 40-bit report for
    host-clocked reads and provides the receiver-present signal.

    The optional Domyos two-mat hub exchanges discovery and active-low mat
    packets through a separate command/report handshake.

    TODO:
    - Measure the physical hub response latency.

***************************************************************************/

#include "emu.h"
#include "xavix_io_xavixport.h"

#include <bit>

DEFINE_DEVICE_TYPE(XAVIXPORT_IO, xavixport_io_device, "xavixport_io", "XaviXPORT I/O Interface")

xavixport_io_device::xavixport_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIXPORT_IO, tag, owner, clock)
	, m_panel_cb(*this, 0)
	, m_sensor_cb(*this, 0)
	, m_power_cb(*this, 1)
	, m_hub_cb(*this, 0)
	, m_event_cb(*this)
	, m_report_type(0)
	, m_receiver_present(false)
{
}

void xavixport_io_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(xavixport_io_device::event_timer), this);

	save_item(NAME(m_report));
	save_item(NAME(m_hub_report));
	save_item(NAME(m_hub_command));
	save_item(NAME(m_bit));
	save_item(NAME(m_hub_bit));
	save_item(NAME(m_hub_command_bits));
	save_item(NAME(m_hub_report_bytes));
	save_item(NAME(m_selected));
	save_item(NAME(m_clock));
	save_item(NAME(m_request));
	save_item(NAME(m_request_output));
	save_item(NAME(m_hub_clock));
	save_item(NAME(m_host_transmit));
	save_item(NAME(m_hub_active));
	save_item(NAME(m_hub_transfer));
}

void xavixport_io_device::device_reset()
{
	m_report.fill(0);
	m_hub_report.fill(0);
	m_hub_command.fill(0);
	m_bit = 0;
	m_hub_bit = 0;
	m_hub_command_bits = 0;
	m_hub_report_bytes = 0;
	m_selected = false;
	m_clock = false;
	m_request = false;
	m_request_output = false;
	m_hub_clock = false;
	m_host_transmit = true;
	m_hub_active = false;
	m_hub_transfer = false;
	m_event_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(xavixport_io_device::event_timer)
{
	m_hub_transfer = true;
	m_event_cb(1);
}

void xavixport_io_device::latch_report()
{
	bool const powered = m_power_cb();
	bool const hub = m_hub_cb();
	u8 const sensor = (powered && !hub) ? m_sensor_cb[0]() : 0;
	m_report[0] = m_panel_cb() & 0xfc;
	m_report[1] = (powered && !hub ? m_report_type : 0) | (sensor & 0x0f);
	m_report[2] = sensor & 0xf0;
	m_report[3] = 0;
	m_report[4] = 0;
	m_bit = 0;
}

void xavixport_io_device::latch_hub_report()
{
	// The optional Domyos two-mat hub returns discovery or active-low mat packets.
	m_hub_report.fill(0);

	if (m_hub_command[0] == 0xc1)
	{
		// The supported pair occupies the hub's first two numbered sockets.
		m_hub_report[1] = 0xd0;
		m_hub_report[2] = 0xd0;
		m_hub_report_bytes = 5;
	}
	else if (m_hub_command[0] == 0x82)
	{
		m_hub_report[1] = ~m_sensor_cb[0]();
		m_hub_report[2] = ~m_sensor_cb[1]();
		m_hub_report_bytes = 3;
	}
	else
	{
		m_hub_report_bytes = 1;
	}

	// Header bits 6-5 contain the population count of bit 7 and the payload modulo four.
	unsigned ones = 1;
	for (unsigned index = 1; index < m_hub_report_bytes; index++)
		ones += std::popcount(m_hub_report[index]);
	m_hub_report[0] = 0x80 | ((ones & 3) << 5);
	m_hub_bit = 0;
}

void xavixport_io_device::hub_command_bit_w(int state)
{
	if (m_hub_command_bits < (8 * m_hub_command.size()))
	{
		u8 &byte = m_hub_command[m_hub_command_bits / 8];
		byte = (byte << 1) | state;
		m_hub_command_bits++;
	}
}

u8 xavixport_io_device::pio_r(offs_t, u8)
{
	u8 result = m_receiver_present ? 0x80 : 0x00;

	// The receiver presents the complemented report bit before each host-generated clock pulse.
	if (m_selected && !BIT(m_report[m_bit / 8], 7 - (m_bit % 8)))
		result |= 0x20;

	if (m_hub_cb() && !m_host_transmit && BIT(m_hub_report[m_hub_bit / 8], 7 - (m_hub_bit % 8)))
		result |= 0x04;

	return result;
}

void xavixport_io_device::pio_w(offs_t, u8 data, u8 direction)
{
	u8 const output = data & direction;
	bool const selected = BIT(output, 4);
	bool const clock = BIT(output, 3);
	bool const request = BIT(output, 1);
	bool const request_output = BIT(direction, 1);
	bool const hub_clock = BIT(output, 4);

	if (selected && !m_selected)
		latch_report();
	else if (selected && m_selected && m_clock && !clock)
		m_bit = (m_bit + 1) % 40;

	if (m_hub_cb())
	{
		if (request_output && m_request_output && m_request && !request)
		{
			m_hub_active = true;
			m_hub_transfer = false;
			if (m_host_transmit)
			{
				m_hub_command.fill(0);
				m_hub_command_bits = 0;
			}
			else
			{
				latch_hub_report();
			}

			// A short response delay lets the host arm its timeout; physical hub latency is unknown.
			m_event_timer->adjust(attotime::from_usec(10));
		}
		else if (request_output && m_request_output && !m_request && request && m_hub_active)
		{
			m_host_transmit = !m_host_transmit;
			m_hub_active = false;
			m_hub_transfer = false;
		}

		if (request_output && !request && m_hub_transfer)
		{
			if (m_host_transmit && !hub_clock)
				hub_command_bit_w(BIT(output, 3));
			else if (!m_host_transmit && !m_hub_clock && hub_clock && m_hub_bit < (8 * m_hub_report_bytes - 1))
				m_hub_bit++;
		}
	}

	m_selected = selected;
	m_clock = clock;
	m_request = request;
	m_request_output = request_output;
	m_hub_clock = hub_clock;
}
