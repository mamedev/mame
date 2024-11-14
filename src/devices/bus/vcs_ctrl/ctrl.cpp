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

DEFINE_DEVICE_TYPE(VCS_CONTROL_PORT, vcs_control_port_device, "vcs_control_port", "Atari VCS controller port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vcs_control_port_interface - constructor
//-------------------------------------------------

device_vcs_control_port_interface::device_vcs_control_port_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vcsctrl")
{
	m_port = dynamic_cast<vcs_control_port_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vcs_control_port_device - constructor
//-------------------------------------------------

vcs_control_port_device::vcs_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_CONTROL_PORT, tag, owner, clock),
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
}


//-------------------------------------------------
//  SLOT_INTERFACE( vcs_control_port_devices )
//-------------------------------------------------

#include "cx85.h"
#include "joybooster.h"
#include "joystick.h"
#include "keypad.h"
#include "lightpen.h"
#include "mouse.h"
#include "paddles.h"
#include "trakball.h"
#include "wheel.h"

void vcs_control_port_devices(device_slot_interface &device)
{
	device.option_add("joy", VCS_JOYSTICK);
	device.option_add("pad", VCS_PADDLES);
	device.option_add("mouse", VCS_MOUSE);
	device.option_add("lp", VCS_LIGHTPEN);
	device.option_add("joybstr", VCS_JOYSTICK_BOOSTER);
	device.option_add("wheel", VCS_WHEEL);
	device.option_add("keypad", VCS_KEYPAD);
	device.option_add("cx85", ATARI_CX85);
	device.option_add("trakball", ATARI_TRAKBALL);
}

void a800_control_port_devices(device_slot_interface &device)
{
	vcs_control_port_devices(device);
	device.set_option_machine_config("pad", &vcs_paddles_device::reverse_players);
}
