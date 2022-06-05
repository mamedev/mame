// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (US model) emulation


Release data from the Sega Retro project:

  Year: 1987    Country/region: US    Model code: 3040

TODO:

- For low-level emulation, a device for the TMP42C66P, a Toshiba 4-bit
  microcontroller, is needed along with a dump of its internal ROM.
- Auto-repeat and Control/Sports mode switches are not emulated.

Notes:

  Games designed for the US model of the Sports Pad controller use the
  TH line of the controller port to select which nibble of the two axis
  to read. The Japanese cartridge Sports Pad Soccer uses a different mode
  when it does not detect use by a Japanese SMS console, because the Sega
  Mark III lacks the TH line. There was a different Sports Pad model released
  in Japan and no information was found about it supporting both modes, so
  that model is currently emulated as a different device (see sportsjp.c).

  It was discovered that games designed for the Paddle Controller, released
  in Japan, will switch to a mode incompatible with the original Paddle when
  the system region is detected as Export. Similar to how the US model of the
  Sports Pad works, that mode uses the TH line as an output to select which
  nibble of the X axis will be read. So, on an Export console version,
  paddle games are somewhat playable with the US Sport Pad model, though it
  needs to be used inverted and the trackball needs to be moved slowly,
  otherwise the software for the paddle think it's moving backward.
  See http://mametesters.org/view.php?id=5872 for discussion.

**********************************************************************/

#include "emu.h"
#include "sports.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_SPORTS_PAD, sms_sports_pad_device, "sms_sports_pad", "Sega SMS Sports Pad (US)")

// time interval not verified
#define SPORTS_PAD_INTERVAL attotime::from_hz(XTAL(10'738'635)/3/512)


TIMER_CALLBACK_MEMBER(sms_sports_pad_device::read_tick)
{
	// values for x and y axis need to be reset for Sports Pad games, but
	// are not reset for paddle games, so it was assumed the reset occurs
	// only when this timer fires after the read state reaches its maximum value.
	if (m_read_state == 3)
	{
		m_x_axis_reset_value = m_sports_x->read();
		m_y_axis_reset_value = m_sports_y->read();
	}
	else
	{
		// set to maximum value, so it wraps to 0 at next increment
		m_read_state = 3;
	}
}


READ_LINE_MEMBER( sms_sports_pad_device::th_pin_r )
{
	return m_th_pin_state;
}


WRITE_LINE_MEMBER( sms_sports_pad_device::th_pin_w )
{
	m_read_state = (m_read_state + 1) & 3;
	m_sportspad_timer->adjust(m_interval);
	m_th_pin_state = state;
}


CUSTOM_INPUT_MEMBER( sms_sports_pad_device::rldu_pins_r )
{
	uint8_t data = 0;

	switch (m_read_state)
	{
	case 0:
		data = (m_sports_x->read() - m_x_axis_reset_value) >> 4;
		break;
	case 1:
		data = (m_sports_x->read() - m_x_axis_reset_value);
		break;
	case 2:
		data = (m_sports_y->read() - m_y_axis_reset_value) >> 4;
		break;
	case 3:
		data = (m_sports_y->read() - m_y_axis_reset_value);
		break;
	}

	// The returned value is inverted due to IP_ACTIVE_LOW mapping.
	return ~(data & 0x0f);
}


static INPUT_PORTS_START( sms_sports_pad )
	PORT_START("SPORTS_IN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(sms_sports_pad_device, rldu_pins_r) // R,L,D,U
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL (Button 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(sms_sports_pad_device, th_pin_r) // TH
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) // TR (Button 2)

	PORT_START("SPORTS_OUT")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) // Directional pins
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // TL (Button 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(sms_sports_pad_device, th_pin_w) // TH
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) // TR (Button 2)

	PORT_START("SPORTS_X")    /* Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_REVERSE

	PORT_START("SPORTS_Y")    /* Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_REVERSE
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_sports_pad_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_sports_pad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_sports_pad_device - constructor
//-------------------------------------------------

sms_sports_pad_device::sms_sports_pad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_SPORTS_PAD, tag, owner, clock),
	device_sms_control_port_interface(mconfig, *this),
	m_sports_in(*this, "SPORTS_IN"),
	m_sports_out(*this, "SPORTS_OUT"),
	m_sports_x(*this, "SPORTS_X"),
	m_sports_y(*this, "SPORTS_Y"),
	m_read_state(0),
	m_th_pin_state(0),
	m_x_axis_reset_value(0x80), // value 0x80 helps when starting paddle games.
	m_y_axis_reset_value(0x80),
	m_interval(SPORTS_PAD_INTERVAL),
	m_sportspad_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_sports_pad_device::device_start()
{
	m_sportspad_timer = timer_alloc(FUNC(sms_sports_pad_device::read_tick), this);

	save_item(NAME(m_read_state));
	save_item(NAME(m_th_pin_state));
	save_item(NAME(m_x_axis_reset_value));
	save_item(NAME(m_y_axis_reset_value));
}

uint8_t sms_sports_pad_device::peripheral_r()
{
	return m_sports_in->read();
}

void sms_sports_pad_device::peripheral_w(uint8_t data)
{
	m_sports_out->write(data);
}
