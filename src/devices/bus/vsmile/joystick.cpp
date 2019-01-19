// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "joystick.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_JOYSTICK, vsmile_joystick_device, "vsmile_joystick", "V.Smile Joystick Controller")


//**************************************************************************
//    V.Smile joystick controller
//**************************************************************************

vsmile_joystick_device::vsmile_joystick_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, VSMILE_JOYSTICK, tag, owner, clock)
	, device_vsmile_ctrl_interface(mconfig, *this)
{
}

vsmile_joystick_device::~vsmile_joystick_device()
{
}

void vsmile_joystick_device::device_start()
{
	// TODO: initialise and register save state
}

void vsmile_joystick_device::cts_w(int state)
{
	// TODO: CTS input changed
}

void vsmile_joystick_device::data_w(uint8_t data)
{
	// TODO: data arrived
}
