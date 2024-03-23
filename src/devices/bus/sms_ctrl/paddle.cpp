// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega Mark III "Paddle Control" emulation


    Release data from the Sega Retro project:

      Year: 1987    Country/region: JP    Model code: HPD-200

    Notes:

      The main chip contained in the device is labeled 315-5243.

      On Japanese systems (Mark III and Master System), the games
      read the port and expect the value on the low four bits
      (usually used joystick switches) to alternate between the low
      and high nybbles of the paddle position, with TR indicating
      the current state.

      On export consoles, the games toggle the TH output before
      reading a nybble, expecting to be able to select the high or
      low nybble of the paddle position.  It spins for longer between
      pulling TH low and reading the low nybble than it does between
      pulling TH high and reading the high nybble, suggesting that
      pulling TH low triggers acquisition.

      Only a single model of paddle controller was released for the
      Japanese market.  Photos show no connection to pin 7.  There
      are a few reports of paddle controllers working on export
      consoles, but this is not possible unless another hardware
      revision exists.

**********************************************************************/

#include "emu.h"
#include "paddle.h"


namespace  {

INPUT_PORTS_START( sms_paddle )
	PORT_START("BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0, 255) PORT_SENSITIVITY(40) PORT_KEYDELTA(20) PORT_CENTERDELTA(0)
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_paddle_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_paddle_device(const machine_config &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_sms_control_interface implementation
	virtual u8 in_r() override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_paddle); }
	virtual void device_start() override { }

private:
	TIMER_CALLBACK_MEMBER(timeout);

	required_ioport m_button;
	required_ioport m_axis;
};


sms_paddle_device::sms_paddle_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SMS_PADDLE, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_button(*this, "BUTTON"),
	m_axis(*this, "PADDLE")
{
}


u8 sms_paddle_device::in_r()
{
	// time interval guessed
	// Player 2 of Galactic Protector is the most sensitive to this timing
	uint8_t const nybble = machine().time().as_ticks(XTAL(10'738'635) / 3 / 100) & 1;
	return (nybble << 5) | (m_button->read() << 4) | BIT(m_axis->read(), nybble ? 4 : 0, 4);
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_PADDLE, device_sms_control_interface, sms_paddle_device, "sms_paddle", "Sega Mark III Paddle (Japan)")
