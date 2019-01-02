// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    International Memories Incorporated IMI 7000 Series bus emulation

**********************************************************************/

#include "emu.h"
#include "imi7000.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(IMI7000_BUS,  imi7000_bus_device,  "imi7000",      "IMI7000 bus")
DEFINE_DEVICE_TYPE(IMI7000_SLOT, imi7000_slot_device, "imi7000_slot", "IMI7000 slot")



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

imi7000_slot_device::imi7000_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IMI7000_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_card(nullptr)
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

imi7000_bus_device::imi7000_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IMI7000_BUS, tag, owner, clock)
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

void imi7000_devices(device_slot_interface &device)
{
	device.option_add("imi5000h", IMI5000H);
}
