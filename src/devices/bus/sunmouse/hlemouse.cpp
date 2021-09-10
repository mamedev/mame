// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "hlemouse.h"

#define LOG_GENERAL (1U << 0)
#define LOG_BIT     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_BIT)
//#define LOG_OUTPUT_STREAM std::cerr

#include "logmacro.h"

#define LOGBIT(...) LOGMASKED(LOG_BIT, __VA_ARGS__)


/*
    Mostly compatible with the Mouse Systems "non-rotatable" protocol.
    The only real difference is that the mouse may send a 3-byte frame
    if the movement in each axis doesn't exceed the range -120 to 127
    counts (values in the range -128 to -121 are indistinguishable from
    button states).
*/


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

DEFINE_DEVICE_TYPE(SUN_1200BAUD_HLE_MOUSE, bus::sunmouse::hle_1200baud_device, "sunmouse_hle1200", "Sun Mouse (1200 Baud, HLE)")
DEFINE_DEVICE_TYPE(SUN_4800BAUD_HLE_MOUSE, bus::sunmouse::hle_4800baud_device, "sunmouse_hle4800", "Sun Mouse (4800 Baud, HLE)")

namespace bus::sunmouse {

namespace {

INPUT_PORTS_START( mouse )
	PORT_START("BTN")
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_device_base, input_changed, 0)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_device_base, input_changed, 0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_device_base, input_changed, 0)

	PORT_START("X")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_device_base, input_changed, 0)

	PORT_START("Y")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_device_base, input_changed, 0)
INPUT_PORTS_END


uint8_t extract_delta_byte(int32_t &delta)
{
	int32_t const result(std::clamp<int32_t>(delta, -120, 127));
	delta -= result;
	return uint8_t(int8_t(result));
}

} // anonymous namespace



/***************************************************************************
    BASE HLE MOUSE DEVICE
***************************************************************************/

hle_device_base::hle_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock,
		unsigned multiplier)
	: device_t(mconfig, type, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, device_sun_mouse_port_interface(mconfig, *this)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
	, m_multiplier(multiplier)
	, m_x_delta(0)
	, m_y_delta(0)
	, m_x_val(0U)
	, m_y_val(0U)
	, m_btn_sent(0x00U)
	, m_btn_val(0x00U)
	, m_phase(0U)
{
}


hle_device_base::~hle_device_base()
{
}


INPUT_CHANGED_MEMBER( hle_device_base::input_changed )
{
	if (is_transmit_register_empty())
	{
		assert((0U == m_phase) || (3U == m_phase));

		check_inputs();
		if (0U == m_phase)
		{
			if (m_x_delta || m_y_delta || (m_btn_sent != m_btn_val))
			{
				LOG("Inputs changed (B=%X->%x X=%d Y=%d) - sending button state\n",
						m_btn_sent,
						m_btn_val,
						m_x_delta,
						m_y_delta);
				transmit_register_setup((m_btn_sent = m_btn_val) | 0x80U);
			}
		}
		else if (m_x_delta || m_y_delta)
		{
			LOG("Inputs changed (B=%X->%x X=%d Y=%d) - sending X delta\n",
					m_btn_sent,
					m_btn_val,
					m_x_delta,
					m_y_delta);
			transmit_register_setup(extract_delta_byte(m_x_delta));
		}
		else if (m_btn_sent != m_btn_val)
		{
			LOG("Inputs changed (B=%X->%x X=%d Y=%d) - sending button state\n",
					m_btn_sent,
					m_btn_val,
					m_x_delta,
					m_y_delta);
			m_phase = 0U;
			transmit_register_setup((m_btn_sent = m_btn_val) | 0x80U);
		}
	}
	else
	{
		LOG("Ignoring input changed while transmit register not empty\n");
	}
}


ioport_constructor hle_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME(mouse);
}


void hle_device_base::device_start()
{
	save_item(NAME(m_x_delta));
	save_item(NAME(m_y_delta));
	save_item(NAME(m_x_val));
	save_item(NAME(m_y_val));
	save_item(NAME(m_phase));

	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD * m_multiplier);
	receive_register_reset();
	transmit_register_reset();

	output_rxd(0);

	m_x_delta = 0;
	m_y_delta = 0;
	m_btn_sent = 0x00U;
	m_phase = 0U;
}


void hle_device_base::device_reset()
{
	m_x_val = m_x_axis->read();
	m_y_val = m_y_axis->read();
	m_btn_val = m_buttons->read();
}


void hle_device_base::tra_callback()
{
	int const bit(transmit_register_get_data_bit() ? 0 : 1);
	LOGBIT("Send bit !%d\n", bit);
	output_rxd(bit);
}


void hle_device_base::tra_complete()
{
	check_inputs();

	if (4U <= m_phase)
		m_phase = 0U;
	else
		++m_phase;

	switch (m_phase)
	{
	// sent second Y delta - send button state if anything changed
	case 0U:
		if (m_x_delta || m_y_delta || (m_btn_sent != m_btn_val))
		{
			LOG("Sent Y delta (B=%X->%x X=%d Y=%d) - sending button state\n",
					m_btn_sent,
					m_btn_val,
					m_x_delta,
					m_y_delta);
			transmit_register_setup((m_btn_sent = m_btn_val) | 0x80U);
		}
		else
		{
			LOG("Sent Y delta (B=%X->%x X=%d Y=%d) - nothing to send\n",
					m_btn_sent,
					m_btn_val,
					m_x_delta,
					m_y_delta);
		}
		break;

	// can wait after sending first Y delta
	case 3U:
		if (!m_x_delta && !m_y_delta && (m_btn_sent == m_btn_val))
		{
			LOG("Sent Y delta (B=%X->%x X=%d Y=%d) - nothing to send\n",
					m_btn_sent,
					m_btn_val,
					m_x_delta,
					m_y_delta);
			//break; uncommenting this causes problems with early versions of Solaris
		}
		[[fallthrough]];
	case 1U:
		LOG("Sent %s (B=%X->%x X=%d Y=%d) - sending X delta\n",
				(1U == m_phase) ? "button state" : "Y delta",
				m_btn_sent,
				m_btn_val,
				m_x_delta,
				m_y_delta);
		transmit_register_setup(extract_delta_byte(m_x_delta));
		break;

	// sent X delta - always follow with Y delta
	case 2U:
	case 4U:
		LOG("Sent X delta (B=%X->%x X=%d Y=%d) - sending Y delta\n",
				m_btn_sent,
				m_btn_val,
				m_x_delta,
				m_y_delta);
		transmit_register_setup(extract_delta_byte(m_y_delta));
		break;
	}
}


void hle_device_base::check_inputs()
{
	// read new axis values and get difference
	uint16_t const x_val(m_x_axis->read());
	uint16_t const y_val(m_y_axis->read());
	int16_t x_delta(x_val - m_x_val);
	int16_t y_delta(y_val - m_y_val);

	// deal with wraparound
	if (0x0800 <= x_delta)
		x_delta -= 0x1000;
	else if (-0x0800 >= x_delta)
		x_delta += 0x1000;
	if (0x0800 <= y_delta)
		y_delta -= 0x1000;
	else if (-0x0800 >= y_delta)
		x_delta += 0x1000;

	// update state
	m_x_delta += x_delta;
	m_y_delta -= y_delta;
	m_x_val = x_val;
	m_y_val = y_val;

	// read new button values
	m_btn_val = m_buttons->read();
}



/***************************************************************************
    1200 BAUD HLE MOUSE DEVICE
***************************************************************************/

hle_1200baud_device::hle_1200baud_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_device_base(mconfig, SUN_1200BAUD_HLE_MOUSE, tag, owner, clock, 1U)
{
}



/***************************************************************************
    4800 BAUD HLE MOUSE DEVICE
***************************************************************************/

hle_4800baud_device::hle_4800baud_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_device_base(mconfig, SUN_4800BAUD_HLE_MOUSE, tag, owner, clock, 4U)
{
}

} // namespace bus::sunmouse
