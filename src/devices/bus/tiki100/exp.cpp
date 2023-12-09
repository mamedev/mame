// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 expansion bus emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TIKI100_BUS,      tiki100_bus_device,      "tiki100bus",      "TIKI-100 expansion bus")
DEFINE_DEVICE_TYPE(TIKI100_BUS_SLOT, tiki100_bus_slot_device, "tiki100bus_slot", "TIKI-100 expansion bus slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tiki100_bus_slot_device - constructor
//-------------------------------------------------

tiki100_bus_slot_device::tiki100_bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIKI100_BUS_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_tiki100bus_card_interface>(mconfig, *this),
	device_z80daisy_interface(mconfig, *this),
	m_bus(*this, finder_base::DUMMY_TAG),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_bus_slot_device::device_start()
{
	m_card = get_card_device();
	if (m_card)
		m_bus->add_card(*m_card);
}


//-------------------------------------------------
//  tiki100_bus_device - constructor
//-------------------------------------------------

tiki100_bus_device::tiki100_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TIKI100_BUS, tag, owner, clock),
	m_irq_cb(*this),
	m_nmi_cb(*this),
	m_busrq_cb(*this),
	m_in_mrq_cb(*this, 0),
	m_out_mrq_cb(*this)
{
}

tiki100_bus_device::~tiki100_bus_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tiki100_bus_device::device_start()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void tiki100_bus_device::add_card(device_tiki100bus_card_interface &card)
{
	m_device_list.emplace_back(card);

	card.m_bus = this;
}


//-------------------------------------------------
//  mrq_r - memory read
//-------------------------------------------------

uint8_t tiki100_bus_device::mrq_r(offs_t offset, uint8_t data, bool &mdis)
{
	for (device_tiki100bus_card_interface &entry : m_device_list)
		data &= entry.mrq_r(offset, data, mdis);

	return data;
}


//-------------------------------------------------
//  mrq_w - memory write
//-------------------------------------------------

void tiki100_bus_device::mrq_w(offs_t offset, uint8_t data)
{
	for (device_tiki100bus_card_interface &entry : m_device_list)
		entry.mrq_w(offset, data);
}


//-------------------------------------------------
//  iorq_r - I/O read
//-------------------------------------------------

uint8_t tiki100_bus_device::iorq_r(offs_t offset, uint8_t data)
{
	for (device_tiki100bus_card_interface &entry : m_device_list)
		data &= entry.iorq_r(offset, data);

	return data;
}


//-------------------------------------------------
//  iorq_w - I/O write
//-------------------------------------------------

void tiki100_bus_device::iorq_w(offs_t offset, uint8_t data)
{
	for (device_tiki100bus_card_interface &entry : m_device_list)
		entry.iorq_w(offset, data);
}


//-------------------------------------------------
//  busak_w - bus acknowledge write
//-------------------------------------------------

void tiki100_bus_device::busak_w(int state)
{
	for (device_tiki100bus_card_interface &entry : m_device_list)
		entry.busak_w(state);
}



//**************************************************************************
//  DEVICE TIKI-100 BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_tiki100bus_card_interface - constructor
//-------------------------------------------------

device_tiki100bus_card_interface::device_tiki100bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "tiki100bus"),
	m_bus(nullptr),
	m_busak(CLEAR_LINE)
{
	m_slot = dynamic_cast<tiki100_bus_slot_device *>(device.owner());
}


void device_tiki100bus_card_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  SLOT_INTERFACE( tiki100_cards )
//-------------------------------------------------

// slot devices
#include "8088.h"
#include "hdc.h"

void tiki100_cards(device_slot_interface &device)
{
	device.option_add("8088", TIKI100_8088);
	device.option_add("hdc", TIKI100_HDC);
}
