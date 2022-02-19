// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer & Entertainment System SNES controller port adapter

**********************************************************************/

#include "emu.h"
#include "snesadapter.h"
// slot devices
#include "bus/snes_ctrl/joypad.h"
#include "bus/snes_ctrl/mouse.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_SNESADAPTER, nes_snesadapter_device, "nes_snesadapter", "SNES Controller Port Adapter")


static void snes_controllers(device_slot_interface &device)
{
	device.option_add("snes_joypad", SNES_JOYPAD);
	device.option_add("snes_mouse", SNES_MOUSE);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_snesadapter_device::device_add_mconfig(machine_config &config)
{
	SNES_CONTROL_PORT(config, m_snesctrl, snes_controllers, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_snesadapter_device - constructor
//-------------------------------------------------

nes_snesadapter_device::nes_snesadapter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_SNESADAPTER, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_snesctrl(*this, "port")
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_snesadapter_device::read_bit0()
{
	return m_snesctrl->read_pin4();
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_snesadapter_device::write(u8 data)
{
	m_snesctrl->write_strobe(data);
}
