// license:BSD-3-Clause
// copyright-holders:Jeremy English
/**********************************************************************

	8008-Super Bus Device aka ThunderDome

	Scott's 8008 Supercomputer a.k.a. Master Blaster
	by Dr. Scott M. Baker

**********************************************************************/

#include "emu.h"
#include "super8008.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SUPER8008_BUS,  super8008_bus_device,  "super8008_bus",  "super8008 bus")
DEFINE_DEVICE_TYPE(SUPER8008_SLOT, super8008_slot_device, "super8008_slot", "super8008 slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  device_super8008_card_interface - constructor
//-------------------------------------------------

device_super8008_card_interface::device_super8008_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "super8008bus"),
	m_bus(nullptr)
{
}

void device_super8008_card_interface::interface_pre_start()
{
	if (!m_bus)
		throw device_missing_dependencies();
}


//-------------------------------------------------
//  super8008_slot_device - constructor
//-------------------------------------------------
super8008_slot_device::super8008_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SUPER8008_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_super8008_card_interface>(mconfig, *this),
	m_bus(*this, DEVICE_SELF_OWNER)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void super8008_slot_device::device_start()
{
	device_super8008_card_interface *const dev = get_card_device();
	if (dev)
		m_bus->add_card(*dev);
		
}


//-------------------------------------------------
//  super8008_bus_device - constructor
//-------------------------------------------------

super8008_bus_device::super8008_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SUPER8008_BUS, tag, owner, clock)
{
	m_ext_take = 0;
}

super8008_bus_device::~super8008_bus_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void super8008_bus_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void super8008_bus_device::device_reset()
{
}

void super8008_bus_device::device_post_load()
{
}

//-------------------------------------------------
//  add_card - add card
//-------------------------------------------------

void super8008_bus_device::add_card(device_super8008_card_interface &card)
{
	card.m_bus = this;
	m_device_list.emplace_back(card);
}

void super8008_bus_device::ext_write(offs_t offset, uint8_t data)
{
	//ext_cs is pulled high through a 10k resistor and is active low
	if (m_ext_cs)
	{
		return;
	}

	for (device_super8008_card_interface &entry : m_device_list)
	{
		entry.ext_write(offset, data);
	}
}

uint8_t super8008_bus_device::ext_read(offs_t offset)
{
	uint8_t data = 0;

	if (m_ext_cs)
	{
		return data;
	}

	for (device_super8008_card_interface &entry : m_device_list)
	{
		data |= entry.ext_read(offset);
	}

	return data;
}

void super8008_bus_device::ext_take(int state)
{ 
	for (device_super8008_card_interface &entry : m_device_list)
	{
		entry.ext_take(state);
	}
	m_ext_take = state;
}

void super8008_bus_device::ext_int()
{
	for (device_super8008_card_interface &entry : m_device_list)
	{
		entry.ext_int();
	}
}

void super8008_bus_device::ext_reset()
{
	for (device_super8008_card_interface &entry : m_device_list)
	{
		entry.ext_reset();
	}
}

void super8008_bus_device::ext_req()
{
	for (device_super8008_card_interface &entry : m_device_list)
	{
		entry.ext_req();
	}	
}

uint8_t super8008_bus_device::ext_run()
{
	uint8_t run_status = 0;
	for (device_super8008_card_interface &entry : m_device_list)
	{
		run_status |= entry.ext_run();
	}
	return run_status;
}


//Cards
#include "super8008_blaster.h"
void super8008_bus_devices(device_slot_interface &device)
{
	device.option_add("super8008_blaster", SUPER8008_BLASTER);
}
