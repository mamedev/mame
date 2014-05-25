// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL expansion port emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "exp.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QL_EXPANSION_SLOT = &device_creator<ql_expansion_slot_t>;



//**************************************************************************
//  DEVICE QL_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ql_expansion_card_interface - constructor
//-------------------------------------------------

device_ql_expansion_card_interface::device_ql_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<ql_expansion_slot_t *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ql_expansion_slot_t - constructor
//-------------------------------------------------

ql_expansion_slot_t::ql_expansion_slot_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QL_EXPANSION_SLOT, "QL expansion port", tag, owner, clock, "ql_expansion_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_ipl0l(*this),
	m_write_ipl1l(*this),
	m_write_berrl(*this),
	m_write_extintl(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ql_expansion_slot_t::device_start()
{
	m_card = dynamic_cast<device_ql_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_ipl0l.resolve_safe();
	m_write_ipl1l.resolve_safe();
	m_write_berrl.resolve_safe();
	m_write_extintl.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( ql_expansion_cards )
//-------------------------------------------------

// slot devices
#include "sandy_superdisk.h"
#include "sandy_superqboard.h"
#include "trumpcard.h"

SLOT_INTERFACE_START( ql_expansion_cards )
	SLOT_INTERFACE("superdisk", SANDY_SUPER_DISK)
	SLOT_INTERFACE("superqboard", SANDY_SUPERQBOARD)
	SLOT_INTERFACE("trumpcard", QL_TRUMP_CARD)
SLOT_INTERFACE_END
