// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        RC2014 bus device

***************************************************************************/

#include "emu.h"
#include "rc2014.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RC2014_BUS,  rc2014_bus_device,  "rc2014_bus",  "rc2014 bus")
DEFINE_DEVICE_TYPE(RC2014_SLOT, rc2014_slot_device, "rc2014_slot", "rc2014 slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rc2014_bus_device 
//-------------------------------------------------

rc2014_bus_device::rc2014_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RC2014_BUS, tag, owner, clock)
	, m_installer{}
	, m_int(*this)
	, m_nmi(*this)
	, m_tx(*this)
	, m_rx(*this)
	, m_user1(*this)
	, m_user2(*this)
	, m_user3(*this)
	, m_user4(*this)
{
}

void rc2014_bus_device::device_start()
{
	if (m_installer[AS_PROGRAM] == nullptr)
		throw emu_fatalerror("Main address installer missing on RC2014 bus !!!");
	// resolve callbacks
	m_int.resolve_safe();
	m_nmi.resolve_safe();
	m_tx.resolve_safe();
	m_rx.resolve_safe();
	m_user1.resolve_safe();
	m_user2.resolve_safe();
	m_user3.resolve_safe();
	m_user4.resolve_safe();
}

void rc2014_bus_device::set_bus_clock(u32 clock)
{
	set_clock(clock);
	notify_clock_changed();
}

void rc2014_bus_device::assign_installer(int index, address_space_installer *installer)
{
	if (m_installer[index] != nullptr )
		throw emu_fatalerror("Address installer already set on RC2014 bus !!!");
	m_installer[index]  = installer;
}

address_space_installer *rc2014_bus_device::installer(int index) const
{
	assert(index >= 0 && index < 4 && m_installer[index]); 
	return m_installer[index];
}

//-------------------------------------------------
//  device_rc2014_card_interface 
//-------------------------------------------------

device_rc2014_card_interface::device_rc2014_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rc2014bus")
	, m_bus(nullptr)
{
}

void device_rc2014_card_interface::set_bus_device(rc2014_bus_device &bus_device)
{
	m_bus = &bus_device;
}

//-------------------------------------------------
//  rc2014_slot_device
//-------------------------------------------------

rc2014_slot_device::rc2014_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RC2014_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_rc2014_card_interface>(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

void rc2014_slot_device::device_start()
{
}

void rc2014_slot_device::device_resolve_objects()
{
	device_rc2014_card_interface *const card(dynamic_cast<device_rc2014_card_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(*m_bus);
}
