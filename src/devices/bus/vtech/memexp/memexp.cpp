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
	device_single_card_slot_interface<device_vtech_memexp_interface>(mconfig, *this),
	m_program(*this, finder_base::DUMMY_TAG, -1),
	m_io(*this, finder_base::DUMMY_TAG, -1),
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vtech_memexp_slot_device::device_config_complete()
{
	// for passthrough connectors, use the parent slot's spaces
	if (dynamic_cast<device_vtech_memexp_interface *>(owner()) != nullptr)
	{
		auto parent = dynamic_cast<vtech_memexp_slot_device *>(owner()->owner());
		if (parent != nullptr)
		{
			if (m_program.finder_tag() == finder_base::DUMMY_TAG)
				m_program.set_tag(parent->m_program, parent->m_program.spacenum());
			if (m_io.finder_tag() == finder_base::DUMMY_TAG)
				m_io.set_tag(parent->m_io, parent->m_io.spacenum());
		}
	}
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


//**************************************************************************
//  CARTRIDGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_vtech_memexp_interface - constructor
//-------------------------------------------------

device_vtech_memexp_interface::device_vtech_memexp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vtechmemexp")
{
	m_slot = dynamic_cast<vtech_memexp_slot_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_vtech_memexp_interface - destructor
//-------------------------------------------------

device_vtech_memexp_interface::~device_vtech_memexp_interface()
{
}
