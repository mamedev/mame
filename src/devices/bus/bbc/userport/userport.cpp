// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC User Port emulation

**********************************************************************/

#include "userport.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type BBC_USERPORT_SLOT = &device_creator<bbc_userport_device>;



//**************************************************************************
//  DEVICE BBC_USERPORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_userport_interface - constructor
//-------------------------------------------------

device_bbc_userport_interface::device_bbc_userport_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_userport_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_userport_interface - destructor
//-------------------------------------------------

device_bbc_userport_interface::~device_bbc_userport_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_userport_slot_device - constructor
//-------------------------------------------------

bbc_userport_device::bbc_userport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BBC_USERPORT_SLOT, "BBC Micro User port", tag, owner, clock, "bbc_userport_slot", __FILE__),
		device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  ~bbc_userport_slot_device - destructor
//-------------------------------------------------

//bbc_userport_device::~bbc_userport_device()
//{
//}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_userport_device::device_start()
{
	m_device = dynamic_cast<device_bbc_userport_interface *>(get_card_device());
}


UINT8 bbc_userport_device::read_portb()
{
	UINT8 data = 0xff;
	if (m_device)
		data |= m_device->read_portb();
	return data;
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_userport_devices )
//-------------------------------------------------


// slot devices
//#include "amxmouse.h"
//#include "tracker.h"


SLOT_INTERFACE_START( bbc_userport_devices )
//  SLOT_INTERFACE("amxmouse",  BBC_AMXMOUSE)       /* AMX Mouse */
//  SLOT_INTERFACE("tracker",   BBC_TRACKER)         /* Acorn Tracker Ball */
//  SLOT_INTERFACE("music4000", BBC_MUSIC4000)       /* Hybrid Music 4000 Keyboard */
SLOT_INTERFACE_END
