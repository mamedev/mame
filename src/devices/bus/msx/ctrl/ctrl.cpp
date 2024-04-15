// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX General Purpose port emulation

**********************************************************************/

#include "emu.h"
#include "ctrl.h"

#include "hypershot.h"
#include "joystick.h"
#include "libbler.h"
#include "magickey.h"
#include "mouse.h"
#include "sgadapt.h"
#include "towns6b.h"
#include "townspad.h"
#include "vaus.h"
#include "xe1ap.h"


DEFINE_DEVICE_TYPE(MSX_GENERAL_PURPOSE_PORT, msx_general_purpose_port_device, "msx_general_purpose_port", "MSX General Purpose port")


device_msx_general_purpose_port_interface::device_msx_general_purpose_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "msxctrl")
	, m_port(dynamic_cast<msx_general_purpose_port_device *>(device.owner()))
{
}


msx_general_purpose_port_device::msx_general_purpose_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_GENERAL_PURPOSE_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_msx_general_purpose_port_interface>(mconfig, *this)
	, m_device(nullptr)
{
}


void msx_general_purpose_port_device::device_start()
{
	m_device = get_card_device();
}


void msx_general_purpose_port_devices(device_slot_interface &device)
{
	device.option_add("hypershot", MSX_HYPERSHOT);
	device.option_add("joystick", MSX_JOYSTICK);
	device.option_add("libbler", MSX_LIBBLERPAD);
	device.option_add("magickey", MSX_MAGICKEY);
	device.option_add("martypad", MSX_MARTYPAD);
	device.option_add("mouse", MSX_MOUSE);
	device.option_add("sega", MSX_SEGACTRL);
	device.option_add("towns6b", MSX_TOWNS6B);
	device.option_add("townspad", MSX_TOWNSPAD);
	device.option_add("vaus", MSX_VAUS);
	device.option_add("xe1ap", MSX_XE1AP);
}
