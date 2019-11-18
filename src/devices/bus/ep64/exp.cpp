// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight Expansion Bus emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EP64_EXPANSION_BUS_SLOT, ep64_expansion_bus_slot_device, "ep64_expansion_bus_slot", "Enterprise Sixty Four expansion bus slot")



//**************************************************************************
//  DEVICE EP64_EXPANSION_BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ep64_expansion_bus_card_interface - constructor
//-------------------------------------------------

device_ep64_expansion_bus_card_interface::device_ep64_expansion_bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "ep64exp")
{
	m_slot = dynamic_cast<ep64_expansion_bus_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ep64_expansion_bus_slot_device - constructor
//-------------------------------------------------

ep64_expansion_bus_slot_device::ep64_expansion_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EP64_EXPANSION_BUS_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_ep64_expansion_bus_card_interface>(mconfig, *this)
	, m_write_irq(*this)
	, m_write_nmi(*this)
	, m_write_wait(*this)
	, m_program_space(*this, finder_base::DUMMY_TAG, -1)
	, m_io_space(*this, finder_base::DUMMY_TAG, -1)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ep64_expansion_bus_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_wait.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( ep64_expansion_bus_cards )
//-------------------------------------------------

// slot devices
#include "exdos.h"

void ep64_expansion_bus_cards(device_slot_interface &device)
{
	device.option_add("exdos", EP64_EXDOS);
}
