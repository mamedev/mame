// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel iSBX bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "isbx.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISBX_SLOT = &device_creator<isbx_slot_device>;



//**************************************************************************
//  DEVICE C64_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_isbx_card_interface - constructor
//-------------------------------------------------

device_isbx_card_interface::device_isbx_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<isbx_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_isbx_card_interface - destructor
//-------------------------------------------------

device_isbx_card_interface::~device_isbx_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isbx_slot_device - constructor
//-------------------------------------------------

isbx_slot_device::isbx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISBX_SLOT, "iSBX bus slot", tag, owner, clock, "isbx_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isbx_slot_device::device_start()
{
	m_card = dynamic_cast<device_isbx_card_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isbx_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( isbx_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( isbx_cards )
SLOT_INTERFACE_END
