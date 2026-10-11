// license:BSD-3-Clause
// copyright-holders:Carl Lom
/**********************************************************************

    Roland EXT port emulation

**********************************************************************/

#include "emu.h"
#include "extport.h"

#include "mouse.h"


DEFINE_DEVICE_TYPE(ROLAND_EXT_PORT, roland_ext_port_device, "roland_ext_port", "Roland EXT port")


device_roland_ext_port_interface::device_roland_ext_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "rolandext")
{
}


device_roland_ext_port_interface::~device_roland_ext_port_interface()
{
}


u8 device_roland_ext_port_interface::read()
{
	return 0xff;
}


void device_roland_ext_port_interface::write(u8 data, u8 mem_mask)
{
}


roland_ext_port_device::roland_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ROLAND_EXT_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_roland_ext_port_interface>(mconfig, *this)
	, m_device(nullptr)
	, m_pindir(0)
{
}


void roland_ext_port_device::device_start()
{
	m_device = get_card_device();

	save_item(NAME(m_pindir));
}


void roland_ext_port_device::write(u8 data)
{
	if (!exists())
		return;

	// pins 6-7 are host-driven only when configured as outputs
	u8 mem_mask = (BIT(m_pindir, 0) ? 0x40 : 0) | (BIT(m_pindir, 1) ? 0x80 : 0);
	m_device->write(data, mem_mask);
}


void roland_ext_port_devices(device_slot_interface &device)
{
	device.option_add("mouse", ROLAND_EXT_MOUSE);
	// TODO: RC-100 external keypad, DT-100 digitizer tablet
}
