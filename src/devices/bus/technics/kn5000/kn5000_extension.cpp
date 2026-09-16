// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Felipe Sanches

// Generic Technics KN5000 extension slot


#include "emu.h"
#include "kn5000_extension.h"

#include "hdae5000.h"


DEFINE_DEVICE_TYPE(KN5000_EXTENSION, kn5000_extension_connector, "kn5000_extension_connector", "Technics KN5000 extension port")

kn5000_extension_connector::kn5000_extension_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KN5000_EXTENSION, tag, owner, clock),
	device_single_card_slot_interface<device_kn5000_extension_interface>(mconfig, *this),
	m_write_irq(*this)
{
}

void kn5000_extension_connector::device_start()
{
}

void kn5000_extension_connector::program_map(address_space_installer &space)
{
	auto card = get_card_device();
	if(card)
		card->program_map(space);
}

void kn5000_extension_intf(device_slot_interface &device)
{
	device.option_add("hdae5000", HDAE5000);
}

device_kn5000_extension_interface::device_kn5000_extension_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "kn5000_extension")
{
}

device_kn5000_extension_interface::~device_kn5000_extension_interface()
{
}
