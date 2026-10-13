// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviXPORT panel and sensor receiver interface. Latches a 40-bit report for
    host-clocked reads and provides the receiver-present signal.

    The receiver owns remote mat controls; console-panel inputs stay on the host.

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
	, m_sensors(*this, { "STEP_SENSOR", "STEP_SENSOR2" })
	, m_power(*this, "STEP_POWER")
	, m_hub(*this, "STEP_CONNECTION")
	, m_event_cb(*this)
	, m_report_type(0)
	, m_receiver_present(false)
{
}

INPUT_PORTS_START( xavix_step_sensor )
	PORT_START("STEP_SENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Step Sensor Enter") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Step Sensor Forward / Right") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Step Sensor Back / Left") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Step Sensor Cancel") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STEP_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Step Sensor Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( xavixport_domyos_step )
	PORT_START("STEP_SENSOR")
	// Mat zones are independent switches; PORT_16WAY preserves opposing simultaneous steps.
	// Spatial P1 bindings keep the mat independent of the console panel's arrow keys.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Far Right / Enter") PORT_CODE(KEYCODE_6_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Lower Right / Select / Next") PORT_CODE(KEYCODE_3_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Lower Left / Select / Previous") PORT_CODE(KEYCODE_1_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Far Left / Back / Cancel") PORT_CODE(KEYCODE_4_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// Upper-zone left/right is inferred from the mat artwork and ROM direction mapping.
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Upper Right / Down") PORT_CODE(KEYCODE_9_PAD) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Upper Left / Up") PORT_CODE(KEYCODE_7_PAD) PORT_PLAYER(1)

	PORT_START("STEP_SENSOR2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Far Right / Enter") PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01) PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_NAME("Lower Right / Select / Next") PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_NAME("Lower Left / Select / Previous") PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Far Left / Back / Cancel") PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01) PORT_CODE(KEYCODE_S) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(1))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_NAME("Upper Right / Down") PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	// Avoid sharing the P2 Up default with the console's recessed Reset button.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_16WAY PORT_NAME("Upper Left / Up") PORT_CODE(KEYCODE_E) PORT_PLAYER(2) PORT_CONDITION("STEP_CONNECTION", 0x01, EQUALS, 0x01)

	PORT_START("STEP_CONNECTION")
	PORT_CONFNAME( 0x01, 0x00, "Step Sensor Connection" )
	PORT_CONFSETTING(    0x00, "One Step Sensor (Direct)" )
	PORT_CONFSETTING(    0x01, "Two Step Sensors (Hub)" )
INPUT_PORTS_END

ioport_constructor xavixport_io_device::device_input_ports() const
{
	if (m_report_type == 0x10)
		return INPUT_PORTS_NAME(xavix_step_sensor);
	if (m_report_type == 0x20)
		return INPUT_PORTS_NAME(xavixport_domyos_step);
	return nullptr;
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
	bool const powered = m_power.read_safe(1);
	bool const hub = m_hub.read_safe(0);
	u8 const sensor = (powered && !hub) ? m_sensors[0].read_safe(0) : 0;
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
		m_hub_report[1] = ~m_sensors[0].read_safe(0);
		m_hub_report[2] = ~m_sensors[1].read_safe(0);
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

	if (m_hub.read_safe(0) && !m_host_transmit && BIT(m_hub_report[m_hub_bit / 8], 7 - (m_hub_bit % 8)))
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

	if (m_hub.read_safe(0))
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
