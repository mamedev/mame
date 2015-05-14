// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpcexp.c  --  Amstrad CPC Expansion port
 *
 *  Created on: 16/07/2011
 *
 */


#include "emu.h"
#include "emuopts.h"
#include "cpcexp.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type CPC_EXPANSION_SLOT = &device_creator<cpc_expansion_slot_device>;


//**************************************************************************
//  DEVICE CPC_EXPANSION CARD INTERFACE
//**************************************************************************


device_cpc_expansion_card_interface::device_cpc_expansion_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
}


device_cpc_expansion_card_interface::~device_cpc_expansion_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_expansion_slot_device::cpc_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, CPC_EXPANSION_SLOT, "Amstrad CPC expansion port", tag, owner, clock, "cpc_expansion_slot", __FILE__),
		device_slot_interface(mconfig, *this),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this),
		m_out_reset_cb(*this),
		m_out_romdis_cb(*this),
		m_out_rom_select(*this)
{
}

cpc_expansion_slot_device::~cpc_expansion_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_cpc_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
	m_out_reset_cb.resolve_safe();
	m_out_romdis_cb.resolve_safe();
	m_out_rom_select.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_expansion_slot_device::device_reset()
{
}


WRITE_LINE_MEMBER( cpc_expansion_slot_device::irq_w ) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::nmi_w ) { m_out_nmi_cb(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::reset_w ) { m_out_reset_cb(state); }
WRITE_LINE_MEMBER( cpc_expansion_slot_device::romdis_w ) { m_out_romdis_cb(state); }
WRITE8_MEMBER( cpc_expansion_slot_device::rom_select ) { m_out_rom_select(data); }
