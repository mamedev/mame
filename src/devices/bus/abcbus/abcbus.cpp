// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC (Databoard 4680) Bus emulation

**********************************************************************/

#include "abcbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ABCBUS_SLOT = &device_creator<abcbus_slot_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_abcbus_card_interface - constructor
//-------------------------------------------------

device_abcbus_card_interface::device_abcbus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<abcbus_slot_t *>(device.owner());
}


//-------------------------------------------------
//  abcbus_slot_t - constructor
//-------------------------------------------------

abcbus_slot_t::abcbus_slot_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABCBUS_SLOT, "ABCBUS slot", tag, owner, clock, "abcbus_slot", __FILE__),
	device_slot_interface(mconfig, *this),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_rdy(*this),
	m_write_resin(*this),
	m_write_pren(*this),
	m_write_trrq(*this),
	m_write_xint2(*this),
	m_write_xint3(*this),
	m_write_xint4(*this),
	m_write_xint5(*this), m_card(nullptr), m_irq(0), m_nmi(0), m_pren(0),
	m_trrq(0), m_xint2(0), m_xint3(0), m_xint4(0), m_xint5(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abcbus_slot_t::device_start()
{
	m_card = dynamic_cast<device_abcbus_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_rdy.resolve_safe();
	m_write_resin.resolve_safe();
	m_write_pren.resolve_safe();
	m_write_trrq.resolve_safe();
	m_write_xint2.resolve_safe();
	m_write_xint3.resolve_safe();
	m_write_xint4.resolve_safe();
	m_write_xint5.resolve_safe();
}


// slot devices
#include "abc890.h"
#include "fd2.h"
#include "hdc.h"
#include "lux10828.h"
#include "lux21046.h"
#include "lux21056.h"
#include "lux4105.h"
#include "memcard.h"
#include "ram.h"
#include "sio.h"
#include "slutprov.h"
#include "turbo.h"
#include "uni800.h"



//-------------------------------------------------
//  SLOT_INTERFACE( abc80_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abc80_cards )
	SLOT_INTERFACE("fd2", ABC_FD2)
	SLOT_INTERFACE("memcard", ABC_MEMORY_CARD)
	SLOT_INTERFACE("abcexp", ABC_EXPANSION_UNIT)
	SLOT_INTERFACE("16k", ABC80_16KB_RAM_CARD)
	SLOT_INTERFACE("slow", LUXOR_55_10828)
	SLOT_INTERFACE("abc830", ABC830)
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( abcbus_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abcbus_cards )
	SLOT_INTERFACE("abc830", ABC830)
	SLOT_INTERFACE("abc832", ABC832)
	SLOT_INTERFACE("abc834", ABC834)
	SLOT_INTERFACE("abc838", ABC838)
	SLOT_INTERFACE("abc850", ABC850)
	SLOT_INTERFACE_INTERNAL("abc850fdd", ABC850_FLOPPY)
	SLOT_INTERFACE("abc852", ABC852)
	SLOT_INTERFACE("abc856", ABC856)
	SLOT_INTERFACE("abc890", ABC890)
	SLOT_INTERFACE("abc894", ABC894)
	SLOT_INTERFACE("hdc", ABC_HDC)
	SLOT_INTERFACE("sio", ABC_SIO)
	SLOT_INTERFACE("slow", LUXOR_55_10828)
	SLOT_INTERFACE("uni800", ABC_UNI800)
	SLOT_INTERFACE("slutprov", ABC_SLUTPROV)
	SLOT_INTERFACE("turbo", TURBO_KONTROLLER)
	SLOT_INTERFACE("xebec", LUXOR_55_21056)
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( abc1600bus_cards )
//-------------------------------------------------

SLOT_INTERFACE_START( abc1600bus_cards )
	SLOT_INTERFACE("4105", LUXOR_4105) // SASI interface
//  SLOT_INTERFACE("4077", LUXOR_4077) // Winchester controller
//  SLOT_INTERFACE("4004", LUXOR_4004) // ICOM I/O (Z80, Z80PIO, Z80SIO/2, Z80CTC, 2 Z80DMAs, 2 PROMs, 64KB RAM)
SLOT_INTERFACE_END
