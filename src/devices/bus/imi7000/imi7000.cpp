// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    International Memories Incorporated IMI 7000 Series bus emulation

**********************************************************************/

#include "imi7000.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IMI7000_BUS = &device_creator<imi7000_bus_device>;
const device_type IMI7000_SLOT = &device_creator<imi7000_slot_device>;



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_imi7000_interface - constructor
//-------------------------------------------------

device_imi7000_interface::device_imi7000_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device), m_slot(nullptr)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  imi7000_slot_device - constructor
//-------------------------------------------------

imi7000_slot_device::imi7000_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, IMI7000_SLOT, "IMI 7000 slot", tag, owner, clock, "imi7000_slot", __FILE__),
		device_slot_interface(mconfig, *this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imi7000_slot_device::device_start()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  imi7000_bus_device - constructor
//-------------------------------------------------

imi7000_bus_device::imi7000_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IMI7000_BUS, "IMI 7000 bus", tag, owner, clock, "imi7000", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void imi7000_bus_device::device_start()
{
}


//-------------------------------------------------
//  SLOT_INTERFACE( imi7000_devices )
//-------------------------------------------------

#include "imi5000h.h"

SLOT_INTERFACE_START( imi7000_devices )
	SLOT_INTERFACE("imi5000h", IMI5000H)
SLOT_INTERFACE_END
