// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Enterprise Sixty Four / One Two Eight Expansion Bus emulation

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EP64_EXPANSION_BUS_SLOT = &device_creator<ep64_expansion_bus_slot_device>;



//**************************************************************************
//  DEVICE EP64_EXPANSION_BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ep64_expansion_bus_card_interface - constructor
//-------------------------------------------------

device_ep64_expansion_bus_card_interface::device_ep64_expansion_bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<ep64_expansion_bus_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ep64_expansion_bus_slot_device - constructor
//-------------------------------------------------

ep64_expansion_bus_slot_device::ep64_expansion_bus_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EP64_EXPANSION_BUS_SLOT, "Enterprise Sixty Four expansion bus slot", tag, owner, clock, "ep64_expansion_bus_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_wait(*this),
	m_dave(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ep64_expansion_bus_slot_device::device_start()
{
	m_card = dynamic_cast<device_ep64_expansion_bus_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_wait.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ep64_expansion_bus_slot_device::device_reset()
{
	if (m_card) get_card_device()->reset();
}


//-------------------------------------------------
//  SLOT_INTERFACE( ep64_expansion_bus_cards )
//-------------------------------------------------

// slot devices
#include "exdos.h"

SLOT_INTERFACE_START( ep64_expansion_bus_cards )
	SLOT_INTERFACE("exdos", EP64_EXDOS)
SLOT_INTERFACE_END
