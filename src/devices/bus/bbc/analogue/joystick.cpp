// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Joystick Controllers

    ANH01 - Acorn Analogue Joystick Controllers
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANH01_JoystickController.html

    Voltmace Delta 3b "Twin" Joystick Controllers (self centering)

**********************************************************************/

#include "emu.h"
#include "joystick.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_ACORNJOY,   bbc_acornjoy_device,   "bbc_acornjoy",   "Acorn Analogue Joysticks")
DEFINE_DEVICE_TYPE(BBC_VOLTMACE3B, bbc_voltmace3b_device, "bbc_voltmace3b", "Voltmace Delta 3b Twin Joysticks")


static INPUT_PORTS_START( acornjoy )
	PORT_START("JOY0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( voltmace3b )
	PORT_START("JOY0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(20) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(20) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(20) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(20) PORT_MINMAX(0x00, 0xff) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_acornjoy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( acornjoy );
}

ioport_constructor bbc_voltmace3b_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( voltmace3b );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_joystick_device - constructor
//-------------------------------------------------

bbc_joystick_device::bbc_joystick_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_analogue_interface(mconfig, *this),
	m_joy(*this, "JOY%u", 0),
	m_buttons(*this, "BUTTONS")
{
}

bbc_acornjoy_device::bbc_acornjoy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_joystick_device(mconfig, BBC_ACORNJOY, tag, owner, clock)
{
}

bbc_voltmace3b_device::bbc_voltmace3b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bbc_joystick_device(mconfig, BBC_VOLTMACE3B, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_joystick_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_joystick_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_joystick_device::ch_r(int channel)
{
	return m_joy[channel]->read();
}

uint8_t bbc_joystick_device::pb_r()
{
	return m_buttons->read() & 0x30;
}
