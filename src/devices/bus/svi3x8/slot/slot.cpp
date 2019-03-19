// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expansion Slot

***************************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  SLOT BUS DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(SVI_SLOT_BUS, svi_slot_bus_device, "svislotbus", "SVI Slot Bus")

//-------------------------------------------------
//  svi_slot_bus_device - constructor
//-------------------------------------------------

svi_slot_bus_device::svi_slot_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SVI_SLOT_BUS, tag, owner, clock)
	, m_int_handler(*this)
	, m_romdis_handler(*this)
	, m_ramdis_handler(*this)
{
}

//-------------------------------------------------
//  svi_slot_bus_device - destructor
//-------------------------------------------------

svi_slot_bus_device::~svi_slot_bus_device()
{
	m_dev.detach_all();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void svi_slot_bus_device::device_start()
{
	// resolve callbacks
	m_int_handler.resolve_safe();
	m_romdis_handler.resolve_safe();
	m_ramdis_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void svi_slot_bus_device::device_reset()
{
}

//-------------------------------------------------
//  add_card - add new card to our bus
//-------------------------------------------------

void svi_slot_bus_device::add_card(device_svi_slot_interface *card)
{
	card->set_bus_device(this);
	m_dev.append(*card);
}

//-------------------------------------------------
//  mreq_r - memory read from slot
//-------------------------------------------------

uint8_t svi_slot_bus_device::mreq_r(offs_t offset)
{
	device_svi_slot_interface *entry = m_dev.first();
	uint8_t data = 0xff;

	while (entry)
	{
		data &= entry->mreq_r(offset);
		entry = entry->next();
	}

	return data;
}

//-------------------------------------------------
//  mreq_w - memory write to slot
//-------------------------------------------------

void svi_slot_bus_device::mreq_w(offs_t offset, uint8_t data)
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->mreq_w(offset, data);
		entry = entry->next();
	}
}

//-------------------------------------------------
//  iorq_r - memory read from slot
//-------------------------------------------------

uint8_t svi_slot_bus_device::iorq_r(offs_t offset)
{
	device_svi_slot_interface *entry = m_dev.first();
	uint8_t data = 0xff;

	while (entry)
	{
		data &= entry->iorq_r(offset);
		entry = entry->next();
	}

	return data;
}

//-------------------------------------------------
//  iorq_w - memory write to slot
//-------------------------------------------------

void svi_slot_bus_device::iorq_w(offs_t offset, uint8_t data)
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->iorq_w(offset, data);
		entry = entry->next();
	}
}

//-------------------------------------------------
//  bk21_w - signal from host to slots
//-------------------------------------------------

WRITE_LINE_MEMBER( svi_slot_bus_device::bk21_w )
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->bk21_w(state);
		entry = entry->next();
	}
}

//-------------------------------------------------
//  bk22_w - signal from host to slots
//-------------------------------------------------

WRITE_LINE_MEMBER( svi_slot_bus_device::bk22_w )
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->bk22_w(state);
		entry = entry->next();
	}
}

//-------------------------------------------------
//  bk31_w - signal from host to slots
//-------------------------------------------------

WRITE_LINE_MEMBER( svi_slot_bus_device::bk31_w )
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->bk31_w(state);
		entry = entry->next();
	}
}

//-------------------------------------------------
//  bk32_w - signal from host to slots
//-------------------------------------------------

WRITE_LINE_MEMBER( svi_slot_bus_device::bk32_w )
{
	device_svi_slot_interface *entry = m_dev.first();

	while (entry)
	{
		entry->bk32_w(state);
		entry = entry->next();
	}
}


//**************************************************************************
//  SVI SLOT DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(SVI_SLOT, svi_slot_device, "svislot", "SVI Slot")

//-------------------------------------------------
//  svi_slot_device - constructor
//-------------------------------------------------

svi_slot_device::svi_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SVI_SLOT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void svi_slot_device::device_start()
{
	device_svi_slot_interface *dev = dynamic_cast<device_svi_slot_interface *>(get_card_device());

	if (dev)
	{
		svi_slot_bus_device *bus = downcast<svi_slot_bus_device *>(owner()->subdevice(SVIBUS_TAG));
		bus->add_card(dev);
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void svi_slot_device::device_reset()
{
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_svi_slot_interface - constructor
//-------------------------------------------------

device_svi_slot_interface::device_svi_slot_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_bus(nullptr),
	m_next(nullptr)
{
}

//-------------------------------------------------
//  ~device_svi_slot_interface - destructor
//-------------------------------------------------

device_svi_slot_interface::~device_svi_slot_interface()
{
}

//-------------------------------------------------
//  set_bus_device - set bus we are attached to
//-------------------------------------------------

void device_svi_slot_interface::set_bus_device(svi_slot_bus_device *bus)
{
	m_bus = bus;
}
