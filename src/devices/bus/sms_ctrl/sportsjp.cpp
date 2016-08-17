// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Sports Pad" (Japanese model) emulation


Release data from the Sega Retro project:

  Year: 1988    Country/region: JP    Model code: SP-500

TODO:

- For low-level emulation, a device for the TMP42C66P, a Toshiba 4bit
  microcontroller, needs to be created, but a dump of its internal ROM
  seems to be required.

Notes:

  The Japanese Sports Pad controller is only required to play the cartridge
  Sports Pad Soccer, released in Japan. It uses a different mode than the
  used by the US model, due to the missing TH line on Sega Mark III
  controller ports.

  A bug was discovered in the player 2 input handling code of the only known
  good ROM dump of Sports Pad Soccer (JP):
  size="131072" crc="41c948bf" sha1="7634ce39e87049dad1ee4f32a80d728e4bd1f81f"
  At address $12D1, instead read the upper 2 bits of port $DC and lower 2 bits
  of port $DD (to obtain the lower nibble of the current axis for player 2),
  the code wrongly reads the lower nibble of port $DC, that is player 1 data.

**********************************************************************/

#include "sportsjp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SMS_SPORTS_PAD_JP = &device_creator<sms_sports_pad_jp_device>;

// time interval not verified
#define SPORTS_PAD_JP_INTERVAL attotime::from_hz(20000)



// The returned value is inverted due to IP_ACTIVE_LOW mapping.
READ_LINE_MEMBER( sms_sports_pad_jp_device::tl_pin_r ) { return ~m_tl_pin_state; }
READ_LINE_MEMBER( sms_sports_pad_jp_device::tr_pin_r ) { return ~m_tr_pin_state; }
CUSTOM_INPUT_MEMBER( sms_sports_pad_jp_device::rldu_pins_r ) { return ~(m_rldu_pins_state & 0x0f); }


static INPUT_PORTS_START( sms_sports_pad_jp )
	PORT_START("SPORTS_JP_IN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, sms_sports_pad_jp_device, rldu_pins_r, nullptr) // R,L,D,U
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // Vcc
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, sms_sports_pad_jp_device, tl_pin_r ) // TL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )  // TH
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER( DEVICE_SELF, sms_sports_pad_jp_device, tr_pin_r ) // TR

	PORT_START("SPORTS_JP_BT")    /* Sports Pad buttons nibble */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPORTS_JP_X")    /* Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)

	PORT_START("SPORTS_JP_Y")    /* Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sms_sports_pad_jp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_sports_pad_jp );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_sports_pad_jp_device - constructor
//-------------------------------------------------

sms_sports_pad_jp_device::sms_sports_pad_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SMS_SPORTS_PAD_JP, "Sega SMS Sports Pad JP", tag, owner, clock, "sms_sports_pad_jp", __FILE__),
	device_sms_control_port_interface(mconfig, *this),
	m_sports_jp_in(*this, "SPORTS_JP_IN"),
	m_sports_jp_bt(*this, "SPORTS_JP_BT"),
	m_sports_jp_x(*this, "SPORTS_JP_X"),
	m_sports_jp_y(*this, "SPORTS_JP_Y"),
	m_rldu_pins_state(0x0f),
	m_tl_pin_state(1),
	m_tr_pin_state(1),
	m_interval(SPORTS_PAD_JP_INTERVAL)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_sports_pad_jp_device::device_start()
{
	m_start_time = machine().time();

	save_item(NAME(m_start_time));
	save_item(NAME(m_rldu_pins_state));
	save_item(NAME(m_tl_pin_state));
	save_item(NAME(m_tr_pin_state));
}


//-------------------------------------------------
//  sms_peripheral_r - sports pad read
//-------------------------------------------------

UINT8 sms_sports_pad_jp_device::peripheral_r()
{
	int num_intervals = (machine().time() - m_start_time).as_double() / m_interval.as_double();

	switch (num_intervals % 5)
	{
	case 0:
		// X high nibble
		m_rldu_pins_state = m_sports_jp_x->read() >> 4;
		m_tl_pin_state = 0;
		m_tr_pin_state = 0;
		break;
	case 1:
		// X low nibble
		m_rldu_pins_state = m_sports_jp_x->read();
		m_tl_pin_state = 1;
		m_tr_pin_state = 0;
		break;
	case 2:
		// Y high nibble
		m_rldu_pins_state = m_sports_jp_y->read() >> 4;
		m_tl_pin_state = 0;
		m_tr_pin_state = 0;
		break;
	case 3:
		// Y low nibble
		m_rldu_pins_state = m_sports_jp_y->read();
		m_tl_pin_state = 1;
		m_tr_pin_state = 0;
		break;
	case 4:
		// buttons 1 and 2
		m_rldu_pins_state = m_sports_jp_bt->read();
		m_tl_pin_state = 1;
		m_tr_pin_state = 1;
		break;
	}

	return m_sports_jp_in->read();
}
