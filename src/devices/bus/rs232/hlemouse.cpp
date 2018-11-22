// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**************************************************************************

    PC Serial Mouse Simulation

    Microsoft mouse is the classic two-button PC mouse.  Data is 7N1 at
    1,200 Baud.  Mouse can be reset by de-asserting RTS.  On start, the
    mouse identifies itself with the string "M".  Bit 6 is only set for
    the first byte of a report - it's clear for all subsequent bytes of
    a report, and also for all identification bytes.

    Reports consist of three bytes: the first byte contains the button
    state and the two high bits of the Y and X delta; the second byte
    contains the low six bits of the X delta; the third byte contains
    the low six bits of the Y delta.  Button bits are set when pressed,
    and positive delta is rightwards/downwards.

    1lryyxx 0xxxxxx 0yyyyyy

    Logitech extended the Microsoft protocol to support a third button.
    The mouse identifies itself with the string "M3".  If the third
    button changes state, and additional byte is sent indicating the
    new state of the third button.

    1lryyxx 0xxxxxx 0yyyyyy 0m00000

    The serial wheel mouse protocol extends the Microsoft mouse protocol
    in a different way.  The mouse identifies itself with the string
    "MZ@\0\0\0".  If the third button or wheel state changes, a fourth
    an additional byte is sent containing the third button state and a
    four-bit wheel delta value.  The wheel value is positive for
    downward movement.  Note that the third button is value is in a
    different position to the logitech protocol.

    1lryyxx 0xxxxxx 0yyyyyy 00mwwww

    The Mouse Systems non-rotatable protocol provides two-axis movement
    and three buttons.  Data is 8N1 at 1,200 Baud (the M-1 mouse could
    also be configured for 300 Baud by turning DIP switch 1 off).  The
    mouse does not send an identification string.  The first byte of a
    report can be identified by a fixed pattern in the five most
    significant bits.

    Reports are five bytes long.  The first byte contains the the button
    state; the second and fourth bytes contain X delta; the third and
    fifth bytes contain Y delta.  The two delta values for each axis
    should be summed.  Delta values range from -120 to 127 to prevent
    being mistaken for the lead byte of a report.  Button bits are clear
    when set, and positive delta is rightwards/upwards.  Delta values
    are generated immediately before transmission - reports are not
    atomic.

    10000lmr xxxxxxxx yyyyyyyy xxxxxxxx yyyyyyyy

    The Mouse systems rotatable protcol allows the host to infer
    rotation around the third axis at the cost of halving the maximum
    sustained movement speed.  The M-1 mouse has two sensors spaced 100
    counts apart horizontally.  If DIP switch 2 is on, the X and Y delta
    for each sensor is reported separately.  The right sensor delta is
    reported before the left sensor delta.  If the mouse is rotated, the
    delta values for the two sensors will differ.

**************************************************************************/

#include "emu.h"
#include "hlemouse.h"

#include <cassert>
#include <cmath>


//**************************************************
//  Device type globals
//**************************************************

DEFINE_DEVICE_TYPE_NS(MSFT_HLE_SERIAL_MOUSE,      bus::rs232, hle_msft_mouse_device,      "rs232_mouse_hle_msft",      "Microsoft 2-Button Serial Mouse (HLE)")
DEFINE_DEVICE_TYPE_NS(LOGITECH_HLE_SERIAL_MOUSE,  bus::rs232, hle_logitech_mouse_device,  "rs232_mouse_hle_logitech",  "Logitech 3-Button Serial Mouse (HLE)")
DEFINE_DEVICE_TYPE_NS(WHEEL_HLE_SERIAL_MOUSE,     bus::rs232, hle_wheel_mouse_device,     "rs232_mouse_hle_wheel",     "Microsoft Serial Mouse with Wheel (HLE)")
DEFINE_DEVICE_TYPE_NS(MSYSTEMS_HLE_SERIAL_MOUSE,  bus::rs232, hle_msystems_mouse_device,  "rs232_mouse_hle_msystems",  "Mouse Systems Non-rotatable Mouse (HLE)")
DEFINE_DEVICE_TYPE_NS(ROTATABLE_HLE_SERIAL_MOUSE, bus::rs232, hle_rotatable_mouse_device, "rs232_mouse_hle_rotatable", "Mouse Systems Rotatable Mouse (HLE)")
DEFINE_DEVICE_TYPE_NS(SGI_HLE_SERIAL_MOUSE,       bus::rs232, hle_sgi_mouse_device,       "rs232_mouse_hle_sgi",       "SGI IRIS Indigo Mouse (HLE)")

namespace bus { namespace rs232 {

namespace {

INPUT_PORTS_START(msft)
	PORT_START("BTN")
	PORT_BIT( 0xfffc, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)

	PORT_START("X")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)

	PORT_START("Y")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)
INPUT_PORTS_END


INPUT_PORTS_START(logitech)
	PORT_INCLUDE(msft)

	PORT_MODIFY("BTN")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)
INPUT_PORTS_END


INPUT_PORTS_START(wheel)
	PORT_INCLUDE(logitech)

	PORT_START("WHEEL")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_DIAL_V ) PORT_SENSITIVITY(10) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msmouse_device_base, input_changed, nullptr)
INPUT_PORTS_END


INPUT_PORTS_START(msystems)
	PORT_START("BTN")
	PORT_BIT( 0xfff8, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(MOUSECODE_BUTTON3) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(MOUSECODE_BUTTON2) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)

	PORT_START("X")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)

	PORT_START("Y")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)
INPUT_PORTS_END


INPUT_PORTS_START(rotatable)
	PORT_INCLUDE(msystems)

	PORT_START("ROT")
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0fff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(10) PORT_CHANGED_MEMBER(DEVICE_SELF, hle_msystems_device_base, input_changed, nullptr)
INPUT_PORTS_END


template <unsigned Bits, typename T>
std::make_signed_t<T> read_axis(ioport_port &port, T &val)
{
	static_assert((1U < Bits) && ((sizeof(T) * 8) > Bits), "invalid field bits");

	T const updated(T(std::make_unsigned_t<T>(port.read())) & make_bitmask<T>(Bits));
	std::make_signed_t<T> delta(std::make_signed_t<T>(updated) - std::make_signed_t<T>(val));
	if (std::make_signed_t<T>(std::make_unsigned_t<T>(1) << (Bits - 1)) <= delta)
		delta -= std::make_signed_t<T>(std::make_unsigned_t<T>(1) << Bits);
	else if (-std::make_signed_t<T>(std::make_unsigned_t<T>(1) << (Bits - 1)) >= delta)
		delta += std::make_signed_t<T>(std::make_unsigned_t<T>(1) << Bits);
	val = updated;
	return delta;
}


template <typename T>
uint8_t report_axis(T &delta, T low, T high)
{
	T const result(std::min<T>(std::max<T>(delta, low), high));
	delta -= result;
	return uint8_t(int8_t(result));
}

} // anonymous namespace


//**************************************************
// Microsoft mouse base
//**************************************************

INPUT_CHANGED_MEMBER(hle_msmouse_device_base::input_changed)
{
	if (fifo_empty() && is_transmit_register_empty())
		check_inputs();
}

hle_msmouse_device_base::hle_msmouse_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: buffered_rs232_device<8>(mconfig, type, tag, owner, clock)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
	, m_x_delta(0)
	, m_y_delta(0)
	, m_x_val(0U)
	, m_y_val(0U)
	, m_btn_val(0x00U)
	, m_btn_sent(0x00U)
	, m_dtr(0U)
	, m_rts(0U)
	, m_enable(0U)
{
}

void hle_msmouse_device_base::device_resolve_objects()
{
	buffered_rs232_device<8>::device_resolve_objects();

	m_dtr = 0U;
	m_rts = 0U;
}

void hle_msmouse_device_base::device_start()
{
	buffered_rs232_device<8>::device_start();

	save_item(NAME(m_x_delta));
	save_item(NAME(m_y_delta));
	save_item(NAME(m_x_val));
	save_item(NAME(m_y_val));
	save_item(NAME(m_btn_val));
	save_item(NAME(m_btn_sent));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_enable));

	set_data_frame(1, 7, PARITY_NONE, STOP_BITS_2);
	set_rate(1'200);
	receive_register_reset();
	transmit_register_reset();

	m_x_delta = m_y_delta = 0;
	m_x_val = m_y_val = 0U;
	m_btn_val = m_btn_sent = 0x00U;
	m_enable = 0U;

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(hle_msmouse_device_base::start_mouse), this));
}

WRITE_LINE_MEMBER(hle_msmouse_device_base::input_dtr)
{
	m_dtr = state ? 1U : 0U;
	check_enable();
}

WRITE_LINE_MEMBER(hle_msmouse_device_base::input_rts)
{
	m_dtr = state ? 1U : 0U;
	check_enable();
}

void hle_msmouse_device_base::tra_complete()
{
	buffered_rs232_device<8>::tra_complete();
	if (fifo_empty() && is_transmit_register_empty())
		check_inputs();
}

void hle_msmouse_device_base::received_byte(u8 byte)
{
}


TIMER_CALLBACK_MEMBER(hle_msmouse_device_base::start_mouse)
{
	check_enable();
}

void hle_msmouse_device_base::check_enable()
{
	bool const enable(!m_dtr && !m_rts);
	if (bool(m_enable) != enable)
	{
		m_enable = enable ? 1U : 0U;
		clear_fifo();
		receive_register_reset();
		transmit_register_reset();
		if (enable)
		{
			read_inputs();
			m_x_delta = m_y_delta = 0;
			m_btn_sent = m_btn_val;
			reset_and_identify();
		}
	}
}

void hle_msmouse_device_base::check_inputs()
{
	if (m_enable && read_inputs())
	{
		uint8_t const x(report_axis<int16_t>(m_x_delta, -128, 127));
		uint8_t const y(report_axis<int16_t>(m_y_delta, -128, 127));
		transmit_byte(0x40U | ((m_btn_val << 4) & 0x30U) | ((y >> 4) & 0x0cU) | ((x >> 6) & 0x03U));
		transmit_byte(x & 0x3fU);
		transmit_byte(y & 0x3fU);
		transmit_extensions(m_btn_val, m_btn_sent);
		m_btn_sent = m_btn_val;
	}
}

bool hle_msmouse_device_base::read_inputs()
{
	m_x_delta += read_axis<12>(*m_x_axis, m_x_val);
	m_y_delta += read_axis<12>(*m_y_axis, m_y_val);
	m_btn_val = m_buttons->read();
	return m_x_delta || m_y_delta || (m_btn_val != m_btn_sent);
}


//**************************************************
// Microsoft 2-button mouse
//**************************************************

hle_msft_mouse_device::hle_msft_mouse_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msmouse_device_base(mconfig, MSFT_HLE_SERIAL_MOUSE, tag, owner, clock)
{
}

ioport_constructor hle_msft_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msft);
}

void hle_msft_mouse_device::reset_and_identify()
{
	// assume ASCII host system
	transmit_byte('M');
}

void hle_msft_mouse_device::transmit_extensions(uint8_t btn_val, uint8_t btn_sent)
{
}


//**************************************************
//  Logitech 3-button mouse
//**************************************************

hle_logitech_mouse_device::hle_logitech_mouse_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: hle_msmouse_device_base(mconfig, LOGITECH_HLE_SERIAL_MOUSE, tag, owner, clock)
{
}

ioport_constructor hle_logitech_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(logitech);
}

void hle_logitech_mouse_device::reset_and_identify()
{
	// assume ASCII host system
	transmit_byte('M');
	transmit_byte('3');
}

void hle_logitech_mouse_device::transmit_extensions(uint8_t btn_val, uint8_t btn_sent)
{
	if (BIT(btn_val | btn_sent, 2))
		transmit_byte(BIT(btn_val, 2) << 5);
}



//**************************************************
// Microsoft wheel mouse
//**************************************************

hle_wheel_mouse_device::hle_wheel_mouse_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msmouse_device_base(mconfig, WHEEL_HLE_SERIAL_MOUSE, tag, owner, clock)
	, m_wheel(*this, "WHEEL")
	, m_wheel_delta(0)
	, m_wheel_val(0U)
{
}

ioport_constructor hle_wheel_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(wheel);
}

void hle_wheel_mouse_device::device_start()
{
	hle_msmouse_device_base::device_start();

	save_item(NAME(m_wheel_delta));
	save_item(NAME(m_wheel_val));

	m_wheel_delta = 0;
	m_wheel_val = 0U;
}

bool hle_wheel_mouse_device::read_inputs()
{
	m_wheel_delta += read_axis<12>(*m_wheel, m_wheel_val);
	return hle_msmouse_device_base::read_inputs() || m_wheel_delta;
}

void hle_wheel_mouse_device::reset_and_identify()
{
	m_wheel_delta = 0;

	// assume ASCII host system
	transmit_byte('M');
	transmit_byte('Z');
	transmit_byte('@');
	transmit_byte('\0');
	transmit_byte('\0');
	transmit_byte('\0');
}

void hle_wheel_mouse_device::transmit_extensions(uint8_t btn_val, uint8_t btn_sent)
{
	if (BIT(btn_val | btn_sent, 2) || m_wheel_delta)
		transmit_byte((BIT(btn_val, 2) << 4) | (report_axis<int16_t>(m_wheel_delta, -8, 7) & 0x0fU));
}


//**************************************************
//  Mouse Systems mouse base
//**************************************************

INPUT_CHANGED_MEMBER(hle_msystems_device_base::input_changed)
{
	if (is_transmit_register_empty())
	{
		assert(0U == m_phase);
		tra_complete();
	}
}

hle_msystems_device_base::hle_msystems_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rs232_port_interface(mconfig, *this)
	, device_serial_interface(mconfig, *this)
	, m_phase(0U)
{
}

void hle_msystems_device_base::device_start()
{
	save_item(NAME(m_phase));

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(1'200);
	receive_register_reset();
	transmit_register_reset();

	m_phase = 0U;

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(hle_msystems_device_base::start_mouse), this));
}

void hle_msystems_device_base::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void hle_msystems_device_base::tra_complete()
{
	if (4U <= m_phase)
		m_phase = 0U;
	else
		++m_phase;

	bool const dirty(read_inputs());
	switch (m_phase)
	{
	case 0U:
		if (dirty)
			transmit_register_setup((report_buttons() & 0x07U) | 0x80U);
		break;
	case 1U:
		transmit_register_setup(report_x1_delta());
		break;
	case 2U:
		transmit_register_setup(report_y1_delta());
		break;
	case 3U:
		transmit_register_setup(report_x2_delta());
		break;
	case 4U:
		transmit_register_setup(report_y2_delta());
		break;
	};
}

TIMER_CALLBACK_MEMBER(hle_msystems_device_base::start_mouse)
{
	if (is_transmit_register_empty())
	{
		assert(0U == m_phase);
		tra_complete();
	}
}


//**************************************************
//  Mouse Systems non-rotatable mouse
//**************************************************

hle_msystems_mouse_device::hle_msystems_mouse_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msystems_device_base(mconfig, type, tag, owner, clock)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
	, m_x_delta(0)
	, m_y_delta(0)
	, m_x_val(0U)
	, m_y_val(0U)
	, m_btn_val(0x00U)
	, m_btn_sent(0x00U)
{
}

hle_msystems_mouse_device::hle_msystems_mouse_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msystems_mouse_device(mconfig, MSYSTEMS_HLE_SERIAL_MOUSE, tag, owner, clock)
{
}

ioport_constructor hle_msystems_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msystems);
}

void hle_msystems_mouse_device::device_start()
{
	hle_msystems_device_base::device_start();

	save_item(NAME(m_x_delta));
	save_item(NAME(m_y_delta));
	save_item(NAME(m_x_val));
	save_item(NAME(m_y_val));
	save_item(NAME(m_btn_val));
	save_item(NAME(m_btn_sent));

	m_x_delta = m_y_delta = 0;
	m_x_val = m_y_val = 0U;
	m_btn_val = m_btn_sent = 0x00U;
}

bool hle_msystems_mouse_device::read_inputs()
{
	m_x_delta += read_axis<12>(*m_x_axis, m_x_val);
	m_y_delta -= read_axis<12>(*m_y_axis, m_y_val);
	m_btn_val = m_buttons->read();
	return m_x_delta || m_y_delta || (m_btn_val != m_btn_sent);
}

uint8_t hle_msystems_mouse_device::report_buttons()
{
	m_btn_sent = m_btn_val;
	return m_btn_sent;
}

uint8_t hle_msystems_mouse_device::report_x1_delta()
{
	return report_axis<int16_t>(m_x_delta, -120, 127);
}

uint8_t hle_msystems_mouse_device::report_y1_delta()
{
	return report_axis<int16_t>(m_y_delta, -120, 127);
}

uint8_t hle_msystems_mouse_device::report_x2_delta()
{
	return report_axis<int16_t>(m_x_delta, -120, 127);
}

uint8_t hle_msystems_mouse_device::report_y2_delta()
{
	return report_axis<int16_t>(m_y_delta, -120, 127);
}


//**************************************************
//  Mouse Systems rotatable mouse
//**************************************************

hle_rotatable_mouse_device::hle_rotatable_mouse_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msystems_device_base(mconfig, ROTATABLE_HLE_SERIAL_MOUSE, tag, owner, clock)
	, m_buttons(*this, "BTN")
	, m_x_axis(*this, "X")
	, m_y_axis(*this, "Y")
	, m_rotation(*this, "ROT")
	, m_x_delta{ 0, 0 }
	, m_y_delta{ 0, 0 }
	, m_x_val(0U)
	, m_y_val(0U)
	, m_rot_val(0U)
	, m_btn_val(0x00U)
	, m_btn_sent(0x00U)
{
}

ioport_constructor hle_rotatable_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(rotatable);
}

void hle_rotatable_mouse_device::device_start()
{
	hle_msystems_device_base::device_start();

	save_item(NAME(m_x_delta));
	save_item(NAME(m_y_delta));
	save_item(NAME(m_x_val));
	save_item(NAME(m_y_val));
	save_item(NAME(m_rot_val));
	save_item(NAME(m_btn_val));
	save_item(NAME(m_btn_sent));

	m_x_delta[0] = m_x_delta[1] = m_y_delta[0] = m_y_delta[1] = 0;
	m_x_val = m_y_val = m_rot_val = 0U;
	m_btn_val = m_btn_sent = 0x00U;
}

bool hle_rotatable_mouse_device::read_inputs()
{
	// translation is straighforward
	int16_t const x_delta(read_axis<12>(*m_x_axis, m_x_val));
	int16_t const y_delta(read_axis<12>(*m_y_axis, m_y_val));
	m_x_delta[0] += x_delta;
	m_x_delta[1] += x_delta;
	m_y_delta[0] -= y_delta;
	m_y_delta[1] -= y_delta;

	// there are two sensors 100 counts apart, one inch below the centre of the mouse
	// for simplicity, assume the mouse is rotated around the point midway the sensors
	int16_t const rot_delta(read_axis<12>(*m_rotation, m_rot_val));
	if (rot_delta)
	{
		double const rot_angle(-0.15 * rot_delta);
		long const x_diff(std::lround((1 - std::cos(rot_angle)) * 100));
		long const y_diff(std::lround(std::sin(rot_angle) * 100));
		m_x_delta[0] -= x_diff / 2;
		m_x_delta[1] += x_diff - (x_diff / 2);
		m_y_delta[0] += y_diff / 2;
		m_y_delta[1] -= y_diff - (y_diff / 2);
	}

	// this is straightforward, too
	m_btn_val = m_buttons->read();
	return m_x_delta[0] || m_x_delta[1] || m_y_delta[0] || m_y_delta[1] || (m_btn_val != m_btn_sent);
}

uint8_t hle_rotatable_mouse_device::report_buttons()
{
	m_btn_sent = m_btn_val;
	return m_btn_sent;
}

uint8_t hle_rotatable_mouse_device::report_x1_delta()
{
	return report_axis<int16_t>(m_x_delta[0], -120, 127);
}

uint8_t hle_rotatable_mouse_device::report_y1_delta()
{
	return report_axis<int16_t>(m_y_delta[0], -120, 127);
}

uint8_t hle_rotatable_mouse_device::report_x2_delta()
{
	return report_axis<int16_t>(m_x_delta[1], -120, 127);
}

uint8_t hle_rotatable_mouse_device::report_y2_delta()
{
	return report_axis<int16_t>(m_y_delta[1], -120, 127);
}

//**************************************************
//  SGI IRIS Indigo mouse
//**************************************************

hle_sgi_mouse_device::hle_sgi_mouse_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: hle_msystems_mouse_device(mconfig, SGI_HLE_SERIAL_MOUSE, tag, owner, clock)
{
}

void hle_sgi_mouse_device::device_start()
{
	hle_msystems_mouse_device::device_start();
	set_rate(4'800);
	receive_register_reset();
	transmit_register_reset();
}

} } // namespace bus::rs232
