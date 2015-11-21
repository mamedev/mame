// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Atari Video Computer System controller port emulation

**********************************************************************/

#include "ctrl.h"



//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

const device_type VCS_CONTROL_PORT = &device_creator<vcs_control_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vcs_control_port_interface - constructor
//-------------------------------------------------

device_vcs_control_port_interface::device_vcs_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_port = dynamic_cast<vcs_control_port_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_control_port_device - constructor
//-------------------------------------------------

vcs_control_port_device::vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VCS_CONTROL_PORT, "Atari VCS control port", tag, owner, clock, "vcs_control_port", __FILE__),
	device_slot_interface(mconfig, *this), m_device(nullptr),
	m_write_trigger(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vcs_control_port_device::device_start()
{
	m_device = dynamic_cast<device_vcs_control_port_interface *>(get_card_device());

	m_write_trigger.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( vcs_control_port_devices )
//-------------------------------------------------

#include "joybooster.h"
#include "joystick.h"
#include "keypad.h"
#include "lightpen.h"
#include "paddles.h"
#include "wheel.h"

SLOT_INTERFACE_START( vcs_control_port_devices )
	SLOT_INTERFACE("joy", VCS_JOYSTICK)
	SLOT_INTERFACE("pad", VCS_PADDLES)
	SLOT_INTERFACE("lp", VCS_LIGHTPEN)
	SLOT_INTERFACE("joybstr", VCS_JOYSTICK_BOOSTER)
	SLOT_INTERFACE("wheel", VCS_WHEEL)
	SLOT_INTERFACE("keypad", VCS_KEYPAD)
SLOT_INTERFACE_END
