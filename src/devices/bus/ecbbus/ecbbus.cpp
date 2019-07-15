// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Kontron Europe Card Bus emulation

**********************************************************************/

#include "emu.h"
#include "ecbbus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ECBBUS_SLOT, ecbbus_slot_device, "ecbbus_slot", "ECB bus slot")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ecbbus_slot_device - constructor
//-------------------------------------------------

ecbbus_slot_device::ecbbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ECBBUS_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_bus_tag(nullptr), m_bus_num(0), m_bus(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ecbbus_slot_device::device_start()
{
	m_bus = machine().device<ecbbus_device>(m_bus_tag);
	device_ecbbus_card_interface *dev = dynamic_cast<device_ecbbus_card_interface *>(get_card_device());
	if (dev) m_bus->add_card(dev, m_bus_num);
}



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ECBBUS, ecbbus_device, "ecbbus", "ECB bus")



//**************************************************************************
//  DEVICE ECBBUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_ecbbus_card_interface - constructor
//-------------------------------------------------

device_ecbbus_card_interface::device_ecbbus_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device)
{
	m_slot = dynamic_cast<ecbbus_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ecbbus_device - constructor
//-------------------------------------------------

ecbbus_device::ecbbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ECBBUS, tag, owner, clock),
	m_write_irq(*this),
	m_write_nmi(*this)
{
	std::fill(std::begin(m_ecbbus_device), std::end(m_ecbbus_device), nullptr);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ecbbus_device::device_start()
{
	// resolve callbacks
	m_write_irq.resolve_safe();
	m_write_nmi.resolve_safe();
}


//-------------------------------------------------
//  add_card - add ECB bus card
//-------------------------------------------------

void ecbbus_device::add_card(device_ecbbus_card_interface *card, int pos)
{
	m_ecbbus_device[pos] = card;
}


//-------------------------------------------------
//  mem_r -
//-------------------------------------------------

READ8_MEMBER( ecbbus_device::mem_r )
{
	uint8_t data = 0;

	for (auto & elem : m_ecbbus_device)
	{
		if (elem != nullptr)
		{
			data |= elem->ecbbus_mem_r(offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  mem_w -
//-------------------------------------------------

WRITE8_MEMBER( ecbbus_device::mem_w )
{
	for (auto & elem : m_ecbbus_device)
	{
		if (elem != nullptr)
		{
			elem->ecbbus_mem_w(offset, data);
		}
	}
}


//-------------------------------------------------
//  io_r -
//-------------------------------------------------

READ8_MEMBER( ecbbus_device::io_r )
{
	uint8_t data = 0;

	for (auto & elem : m_ecbbus_device)
	{
		if (elem != nullptr)
		{
			data |= elem->ecbbus_io_r(offset);
		}
	}

	return data;
}


//-------------------------------------------------
//  io_w -
//-------------------------------------------------

WRITE8_MEMBER( ecbbus_device::io_w )
{
	for (auto & elem : m_ecbbus_device)
	{
		if (elem != nullptr)
		{
			elem->ecbbus_io_w(offset, data);
		}
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( ecbbus_cards )
//-------------------------------------------------

// slot devices
#include "grip.h"

void ecbbus_cards(device_slot_interface &device)
{
	device.option_add("grip21", ECB_GRIP21);
/*  device.option_add("grip25", ECB_GRIP25);
    device.option_add("grip26", ECB_GRIP26);
    device.option_add("grip31", ECB_GRIP31);
    device.option_add("grip562", ECB_GRIP562);
    device.option_add("grips115", ECB_GRIPS115);*/
}
