// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a1bus.c - Apple I slot bus and card emulation

***************************************************************************/

#include "emu.h"
#include "a1bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A1BUS_SLOT, a1bus_slot_device, "a1bus_slot", "Apple I Slot")

template class device_finder<device_a1bus_card_interface, false>;
template class device_finder<device_a1bus_card_interface, true>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a1bus_slot_device - constructor
//-------------------------------------------------
a1bus_slot_device::a1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a1bus_slot_device(mconfig, A1BUS_SLOT, tag, owner, clock)
{
}

a1bus_slot_device::a1bus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface<device_a1bus_card_interface>(mconfig, *this)
	, m_a1bus(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_slot_device::device_resolve_objects()
{
	device_a1bus_card_interface *const a1bus_card(dynamic_cast<device_a1bus_card_interface *>(get_card_device()));
	if (a1bus_card)
		a1bus_card->set_a1bus(m_a1bus, tag());
}

void a1bus_slot_device::device_start()
{
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A1BUS, a1bus_device, "a1bus", "Apple I Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a1bus_device - constructor
//-------------------------------------------------

a1bus_device::a1bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a1bus_device(mconfig, A1BUS, tag, owner, clock)
{
}

a1bus_device::a1bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
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

void a1bus_device::device_resolve_objects()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
}

void a1bus_device::device_start()
{
	// clear slot
	m_device = nullptr;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a1bus_device::device_reset()
{
}

device_a1bus_card_interface *a1bus_device::get_a1bus_card()
{
	return m_device;
}

void a1bus_device::add_a1bus_card(device_a1bus_card_interface *card)
{
	m_device = card;
}

void a1bus_device::set_irq_line(int state)
{
	m_out_irq_cb(state);
}

void a1bus_device::set_nmi_line(int state)
{
	m_out_nmi_cb(state);
}

void a1bus_device::install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void a1bus_device::install_bank(offs_t start, offs_t end, uint8_t *data)
{
//  printf("install_bank: %s @ %x->%x\n", tag, start, end);
	m_space->install_ram(start, end, data);
}

// interrupt request from a1bus card
WRITE_LINE_MEMBER( a1bus_device::irq_w ) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER( a1bus_device::nmi_w ) { m_out_nmi_cb(state); }

//**************************************************************************
//  DEVICE CONFIG A1BUS CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A1BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a1bus_card_interface - constructor
//-------------------------------------------------

device_a1bus_card_interface::device_a1bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "a1bus")
	, m_a1bus_finder(device, finder_base::DUMMY_TAG), m_a1bus(nullptr)
	, m_a1bus_slottag(nullptr), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_a1bus_card_interface - destructor
//-------------------------------------------------

device_a1bus_card_interface::~device_a1bus_card_interface()
{
}

void device_a1bus_card_interface::interface_validity_check(validity_checker &valid) const
{
	if (m_a1bus_finder && m_a1bus && (m_a1bus != m_a1bus_finder))
		osd_printf_error("Contradictory buses configured (%s and %s)\n", m_a1bus_finder->tag(), m_a1bus->tag());
}

void device_a1bus_card_interface::interface_pre_start()
{
	if (!m_a1bus)
	{
		m_a1bus = m_a1bus_finder;
		if (!m_a1bus)
			fatalerror("Can't find Apple I Bus device %s\n", m_a1bus_finder.finder_tag());
	}

	if (!m_a1bus->started())
		throw device_missing_dependencies();

	m_a1bus->add_a1bus_card(this);
}

void device_a1bus_card_interface::install_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_a1bus->install_device(start, end, rhandler, whandler);
}

void device_a1bus_card_interface::install_bank(offs_t start, offs_t end, uint8_t *data)
{
	m_a1bus->install_bank(start, end, data);
}
