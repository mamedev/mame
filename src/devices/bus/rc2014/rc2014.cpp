// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Bus Device

***************************************************************************/

#include "emu.h"
#include "rc2014.h"

//**************************************************************************
//  RC2014 Standard Bus
//**************************************************************************

//-------------------------------------------------
//  rc2014_bus_device
//-------------------------------------------------

rc2014_bus_device::rc2014_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_installer{}
	, m_clk(*this)
	, m_int(*this)
	, m_tx(*this)
	, m_rx(*this)
	, m_user1(*this)
	, m_user2(*this)
	, m_user3(*this)
	, m_user4(*this)
	, m_daisy_chain{}
{
}

rc2014_bus_device::rc2014_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_bus_device(mconfig, RC2014_BUS, tag, owner, clock)
{
}

rc2014_bus_device::~rc2014_bus_device()
{
	for(size_t i = 0; i < m_daisy.size(); i++)
		delete [] m_daisy_chain[i];
	delete [] m_daisy_chain;
}

void rc2014_bus_device::device_start()
{
	// resolve callbacks
	m_clk.resolve_safe();
	m_int.resolve_safe();
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
	assert(index >= 0 && index < 4);
	if (m_installer[index] == nullptr )
		throw emu_fatalerror("Address installer not set on RC2014 bus !!! Add CPU module.");
	return m_installer[index];
}

const z80_daisy_config* rc2014_bus_device::get_daisy_chain()
{
	m_daisy_chain = new char*[m_daisy.size() + 1];
	for(size_t i = 0; i < m_daisy.size(); i++)
	{
		m_daisy_chain[i] = new char[m_daisy[i].size() + 1];
		strcpy(m_daisy_chain[i], m_daisy[i].c_str());
	}
	m_daisy_chain[m_daisy.size()] = nullptr;
	return (const z80_daisy_config*)m_daisy_chain;
}

//-------------------------------------------------
//  device_rc2014_card_interface
//-------------------------------------------------

device_rc2014_card_interface::device_rc2014_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rc2014bus")
	, m_bus(nullptr)
{
}

void device_rc2014_card_interface::set_bus_device(rc2014_bus_device *bus_device)
{
	m_bus = bus_device;
}

//-------------------------------------------------
//  rc2014_slot_device
//-------------------------------------------------

rc2014_slot_device::rc2014_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}

rc2014_slot_device::rc2014_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_slot_device(mconfig, RC2014_SLOT, tag, owner, clock)
{
}

void rc2014_slot_device::device_start()
{
}

void rc2014_slot_device::device_resolve_objects()
{
	device_rc2014_card_interface *const card(dynamic_cast<device_rc2014_card_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(m_bus);
}

//**************************************************************************
//  RC2014 Extended Bus
//**************************************************************************

//-------------------------------------------------
//  rc2014_ext_bus_device
//-------------------------------------------------

rc2014_ext_bus_device::rc2014_ext_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_ext_bus_device(mconfig, RC2014_EXT_BUS, tag, owner, clock)
{
}

rc2014_ext_bus_device::rc2014_ext_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_bus_device(mconfig, type, tag, owner, clock)
	, m_clk2(*this)
	, m_page(*this)
	, m_nmi(*this)
	, m_tx2(*this)
	, m_rx2(*this)
	, m_user5(*this)
	, m_user6(*this)
	, m_user7(*this)
	, m_user8(*this)
{
}

void rc2014_ext_bus_device::device_start()
{
	rc2014_bus_device::device_start();
	m_clk2.resolve_safe();
	m_page.resolve_safe();
	m_nmi.resolve_safe();
	m_tx2.resolve_safe();
	m_rx2.resolve_safe();
	m_user5.resolve_safe();
	m_user6.resolve_safe();
	m_user7.resolve_safe();
	m_user8.resolve_safe();
}

//-------------------------------------------------
//  device_rc2014_ext_card_interface
//-------------------------------------------------

device_rc2014_ext_card_interface::device_rc2014_ext_card_interface(const machine_config &mconfig, device_t &device)
	: device_rc2014_card_interface(mconfig,device)
	, m_bus(nullptr)
{
}

void device_rc2014_ext_card_interface::set_bus_device(rc2014_ext_bus_device *bus_device)
{
	m_bus = bus_device;
}

//-------------------------------------------------
//  rc2014_ext_slot_device
//-------------------------------------------------

rc2014_ext_slot_device::rc2014_ext_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_ext_slot_device(mconfig, RC2014_EXT_SLOT, tag, owner, clock)
{
}

rc2014_ext_slot_device::rc2014_ext_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_slot_device(mconfig, type, tag, owner, clock)
{
}

void rc2014_ext_slot_device::device_start()
{
}

void rc2014_ext_slot_device::device_resolve_objects()
{
	rc2014_slot_device::device_resolve_objects();
	device_rc2014_ext_card_interface *const card(dynamic_cast<device_rc2014_ext_card_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(dynamic_cast<rc2014_ext_bus_device *>(m_bus.target()));
}

//**************************************************************************
//  RC2014 RC80 Bus
//**************************************************************************

//-------------------------------------------------
//  rc2014_rc80_bus_device
//-------------------------------------------------

rc2014_rc80_bus_device::rc2014_rc80_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_rc80_bus_device(mconfig, RC2014_RC80_BUS, tag, owner, clock)
{
}

rc2014_rc80_bus_device::rc2014_rc80_bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_ext_bus_device(mconfig, type, tag, owner, clock)

{
}

void rc2014_rc80_bus_device::device_start()
{
	rc2014_ext_bus_device::device_start();
}

//-------------------------------------------------
//  device_rc2014_rc80_card_interface
//-------------------------------------------------

device_rc2014_rc80_card_interface::device_rc2014_rc80_card_interface(const machine_config &mconfig, device_t &device)
	: device_rc2014_ext_card_interface(mconfig,device)
	, m_bus(nullptr)
{
}

void device_rc2014_rc80_card_interface::set_bus_device(rc2014_rc80_bus_device *bus_device)
{
	m_bus = bus_device;
}

//-------------------------------------------------
//  rc2014_rc80_slot_device
//-------------------------------------------------

rc2014_rc80_slot_device::rc2014_rc80_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rc2014_ext_slot_device(mconfig, RC2014_RC80_SLOT, tag, owner, clock)
{
}

void rc2014_rc80_slot_device::device_start()
{
}

void rc2014_rc80_slot_device::device_resolve_objects()
{
	rc2014_ext_slot_device::device_resolve_objects();
	device_rc2014_rc80_card_interface *const card(dynamic_cast<device_rc2014_rc80_card_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(dynamic_cast<rc2014_rc80_bus_device *>(m_bus.target()));
}

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RC2014_BUS,  rc2014_bus_device,  "rc2014_bus",  "RC2014 Standard Bus")
DEFINE_DEVICE_TYPE(RC2014_SLOT, rc2014_slot_device, "rc2014_slot", "RC2014 Standard Bus Slot")

DEFINE_DEVICE_TYPE(RC2014_EXT_BUS,  rc2014_ext_bus_device,  "rc2014_ext_bus",  "RC2014 Extended Bus")
DEFINE_DEVICE_TYPE(RC2014_EXT_SLOT, rc2014_ext_slot_device, "rc2014_ext_slot", "RC2014 Extended Bus Slot")

DEFINE_DEVICE_TYPE(RC2014_RC80_BUS,  rc2014_rc80_bus_device,  "rc2014_rc80_bus",  "RC2014 RC80 Bus")
DEFINE_DEVICE_TYPE(RC2014_RC80_SLOT, rc2014_rc80_slot_device, "rc2014_rc80_slot", "RC2014 RC80 Bus Slot")
