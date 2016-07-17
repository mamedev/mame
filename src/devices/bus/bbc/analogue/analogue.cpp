// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC analogue port emulation

**********************************************************************/

#include "analogue.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BBC_ANALOGUE_SLOT = &device_creator<bbc_analogue_slot_device>;



//**************************************************************************
//  DEVICE BBC_ANALOGUE PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bbc_analogue_interface - constructor
//-------------------------------------------------

device_bbc_analogue_interface::device_bbc_analogue_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<bbc_analogue_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bbc_analogue_interface - destructor
//-------------------------------------------------

device_bbc_analogue_interface::~device_bbc_analogue_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_analogue_slot_device - constructor
//-------------------------------------------------

bbc_analogue_slot_device::bbc_analogue_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, BBC_ANALOGUE_SLOT, "BBC Micro Analogue port", tag, owner, clock, "bbc_analogue_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_analogue_slot_device::device_start()
{
	m_card = dynamic_cast<device_bbc_analogue_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_analogue_slot_device::device_reset()
{
	if (get_card_device())
	{
		get_card_device()->reset();
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( bbc_analogue_devices )
//-------------------------------------------------


// slot devices
//#include "joystick.h"
//#include "quinkey.h"


SLOT_INTERFACE_START( bbc_analogue_devices )
//	SLOT_INTERFACE("joystick",  BBC_JOYSTICKS)        /* Acorn ANH01 BBC Micro Joysticks */
//	SLOT_INTERFACE("quinkey",   BBC_QUINKEY)          /* Microwriter Quinkey */
SLOT_INTERFACE_END
