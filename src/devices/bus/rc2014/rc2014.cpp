// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        RC2014 bus device

***************************************************************************/

#include "emu.h"
#include "rc2014.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(RC2014_BUS,  rc2014_bus_device,  "rc2014_bus",  "rc2014 bus")
DEFINE_DEVICE_TYPE(RC2014_SLOT, rc2014_slot_device, "rc2014_slot", "rc2014 slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_rc2014_card_interface - constructor
//-------------------------------------------------

device_rc2014_card_interface::device_rc2014_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rc2014bus")
	, m_bus(nullptr)
{
}

//-------------------------------------------------
//  rc2014_slot_device - constructor
//-------------------------------------------------
rc2014_slot_device::rc2014_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RC2014_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_rc2014_card_interface>(mconfig, *this)
	, m_bus(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rc2014_slot_device::device_start()
{
}

void rc2014_slot_device::device_resolve_objects()
{
	device_rc2014_card_interface *const card(dynamic_cast<device_rc2014_card_interface *>(get_card_device()));

	if (card)
		card->set_bus_device(*m_bus);
}


//-------------------------------------------------
//  rc2014_bus_device - constructor
//-------------------------------------------------

rc2014_bus_device::rc2014_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, RC2014_BUS, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_mem_config("mem", ENDIANNESS_LITTLE, 8, 16)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8)
	/*
	,m_write_irq(*this)
	,m_write_nmi(*this)*/
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rc2014_bus_device::device_start()
{
	// resolve callbacks
/*	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
*/
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rc2014_bus_device::device_reset()
{
}


device_memory_interface::space_config_vector rc2014_bus_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_mem_config), std::make_pair(AS_IO, &m_io_config) };
}


//-------------------------------------------------
//  set_bus_clock - set main bus clock
//-------------------------------------------------

void rc2014_bus_device::set_bus_clock(u32 clock)
{
	set_clock(clock);
	notify_clock_changed();
}


void device_rc2014_card_interface::set_bus_device(rc2014_bus_device &bus_device)
{
	m_bus = &bus_device;
}
