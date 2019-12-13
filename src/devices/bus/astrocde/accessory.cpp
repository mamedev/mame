// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "accessory.h"

#include <cassert>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ASTROCADE_ACCESSORY_PORT, astrocade_accessory_port_device, "astrocade_accessory_port", "Bally Astrocade Accessory Port")


//**************************************************************************
//    Bally Astrocade accessory interface
//**************************************************************************

device_astrocade_accessory_interface::device_astrocade_accessory_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "astrocadeacc")
	, m_port(dynamic_cast<astrocade_accessory_port_device *>(device.owner()))
{
}

device_astrocade_accessory_interface::~device_astrocade_accessory_interface()
{
}

void device_astrocade_accessory_interface::interface_validity_check(validity_checker &valid) const
{
	if (device().owner() && !m_port)
	{
		osd_printf_error("Owner device %s (%s) is not an astrocade_accessory_port_device\n", device().owner()->tag(), device().owner()->name());
	}
}

void device_astrocade_accessory_interface::interface_pre_start()
{
	if (m_port && !m_port->started())
		throw device_missing_dependencies();
}


//**************************************************************************
//    Bally Astrocade accessory port
//**************************************************************************

astrocade_accessory_port_device::astrocade_accessory_port_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ASTROCADE_ACCESSORY_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_astrocade_accessory_interface>(mconfig, *this)
	, m_ltpen(0)
	, m_ltpen_handler(*this)
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_device(nullptr)
{
}

astrocade_accessory_port_device::~astrocade_accessory_port_device()
{
}

void astrocade_accessory_port_device::device_resolve_objects()
{
	m_device = get_card_device();
	if (m_device)
		m_device->set_screen(m_screen);

	m_ltpen_handler.resolve_safe();
}

void astrocade_accessory_port_device::device_start()
{
	save_item(NAME(m_ltpen));

	m_ltpen = 0;

	m_ltpen_handler(0);
}

#include "lightpen.h"

void astrocade_accessories(device_slot_interface &device)
{
	device.option_add("lightpen", ASTROCADE_LIGHTPEN);
}
