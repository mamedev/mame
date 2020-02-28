// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel Multibus I/O Expansion Bus IEEE-P959 (iSBX) emulation

**********************************************************************/

#include "emu.h"
#include "isbx.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ISBX_SLOT, isbx_slot_device, "isbx_slot", "iSBX bus slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_isbx_card_interface - constructor
//-------------------------------------------------

device_isbx_card_interface::device_isbx_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "isbxbus")
{
	m_slot = dynamic_cast<isbx_slot_device *>(device.owner());
}


//-------------------------------------------------
//  isbx_slot_device - constructor
//-------------------------------------------------

isbx_slot_device::isbx_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISBX_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_isbx_card_interface>(mconfig, *this),
	m_write_mintr0(*this),
	m_write_mintr1(*this),
	m_write_mdrqt(*this),
	m_write_mwait(*this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isbx_slot_device::device_start()
{
	m_card = get_card_device();

	// resolve callbacks
	m_write_mintr0.resolve_safe();
	m_write_mintr1.resolve_safe();
	m_write_mdrqt.resolve_safe();
	m_write_mwait.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( isbx_cards )
//-------------------------------------------------

// slot devices
#include "compis_fdc.h"
#include "isbc_218a.h"

void isbx_cards(device_slot_interface &device)
{
	device.option_add("fdc", COMPIS_FDC);
	device.option_add("fdc_218a", ISBC_218A);
}
