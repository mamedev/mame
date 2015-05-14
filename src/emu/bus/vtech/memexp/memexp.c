// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot

    44-pin slot

***************************************************************************/

#include "memexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MEMEXP_SLOT = &device_creator<memexp_slot_device>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  memexp_slot_device - constructor
//-------------------------------------------------

memexp_slot_device::memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MEMEXP_SLOT, "Memory Expansion Slot", tag, owner, clock, "memexp_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_program(NULL),
	m_io(NULL),
	m_cart(NULL),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}

//-------------------------------------------------
//  memexp_slot_device - destructor
//-------------------------------------------------

memexp_slot_device::~memexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void memexp_slot_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void memexp_slot_device::device_reset()
{
}

//-------------------------------------------------
//  set_program_space - set address space we are attached to
//-------------------------------------------------

void memexp_slot_device::set_program_space(address_space *program)
{
	m_program = program;
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void memexp_slot_device::set_io_space(address_space *io)
{
	m_io = io;
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_memexp_interface - constructor
//-------------------------------------------------

device_memexp_interface::device_memexp_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<memexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_memexp_interface - destructor
//-------------------------------------------------

device_memexp_interface::~device_memexp_interface()
{
}
