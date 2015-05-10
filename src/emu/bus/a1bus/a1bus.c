// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  a1bus.c - Apple I slot bus and card emulation

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "a1bus.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A1BUS_SLOT = &device_creator<a1bus_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a1bus_slot_device - constructor
//-------------------------------------------------
a1bus_slot_device::a1bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A1BUS_SLOT, "Apple I Slot", tag, owner, clock, "a1bus_slot", __FILE__),
		device_slot_interface(mconfig, *this)
{
}

a1bus_slot_device::a1bus_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_slot_interface(mconfig, *this)
{
}

void a1bus_slot_device::static_set_a1bus_slot(device_t &device, const char *tag, const char *slottag)
{
	a1bus_slot_device &a1bus_card = dynamic_cast<a1bus_slot_device &>(device);
	a1bus_card.m_a1bus_tag = tag;
	a1bus_card.m_a1bus_slottag = slottag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_slot_device::device_start()
{
	device_a1bus_card_interface *dev = dynamic_cast<device_a1bus_card_interface *>(get_card_device());

	if (dev) device_a1bus_card_interface::static_set_a1bus_tag(*dev, m_a1bus_tag, m_a1bus_slottag);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A1BUS = &device_creator<a1bus_device>;

void a1bus_device::static_set_cputag(device_t &device, const char *tag)
{
	a1bus_device &a1bus = downcast<a1bus_device &>(device);
	a1bus.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a1bus_device - constructor
//-------------------------------------------------

a1bus_device::a1bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A1BUS, "Apple I Bus", tag, owner, clock, "a1bus", __FILE__),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this)
{
}

a1bus_device::a1bus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		m_out_irq_cb(*this),
		m_out_nmi_cb(*this)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_device::device_start()
{
	m_maincpu = machine().device<cpu_device>(m_cputag);

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	m_out_nmi_cb.resolve_safe();

	// clear slot
	m_device = NULL;
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

void a1bus_device::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	m_maincpu = machine().device<cpu_device>(m_cputag);

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(start, end, rhandler, whandler);
}

void a1bus_device::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
//  printf("install_bank: %s @ %x->%x mask %x mirror %x\n", tag, start, end, mask, mirror);
	m_maincpu = machine().device<cpu_device>(m_cputag);
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_bank(start, end, mask, mirror, tag );
	machine().root_device().membank(tag)->set_base(data);
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
	: device_slot_card_interface(mconfig, device),
		m_a1bus(NULL),
		m_a1bus_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_a1bus_card_interface - destructor
//-------------------------------------------------

device_a1bus_card_interface::~device_a1bus_card_interface()
{
}

void device_a1bus_card_interface::static_set_a1bus_tag(device_t &device, const char *tag, const char *slottag)
{
	device_a1bus_card_interface &a1bus_card = dynamic_cast<device_a1bus_card_interface &>(device);
	a1bus_card.m_a1bus_tag = tag;
	a1bus_card.m_a1bus_slottag = slottag;
}

void device_a1bus_card_interface::set_a1bus_device()
{
	m_a1bus = dynamic_cast<a1bus_device *>(device().machine().device(m_a1bus_tag));
	m_a1bus->add_a1bus_card(this);
}

void device_a1bus_card_interface::install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler)
{
	m_a1bus->install_device(start, end, rhandler, whandler);
}

void device_a1bus_card_interface::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, char *tag, UINT8 *data)
{
	m_a1bus->install_bank(start, end, mask, mirror, tag, data);
}
