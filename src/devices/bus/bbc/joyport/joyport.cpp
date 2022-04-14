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

DEFINE_DEVICE_TYPE(BBC_JOYPORT_SLOT, bbc_joyport_slot_device, "bbc_joyport_slot", "BBC Master Compact Joystick/Mouse port")


//**************************************************************************
//  DEVICE BBC_JOYPORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_joyport_interface - constructor
//-------------------------------------------------

device_bbc_joyport_interface::device_bbc_joyport_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "bbcjoyport")
{
	m_slot = dynamic_cast<bbc_joyport_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbcmc_joyport_slot_device - constructor
//-------------------------------------------------

bbc_joyport_slot_device::bbc_joyport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_JOYPORT_SLOT, tag, owner, clock)
	, device_single_card_slot_interface(mconfig, *this)
	, m_device(nullptr)
	, m_cb1_handler(*this)
	, m_cb2_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_joyport_slot_device::device_start()
{
	m_device = get_card_device();

	// resolve callbacks
	m_cb1_handler.resolve_safe();
	m_cb2_handler.resolve_safe();
}


//-------------------------------------------------
//  pb_r
//-------------------------------------------------

uint8_t bbc_joyport_slot_device::pb_r()
{
	if (m_device)
		return 0xe0 | m_device->pb_r();
	else
		return 0xff;
}


//-------------------------------------------------
//  pb_w
//-------------------------------------------------

void bbc_joyport_slot_device::pb_w(uint8_t data)
{
	if (m_device)
		m_device->pb_w(data);
}


//-------------------------------------------------
//  write_cb1
//-------------------------------------------------

void bbc_joyport_slot_device::write_cb1(int state)
{
	if (m_device)
		m_device->write_cb1(state);
}


//-------------------------------------------------
//  write_cb2
//-------------------------------------------------

void bbc_joyport_slot_device::write_cb2(int state)
{
	if (m_device)
		m_device->write_cb2(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_joyport_devices )
//-------------------------------------------------


// slot devices
#include "joystick.h"
#include "mouse.h"


void bbc_joyport_devices(device_slot_interface &device)
{
	device.option_add("joystick", BBCMC_JOYSTICK);
	device.option_add("mouse", BBCMC_MOUSE);
	//device.option_add("sat", BBCMC_SAT);     /* Solidisk Advanced Teletext */
}
