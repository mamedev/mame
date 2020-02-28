// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "joy.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_JOY, astrocade_joy_device, "astrocade_joy", "Bally Astrocade Joystick")


//**************************************************************************
//    Bally Astrocade joystick control
//**************************************************************************

astrocade_joy_device::astrocade_joy_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASTROCADE_JOY, tag, owner, clock)
	, device_astrocade_ctrl_interface(mconfig, *this)
	, m_handle(*this, "HANDLE")
	, m_knob(*this, "KNOB")
{
}

astrocade_joy_device::~astrocade_joy_device()
{
}

uint8_t astrocade_joy_device::read_handle()
{
	return m_handle->read();
}

uint8_t astrocade_joy_device::read_knob()
{
	return m_knob->read();
}

static INPUT_PORTS_START( astrocade_joy )
	PORT_START("HANDLE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KNOB")
	PORT_BIT(0xff, 0x00, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(85) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)
INPUT_PORTS_END

ioport_constructor astrocade_joy_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( astrocade_joy );
}
