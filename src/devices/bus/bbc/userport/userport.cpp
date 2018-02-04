// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC User Port emulation

**********************************************************************/

#include "emu.h"
#include "userport.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_USERPORT_SLOT, bbc_userport_slot_device, "bbc_userport_slot", "BBC Micro User port")



//**************************************************************************
//  DEVICE BBC_USERPORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_userport_interface - constructor
//-------------------------------------------------

device_bbc_userport_interface::device_bbc_userport_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_userport_slot_device *>(device.owner());
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

bbc_userport_slot_device::bbc_userport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BBC_USERPORT_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr),
	m_cb1_handler(*this),
	m_cb2_handler(*this)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_userport_slot_device::device_start()
{
	m_device = dynamic_cast<device_bbc_userport_interface *>(get_card_device());

	// resolve callbacks
	m_cb1_handler.resolve_safe();
	m_cb2_handler.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_userport_slot_device::device_reset()
{
}


//-------------------------------------------------
//  pb_r
//-------------------------------------------------

READ8_MEMBER(bbc_userport_slot_device::pb_r)
{
	if (m_device)
		return m_device->pb_r(space, 0);
	else
		return 0xff;
}


//-------------------------------------------------
//  pb_w
//-------------------------------------------------

WRITE8_MEMBER(bbc_userport_slot_device::pb_w)
{
	if (m_device)
		m_device->pb_w(space, 0, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_userport_devices )
//-------------------------------------------------


// slot devices
//#include "mouse.h"
#include "cfa3000kbd.h"


SLOT_INTERFACE_START( bbc_userport_devices )
//  SLOT_INTERFACE("amxmouse",   BBC_AMXMOUSE)        /* AMX Mouse */
//  SLOT_INTERFACE("m512mouse",  BBC_M512MOUSE)       /* Acorn Mouse (provided with Master 512) */
//  SLOT_INTERFACE("tracker",   BBC_TRACKER)         /* Marconi RB2 Tracker Ball / Acorn Tracker Ball */
//  SLOT_INTERFACE("music4000", BBC_MUSIC4000)       /* Hybrid Music 4000 Keyboard */
	SLOT_INTERFACE("cfa3000kbd", CFA3000_KBD)         /* Henson CFA 3000 Keyboard */
SLOT_INTERFACE_END
