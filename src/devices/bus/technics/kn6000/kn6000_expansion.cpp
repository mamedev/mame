// license:GPL-2.0+
// copyright-holders:Felipe Sanches

#include "emu.h"
#include "kn6000_expansion.h"

#include "hdsx3.h"

DEFINE_DEVICE_TYPE(KN6000_EXPANSION, kn6000_expansion_connector, "kn6000_expansion", "SX-KN6000 expansion connector")

kn6000_expansion_connector::kn6000_expansion_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KN6000_EXPANSION, tag, owner, clock)
	, device_single_card_slot_interface<device_kn6000_expansion_interface>(mconfig, *this)
{
}

void kn6000_expansion_connector::device_start()
{
}

void kn6000_expansion_connector::program_map(address_space_installer &space)
{
	device_kn6000_expansion_interface *const card = get_card_device();
	if (!card)
		return;

	// ISOROM decodes only A23 and A24, so the unit's 2 MiB window repeats
	address_map_constructor isorom(&device_kn6000_expansion_interface::isorom_map, "isorom", card);
	for (offs_t base = 0x97800000; base < 0x98000000; base += 0x200000)
		space.install_device_delegate(base, base + 0x1fffff, card->device(), isorom);

	address_map_constructor hddcs(&device_kn6000_expansion_interface::hddcs_map, "hddcs", card);
	space.install_device_delegate(0x98060000, 0x9806ffff, card->device(), hddcs);
}

device_kn6000_expansion_interface::device_kn6000_expansion_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "kn6000exp")
{
}

void kn6000_expansion_intf(device_slot_interface &device)
{
	device.option_add("hdsx3", HDSX3);
}
