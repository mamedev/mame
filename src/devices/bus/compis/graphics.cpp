// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis graphics bus emulation

**********************************************************************/

#include "emu.h"
#include "graphics.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPIS_GRAPHICS_SLOT, compis_graphics_slot_device, "compisgfx_slot", "Compis graphics slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_compis_graphics_card_interface - constructor
//-------------------------------------------------

device_compis_graphics_card_interface::device_compis_graphics_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<compis_graphics_slot_device *>(device.owner());
}


//-------------------------------------------------
//  compis_graphics_slot_device - constructor
//-------------------------------------------------

compis_graphics_slot_device::compis_graphics_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMPIS_GRAPHICS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_write_dma_request(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compis_graphics_slot_device::device_start()
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
	SLOT_INTERFACE("uhrg", COMPIS_UHRG)
SLOT_INTERFACE_END
