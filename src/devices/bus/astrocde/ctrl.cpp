// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "ctrl.h"

#include <cassert>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_CTRL_PORT, astrocade_ctrl_port_device, "astrocade_ctrl_port", "Bally Astrocade Control Port")


//**************************************************************************
//    Bally Astrocade controller interface
//**************************************************************************

device_astrocade_ctrl_interface::device_astrocade_ctrl_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<astrocade_ctrl_port_device *>(device.owner()))
{
}

device_astrocade_ctrl_interface::~device_astrocade_ctrl_interface()
{
}

void device_astrocade_ctrl_interface::interface_validity_check(validity_checker &valid) const
{
	device_slot_card_interface::interface_validity_check(valid);

	if (device().owner() && !m_port)
	{
		osd_printf_error("Owner device %s (%s) is not an astrocade_ctrl_port_device\n", device().owner()->tag(), device().owner()->name());
	}
}

void device_astrocade_ctrl_interface::interface_pre_start()
{
	device_slot_card_interface::interface_pre_start();

	if (m_port && !m_port->started())
		throw device_missing_dependencies();
}


//**************************************************************************
//    Bally Astrocade controller port
//**************************************************************************

astrocade_ctrl_port_device::astrocade_ctrl_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASTROCADE_CTRL_PORT, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_ltpen(0)
	, m_ltpen_handler(*this)
	, m_device(nullptr)
{
}

astrocade_ctrl_port_device::~astrocade_ctrl_port_device()
{
}

void astrocade_ctrl_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_astrocade_ctrl_interface *>(card))
	{
		osd_printf_error("Card device %s (%s) does not implement device_astrocade_ctrl_interface\n", card->tag(), card->name());
	}
}

void astrocade_ctrl_port_device::device_resolve_objects()
{
	device_astrocade_ctrl_interface *const card(dynamic_cast<device_astrocade_ctrl_interface *>(get_card_device()));
	if (card)
		m_device = card;

	m_ltpen_handler.resolve_safe();
}

void astrocade_ctrl_port_device::device_start()
{
	device_t *const card(get_card_device());
	if (card)
	{
		if (!m_device)
		{
			throw emu_fatalerror("astrocade_ctrl_port_device: card device %s (%s) does not implement device_astrocade_ctrl_interface\n", card->tag(), card->name());
		}
	}

	save_item(NAME(m_ltpen));

	m_ltpen = 0;

	m_ltpen_handler(0);
}

uint8_t astrocade_ctrl_port_device::read_handle()
{
	return m_device ? m_device->read_handle() : 0;
}

uint8_t astrocade_ctrl_port_device::read_knob()
{
	return m_device ? m_device->read_knob() : 0;
}

#include "joy.h"
#include "cassette.h"

void astrocade_controllers(device_slot_interface &device)
{
	device.option_add("joy",      ASTROCADE_JOY);
	device.option_add("cassette", ASTROCADE_CASSETTE);
}
