// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis graphics bus emulation

**********************************************************************/

#include "graphics.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type COMPIS_GRAPHICS_SLOT = &device_creator<compis_graphics_slot_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_compis_graphics_card_interface - constructor
//-------------------------------------------------

device_compis_graphics_card_interface::device_compis_graphics_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<compis_graphics_slot_t *>(device.owner());
}


//-------------------------------------------------
//  compis_graphics_slot_t - constructor
//-------------------------------------------------

compis_graphics_slot_t::compis_graphics_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, COMPIS_GRAPHICS_SLOT, "Compis graphics slot", tag, owner, clock, "isbx_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_dma_request(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_graphics_slot_t::device_start()
{
	m_card = dynamic_cast<device_compis_graphics_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_dma_request.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( compis_graphics_cards )
//-------------------------------------------------

// slot devices
#include "hrg.h"

SLOT_INTERFACE_START( compis_graphics_cards )
	SLOT_INTERFACE("hrg", COMPIS_HRG)
SLOT_INTERFACE_END
