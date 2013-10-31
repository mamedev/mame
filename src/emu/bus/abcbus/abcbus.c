// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC (Databoard 4680) Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "abcbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ABCBUS_SLOT = &device_creator<abcbus_slot_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_abcbus_card_interface - constructor
//-------------------------------------------------

device_abcbus_card_interface::device_abcbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<abcbus_slot_device *>(device.owner());
}


//-------------------------------------------------
//  abcbus_slot_device - constructor
//-------------------------------------------------

abcbus_slot_device::abcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABCBUS_SLOT, "ABCBUS slot", tag, owner, clock, "abcbus_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_int(*this),
	m_write_nmi(*this),
	m_write_rdy(*this),
	m_write_resin(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abcbus_slot_device::device_start()
{
	m_card = dynamic_cast<device_abcbus_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_int.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_rdy.resolve_safe();
	m_write_resin.resolve_safe();
}


//-------------------------------------------------
//  SLOT_INTERFACE( abcbus_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abcbus_cards )
	SLOT_INTERFACE("exp", ABC890)
	SLOT_INTERFACE("exp3", ABC894)
	SLOT_INTERFACE("hdc", ABC_HDC)
	SLOT_INTERFACE("hdd", ABC850)
	SLOT_INTERFACE("dos", ABC_DOS)
	SLOT_INTERFACE("fd2", ABC_FD2)
	SLOT_INTERFACE("sio", ABC_SIO)
	SLOT_INTERFACE("slow", LUXOR_55_10828)
	SLOT_INTERFACE("fast", LUXOR_55_21046)
	SLOT_INTERFACE("uni800", ABC_UNI800)
	SLOT_INTERFACE("slutprov", ABC_SLUTPROV)
	SLOT_INTERFACE("turbo", TURBO_KONTROLLER)
	SLOT_INTERFACE("xebec", LUXOR_55_21056)
SLOT_INTERFACE_END
