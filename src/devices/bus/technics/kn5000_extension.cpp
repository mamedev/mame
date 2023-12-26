// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Felipe Sanches

// Generic Technics KN5000 extension slot


#include "emu.h"
#include "kn5000_extension.h"

DEFINE_DEVICE_TYPE(KN5000_EXTENSION, kn5000_extension_device, "kn5000_extension", "Technics KN5000 extension port")

kn5000_extension_device::kn5000_extension_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KN5000_EXTENSION, tag, owner, clock),
	device_single_card_slot_interface<kn5000_extension_interface>(mconfig, *this)
// FIXME: do we need these?
//	m_firq_callback(*this),
//	m_irq_callback(*this)
{
}

void kn5000_extension_device::rom_map(address_space_installer &space, offs_t start, offs_t end)
{
	auto dev = get_card_device();
	if(dev)
		space.install_device(start, end, *dev, &kn5000_extension_interface::rom_map);
}

void kn5000_extension_device::io_map(address_space_installer &space, offs_t start, offs_t end)
{
	auto dev = get_card_device();
	if(dev)
		space.install_device(start, end, *dev, &kn5000_extension_interface::io_map);
}

void kn5000_extension_device::device_start()
{
}

kn5000_extension_interface::kn5000_extension_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "extension"),
	m_ext(dynamic_cast<kn5000_extension_device *>(device.owner()))
{
}

// FIXME: do we need these?
//WRITE_LINE_MEMBER(kn5000_extension_interface::firq_w)
//{
//	if(m_ext)
//		m_ext->m_firq_callback(state);
//}
//
//WRITE_LINE_MEMBER(kn5000_extension_interface::irq_w)
//{
//	if(m_ext)
//		m_ext->m_irq_callback(state);
//}
