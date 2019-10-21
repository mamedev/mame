// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    S-100 (IEEE-696/1983) bus emulation

**********************************************************************/

#include "emu.h"
#include "s100.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(S100_BUS,  s100_bus_device,  "s100_bus",  "S100 bus")
DEFINE_DEVICE_TYPE(S100_SLOT, s100_slot_device, "s100_slot", "S100 slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_s100_card_interface - constructor
//-------------------------------------------------

device_s100_card_interface::device_s100_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_bus(nullptr),
	m_next(nullptr)
{
}


//-------------------------------------------------
//  s100_slot_device - constructor
//-------------------------------------------------
s100_slot_device::s100_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S100_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_bus(*this, DEVICE_SELF_OWNER)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_slot_device::device_start()
{
	device_s100_card_interface *dev = dynamic_cast<device_s100_card_interface *>(get_card_device());
	if (dev) m_bus->add_card(dev);
}


//-------------------------------------------------
//  s100_bus_device - constructor
//-------------------------------------------------

s100_bus_device::s100_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, S100_BUS, tag, owner, clock),
	m_write_irq(*this),
	m_write_nmi(*this),
	m_write_vi0(*this),
	m_write_vi1(*this),
	m_write_vi2(*this),
	m_write_vi3(*this),
	m_write_vi4(*this),
	m_write_vi5(*this),
	m_write_vi6(*this),
	m_write_vi7(*this),
	m_write_dma0(*this),
	m_write_dma1(*this),
	m_write_dma2(*this),
	m_write_dma3(*this),
	m_write_rdy(*this),
	m_write_hold(*this),
	m_write_error(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_bus_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_vi0.resolve_safe();
	m_write_vi1.resolve_safe();
	m_write_vi2.resolve_safe();
	m_write_vi3.resolve_safe();
	m_write_vi4.resolve_safe();
	m_write_vi5.resolve_safe();
	m_write_vi6.resolve_safe();
	m_write_vi7.resolve_safe();
	m_write_dma0.resolve_safe();
	m_write_dma1.resolve_safe();
	m_write_dma2.resolve_safe();
	m_write_dma3.resolve_safe();
	m_write_rdy.resolve_safe();
	m_write_hold.resolve_safe();
	m_write_error.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_bus_device::device_reset()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void s100_bus_device::add_card(device_s100_card_interface *card)
{
	card->m_bus = this;
	m_device_list.append(*card);
}


//-------------------------------------------------
//  smemr_r - memory read
//-------------------------------------------------

uint8_t s100_bus_device::smemr_r(offs_t offset)
{
	uint8_t data = 0xff;

	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		data &= entry->s100_smemr_r(offset);
		entry = entry->next();
	}

	return data;
}


//-------------------------------------------------
//  mwrt_w - memory write
//-------------------------------------------------

void s100_bus_device::mwrt_w(offs_t offset, uint8_t data)
{
	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->s100_mwrt_w(offset, data);
		entry = entry->next();
	}
}


//-------------------------------------------------
//  sinp_r - I/O read
//-------------------------------------------------

uint8_t s100_bus_device::sinp_r(offs_t offset)
{
	uint8_t data = 0xff;

	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		data &= entry->s100_sinp_r(offset);
		entry = entry->next();
	}

	return data;
}


//-------------------------------------------------
//  sout_w - I/O write
//-------------------------------------------------

void s100_bus_device::sout_w(offs_t offset, uint8_t data)
{
	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->s100_sout_w(offset, data);
		entry = entry->next();
	}
}
