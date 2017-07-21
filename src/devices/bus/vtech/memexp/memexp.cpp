// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot

    44-pin slot

***************************************************************************/

#include "emu.h"
#include "memexp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_MEMEXP_SLOT, vtech_memexp_slot_device, "vtech_memexp_slot", "Laser/VZ Memory Expansion Slot")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_memexp_slot_device - constructor
//-------------------------------------------------

vtech_memexp_slot_device::vtech_memexp_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VTECH_MEMEXP_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_program(nullptr),
	m_io(nullptr),
	m_cart(nullptr),
	m_int_handler(*this),
	m_nmi_handler(*this),
	m_reset_handler(*this)
{
}

//-------------------------------------------------
//  vtech_memexp_slot_device - destructor
//-------------------------------------------------

vtech_memexp_slot_device::~vtech_memexp_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_memexp_slot_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vtech_memexp_slot_device::device_reset()
{
}

//-------------------------------------------------
//  set_program_space - set address space we are attached to
//-------------------------------------------------

void vtech_memexp_slot_device::set_program_space(address_space *program)
{
	m_program = program;
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void vtech_memexp_slot_device::set_io_space(address_space *io)
{
	m_io = io;
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtech_memexp_interface - constructor
//-------------------------------------------------

device_vtech_memexp_interface::device_vtech_memexp_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<vtech_memexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_vtech_memexp_interface - destructor
//-------------------------------------------------

device_vtech_memexp_interface::~device_vtech_memexp_interface()
{
}
