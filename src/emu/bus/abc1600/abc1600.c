// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Expansion Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "abc1600.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ABC1600BUS_SLOT = &device_creator<abc1600bus_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_abc1600bus_card_interface - constructor
//-------------------------------------------------

device_abc1600bus_card_interface::device_abc1600bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<abc1600bus_slot_device *>(device.owner());
}


//-------------------------------------------------
//  abc1600bus_slot_device - constructor
//-------------------------------------------------

abc1600bus_slot_device::abc1600bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC1600BUS_SLOT, "ABC 1600 bus slot", tag, owner, clock, "abc1600bus_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_int(CLEAR_LINE),
	m_pren(1),
	m_trrq(1),
	m_nmi(CLEAR_LINE),
	m_xint2(CLEAR_LINE),
	m_xint3(CLEAR_LINE),
	m_xint4(CLEAR_LINE),
	m_xint5(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void abc1600bus_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const abc1600bus_interface *intf = reinterpret_cast<const abc1600bus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<abc1600bus_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_pren_cb, 0, sizeof(m_out_pren_cb));
		memset(&m_out_trrq_cb, 0, sizeof(m_out_trrq_cb));
		memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
		memset(&m_out_xint2_cb, 0, sizeof(m_out_xint2_cb));
		memset(&m_out_xint3_cb, 0, sizeof(m_out_xint3_cb));
		memset(&m_out_xint4_cb, 0, sizeof(m_out_xint4_cb));
		memset(&m_out_xint5_cb, 0, sizeof(m_out_xint5_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600bus_slot_device::device_start()
{
	m_card = dynamic_cast<device_abc1600bus_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_pren_func.resolve(m_out_pren_cb, *this);
	m_out_trrq_func.resolve(m_out_trrq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_xint2_func.resolve(m_out_xint2_cb, *this);
	m_out_xint3_func.resolve(m_out_xint3_cb, *this);
	m_out_xint4_func.resolve(m_out_xint4_cb, *this);
	m_out_xint5_func.resolve(m_out_xint5_cb, *this);

	// state saving
	save_item(NAME(m_int));
	save_item(NAME(m_pren));
	save_item(NAME(m_trrq));
	save_item(NAME(m_nmi));
	save_item(NAME(m_xint2));
	save_item(NAME(m_xint3));
	save_item(NAME(m_xint4));
	save_item(NAME(m_xint5));
}


//-------------------------------------------------
//  SLOT_INTERFACE( abc1600bus_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abc1600bus_cards )
	SLOT_INTERFACE("4105", LUXOR_4105) // SASI interface
//  SLOT_INTERFACE("4077", LUXOR_4077) // Winchester controller
//  SLOT_INTERFACE("4004", LUXOR_4004) // ICOM I/O (Z80, Z80PIO, Z80SIO/2, Z80CTC, 2 Z80DMAs, 2 PROMs, 64KB RAM)
	SLOT_INTERFACE("fast", LUXOR_55_21046)
SLOT_INTERFACE_END
