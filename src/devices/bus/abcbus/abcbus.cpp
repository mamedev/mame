// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC (Databoard 4680) Bus emulation

**********************************************************************/

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ABCBUS_SLOT, abcbus_slot_device, "abcbus_slot", "ABCBUS slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_abcbus_card_interface - constructor
//-------------------------------------------------

device_abcbus_card_interface::device_abcbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "abcbus")
{
	m_slot = dynamic_cast<abcbus_slot_device *>(device.owner());
}


//-------------------------------------------------
//  abcbus_slot_device - constructor
//-------------------------------------------------

abcbus_slot_device::abcbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABCBUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_abcbus_card_interface>(mconfig, *this),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_rdy(*this),
	m_write_resin(*this),
	m_write_pren(*this),
	m_write_trrq(*this),
	m_write_xint2(*this),
	m_write_xint3(*this),
	m_write_xint4(*this),
	m_write_xint5(*this),
	m_card(nullptr),
	m_irq(1),
	m_nmi(1),
	m_pren(1),
	m_trrq(1),
	m_xint2(1),
	m_xint3(1),
	m_xint4(1),
	m_xint5(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abcbus_slot_device::device_start()
{
	m_card = get_card_device();

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


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abcbus_slot_device::device_reset()
{
	m_write_irq(1);
	m_write_pren(1);
	m_write_trrq(1);
}


// slot devices
#include "abc890.h"
#include "cadmouse.h"
#include "db4106.h"
#include "db4107.h"
#include "db4112.h"
#include "fd2.h"
#include "lux10828.h"
#include "lux21046.h"
#include "lux21056.h"
#include "lux4105.h"
#include "memcard.h"
#include "ram.h"
#include "sio.h"
#include "slutprov.h"
#include "ssa.h"
#include "uni800.h"
#include "unidisk.h"



//-------------------------------------------------
//  SLOT_INTERFACE( abc80_cards )
//-------------------------------------------------

void abc80_cards(device_slot_interface &device)
{
	device.option_add("16k", ABC80_16KB_RAM_CARD);
	device.option_add("abc830", ABC830);
	device.option_add("abcexp", ABC_EXPANSION_UNIT);
	device.option_add("cadabc", ABC_CADMOUSE);
	device.option_add("db4106", DATABOARD_4106);
	device.option_add("db4107", DATABOARD_4107);
	device.option_add("db4112", DATABOARD_4112);
	device.option_add("fd2", ABC_FD2);
	device.option_add("memcard", ABC_MEMORY_CARD);
	device.option_add("slow", LUXOR_55_10828);
	device.option_add("ssa", ABC_SUPER_SMARTAID);
	device.option_add("unidisk", ABC_UNIDISK);
}


//-------------------------------------------------
//  SLOT_INTERFACE( abcbus_cards )
//-------------------------------------------------

void abcbus_cards(device_slot_interface &device)
{
	device.option_add("abc830", ABC830);
	device.option_add("abc832", ABC832);
	device.option_add("abc834", ABC834);
	device.option_add("abc838", ABC838);
	device.option_add("abc850", ABC850);
	device.option_add_internal("abc850fdd", ABC850_FLOPPY);
	device.option_add("abc852", ABC852);
	device.option_add("abc856", ABC856);
	device.option_add("abc890", ABC890);
	device.option_add("abc894", ABC894);
	device.option_add("db4106", DATABOARD_4106);
	device.option_add("db4107", DATABOARD_4107);
	device.option_add("db4112", DATABOARD_4112);
	device.option_add("sio", ABC_SIO);
	device.option_add("slow", LUXOR_55_10828);
	device.option_add("slutprov", ABC_SLUTPROV);
	device.option_add("uni800", ABC_UNI800);
	device.option_add("unidisk", ABC_UNIDISK);
	device.option_add("xebec", LUXOR_55_21056);
}


//-------------------------------------------------
//  SLOT_INTERFACE( abc1600bus_cards )
//-------------------------------------------------

void abc1600bus_cards(device_slot_interface &device)
{
	device.option_add("4105", LUXOR_4105); // SASI interface
//  device.option_add("4077", LUXOR_4077); // Winchester controller
//  device.option_add("4004", LUXOR_4004); // ICOM I/O (Z80, Z80PIO, Z80SIO/2, Z80CTC, 2 Z80DMAs, 2 PROMs, 64KB RAM)
}
