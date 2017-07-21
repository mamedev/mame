// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

**********************************************************************/

#include "emu.h"
#include "ctrl.h"



//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

DEFINE_DEVICE_TYPE(A7800_CONTROL_PORT, a7800_control_port_device, "a7800_control_port", "Atari 7800 controller port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a7800_control_port_interface - constructor
//-------------------------------------------------

device_a7800_control_port_interface::device_a7800_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_port = dynamic_cast<a7800_control_port_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a7800_control_port_device - constructor
//-------------------------------------------------

a7800_control_port_device::a7800_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A7800_CONTROL_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this), m_device(nullptr),
	m_write_trigger(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a7800_control_port_device::device_start()
{
	m_device = dynamic_cast<device_a7800_control_port_interface *>(get_card_device());

	m_write_trigger.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( a7800_control_port_devices )
//-------------------------------------------------

#include "joyproline.h"
#include "joystick.h"
#include "keypad.h"
#include "lightgun.h"
#include "paddles.h"
#include "wheel.h"
#include "trackball.h"
#include "stmouse.h"
#include "amigamouse.h"

SLOT_INTERFACE_START( a7800_control_port_devices )
	SLOT_INTERFACE("proline_joystick", A7800_JOYSTICK_PROLINE)
	SLOT_INTERFACE("vcs_joystick", A7800_JOYSTICK)
	SLOT_INTERFACE("paddle", A7800_PADDLES)
	SLOT_INTERFACE("lightgun", A7800_LIGHTGUN)
	SLOT_INTERFACE("driving_wheel", A7800_WHEEL)
	SLOT_INTERFACE("cx22_trakball", A7800_TRACKBALL)
	SLOT_INTERFACE("keypad", A7800_KEYPAD)
	SLOT_INTERFACE("amiga_mouse", A7800_AMIGAMOUSE)
	SLOT_INTERFACE("st_mouse", A7800_STMOUSE)
SLOT_INTERFACE_END
