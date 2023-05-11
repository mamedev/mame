// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Generic Thomson TO*/MO* extension slot


#include "emu.h"
#include "extension.h"

DEFINE_DEVICE_TYPE(THOMSON_EXTENSION, thomson_extension_device, "thomson_extension", "Thomson TO*/MO* extension port")

thomson_extension_device::thomson_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, THOMSON_EXTENSION, tag, owner, clock),
	device_single_card_slot_interface<thomson_extension_interface>(mconfig, *this),
	m_firq_callback(*this),
	m_irq_callback(*this)
{
}

void thomson_extension_device::rom_map(address_space_installer &space, offs_t start, offs_t end)
{
	auto dev = get_card_device();
	if(dev)
		space.install_device(start, end, *dev, &thomson_extension_interface::rom_map);
}

void thomson_extension_device::io_map(address_space_installer &space, offs_t start, offs_t end)
{
	auto dev = get_card_device();
	if(dev)
		space.install_device(start, end, *dev, &thomson_extension_interface::io_map);
}

void thomson_extension_device::device_resolve_objects()
{
	m_firq_callback.resolve_safe();
	m_irq_callback.resolve_safe();
}

void thomson_extension_device::device_start()
{
}

thomson_extension_interface::thomson_extension_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "extension"),
	m_ext(dynamic_cast<thomson_extension_device *>(device.owner()))
{
}

WRITE_LINE_MEMBER(thomson_extension_interface::firq_w)
{
	if(m_ext)
		m_ext->m_firq_callback(state);
}

WRITE_LINE_MEMBER(thomson_extension_interface::irq_w)
{
	if(m_ext)
		m_ext->m_irq_callback(state);
}
