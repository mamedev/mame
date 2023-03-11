// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "minicart.h"
#include "bus/msx/cart/nomapper.h"

void msx_yamaha_minicart(device_slot_interface &device, bool is_in_subslot)
{
	device.option_add_internal("nomapper", MSX_CART_NOMAPPER);
}

DEFINE_DEVICE_TYPE(MSX_SLOT_YAMAHA_MINICART,  msx_slot_yamaha_minicart_device,  "msx_slot_yamaha_minicart",  "MSX Yamaha Minicart slot")


msx_slot_yamaha_minicart_device::msx_slot_yamaha_minicart_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_cartridge_base_device(mconfig, MSX_SLOT_YAMAHA_MINICART, tag, owner, clock)
{
}

void msx_slot_yamaha_minicart_device::device_start()
{
}

std::string msx_slot_yamaha_minicart_device::get_default_card_software(get_default_card_software_hook &hook) const
{
	return software_get_default_slot("nomapper");
}
