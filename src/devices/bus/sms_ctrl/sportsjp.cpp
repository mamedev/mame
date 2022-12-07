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

  This Japanese Sports Pad controller device is only required to play the
  cartridge Sports Pad Soccer, released in Japan, on non-SMSJ consoles without
  ROM header validation, like the Sega Mark III and the Korean SMS2 version.

  The Japanese version of Sports Pad Soccer has code to operate the Sports
  Pad controller in two diffent modes. When it detects a Japanese SMS (testing
  if port $F2 has two bits for mute control), the operation is the same used
  by US Sports Pad games. Otherwise, it uses a mode that polls bits TR and TL
  of the controller ports, compatible with the Sega Mark III, that lacks the
  TH line used by the US Sports Pad mode. This Mark III mode is also used on
  other non-SMSJ consoles, like the Korean SMS2 version. The two controller
  modes are significantly different from each other and no information was
  found about support for both modes on the Japanese Sports Pad model, so that
  model is currently emulated as a different device.

  A bug was discovered in the player 2 input handling code of the Mark III
  compatible mode of the only known good ROM dump of Sports Pad Soccer (JP):
  size="131072" crc="41c948bf" sha1="7634ce39e87049dad1ee4f32a80d728e4bd1f81f"
  At address $12D1, instead read the upper 2 bits of port $DC and lower 2 bits
  of port $DD (to obtain the lower nibble of the current axis for player 2),
  the code wrongly reads the lower nibble of port $DC, that is player 1 data.

**********************************************************************/

#include "emu.h"
#include "sportsjp.h"


namespace {

INPUT_PORTS_START( sms_sports_pad_jp )
	PORT_START("SPORTS_JP_BT")    // buttons
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // TR
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPORTS_JP_X")    // X axis
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)

	PORT_START("SPORTS_JP_Y")    // Y axis
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40)
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_sports_pad_jp_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_sports_pad_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_sports_pad_jp); }
	virtual void device_start() override { }

private:
	required_ioport m_buttons;
	required_ioport m_x_axis;
	required_ioport m_y_axis;
};


//-------------------------------------------------
//  sms_sports_pad_jp_device - constructor
//-------------------------------------------------

sms_sports_pad_jp_device::sms_sports_pad_jp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_SPORTS_PAD_JP, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "SPORTS_JP_BT"),
	m_x_axis(*this, "SPORTS_JP_X"),
	m_y_axis(*this, "SPORTS_JP_Y")
{
}


//-------------------------------------------------
//  sms_peripheral_r - sports pad read
//-------------------------------------------------

uint8_t sms_sports_pad_jp_device::in_r()
{
	int const num_intervals = machine().time().as_ticks(20000); // time interval not verified

	switch (num_intervals % 5)
	{
	default: // shut up dumb compilers
	case 0: // X high nibble
		return 0x00 | ((m_x_axis->read() >> 4) & 0x0f);
	case 1: // X low nibble
		return 0x10 | (m_x_axis->read() & 0x0f);
	case 2: // Y high nibble
		return 0x00 | ((m_y_axis->read() >> 4) & 0x0f);
	case 3: // Y low nibble
		return 0x10 | (m_y_axis->read() & 0x0f);
	case 4: // buttons 1 and 2
		return 0x30 | m_buttons->read();
	}
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_SPORTS_PAD_JP, device_sms_control_interface, sms_sports_pad_jp_device, "sms_sports_pad_jp", "Sega Mark III Sports Pad (Japan)")
