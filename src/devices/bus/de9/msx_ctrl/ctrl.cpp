// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX General Purpose port

**********************************************************************/

#include "emu.h"
#include "joystick.h"
#include "mouse.h"

void msx_general_purpose_port_devices(device_slot_interface &device)
{
	device.option_add("joystick", MSX_JOYSTICK);
	device.option_add("mouse", MSX_MOUSE);
}
