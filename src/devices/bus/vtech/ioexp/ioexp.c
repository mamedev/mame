// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot

    30-pin slot

***************************************************************************/

#include "ioexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type IOEXP_SLOT = &device_creator<ioexp_slot_device>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  ioexp_slot_device - constructor
//-------------------------------------------------

ioexp_slot_device::ioexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, IOEXP_SLOT, "Peripheral Expansion Slot", tag, owner, clock, "ioexp_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_io(NULL),
	m_cart(NULL)
{
}

//-------------------------------------------------
//  ioexp_slot_device - destructor
//-------------------------------------------------

ioexp_slot_device::~ioexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ioexp_slot_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ioexp_slot_device::device_reset()
{
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void ioexp_slot_device::set_io_space(address_space *io)
{
	m_io = io;
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ioexp_interface - constructor
//-------------------------------------------------

device_ioexp_interface::device_ioexp_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<ioexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_ioexp_interface - destructor
//-------------------------------------------------

device_ioexp_interface::~device_ioexp_interface()
{
}
