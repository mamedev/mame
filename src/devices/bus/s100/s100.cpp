// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    S-100 (IEEE-696/1983) bus emulation

**********************************************************************/

#include "s100.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type S100_BUS = &device_creator<s100_bus_t>;
const device_type S100_SLOT = &device_creator<s100_slot_t>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_s100_card_interface - constructor
//-------------------------------------------------

device_s100_card_interface::device_s100_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	m_bus(nullptr),
	m_next(nullptr)
{
}


//-------------------------------------------------
//  s100_slot_t - constructor
//-------------------------------------------------
s100_slot_t::s100_slot_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, S100_SLOT, "S100 slot", tag, owner, clock, "s100_slot", __FILE__),
	device_slot_interface(mconfig, *this), m_bus(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_slot_t::device_start()
{
	m_bus = machine().device<s100_bus_t>(S100_TAG);
	device_s100_card_interface *dev = dynamic_cast<device_s100_card_interface *>(get_card_device());
	if (dev) m_bus->add_card(dev);
}


//-------------------------------------------------
//  s100_bus_t - constructor
//-------------------------------------------------

s100_bus_t::s100_bus_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, S100_BUS, "S100", tag, owner, clock, "s100", __FILE__),
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

void s100_bus_t::device_start()
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

void s100_bus_t::device_reset()
{
}


//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void s100_bus_t::add_card(device_s100_card_interface *card)
{
	card->m_bus = this;
	m_device_list.append(*card);
}


//-------------------------------------------------
//  smemr_r - memory read
//-------------------------------------------------

READ8_MEMBER( s100_bus_t::smemr_r )
{
	UINT8 data = 0;

	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		data |= entry->s100_smemr_r(space, offset);
		entry = entry->next();
	}

	return data;
}


//-------------------------------------------------
//  mwrt_w - memory write
//-------------------------------------------------

WRITE8_MEMBER( s100_bus_t::mwrt_w )
{
	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->s100_mwrt_w(space, offset, data);
		entry = entry->next();
	}
}


//-------------------------------------------------
//  sinp_r - I/O read
//-------------------------------------------------

READ8_MEMBER( s100_bus_t::sinp_r )
{
	UINT8 data = 0;

	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		data |= entry->s100_sinp_r(space, offset);
		entry = entry->next();
	}

	return data;
}


//-------------------------------------------------
//  sout_w - I/O write
//-------------------------------------------------

WRITE8_MEMBER( s100_bus_t::sout_w )
{
	device_s100_card_interface *entry = m_device_list.first();

	while (entry)
	{
		entry->s100_sout_w(space, offset, data);
		entry = entry->next();
	}
}
