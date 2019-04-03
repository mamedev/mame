// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a2eauxslot.c - Apple IIe auxiliary slot and card emulation

  by R. Belmont

***************************************************************************/

#include "emu.h"
#include "a2eauxslot.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2EAUXSLOT_SLOT, a2eauxslot_slot_device, "a2eauxslot_slot", "Apple IIe AUX Slot")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2eauxslot_slot_device - constructor
//-------------------------------------------------
a2eauxslot_slot_device::a2eauxslot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2eauxslot_slot_device(mconfig, A2EAUXSLOT_SLOT, tag, owner, clock)
{
}

a2eauxslot_slot_device::a2eauxslot_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_a2eauxslot(*this, finder_base::DUMMY_TAG)
{
}

void a2eauxslot_slot_device::device_resolve_objects()
{
	device_a2eauxslot_card_interface *dev = dynamic_cast<device_a2eauxslot_card_interface *>(get_card_device());

	if (dev)
		dev->set_a2eauxslot_device(m_a2eauxslot.target());
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2EAUXSLOT, a2eauxslot_device, "a2eauxslot", "Apple IIe AUX Bus")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a2eauxslot_device - constructor
//-------------------------------------------------

a2eauxslot_device::a2eauxslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a2eauxslot_device(mconfig, A2EAUXSLOT, tag, owner, clock)
{
}

a2eauxslot_device::a2eauxslot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_out_irq_cb(*this)
	, m_out_nmi_cb(*this)
	, m_device(nullptr)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void a2eauxslot_device::device_resolve_objects()
{
	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2eauxslot_device::device_start()
{
}

device_a2eauxslot_card_interface *a2eauxslot_device::get_a2eauxslot_card()
{
	return m_device;
}

void a2eauxslot_device::add_a2eauxslot_card(device_a2eauxslot_card_interface *card)
{
	m_device = card;
}

void a2eauxslot_device::set_irq_line(int state)
{
	m_out_irq_cb(state);
}

void a2eauxslot_device::set_nmi_line(int state)
{
	m_out_nmi_cb(state);
}

// interrupt request from a2eauxslot card
WRITE_LINE_MEMBER( a2eauxslot_device::irq_w ) { m_out_irq_cb(state); }
WRITE_LINE_MEMBER( a2eauxslot_device::nmi_w ) { m_out_nmi_cb(state); }

//**************************************************************************
//  DEVICE CONFIG A2EAUXSLOT CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE A2EAUXSLOT CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_a2eauxslot_card_interface - constructor
//-------------------------------------------------

device_a2eauxslot_card_interface::device_a2eauxslot_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
		m_a2eauxslot(nullptr),
		m_slot(0), m_next(nullptr)
{
}


//-------------------------------------------------
//  ~device_a2eauxslot_card_interface - destructor
//-------------------------------------------------

device_a2eauxslot_card_interface::~device_a2eauxslot_card_interface()
{
}

void device_a2eauxslot_card_interface::set_a2eauxslot_device(a2eauxslot_device *a2eauxslot)
{
	m_a2eauxslot = a2eauxslot;
	m_a2eauxslot->add_a2eauxslot_card(this);
}
