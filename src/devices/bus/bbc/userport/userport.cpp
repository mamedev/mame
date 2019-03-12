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

uint8_t bbc_userport_slot_device::pb_r()
{
	if (m_device)
		return m_device->pb_r();
	else
		return 0xff;
}


//-------------------------------------------------
//  pb_w
//-------------------------------------------------

void bbc_userport_slot_device::pb_w(uint8_t data)
{
	if (m_device)
		m_device->pb_w(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_userport_devices )
//-------------------------------------------------


// slot devices
#include "beebspch.h"
#include "pointer.h"
#include "cfa3000kbd.h"


void bbc_userport_devices(device_slot_interface &device)
{
	device.option_add("amxmouse",   BBC_AMXMOUSE);        /* AMX Mouse */
	device.option_add("beebspch",   BBC_BEEBSPCH);        /* Beeb Speech Synthesiser (Watford Electronics) */
	device.option_add("m512mouse",  BBC_M512MOUSE);       /* Acorn Mouse (provided with Master 512) */
	device.option_add("tracker",    BBC_TRACKER);         /* Marconi RB2 Tracker Ball / Acorn Tracker Ball */
//  device.option_add("music4000",  BBC_MUSIC4000);       /* Hybrid Music 4000 Keyboard */
	device.option_add("cfa3000kbd", CFA3000_KBD);         /* Henson CFA 3000 Keyboard */
}
