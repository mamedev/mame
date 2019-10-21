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

device_bbc_joyport_interface::device_bbc_joyport_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_joyport_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbcmc_joyport_slot_device - constructor
//-------------------------------------------------

bbc_joyport_slot_device::bbc_joyport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_JOYPORT_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr),
	m_cb1_handler(*this),
	m_cb2_handler(*this)
{
}


//-------------------------------------------------
//  device_validity_check -
//-------------------------------------------------

void bbc_joyport_slot_device::device_validity_check(validity_checker &valid) const
{
	device_t *const carddev = get_card_device();
	if (carddev && !dynamic_cast<device_bbc_joyport_interface *>(carddev))
		osd_printf_error("Card device %s (%s) does not implement device_bbc_joyport_interface\n", carddev->tag(), carddev->name());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_joyport_slot_device::device_start()
{
	device_t *const carddev = get_card_device();
	m_device = dynamic_cast<device_bbc_joyport_interface *>(carddev);
	if (carddev && !m_device)
		fatalerror("Card device %s (%s) does not implement device_bbc_joyport_interface\n", carddev->tag(), carddev->name());

  // resolve callbacks
	m_cb1_handler.resolve_safe();
	m_cb2_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_joyport_slot_device::device_reset()
{
}


//-------------------------------------------------
//  pb_r
//-------------------------------------------------

uint8_t bbc_joyport_slot_device::pb_r()
{
	// TODO: Joyport connected to PB0-PB4 only. PB5-PB7 are expansion port.
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
//  SLOT_INTERFACE( bbc_joyport_devices )
//-------------------------------------------------


// slot devices
#include "joystick.h"
//#include "mouse.h"


void bbc_joyport_devices(device_slot_interface &device)
{
	device.option_add("joystick", BBCMC_JOYSTICK);
	//device.option_add("mouse", BBCMC_MOUSE);
}
