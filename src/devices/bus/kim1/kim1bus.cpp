// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  kim1bus.cpp - KIM-1 expansion bus emulation

***************************************************************************/

#include "emu.h"
#include "kim1bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KIM1BUS_SLOT, kim1bus_slot_device, "kim1bus_slot", "KIM-1 Backplane Slot")

template class device_finder<device_kim1bus_card_interface, false>;
template class device_finder<device_kim1bus_card_interface, true>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kim1bus_slot_device - constructor
//-------------------------------------------------
kim1bus_slot_device::kim1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kim1bus_slot_device(mconfig, KIM1BUS_SLOT, tag, owner, clock)
{
}

kim1bus_slot_device::kim1bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_kim1bus_card_interface>(mconfig, *this)
	, m_kim1bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kim1bus_slot_device::device_resolve_objects()
{
	device_kim1bus_card_interface *const kim1bus_card(dynamic_cast<device_kim1bus_card_interface *>(get_card_device()));
	if (kim1bus_card)
		kim1bus_card->set_kim1bus(m_kim1bus, tag());
}

void kim1bus_slot_device::device_start()
{
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(KIM1BUS, kim1bus_device, "kim1bus", "KIM-1 Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kim1bus_device - constructor
//-------------------------------------------------

kim1bus_device::kim1bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kim1bus_device(mconfig, KIM1BUS, tag, owner, clock)
{
}

kim1bus_device::kim1bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
	, m_device(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kim1bus_device::device_start()
{
	// clear slot
	m_device = nullptr;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kim1bus_device::device_reset()
{
}

device_kim1bus_card_interface *kim1bus_device::get_kim1bus_card()
{
	return m_device;
}

void kim1bus_device::add_kim1bus_card(device_kim1bus_card_interface *card)
{
	m_device = card;
}

void kim1bus_device::set_irq_line(int state)
{
	m_out_irq_cb(state);
}

void kim1bus_device::set_nmi_line(int state)
{
	m_out_nmi_cb(state);
}

void kim1bus_device::install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void kim1bus_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
//  printf("install_bank: %s @ %x->%x\n", tag, start, end);
	m_space->install_ram(start, end, data);
}

// interrupt request from kim1bus card
void kim1bus_device::irq_w(int state) { m_out_irq_cb(state); }
void kim1bus_device::nmi_w(int state) { m_out_nmi_cb(state); }

device_kim1bus_card_interface::device_kim1bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "kim1bus")
	, m_kim1bus_finder(device, finder_base::DUMMY_TAG), m_kim1bus(nullptr)
	, m_kim1bus_slottag(nullptr)
{
}

device_kim1bus_card_interface::~device_kim1bus_card_interface()
{
}

void device_kim1bus_card_interface::interface_validity_check(validity_checker &valid) const
{
	if (m_kim1bus_finder && m_kim1bus && (m_kim1bus != m_kim1bus_finder))
		osd_printf_error("Contradictory buses configured (%s and %s)\n", m_kim1bus_finder->tag(), m_kim1bus->tag());
}

void device_kim1bus_card_interface::interface_pre_start()
{
	if (!m_kim1bus)
	{
		m_kim1bus = m_kim1bus_finder;
		if (!m_kim1bus)
			fatalerror("Can't find KIM-1 Bus device %s\n", m_kim1bus_finder.finder_tag());
	}

	if (!m_kim1bus->started())
		throw device_missing_dependencies();

	m_kim1bus->add_kim1bus_card(this);
}

void device_kim1bus_card_interface::install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_kim1bus->install_device(start, end, rhandler, whandler);
}

void device_kim1bus_card_interface::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_kim1bus->install_bank(start, end, data);
}
