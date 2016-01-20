// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Expansion Slot

    50-pin slot

***************************************************************************/

#include "expansion.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EXPANSION_SLOT = &device_creator<expansion_slot_device>;


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  expansion_slot_device - constructor
//-------------------------------------------------

expansion_slot_device::expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EXPANSION_SLOT, "Expansion Slot", tag, owner, clock, "expansion_slot", __FILE__),
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
//  expansion_slot_device - destructor
//-------------------------------------------------

expansion_slot_device::~expansion_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void expansion_slot_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_nmi_handler.resolve_safe();
	m_reset_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void expansion_slot_device::device_reset()
{
}

//-------------------------------------------------
//  set_program_space - set address space we are attached to
//-------------------------------------------------

void expansion_slot_device::set_program_space(address_space *program)
{
	m_program = program;
}

//-------------------------------------------------
//  set_io_space - set address space we are attached to
//-------------------------------------------------

void expansion_slot_device::set_io_space(address_space *io)
{
	m_io = io;
}


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_expansion_interface - constructor
//-------------------------------------------------

device_expansion_interface::device_expansion_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<expansion_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_expansion_interface - destructor
//-------------------------------------------------

device_expansion_interface::~device_expansion_interface()
{
}
