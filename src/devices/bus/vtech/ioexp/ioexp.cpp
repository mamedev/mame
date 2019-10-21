// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot

    30-pin slot

***************************************************************************/

#include "emu.h"
#include "ioexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_IOEXP_SLOT, vtech_ioexp_slot_device, "vtech_ioexp_slot", "Laser/VZ I/O Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_ioexp_slot_device - constructor
//-------------------------------------------------

vtech_ioexp_slot_device::vtech_ioexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_IOEXP_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_io(*this, finder_base::DUMMY_TAG, -1),
	m_cart(nullptr)
{
}

//-------------------------------------------------
//  vtech_ioexp_slot_device - destructor
//-------------------------------------------------

vtech_ioexp_slot_device::~vtech_ioexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_ioexp_slot_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_ioexp_slot_device::device_reset()
{
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtech_ioexp_interface - constructor
//-------------------------------------------------

device_vtech_ioexp_interface::device_vtech_ioexp_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<vtech_ioexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_vtech_ioexp_interface - destructor
//-------------------------------------------------

device_vtech_ioexp_interface::~device_vtech_ioexp_interface()
{
}
