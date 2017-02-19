// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/****************************************************************************

    BBC Master Compact Joystick/Mouse port

*****************************************************************************/

#include "emu.h"
#include "joyport.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_JOYPORT_SLOT = &device_creator<bbc_joyport_slot_device>;


//**************************************************************************
//  DEVICE BBC_JOYPORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_joyport_interface - constructor
//-------------------------------------------------

device_bbc_joyport_interface::device_bbc_joyport_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_joyport_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_joyport_interface - destructor
//-------------------------------------------------

device_bbc_joyport_interface::~device_bbc_joyport_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbcmc_joyport_slot_device - constructor
//-------------------------------------------------

bbc_joyport_slot_device::bbc_joyport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, BBC_JOYPORT_SLOT, "BBC Master Compact Joystick/Mouse port", tag, owner, clock, "bbc_joyport_slot", __FILE__),
		device_slot_interface(mconfig, *this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_joyport_slot_device::device_start()
{
	m_device = dynamic_cast<device_bbc_joyport_interface *>(get_card_device());
}

uint8_t bbc_joyport_slot_device::cb_r()
{
	if (m_device)
		return m_device->cb_r();
	else
		return 0xff;
}

uint8_t bbc_joyport_slot_device::pb_r()
{
	if (m_device)
		return m_device->pb_r();
	else
		return 0x1f;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_joyport_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_joyport_devices )
//-------------------------------------------------


// slot devices
#include "joystick.h"
//#include "mouse.h"


SLOT_INTERFACE_START( bbc_joyport_devices )
	SLOT_INTERFACE("joystick", BBCMC_JOYSTICK)
	//SLOT_INTERFACE("mouse", BBCMC_MOUSE)
SLOT_INTERFACE_END
