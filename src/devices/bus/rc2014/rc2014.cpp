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
}

void rc2014_bus_device::device_reset()
{
	if (m_installer[AS_IO])
		installer(AS_IO)->unmap_readwrite(0, (1 << installer(AS_IO)->space_config().addr_width()) - 1);
}

void rc2014_bus_device::add_card(device_rc2014_card_interface &card)
{
	card.m_bus = this;
	m_device_list.emplace_back(card);
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

void rc2014_bus_device::clk_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_clk_w(state);
}

void rc2014_bus_device::int_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_int_w(state);
}

void rc2014_bus_device::tx_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_tx_w(state);
}

void rc2014_bus_device::rx_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_rx_w(state);
}

void rc2014_bus_device::user1_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_user1_w(state);
}

void rc2014_bus_device::user2_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_user2_w(state);
}

void rc2014_bus_device::user3_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_user3_w(state);
}

void rc2014_bus_device::user4_w(int state)
{
	for (device_rc2014_card_interface &entry : m_device_list)
		entry.card_user4_w(state);
}

//-------------------------------------------------
//  device_rc2014_card_interface
//-------------------------------------------------

device_rc2014_card_interface::device_rc2014_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rc2014bus")
	, m_bus(nullptr)
{
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
		m_bus->add_card(*card);
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
{
}

void rc2014_ext_bus_device::add_card(device_rc2014_ext_card_interface &card)
{
	card.m_bus = this;
	m_device_list.emplace_back(card);
}

//-------------------------------------------------
//  device_rc2014_ext_card_interface
//-------------------------------------------------

device_rc2014_ext_card_interface::device_rc2014_ext_card_interface(const machine_config &mconfig, device_t &device)
	: device_rc2014_card_interface(mconfig, device)
	, m_bus(nullptr)
{
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

void rc2014_ext_bus_device::clk2_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_clk2_w(state);
}

void rc2014_ext_bus_device::page_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_page_w(state);
}

void rc2014_ext_bus_device::nmi_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_nmi_w(state);
}

void rc2014_ext_bus_device::tx2_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_tx2_w(state);
}

void rc2014_ext_bus_device::rx2_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_rx2_w(state);
}

void rc2014_ext_bus_device::user5_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_user5_w(state);
}

void rc2014_ext_bus_device::user6_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_user6_w(state);
}

void rc2014_ext_bus_device::user7_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_user7_w(state);
}

void rc2014_ext_bus_device::user8_w(int state)
{
	for (device_rc2014_ext_card_interface &entry : m_device_list)
		entry.card_user8_w(state);
}

void rc2014_ext_slot_device::device_resolve_objects()
{
	rc2014_slot_device::device_resolve_objects();
	device_rc2014_ext_card_interface *const card(dynamic_cast<device_rc2014_ext_card_interface *>(get_card_device()));

	if (card)
		((rc2014_ext_bus_device*)m_bus.lookup())->add_card(*card);
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

void rc2014_rc80_bus_device::add_card(device_rc2014_rc80_card_interface &card)
{
	card.m_bus = this;
	m_device_list.emplace_back(card);
}

//-------------------------------------------------
//  device_rc2014_rc80_card_interface
//-------------------------------------------------

device_rc2014_rc80_card_interface::device_rc2014_rc80_card_interface(const machine_config &mconfig, device_t &device)
	: device_rc2014_ext_card_interface(mconfig, device)
	, m_bus(nullptr)
{
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
		((rc2014_rc80_bus_device*)m_bus.lookup())->add_card(*card);
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
