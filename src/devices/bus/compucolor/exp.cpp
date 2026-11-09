// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor II I/O expansion slot

**********************************************************************/

#include "emu.h"
#include "exp.h"

#include "joysnd.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMPUCOLOR_EXPANSION_SLOT, compucolor_expansion_slot_device, "compclr_exp", "Compucolor II I/O expansion slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  compucolor_expansion_slot_device - constructor
//-------------------------------------------------

compucolor_expansion_slot_device::compucolor_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COMPUCOLOR_EXPANSION_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_compucolor_expansion_card_interface>(mconfig, *this)
	, m_io(*this, finder_base::DUMMY_TAG, -1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void compucolor_expansion_slot_device::device_start()
{
}


//-------------------------------------------------
//  device_compucolor_expansion_card_interface - constructor
//-------------------------------------------------

device_compucolor_expansion_card_interface::device_compucolor_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "compclrexp")
	, m_slot(dynamic_cast<compucolor_expansion_slot_device *>(device.owner()))
{
}


//-------------------------------------------------
//  compucolor_expansion_cards - slot options
//-------------------------------------------------

void compucolor_expansion_cards(device_slot_interface &device)
{
	device.option_add("joysnd", COMPUCOLOR_JOYSND);
}
